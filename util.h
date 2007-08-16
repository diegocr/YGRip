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


#include <string.h>
#include <stdarg.h>
#include <exec/types.h>
#include <dos/dosextens.h>

struct ClassData;

/****************************************************************************/

GLOBAL STRPTR ErrorString( long code );

GLOBAL struct SignalSemaphore * __create_semaphore(void);
GLOBAL void __delete_semaphore(struct SignalSemaphore * semaphore);

/****************************************************************************/

#ifdef DEBUG
GLOBAL void hexdump (	const void * data,ULONG offset, ULONG count );
GLOBAL void WriteDebugFile( const char * pfx, APTR data, LONG datalen );
#else
# define hexdump( args... )		((void)0)
# define WriteDebugFile(args...)	((void)0)
#endif

/****************************************************************************/

#undef MIN
#undef MAX
#define MAX(a,b)		\
({				\
	typeof(a) _a = (a);	\
	typeof(b) _b = (b);	\
				\
	(_a > _b) ? _a : _b;	\
})
#define MIN(a,b)		\
({				\
	typeof(a) _a = (a);	\
	typeof(b) _b = (b);	\
				\
	(_a > _b) ? _b : _a;	\
})

/****************************************************************************/

/* stdlib.c */

GLOBAL LONG VSNPrintf(STRPTR outbuf, LONG size, CONST_STRPTR fmt, _BSD_VA_LIST_ args);
GLOBAL LONG  SNPrintf(STRPTR outbuf, LONG size, CONST_STRPTR fmt, ... );

#define strf( fmt... )	\
({	UBYTE buf[4096];SNPrintf( buf, sizeof(buf)-1, fmt );	buf;	})

GLOBAL int MakeDir( char * fullpath );

#define time_t long
GLOBAL time_t time(time_t *tloc);

GLOBAL APTR _malloc( struct ClassData *data, ULONG size );
GLOBAL VOID _free( struct ClassData *data, APTR mem );
GLOBAL APTR _realloc( struct ClassData *data, APTR old, ULONG size );
GLOBAL APTR _calloc( struct ClassData *data, ULONG num, ULONG size);
GLOBAL STRPTR _strdup( struct ClassData *data, STRPTR src );
GLOBAL STRPTR _strndup( struct ClassData *data, STRPTR src, long size );

#define malloc( size )		_malloc( data, (size))
#define realloc(o,size)		_realloc( data, (o), (size))
#define calloc( a, b )		_calloc( data, (a), (b))
#define free( ptr )		_free( data, (ptr))
#define strdup(str)		_strdup( data, (str))
#define strndup(str,len)	_strndup( data, (str), (len))

#ifdef USE_UCHAR_DUPS
GLOBAL unsigned char * uchar_dup( struct ClassData *data, unsigned char * src );
GLOBAL unsigned char * uchar_ndup(struct ClassData *data,unsigned char *src,long slen);
GLOBAL void uchar_free(struct ClassData *data, unsigned char * src );
#else
#define uchar_dup(data, str)	_strdup( data, (str))
#define uchar_ndup(data,str,l)	_strndup( data, (str), (l))
#define uchar_free(data,str)	_free( data, (str))
#endif

/****************************************************************************/

GLOBAL UBYTE __bsdsocketname[];
GLOBAL ULONG Resolver(STRPTR hostname);

/****************************************************************************/

GLOBAL int LoadProxyList(struct ClassData * data);
GLOBAL void FreeProxyList(struct ClassData * data);

/****************************************************************************/

GLOBAL int LoadEOMTags(struct ClassData * data);
GLOBAL void FreeEOMTags(struct ClassData * data);

/****************************************************************************/

GLOBAL int LoadSingleNListContents(Object * list, STRPTR openfile);
GLOBAL int SaveSingleNListContents(Object * list, STRPTR savefile);

/****************************************************************************/

GLOBAL void iText(struct ClassData * data, const char * fmt, ...);

#define InfoText( msg... )	iText( data, msg )
#define InfoTextError( code )	InfoText(ErrorString(code)), DisplayBeep(NULL)
#define InfoTextErrorMsg(f,c)	InfoText(f, ErrorString(c)), DisplayBeep(NULL)

/****************************************************************************/

GLOBAL int SetGroupFiles(struct ClassData * data);
GLOBAL int FileToMem(struct ClassData * data, STRPTR file, STRPTR *fdata, ULONG *len );

/****************************************************************************/

GLOBAL int FindPos( unsigned char * pajar, unsigned char * aguja );
GLOBAL int FindPosNoCase( unsigned char * pajar, unsigned char * aguja );

/****************************************************************************/

GLOBAL VOID DoRequest( const char * msg );
GLOBAL VOID DoRequestFmt(const char *fmt, ...);

/****************************************************************************/


/****************************************************************************/


/****************************************************************************/


/****************************************************************************/


