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


#undef NO_INLINE_STDARG
#include <dos/dostags.h>
#include "inet.h"
#include <amitcp/socketbasetags.h>

#include "YGRGroup_mcc_priv.h"
#include "stringsep.h"
#include "subtask.h"
#include "debug.h"
#include "util.h"
#include "http.h"
#include "mails.h"

#define SocketBase	data->subtask.socketbase
#define errno		data->subtask.ErrNo
#define h_errno		data->subtask.hErrNo

GLOBAL unsigned char * HTMLToText( unsigned char * htmlstring );

/***************************************************************************/

INLINE void UpdateGaugeProgress(struct ClassData *data,ULONG current,ULONG total)
{
	STATIC UBYTE report[128];
	STATIC ULONG lbr=0,lbs=0;
	long x, took;
	
	if((took = time(NULL) - data->subtask.start_time) > 0 )
	{
		lbr = data->subtask.bytes_received/took;
		lbs = data->subtask.bytes_written/took;
	}
	
	x = SNPrintf( report, sizeof(report)-1,
		"Receiving %s's message %ld of ",
			data->subtask.folder->YahooGroup, current );
	
	SNPrintf( report+x, sizeof(report)-x,
		(( total <= 0 ) ? "????":"%ld (%ld%%) [ recv %ldB/s send %ldB/s ] "),
		total, (total ? current*100/total:0), lbr, lbs );
	
	PushSet(data->appobj, data->gauge,2,MUIA_Gauge_InfoText,(ULONG)report);
	
	if(total)
		PushSet(data->appobj, data->gauge,2,
			MUIA_Gauge_Current, current*100/total );
}

/***************************************************************************/

INLINE unsigned char * StripMessage(struct ClassData * data, String * rdata)
{
	unsigned char * msg = NULL, * ptr = rdata->str;
	
	DBG("Getting Message part from html and converting to text...\n");
	
	while(*ptr)
	{	if(!strncmp( ptr, "<pre>", 5))
			break;
		ptr++;
	}
	
	if(ptr && *ptr && (ptr = &ptr[5]) && *ptr && (msg = ptr))
	{
		while(*ptr)
		{	if(!strncmp( ptr, "</pre>", 6))
				break;
			ptr++;
		}
		*ptr = 0;
		
		DBG("Converting HTML to Text...\n");
		
		msg = HTMLToText( msg );
		
		DBG("OK, html converted, now removing trailing chars...\n");
		
		while(*msg == ' ' || *msg == '\r' || *msg == '\n' || *msg == '\t')
			msg++;
		
		DBG_ASSERT(!strncmp( msg, "From ", 5));
		
		ptr = msg + strlen(msg) - 1;
		
		while(*ptr == ' ' || *ptr == '\r' || *ptr == '\t' || *ptr == '\n')
			*ptr-- = '\0';
		
		*(ptr+1) = '\n';
		*(ptr+2) = '\0';
	}
	
	return msg;
}

/***************************************************************************/

INLINE HTTP * AllocHTTPData(struct ClassData * data)
{
	HTTP * http;
	long clen;
	
	if(!(http = malloc(sizeof(*http))))
		return NULL;
	
	bzero( http, sizeof(*http));
	
	clen = strlen(data->yahoo.cookie);
	
	DBG_ASSERT(clen > (long)sizeof(char));
	
	if(!(http->cookie = string_new( clen + 24 )))
	{
		free(http);
		return NULL;
	}
	
	if(0> string_append( http->cookie, data->yahoo.cookie, clen ))
	{
		string_free( http->cookie );
		free(http);
		return NULL;
	}
	
	return(http);
}

/***************************************************************************/

static int FindMessageNumbers(struct ClassData * data, String * rdata, ULONG * current, ULONG * total )
{
	/**
	 * buscar el numero de mensage y el total, 
	 * en el msg debe aver algo como:
	 * <span style="font-weight: bold">#1 of 3048</span>
	 * data->rules[0] debe ser: font-weight: bold">#
	 */
	
	long x, y;
	StringChar ptr;
	
	DBG("Getting MessageNumbers...\n");
	
	if(!(x = FindPos( rdata->str, data->rules[0] )))
	{
		DBG("\a ++++ cannot found rule \"%s\" in message !!\n", data->rules[0]);
		return ERR_SYNTAX;
	}
	
	hexdump( &rdata->str[x-16], 0, 64 );
	ptr = &rdata->str[x];
	
	if(!(*ptr >= '0' && *ptr <= '9'))
	{
		DBG("lo que sigue a la rule[0] no es un numero!\n\a");
		return ERR_SYNTAX;
	}
	
	// StrToLong() BUGS
	// Before V39, if there were no convertible characters it returned the
	// number of leading white-space characters (space and tab in this case).
	if((y = StrToLong( ptr, current )) == -1)
	{
		DBG(" WTF!, StrToLong FAILED!\n\a");
		return ERR_SYNTAX; // <- this may should be other error
	}
	DBG_VALUE(*current);
	
	ptr += (y + 4); // el 4 es por el " of " (tal como en la cadena de arriba)
	
	if(!(*ptr >= '0' && *ptr <= '9'))
	{
		DBG("el primer digito para el TotalMsgs no es un numero!\a\n");
		hexdump( &ptr[-y-2], 0, 32 );
		return ERR_SYNTAX;
	}
	
	if((y = StrToLong( ptr, total )) == -1)
	{
		DBG(" WTF!, StrToLong FAILED (para el TOTAL)!\n\a");
		return ERR_SYNTAX; // <- this may should be other error
	}
	DBG_VALUE(*total);
	
	return ERR_NOERROR;
}


void YahooGroupsRipper ( void )
{
	struct ClassData * data;
	long error = ERR_NOERROR;
	struct Task * ThisChild = FindTask(NULL);
	HTTP * http = NULL;
	
	Wait(SIGF_SINGLE);
	data = (struct ClassData *) ThisChild->tc_UserData;
	
	ObtainSemaphore(data->sem);
	
	DBG("Booya!, Im into the child!... the prove is 0x%08lx :-)\n", ThisChild );
	
	((struct Process *)ThisChild)->pr_WindowPtr = data->window;
	data->subtask.folder = data->ActiveGroup;
	
	Forbid();
	Signal( data->apptask, SIGF_SINGLE );
	Permit();
	ReleaseSemaphore(data->sem);
	
	if((SocketBase = OpenLibrary(__bsdsocketname, 4)))
	{
		ULONG current_msg, maximun_msg, total_msgs = 0;
		
		DBG_POINTER(SocketBase);
		
		SocketBaseTags(
			SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(errno))),   (ULONG) &errno,
			SBTM_SETVAL(SBTC_HERRNOLONGPTR),             (ULONG) &h_errno,
			SBTM_SETVAL(SBTC_LOGTAGPTR),                 (ULONG) data->subtask.name,
		TAG_DONE );
		
		data->subtask.bytes_received	= 0;
		data->subtask.bytes_written	= 0;
		data->subtask.start_time	= (ULONG) time(NULL);
		data->subtask.stop_time		= 0;
		
		current_msg = data->yahoo.fmsgnum - 1;
		maximun_msg = data->yahoo.lmsgnum;
		
		if(current_msg > maximun_msg)
			maximun_msg = (ULONG) -1;
		
		if(!(http = AllocHTTPData( data )))
		{
			DBG("failed allocating HTTP !\a\n");
			error = ERR_NOMEM;
		}
		else
		{
			do {
				String * rdata = NULL;
				
				UpdateGaugeProgress(data,++current_msg,total_msgs);
				
				// &var=0 is para que use 'Fixed Width Font' (<pre>)
				
				SNPrintf( http->CurrentURL, sizeof(http->CurrentURL),
					"http://%s/group/%s/message/%ld?source=1&var=0",
						data->yahoo.hostname, data->yahoo.group, current_msg );
				
				rdata = GetURL(data,http,http->CurrentURL,&error,&data->subtask.bytes_received,&data->subtask.bytes_written,SocketBase);
				
				if((rdata != NULL) && (http->rcode == 200) && (BreakSignal() == FALSE))
				{
					unsigned char * msg;
					ULONG msgnum;
					
					DBG(" -->->> Received data = 0x%08lx, str = 0x%08lx, len = %ld\n", rdata, rdata->str, rdata->len );
					
					error = FindMessageNumbers(data,http->rdata, &msgnum, &total_msgs);
					if(error != ERR_NOERROR)
					{
						DoRequest(ErrorString(error));
						break;
					}
					
					DBG_ASSERT(current_msg == msgnum);
					
					if(msgnum > current_msg)
					{
						InfoText("Message %ld does not exist, Fetching #%ld...\n", current_msg, msgnum );
						current_msg = msgnum;
					}
					else if(current_msg > maximun_msg)
					{
						DBG("maximun message number reached...\n");
						error = ERR_NOERROR; break;
					}
					
					msg = StripMessage( data, rdata );
					
					WriteDebugFile( "MSG", msg, strlen(msg));
					
					error = SaveMessage( data, msg, current_msg );
					
				//	break;
				}
				
				if(http->hibernation)
				{
					DBG("Into HIBERNATION loop...\n");
					DBG_ASSERT(error != ERR_NOERROR);
					
					while((http->hiberTime-- > 0) && (BreakSignal() == FALSE))
					{
						InfoText("[%02ld:%02ld:%02ld] :: RUNNING HIBERNATION MODE :: %s",
							http->hiberTime/3600, (http->hiberTime%3600)/60, (http->hiberTime%3600)%60, ErrorString(error));
						
						Delay( TICKS_PER_SECOND );
					}
					--current_msg;
					
					// we should go alive after hibernation...
					error = ERR_NOERROR;
				}
				else if(!rdata || http->rcode != 200)
				{
					DBG_ASSERT(error != ERR_NOERROR);
					
					DBG("DOH, EXITING...\n");
					break;
				}
				else if(total_msgs && total_msgs == (current_msg+1))
				{
					DBG_ASSERT(error == ERR_NOERROR);
					DBG("Total number of messages (%ld) received\n", total_msgs);
					break;
				}
			} while((error == ERR_NOERROR) && (BreakSignal() == FALSE));
			
			
			if(http->rdata)
				string_free( http->rdata );
			if(http->cookie)
				string_free( http->cookie );
			free(http);
		}
		
		CloseLibrary( SocketBase );
	}
	else	error = ERR_NOTCPIP;
	
	ObtainSemaphore(data->sem);
	
	DBG(" ++++++ We are now exiting from the child...\n");
	DBG_VALUE(data->disposing);
	
	data->LastError = (ULONG) error;
	
	if(data->disposing == FALSE)
	{
		FreeDNSCache(data);
		
		PushMethod( data->appobj, data->selfobj, 
			2, MUIM_YGRGroup_SubTaskStopped,(ULONG) error );
		
		Wait( SIGBREAKF_CTRL_E );
	}
	
	Forbid ( );
	ReleaseSemaphore(data->sem);
}

/***************************************************************************/

BOOL subtask( struct ClassData * data )
{
	DBG("Launcing SubTask...\n");
	
	data->subtask.task = (struct Task *)
		CreateNewProcTags (
			NP_Entry,	(ULONG) &YahooGroupsRipper,
			NP_Name,	(ULONG) data->subtask.name,
			NP_Priority,	data->subtask.pri,
			NP_StackSize,	SUBTASK_STACKSIZE,
		TAG_DONE );
	
	if(data->subtask.task == NULL)
	{
		DBG("\aCannot create subtask!\n");
		return FALSE;
	}
	
	data->subtask.task->tc_UserData = data;
	Signal(data->subtask.task,SIGF_SINGLE);
	Wait(SIGF_SINGLE);
	
	DBG("\n\t ++++ SubTask = 0x%08lx\n", data->subtask.task );
	
	return TRUE;
}
