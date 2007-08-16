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


#include "YGRGroup_mcc_priv.h"
#include "util.h"
#include "debug.h"
#include "mails.h"
#include <MUI/NList_mcc.h>
#include <MUI/textinput_mcc.h>

/****************************************************************************/

void FreeIndex( struct ClassData * data, struct GroupFolder ** grp )
{
	struct YGRDBIndex * c, * n ;
	
	DBG("Freeing Index, %ld messages...\n", (*grp)->Total );
	
	for( c = (*grp)->idx ; c ; c = n )
	{
		n = c->next;
		
		free( c );
	}
	
	(*grp)->idx	= NULL;
	(*grp)->Total	= 0;
}

/****************************************************************************/

void FreeGroupFolder(struct ClassData * data, struct GroupFolder ** grp )
{
	DBG_POINTER(grp);
	
	if(data->disposing == TRUE)
		return;
	
	if(grp && *grp)
	{
		FreeIndex( data, grp );
		free((*grp)->YahooGroup);
		free((*grp)->index);
		free((*grp)->unixf);
		free((*grp)->folder);
		free((*grp));
		
		(*grp) = NULL;
	}
}

/****************************************************************************/

void ExpungeIndex( struct ClassData * data, struct GroupFolder ** grp )
{
	DBG_POINTER(grp);
	
	if(!(grp && *grp))
		return;
	
	if((*grp)->New > 0)
	{
		MoveIndexToDisk( data, grp );
	}
	else
	{
		FreeGroupFolder(data, grp );
	}
}

/****************************************************************************/

int SetActiveGroup(struct ClassData * data, STRPTR YahooGroup )
{
	UBYTE buf[2048];
	
	/* crea los nombres de fichero done luego se guardaran los mensages
	 * y el index ( group.idx y group.db )
	 */
	
	DBG_STRING(data->MailsFolder);
	DBG_STRING(YahooGroup);
	
	// this must be called with the semaphore locked !
	if(data->subtask.task == NULL)
	{
		/* Si la subtask esta activa debe tener el control de
		 * data->ActiveGroup, si no tiene que ser liberado
		 */
		
		ExpungeIndex(data, &(data->ActiveGroup));
	}
	
	if(!(data->ActiveGroup = malloc(sizeof(struct GroupFolder))))
		goto done;
	
	if(!(data->ActiveGroup->YahooGroup = strdup( YahooGroup )))
		goto done;
	
	*buf = 0;
	data->LastError = ERR_OVERFLOW;
	if(AddPart( buf, data->MailsFolder, sizeof(buf)-1))
	{
		if(AddPart( buf, data->ActiveGroup->YahooGroup, sizeof(buf)-1))
		{
			STRPTR gfolder = NULL, db = NULL, idx = NULL;
			long buflen = strlen(buf) 
				+ MAX(strlen(IDX_EXT),strlen(DB_EXT)) + 2;
			
			data->LastError = ERR_NOMEM;
			if((gfolder = strdup( buf )))
			{
				if((db = malloc( buflen )))
				{
					if((idx = malloc( buflen )))
					{
						free( data->ActiveGroup->index );
						free( data->ActiveGroup->unixf );
						free( data->ActiveGroup->folder);
						
						data->ActiveGroup->index	= idx;
						data->ActiveGroup->unixf	= db;
						data->ActiveGroup->folder	= gfolder;
						
						SNPrintf(data->ActiveGroup->unixf, buflen-1, "%s%s", gfolder, DB_EXT );
						SNPrintf(data->ActiveGroup->index, buflen-1, "%s%s", gfolder, IDX_EXT );
						
						data->LastError = ERR_NOERROR;
						
						ClearReaderMail( data );
						
						DBG_STRING(data->ActiveGroup->index);
						DBG_STRING(data->ActiveGroup->unixf);
						DBG_STRING(data->ActiveGroup->folder);
					}
				}
			}
			
			if( data->LastError != ERR_NOERROR )
			{
				free( gfolder );
				free( db );
				free( idx );
			}
		}
	}
	
done:
	if( data->LastError != ERR_NOERROR )
	{
		if( data->ActiveGroup )
		{
			free( data->ActiveGroup->YahooGroup );
			free( data->ActiveGroup );
			data->ActiveGroup = NULL;
		}
	}
	return(data->LastError);
}

/****************************************************************************/

int InsertIndex(struct ClassData * data)
{
	struct YGRDBIndex * idx, ** array;
	int pos;
	
	DBG_VALUE(data->ActiveGroup->Total);
	
	if(data->ActiveGroup->Total == 0)
	{
		DBG("No messages found on the index...\n");
		return data->LastError = ERR_NOERROR;
	}
	
	array = (struct YGRDBIndex **) calloc(data->ActiveGroup->Total+1,sizeof(struct YGRDBIndex *));
	
	if(!( array ))
		return data->LastError;
	
	for( pos = 0, idx = data->ActiveGroup->idx ; idx ; idx = idx->next )
	{
		array[pos++] = idx ; 
	}
	
	DBG_ASSERT(pos == data->ActiveGroup->Total);
	
	DoMethod( data->rightlist, MUIM_NList_Clear );
	DoMethod( data->rightlist, MUIM_NList_Insert, array, 
		data->ActiveGroup->Total, MUIV_NList_Insert_Sorted,
			 MUIF_NONE /*: MUIV_NList_Insert_Flag_Raw*/ );
	
	free( array );
	
	return data->LastError = ERR_NOERROR;
}

/****************************************************************************/

int LoadIndex(struct ClassData * data)
{
	STRPTR fdata = NULL, f;
	ULONG flen;
	
	/* Carga el index (si existe) y lo copia en la structura provada
	 */
	
	DBG("Cargando el fichero indice para el grupo %s\n", data->ActiveGroup->YahooGroup );
	
	if(data->ActiveGroup->Total != 0)
	{
		DBG("data->ActiveGroup->Total = %ld\n", data->ActiveGroup->Total);
		return data->LastError = ERR_INTERNAL;
	}
	data->LastError = FileToMem(data,data->ActiveGroup->index,&fdata,&flen);
	
	if(data->LastError != ERR_NOERROR)
	{
		if(data->LastError == ERROR_OBJECT_NOT_FOUND)
		{
			/* el fichero no existe, asi pues no lo consideres
			 * como un error!
			 */
			
			data->LastError = ERR_NOERROR;
		}
		
		goto done;
	}
	
	if(*((ULONG *) fdata ) != IDX_ID)
	{
		DBG("\a El ID del index no coincide !?\n");
		data->LastError = ERR_INDEX;
		goto done;
	}
	
	f = &fdata[sizeof(ULONG)];
	
	do {
		unsigned int slen;
		struct YGRDBIndex * idx;
		
		/**
		 * El formato del index es el siguiente (cada chunk):
		 *
		 * [ ULONG | UBYTE | STRPTR | UBYTE | STRPTR | UBYTE | STRPTR ]
		 *   msgnum       from             subject          date
		 *
		 * Cada UBYTE contiene la longitud de la cadena STRPTR, y cada
		 * chunk esta separado por el bit 160, siendo 159 el bit EOF
		 */
		
		if(*((UBYTE *) f ) != 160 )
		{
			if(*((UBYTE *) f ) == 159 )
				break;
			
			data->LastError = ERR_INDEX;
			goto done;
		}
		
		f += sizeof(UBYTE);
		
		if(!(idx = malloc(sizeof(struct YGRDBIndex))))
			goto done;
		
		idx->msgnum = *((ULONG *) f );	f += sizeof(ULONG);
		DBG_VALUE(idx->msgnum);
		
		slen = (unsigned int) *((UBYTE *) f );	f += sizeof(UBYTE);
		memcpy( idx->from, f, MIN(slen,sizeof(idx->from)-1));
		f += slen;	DBG_STRING(idx->from);
		
		slen = (unsigned int) *((UBYTE *) f );	f += sizeof(UBYTE);
		memcpy( idx->subj, f, MIN(slen,sizeof(idx->subj)-1));
		f += slen;	DBG_STRING(idx->subj);
		
		slen = (unsigned int) *((UBYTE *) f );	f += sizeof(UBYTE);
		memcpy( idx->date, f, MIN(slen,sizeof(idx->date)-1));
		f += slen;	DBG_STRING(idx->date);
		
		idx->next = data->ActiveGroup->idx;
		data->ActiveGroup->idx = idx;
		
		data->ActiveGroup->Total++;
		
	} while(1);
	
	DBG_VALUE(data->ActiveGroup->Total);
	data->LastError = ERR_NOERROR;
	
done:
	free( fdata );
	
	return data->LastError;
}

/****************************************************************************/

#define WriteCheck( data, len )			\
	if(Write( fd, data, len ) != len )	\
		goto done

int SaveIndexToDisk( struct ClassData * data, struct GroupFolder ** grp )
{
	BPTR fd;
	ULONG idxid = IDX_ID;
	struct YGRDBIndex * idx;
	unsigned char len;
	
	/* THIS FUNCTION CALLER *MUST* LOCK THE SEMAPHORE !(?).. */
	
	InfoText("Saving index for group \"%s\" to disk, please wait...",
		(*grp)->YahooGroup );
	
	if(!(fd = Open((*grp)->index, MODE_NEWFILE )))
		goto done;
	
	WriteCheck( &idxid, sizeof(ULONG));
	DBG_VALUE((*grp)->New);
	
	for( idx = (*grp)->idx ; idx ; idx = idx->next )
	{
		len = 160; // chunk separator
		WriteCheck( &len, sizeof(unsigned char));
		
		WriteCheck( &(idx->msgnum), sizeof(ULONG));
		
		DBG_STRING(idx->from);
		len = (unsigned char) strlen(idx->from);
		WriteCheck( &len, sizeof(unsigned char));
		WriteCheck( idx->from, len );
		
		DBG_STRING(idx->subj);
		len = (unsigned char) strlen(idx->subj);
		WriteCheck( &len, sizeof(unsigned char));
		WriteCheck( idx->subj, len );
		
		DBG_STRING(idx->date);
		len = (unsigned char) strlen(idx->date);
		WriteCheck( &len, sizeof(unsigned char));
		WriteCheck( idx->date, len );
		
		(*grp)->New--;
	}
	
	len = 159; // EOF bit mark
	WriteCheck( &len, sizeof(unsigned char));
	
	#ifdef DEBUG
	if(((long)((*grp)->New)) < 0)
	{
		DBG(" \a +++++++++++++++ (*grp)->New < 0 !!!!! (%ld)\n", (long) (*grp)->New);
		(*grp)->New = 0;
	}
	#endif
	
	Close( fd );
done:
	if((data->LastError = IoErr()) == 0)
		data->LastError = ERR_NOERROR;
	
	return data->LastError;
}

/****************************************************************************/

int MoveIndexToDisk( struct ClassData * data, struct GroupFolder ** grp )
{
	/* move the index to disk (saves it and then free it), and also
	 * free the whole GroupFolder struct
	 */
	
	data->LastError = SaveIndexToDisk( data, grp );
	
	FreeGroupFolder( data, grp );
	
	return data->LastError;
}

/****************************************************************************/

BOOL SameGroupFolder( struct ClassData * data, BOOL lock )
{
	/* comprueba si el grupo que esta manejando la subtask
	 * es el mismo que se encuentra activo en la ventana principal
	 */
	
	BOOL rc = FALSE;
	
	if( lock )
		ObtainSemaphore(data->sem);
	
	if((data->ActiveGroup != NULL) && (data->subtask.folder != NULL))
	{
		rc = (Stricmp( data->ActiveGroup->YahooGroup, 
			data->subtask.folder->YahooGroup) == 0);
	}
	
	if( lock )
		ReleaseSemaphore(data->sem);
	
	return(rc);
}

/****************************************************************************/
/* THIS IS CALLED FROM THE SUBTASK !! */

int SaveMessage( struct ClassData * data, STRPTR Message, ULONG msgnum )
{
	struct YGRDBIndex * idx;
	int rc = ERR_NOMEM, x=0;
	UBYTE *msg=Message, file[1024];
	BPTR fd;
	
	DBG("Saving message...\n");
	DBG_ASSERT(Message && *Message);
	if(!(Message && *Message))
		return ERR_INTERNAL;
	
	ObtainSemaphore(data->sem);
	
	if(!(idx = malloc(sizeof(struct YGRDBIndex))))
		goto done;
	
//	bzero( idx, sizeof(struct YGRDBIndex));
	DBG_ASSERT((strlen(Message) > 5) && !strncmp( Message, "From ", 5 ));
	
	#define cpln( src, dst )	\
		memcpy(dst,src, MIN(FindPos(src,"\n"),(int)sizeof(dst)-1))
	
	while(*msg && (x < 3))
	{
		if(!Strnicmp( msg, "From:", 5))
		{x++;	cpln(&msg[6], idx->from);}
		else if(!Strnicmp( msg, "Subject:", 8))
		{x++;	cpln(&msg[9], idx->subj);}
		else if(!Strnicmp( msg, "Date:", 5))
		{x++;	cpln(&msg[6], idx->date);}
		msg++;
	}
	idx->msgnum = msgnum;
	idx->udata = data->subtask.folder;
	
	DBG_VALUE(msgnum);
	DBG_STRING(idx->from);
	DBG_STRING(idx->subj);
	DBG_STRING(idx->date);
	
	idx->next = data->subtask.folder->idx;
	data->subtask.folder->idx = idx;
	
	data->subtask.folder->Total++;
	data->subtask.folder->New++;
	
	if(SameGroupFolder( data, FALSE ))
	{
		PushMethod( data->appobj, data->rightlist, 3, 
			MUIM_NList_InsertSingle, idx, MUIV_NList_Insert_Sorted);
	}
	
	rc = MakeMailFile( data->subtask.folder, file, sizeof(file)-1, 
		msgnum, (data->subtask.folder->New == 1));
	
	if(rc != ERR_NOERROR)
		goto done;
	
	if((fd = Open( file, MODE_NEWFILE)))
	{
		Write( fd, Message, strlen(Message));
		#ifdef DEBUG
		Flush( fd );
		#endif
		Close( fd );
	}
	
	DBG_STRING(file);
	
	if((rc = IoErr()) == 0)
		rc = ERR_NOERROR;
done:
	ReleaseSemaphore(data->sem);
	
	return(rc);
}

/****************************************************************************/

BOOL IsNewGroup(struct ClassData * data, STRPTR YahooGroup )
{
	int pos = -1;
	
	do {
		STRPTR group = NULL;
		
		DoMethod( data->leftlist, MUIM_NList_GetEntry, ++pos, &group );
		if(!group)
			break;
		
		if(!Stricmp( group, YahooGroup))
			return FALSE;
		
	} while(1);
	
	return TRUE;
}

/****************************************************************************/

void LeftListInsert( struct ClassData * data, STRPTR IndexFile )
{
	UBYTE tmp[128], *t=tmp, *f=FilePart(IndexFile);
	int maxlen = sizeof(tmp)-1;
	
	while(*f && *f != '.' && (maxlen-- > 0))
	{
		*t++ = *f++;
	}
	*t = 0;
	
	DoMethod( data->leftlist, MUIM_NList_InsertSingle,
		(ULONG) tmp, MUIV_List_Insert_Sorted );
}

/****************************************************************************/

BOOL LeftListSelect( struct ClassData * data, STRPTR YahooGroup )
{
	long pos = -1;
	
	do {
		STRPTR group = NULL;
		
		DoMethod( data->leftlist, MUIM_NList_GetEntry, ++pos, &group );
		if(!group)
			break;
		
		if(!Stricmp( group, YahooGroup))
		{
			set( data->leftlist, MUIA_NList_Active, pos );
			return TRUE;
		}
	} while(1);
	
	return FALSE;
}

/****************************************************************************/

int MakeMailFile(struct GroupFolder * folder, STRPTR outbuf, LONG outlen, ULONG MsgNum, BOOL makeDir )
{
	int rc = ERR_OVERFLOW;
	
	DBG_ASSERT(folder != NULL);
	
	*outbuf = 0;
	if(AddPart( outbuf, folder->folder, outlen ))
	{
		if(AddPart( outbuf, strf("%08ld", MsgNum), outlen))
		{
			if(makeDir) {
				/* make sure the folder pointing to this file exists */
				MakeDir( outbuf );
			}
			
			rc = ERR_NOERROR;
			
			hexdump( outbuf, 0, strlen(outbuf)+4);
		}
	}
	
	DBG_STRING(outbuf);
	
	return(rc);
}

/****************************************************************************/

VOID ClearReaderMail( struct ClassData * data )
{
	if(data->reader.Mail)
		free(data->reader.Mail);
	data->reader.Mail = NULL;
	
	data->reader.MailNum   = -1;
	data->reader.MailSize  =  0;
	data->reader.HeaderPos = -1;
	
	SetReaderMail( data, NULL,NULL );
}

VOID SetReaderMail( struct ClassData * data, STRPTR header, STRPTR Mail )
{
	set( data->readhead, MUIA_Textinput_Contents, (ULONG) header );
	set( data->readlist, MUIA_Textinput_Contents, (ULONG) Mail   );
}

/****************************************************************************/



/****************************************************************************/

