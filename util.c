/* ***** BEGIN LICENSE BLOCK *****
 * Version: BSD License
 * 
 * Copyright (c) 2006, Diego Casorran <dcasorran@gmail.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 ** Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  
 ** Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */


#include <clib/dos_protos.h>
#include <proto/socket.h>
#include "YGRGroup_mcc_priv.h"
#include <MUI/NList_mcc.h>
#include <MUI/InfoText_mcc.h>
#include "util.h"
#include "debug.h"
#include <stdarg.h>
#include <amitcp/socketbasetags.h>

/****************************************************************************/

STRPTR ErrorString( long code )
{
	STATIC UBYTE IoErrStr[80],
	 * __error_strings[] = {
		"There was no error!",
		"No TCP/IP Stack Running!",
		"Cannot resolve hostname",
		"Cannot stablish connection",
		"Timeout waiting for data",
		"Internal error, a very rare issue happened...",
		"Corrupted or invalid Index file found!",
		"Interrupt caught received.",
		"Invalid or expired Cookie used.",
		"Error proccesing message,  Yahoo! syntax changed (?)",
		"unknown error (!?)...",
		NULL
	};
	
	if((code==0) || ((code - ERR_NOERROR)==0) || ((code - BSD_ERRTAG)==0))
		return __error_strings[0];
	
	if((code >= ERR_NOERROR) && (code <= ERR_UNKNOWN))
		return __error_strings[code - ERR_NOERROR];
	
	if(code >= BSD_ERRTAG)
	{
		struct Library * SocketBase;
		
		if((SocketBase = OpenLibrary(__bsdsocketname, 0)))
		{
			STATIC ULONG taglist[3];
			long error = code - BSD_ERRTAG;
			
			taglist[0] = SBTM_GETVAL(SBTC_ERRNOSTRPTR);
			taglist[1] = error;
			taglist[2] = TAG_DONE;
			
			SocketBaseTagList((struct TagItem *)taglist);
			
			CloseLibrary( SocketBase );
			
			return (STRPTR) taglist[1];
		}
		
		return __error_strings[ERR_UNKNOWN - ERR_NOERROR];
	}
	
	Fault( code, NULL, IoErrStr, sizeof(IoErrStr) - 1 );
	
	return( IoErrStr );
}

/****************************************************************************/

struct SignalSemaphore * __create_semaphore(void)
{
	struct SignalSemaphore * semaphore;

	#if defined(__amigaos4__)
	{
		semaphore = AllocSysObject(ASOT_SEMAPHORE,NULL);
	}
	#else
	{
		semaphore = AllocVec(sizeof(*semaphore),MEMF_ANY|MEMF_PUBLIC);
		if(semaphore != NULL)
			InitSemaphore(semaphore);
	}
	#endif /* __amigaos4 */

	return(semaphore);
}

/****************************************************************************/

void __delete_semaphore(struct SignalSemaphore * semaphore)
{
	if(semaphore != NULL)
	{
		#if defined(__amigaos4__)
		{
			FreeSysObject(ASOT_SEMAPHORE,semaphore);
		}
		#else
		{
			if( semaphore->ss_Owner != NULL )
			{
				DBG("ERROR: semaphore->ss_Owner MUST BE NULL!\n");
				return;
			}

			#if defined(DEBUG)
			{
				/* Just in case somebody tries to reuse this data
				   structure; this should produce an alert if
				   attempted. */
				memset(semaphore,0,sizeof(*semaphore));
			}
			#endif /* DEBUG */

			FreeVec(semaphore);
		}
		#endif /* __amigaos4 */
	}
}

/****************************************************************************/
/****************************************************************************/

APTR _malloc( struct ClassData *data, ULONG size )
{
	ULONG *mem;
	
	DBG_ASSERT(data && data->MagicID == magicid);
	
	mem = NULL;
	if(!data || (data->MagicID != magicid) || (data->mempool == NULL))
	{
		DBG("WTF, The pool ISNT Valid!\a\n");
		SetIoErr( data->LastError = ERROR_OBJECT_WRONG_TYPE );
		return NULL;
	}
	
	if(((long)size) <= 0)
	{
		DBG("Hey boy, you give me %ld bytes to allocate !?\a\n",size);
		SetIoErr( data->LastError = ERROR_OBJECT_WRONG_TYPE );
		return NULL;
	}
	
	size += sizeof(ULONG) + MEM_BLOCKMASK;
	size &= ~MEM_BLOCKMASK;
	
	ObtainSemaphore(data->semalloc);
	
	if((mem = AllocPooled( data->mempool, size)))
		*mem++=size;
	else SetIoErr( data->LastError = ERROR_NO_FREE_STORE );
	
	ReleaseSemaphore(data->semalloc);
	
	return mem;
}

VOID _free( struct ClassData *data, APTR mem )
{
	ULONG *omem=mem;
	
	DBG_ASSERT(data && data->MagicID == magicid);
	
	if(data && (data->MagicID == magicid) && data->mempool && mem && omem)
	{
		ULONG size = *(--omem);
		
		ObtainSemaphore(data->semalloc);
		
		FreePooled( data->mempool, omem, size );
		
		ReleaseSemaphore(data->semalloc);
	}
	else SetIoErr( data->LastError = ERROR_OBJECT_WRONG_TYPE );
}

APTR _realloc( struct ClassData *data, APTR old, ULONG size )
{
	LONG nsize = size, osize, *o=old;
	APTR nmem;
	
	if(!old)
		return _malloc( data, nsize );
	
	osize = (*(o-1)) - sizeof(ULONG);
	if (nsize <= osize)
		return old;
	
	if((nmem = _malloc( data, nsize )))
	{
		ULONG *n = nmem;
		
		osize >>= 2;
		while(osize--)
			*n++ = *o++;
		
		_free( data, old );
	}
	
	return nmem;
}

APTR _calloc( struct ClassData *data, ULONG num, ULONG size)
{
	ULONG total;
	
	if((((long)num) <= 0) || (((long)size) <= 0))
		return NULL;
	
	total = size * num;
	
	return _malloc( data, total );
}

/****************************************************************************/

STRPTR _strndup( struct ClassData *data, STRPTR src, long size )
{
	STRPTR dst = NULL;
	
	if((size > 0) && (src != NULL))
	{
		if((dst = _malloc( data, size+2 )))
		{
			memcpy( dst, src, size );
			dst[size] = '\0';
		}
	}
	else SetIoErr( data->LastError = ERROR_OBJECT_WRONG_TYPE );
	
	return dst;
}

STRPTR _strdup( struct ClassData *data, STRPTR src )
{
	return _strndup( data, src, strlen(src)+1);
}

/****************************************************************************/

#ifdef USE_UCHAR_DUPS
unsigned char * uchar_ndup(struct ClassData *data,unsigned char *src,long slen)
{
	unsigned char * dst = NULL;
	
	if((data && data->mempool) && (src && *src))
	{
		if(slen >= 255)
			slen = 254;
		
		ObtainSemaphore(data->semalloc);
		dst = AllocPooled( data->mempool, slen+sizeof(unsigned char)+1);
		ReleaseSemaphore(data->semalloc);
		
		if(dst != NULL)
		{
			*dst++ = (unsigned char) slen;
			
			memcpy( dst, src, slen );
			dst[slen] = '\0';
		}
		else SetIoErr( data->LastError = ERROR_NO_FREE_STORE );
	}
	else SetIoErr( data->LastError = ERROR_OBJECT_WRONG_TYPE );
	
	return dst;
}

unsigned char * uchar_dup( struct ClassData *data, unsigned char * src )
{
	return uchar_ndup( data, src, strlen( src ));
}

void uchar_free(struct ClassData *data, unsigned char * src )
{
	if((data && data->mempool != NULL) && (src != NULL))
	{
		ObtainSemaphore(data->semalloc);
		FreePooled( data->mempool, src, (long)src[-1]);
		ReleaseSemaphore(data->semalloc);
	}
	else SetIoErr( data->LastError = ERROR_OBJECT_WRONG_TYPE );
}
#endif /* USE_UCHAR_DUPS */

/****************************************************************************/

void iText(struct ClassData * data, const char * fmt, ...)
{
	va_list args;
	
	va_start( args, fmt );
	VSNPrintf( data->infobuf, sizeof(data->infobuf)-1, fmt, args );
	va_end( args );
	
	//set( data->info, MUIA_InfoText_Contents, (ULONG) data->infobuf );
	PushSet( data->appobj, data->info, 2, MUIA_InfoText_Contents, (ULONG) data->infobuf );
	
	DBG("%s\n", data->infobuf );
}


/****************************************************************************/
/****************************************************************************/

void FreeProxyList(struct ClassData * data)
{
	struct YGRProxy * a, * b;
	
	for( a = data->proxy ; a ; a = b )
	{
		b = a->next;
		
		free( a );
	}
	
	data->proxy = NULL;
}

int LoadProxyList(struct ClassData * data)
{
	long pos = -1;
	int rc = ERR_NOERROR;
	
	FreeProxyList( data );
	
	do {
		struct YGRProxy * entry;
		STRPTR proxy = NULL, ptr;
		long port = 80;
		ULONG ip;
		
		DoMethod( data->proxys, MUIM_NList_GetEntry, ++pos, &proxy);
		if(!proxy)
			break;
		
		ptr = proxy;
		while(*ptr && *ptr++ != ':');
		
		if(*ptr)
		{
			/* an entry of type hostname:port was found */
			
			if(StrToLong( ptr, &port) == -1)
			{
				DBG("\aStrToLong failed\n");
				continue;
			}
			
			*--ptr = 0;
		}
		
		if(!((port > 0) && (port < 65535)))
		{
			InfoText("Invalid proxy port %ld", port );
			continue;
		}
		
		ip = Resolver( proxy );
		
		if((ip == (ULONG) -1) || !ip)
		{
			InfoText("Cannot resolve proxy \"%s\"", proxy );
			Delay( 25 ); // half second
			continue;
		}
		
		rc = ERR_NOMEM;
		if((entry = malloc(sizeof(struct YGRProxy))))
		{
			entry->ip = ip;
			entry->port = (UWORD) port;
			
			entry->next = data->proxy;
			data->proxy = entry;
			
			rc = ERR_NOERROR;
		}
	} while(rc == ERR_NOERROR);
	
	return rc;
}

/****************************************************************************/

void FreeEOMTags(struct ClassData * data)
{
	struct YGREOMTags * a, * b;
	
	for( a = data->eomtag ; a ; a = b )
	{
		b = a->next;
		
		uchar_free( data->mempool, a->tag );
		free( a );
	}
	
	data->eomtag = NULL;
}

int LoadEOMTags(struct ClassData * data)
{
	long pos = -1;
	int rc = ERR_NOERROR;
	
	FreeEOMTags( data );
	
	do {
		STRPTR eomtag = NULL;
		struct YGREOMTags * entry;
		
		DoMethod( data->eomtags, MUIM_NList_GetEntry, ++pos, &eomtag);
		if(!eomtag)
			break;
		
		if(!*eomtag)
			continue;
		
		rc = ERR_NOMEM;
		if((entry = malloc(sizeof(struct YGREOMTags))))
		{
			if((entry->tag = uchar_dup( data->mempool, eomtag )))
			{
				entry->next = data->eomtag;
				data->eomtag = entry;
				
				rc = ERR_NOERROR;
			}
			else free(entry);
		}
	} while(rc == ERR_NOERROR);
	
	return(rc);
}

/****************************************************************************/

int SaveSingleNListContents(Object * list, STRPTR savefile)
{
	BPTR fd;
	int rc = ERR_INTERNAL;
	
	if((fd = Open( savefile, MODE_NEWFILE)))
	{
		long pos = -1;
		
		do {
			STRPTR entry = NULL;
			
			DoMethod( list, MUIM_NList_GetEntry, ++pos, &entry);
			if(!entry)
				break;
			
			if(!*entry)
				continue;
			
			FPrintf( fd, "%s\n", (long) entry );
			
		} while(1);
		
		Close( fd );
		
		rc = ERR_NOERROR;
	}
	
	if(rc != ERR_NOERROR)
		rc = IoErr();
	
	return(rc);
}

int LoadSingleNListContents(Object * list, STRPTR openfile)
{
	BPTR fd;
	int rc = ERR_INTERNAL;
	
	if((fd = Open( openfile, MODE_OLDFILE )))
	{
		UBYTE ln[1024];
		
		DoMethod( list, MUIM_NList_Clear, TRUE );
		
		while(FGets( fd, ln, sizeof(ln)-1) != NULL)
		{
			DoMethod( list, MUIM_NList_InsertSingle, 
				ln, MUIV_NList_Insert_Sorted );
		}
		
		Close( fd );
		
		rc = ERR_NOERROR;
	}
	
	if(rc != ERR_NOERROR)
		rc = IoErr();
	
	return(rc);
}



/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


#ifdef DEBUG
#define isprint( ch )	\
	((((ch) > 31) && ((ch) < 127)) || (((ch) > 160) && ((ch) < 255)))

void hexdump (	const void * data,ULONG offset, ULONG count )
{
    ULONG t, end;
    int   i;

    end = (count + 15) & -16;

    for (t=0; t<end; t++)
    {
	if ((t&15) == 0)
	    kprintf ("%08lx:", offset+t);

	if ((t&3) == 0)
	    kputc(' ');

	if (t < count)
	    kprintf ("%02lx", ((UBYTE *)data)[t]);
	else
	    kprintf ("  ");

	if ((t&15) == 15)
	{
	    kputc(' ');

	    for (i=15; i>=0; i--)
	    {
		if (isprint (((UBYTE *)data)[t-i]))
		    kputc(((UBYTE *)data)[t-i]);
		else
		    kputc('.');
	    }

	    kputc('\n');
	}
    }
}


void WriteDebugFile( const char * pfx, APTR data, LONG datalen )
{
	UBYTE file[512];
	BPTR fd;
	struct DosLibrary * DOSBase;
	
	if(!(DOSBase = (struct DosLibrary *)OpenLibrary("dos.library",0)))
		return;
	
	SNPrintf( file, sizeof(file), "T:_%s_%ld", pfx ? pfx : "dbg",time(NULL));
	
	DBG_POINTER(data);
	DBG_VALUE(datalen);
	kprintf(" >> Writing file \"%s\"....  ", file );
	
	if((fd = Open( file, MODE_NEWFILE )))
	{
		Write( fd, data, datalen );
		Close( fd );
		
		kprintf(" OK\n");
	}
	else	kprintf(" FAILED!\n");
	
	CloseLibrary((struct Library *) DOSBase );
}

#endif /* DEBUG */


/****************************************************************************/

int FileToMem(struct ClassData * data, STRPTR file, STRPTR *fdata, ULONG *len )
{
	struct FileInfoBlock fib;
	BPTR fd = 0;
	STRPTR f_data;
	LONG f_len;
	
	data->LastError = ERROR_OBJECT_WRONG_TYPE;
	
	if(!file || !fdata)
		goto done;
	
	*fdata = NULL;
	
	if(len)
		*len = 0;
	
	if(!(fd = Open( file, MODE_OLDFILE )))
	{
		// hopefuly should be ERROR_OBJECT_NOT_FOUND
		DBG("Can't open %s: %s", file, ErrorString(IoErr()));
		goto done;
	}
	
	if(!ExamineFH( fd, &fib))
	{
		DBG("Can't stat %s: %s", file, ErrorString(IoErr()));
		goto done;
	}
	
	if((f_len = fib.fib_Size) == 0)
	{
		DBG("File \"%s\" has Zero size!", file );
		goto done;
	}
	
	if(!(f_data = malloc(f_len+1)))
		goto done;
	
	if(Read( fd, f_data, f_len) != f_len)
	{
		DBG("failed to read %s: %s", file, ErrorString(IoErr()));
		free (f_data);
		goto done;
	}
	
	f_data[f_len] = 0;
	
	*fdata = f_data;
	
	if(len)
		*len = f_len;
	
done:
	if((data->LastError = IoErr( )) == 0 )
		data->LastError = ERR_NOERROR;
	
	if( fd )
		Close( fd );
	
	return(data->LastError);
}


/****************************************************************************/

int MakeDir( char * fullpath )
{
	UBYTE subdir[4096];
	char *sep, *xpath=(char *)fullpath;
	int rc = ERR_NOERROR;
	
	sep = (char *) fullpath;
	sep = strchr(sep, '/');
	
	while( sep )
	{
		BPTR dlock;
		int len;
		
		len = sep - xpath;
		memcpy( subdir, xpath, len);
		subdir[len] = 0;
		
		DBG(" +++ Creating Directory \"%s\"...\n", subdir );
		
		if((dlock = CreateDir( subdir )))
			UnLock( dlock );
		else
		{
			DBG(" --- CreateDir() failed\n");
			
			if((rc = IoErr()) == ERROR_OBJECT_EXISTS)
			{
				DBG(" --- check for existing directory\n");
				
				dlock = Lock( subdir, SHARED_LOCK );
				
				if( !dlock )
				{
					/* this can't happend!, I think.. */
					DBG("\a *** LOCK FAILED\n");
					rc = ERR_INTERNAL;
				}
				else
				{
					struct FileInfoBlock fib;
					
					if(Examine(dlock,&fib) == DOSFALSE)
					{
						DBG("\a **** Examine FAILED\n");
						rc = IoErr();
					}
					else
					{
						if(fib.fib_DirEntryType > 0)
							rc = ERR_NOERROR;
						
						#ifdef DEBUG
						if((rc != ERR_NOERROR) || fib.fib_DirEntryType == ST_SOFTLINK)
						{
							DBG("\aDirectory Name exists and %spoint to a file !!!!\n", ((fib.fib_DirEntryType == ST_SOFTLINK) ? "MAY ":""));
						}
						#endif
					}
					
					UnLock( dlock );
				}
			}
			
			if(rc != ERR_NOERROR)
			{
				DBG("\a ERROR: Directory '%s' will be missing\n", fullpath );
				break;
			}
		}
		
		sep = strchr(sep+1, '/');
	}
	
	return(rc);
}

/****************************************************************************/

#if 0
typedef void *(*ExAllCB)(struct ExAllData * ead, APTR udata);

LONG ReadDirectory( STRPTR directory, ULONG NumOfBufs, STRPTR eac_MatchString,
	struct Hook * eac_MatchFunc, LONG ExAllType, APTR func, APTR udata)
{
	register struct ExAllControl * eaControl;
	register struct ExAllData    * eaBuffer, * eaData;
	register LONG more, eaBuffSize = NumOfBufs * sizeof(struct ExAllData);
	register BPTR lock;
	LONG TotalEntrys = -1;
	
	if((lock = Lock( directory, SHARED_LOCK)))
	{
		struct FileInfoBlock fib;
		
		if(Examine(lock, &fib) && (fib.fib_DirEntryType > 0))
		{
			eaControl = (struct ExAllControl *) AllocDosObject(DOS_EXALLCONTROL, NULL);
			
			if( eaControl != NULL )
			{
				eaControl->eac_LastKey = 0;
				eaControl->eac_MatchString = eac_MatchString;
				eaControl->eac_MatchFunc = eac_MatchFunc;
				
				eaBuffer = (struct ExAllData *) AllocMem(eaBuffSize, MEMF_ANY);
				
				if( eaBuffer != NULL)
				{
					TotalEntrys = 0;
					
					do {
						more = ExAll( lock, eaBuffer, eaBuffSize, ExAllType, eaControl );
						
						if( (!more) && (IoErr() != ERROR_NO_MORE_ENTRIES) )
						{
							DBG("ExAll failed abnormally!\n");
							break;
						}
						
						if (eaControl->eac_Entries == 0)
						{
							DBG("no more entries\n");
							continue;
						}
						
						TotalEntrys += eaControl->eac_Entries;
						
						if(func == NULL)
							continue;
						
						eaData = (struct ExAllData *) eaBuffer;
						
						while( eaData != NULL )
						{
							(*func)( eaData, udata );
							
							eaData = eaData->ed_Next;
						}
						
					} while(more);
					
					FreeMem( eaBuffer, eaBuffSize );
				}
				
				FreeDosObject(DOS_EXALLCONTROL, eaControl);
			}
		}
		
		UnLock(lock);
	}
	
	return(TotalEntrys);
}
#endif

/****************************************************************************/

INLINE unsigned char __ToLower(unsigned char ch)
{
   return((((ch > 64) && (ch < 91)) || ((ch > 191) && (ch < 224)))?(ch+32):ch);
}

int FindPos( unsigned char * pajar, unsigned char * aguja )
{
	const unsigned char *src = (const unsigned char *) pajar;
	
	if(!(pajar && *pajar) || !(aguja && *aguja))
		return 0;
	
	while(*src)
	{
		const unsigned char * a = (const unsigned char *) aguja;
		
		while(*src && (*src++ == *a++))
		{
			if(!(*a))
				return ~(pajar-++src);
		}
	}
	
	return 0;
}

int FindPosNoCase( unsigned char * pajar, unsigned char * aguja )
{
	const unsigned char *src = (const unsigned char *) pajar;
	
	if(!(pajar && *pajar) || !(aguja && *aguja))
		return 0;
	
	while(*src)
	{
		const unsigned char * a = (const unsigned char *) aguja;
		
		while(*src && (__ToLower(*src++) == __ToLower(*a++)))
		{
			if(!(*a))
				return ~(pajar-++src);
		}
	}
	
	return 0;
}

/****************************************************************************/
#include <graphics/gfxmacros.h> /* JAM */

void DoRequest( const char * msg )
{
	STATIC UBYTE Gad[] = "I've been warned";
	
	DBG("Mostrando requester....\n");
	DBG_VALUE(((struct Library *)IntuitionBase)->lib_Version);
	
	if(((struct Library *)IntuitionBase)->lib_Version < 37)
	{
		static struct IntuiText body = { 0,1,JAM1, 15,5, NULL, NULL, NULL };
		static struct IntuiText   ok = { 0,1,JAM1,  6,3, NULL,  Gad, NULL };
		
		body.IText = (STRPTR) msg;
		
		AutoRequest(NULL,&body,NULL,&ok,0,0,640,72);
	}
	else
	{
		struct EasyStruct es;
		bzero(&es,sizeof(es));
		
		es.es_StructSize	= sizeof(es);
		es.es_Title		= (STRPTR) FindTask(NULL)->tc_Node.ln_Name;
		es.es_TextFormat	= (STRPTR) msg;
		es.es_GadgetFormat	= (STRPTR) Gad;
		
		EasyRequestArgs(NULL,&es,NULL,NULL);
	}
	
	DBG("%s: %s\n\n\a", FindTask(NULL)->tc_Node.ln_Name, msg );
}

void DoRequestFmt(const char *fmt, ...)
{
	va_list ap;
	STATIC UBYTE message[1024];
	
	va_start(ap, fmt);
	VSNPrintf( message, sizeof(message)-1, fmt, ap);
	va_end(ap);
	
	DoRequest((const char *) message );
}


/****************************************************************************/
