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


/** $Id: YGRGroup_mcc_priv.h,v 0.1 2006/07/04 02:18:29 diegocr Exp $
 **/

#ifndef YGRGROUP_MCC_PRIV_H
#define YGRGROUP_MCC_PRIV_H

#define YGRGROUP_IS_PUBLIC	0

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <exec/resident.h>
#include <exec/execbase.h>
#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

#include <SDI_mui.h>
#include <clib/alib_protos.h>

#include "YGRGroup_mcc.h"

/***************************************************************************/

struct GroupFolder
{
	struct YGRDBIndex * idx;
	ULONG Total;	/* total number of messages */
	ULONG New;	/* total number of NEW messages */
	
	STRPTR folder;	/* full path to where to save msgs */
	STRPTR index;	/* full path to the messages index */
	STRPTR unixf;	/* full path to the unix-like file */
	
	STRPTR YahooGroup;
};

/***************************************************************************/

struct ClassData
{
	ULONG MagicID;
	#define magicid 0x9f782221
	
	APTR mempool;
	
	/* clicking on leftlist's entry, this struct is filled with 
	 * the data/info of the selected Yahoo! group.
	 * NOTE: struct GroupFolder should be long aligned...
	 */
	struct GroupFolder * ActiveGroup;
	
	struct Hook construct_hook;
	struct Hook destruct_hook;
	struct Hook display_hook;
	struct Hook compare_hook;
	
	struct SignalSemaphore * sem;
	struct SignalSemaphore * semalloc;
	struct Task * apptask;
	Object * appobj;
	
	struct {
		struct GroupFolder * folder;
		struct Task * task;
		STRPTR name;
		long pri;
		
		/* statistical data */
		ULONG bytes_received;
		ULONG bytes_written;
		ULONG start_time;	/* subtask start time */
		ULONG stop_time;	/* subtask end time */
		
		int ErrNo, hErrNo;
		struct Library *socketbase;
		
	} subtask;
	
	struct {
		STRPTR hostname, group, cookie;
		ULONG ip, fmsgnum, lmsgnum;
	} yahoo;
	
	BOOL disposing;
	BOOL showleftlist;
	BOOL RegisterFrame;
	BOOL unused_aligment;
	
	char ** RegisterTitles;
	
	Object * selfobj;
	
	Object * info;
	Object * leftlist;
	Object * rightlist;
	Object * lrbar;
	
	Object * startstask;
	Object * stopstask;
	Object * page;
	Object * gauge;
	
	STRPTR MailsFolder;	/* main (root) folder to save data */
	
	UBYTE numbuf[32];
	UBYTE infobuf[255];
	LONG LastError;
	
	/* Messages Frame */
	Object * group;
	Object * fmsgnum;
	Object * lmsgnum;
	
	/* Settings Frame */
	Object * hostname;
	Object * cookie;
	Object * RecvTimeoutObj;
	Object * RulesObj;
	
	/* End Of Messages Tags Frame */
	Object * proxys;	/* nlist */
	Object * proxycache;	/* string */
	Object * proxyadd;	/* button */
	Object * proxydel;	/* button */
	Object * proxyentry;	/* string */
	
	/* Proxys Frame */
	Object * eomtags;	/* nlist */
	Object * eomcache;	/* string */
	Object * eomadd;	/* button */
	Object * eomdel;	/* button */
	Object * eomcriteria;	/* cycle */
	Object * eomentry;	/* string */
	
	/* Reader Frame */
	Object * readhead;	/* TextInput */
	Object * readprev;	/* button */
	Object * readnext;	/* button */
	Object * readlist;	/* TextInput */
	struct {
		LONG MailNum;
		STRPTR Mail;
		LONG MailSize;
		LONG HeaderPos;
	} reader;
	
	/* objects data */
	STRPTR proxyscachefile;
	STRPTR eomtagscachefile;
	ULONG recvTimeout;
	APTR * rules;
	
	struct YGRProxy		* proxy;
	struct YGREOMTags	* eomtag;
	
	
	struct Window * window;
#ifdef _PROTO_ICON_H
	struct DiskObject * dobj;
#endif
};

enum {
	__YGRGroup_TAGBASE_Private	= __YGRGroup_TAGBASE + 3000,
	
	/* private methods */
	
	MUIM_YGRGroup_GroupSelected,
	MUIM_YGRGroup_SubTaskStopped,
	MUIM_YGRGroup_AddEOMTagEntry,
	MUIM_YGRGroup_DelEOMTagEntry,
	MUIM_YGRGroup_AddProxyEntry,
	MUIM_YGRGroup_DelProxyEntry,
	MUIM_YGRGroup_GoToReader,
	MUIM_YGRGroup_ReaderNext,
	MUIM_YGRGroup_ReaderPrev,
	MUIM_YGRGroup_ReadMailsFolder,
};

/***************************************************************************/

#define IDX_EXT		".idx"
#define DB_EXT		".db"

#define IDX_ID		MAKE_ID('Y','I','D','X')
#define DB_ID		MAKE_ID('Y','G','D','B')

/***************************************************************************/
/* common errors */

enum {
	ERR_NOERROR = 0x1000,
	ERR_NOTCPIP,
	ERR_RESOLVER,
	ERR_CONNECT,
	ERR_TIMEOUT,
	ERR_INTERNAL,
	ERR_INDEX,
	ERR_BREAK,
	ERR_COOKIE,
	ERR_SYNTAX,
	ERR_UNKNOWN
};

#define ERR_OVERFLOW		ERROR_BUFFER_OVERFLOW
#define ERR_NOMEM		ERROR_NO_FREE_STORE

#define BSD_ERRTAG		0x9000
#define BSDERROR(errno)		((BSD_ERRTAG)+(errno))
#define ERR_BSD			BSDERROR(Errno())

/***************************************************************************/

GLOBAL ULONG STDARGS DoSuperNew(struct IClass *cl,Object *obj,ULONG tag1,...);
GLOBAL Object * YahooGroupsLogo ( VOID );

/***************************************************************************/

#define PushMethod( app, obj, numargs, args... )	\
	DoMethod( app, MUIM_Application_PushMethod, (obj),(numargs), args )

#define PushSet( app, obj, numargs, args... ) \
	PushMethod( app, obj, ((numargs)+1), MUIM_Set, args )

/***************************************************************************/

#define isFlagSet(v,f)      (((v) & (f)) == (f))  // return TRUE if the flag is set
#define hasFlag(v,f)        (((v) & (f)) != 0)    // return TRUE if one of the flags in f is set in v
#define isFlagClear(v,f)    (((v) & (f)) == 0)    // return TRUE if flag f is not set in v
#define SET_FLAG(v,f)       ((v) |= (f))          // set the flag f in v
#define CLEAR_FLAG(v,f)     ((v) &= ~(f))         // clear the flag f in v
#define MASK_FLAG(v,f)      ((v) &= (f))          // mask the variable v with flag f bitwise

/***************************************************************************/

#ifndef MUIF_NONE
# define MUIF_NONE	0
#endif

/***************************************************************************/

#if !YGRGROUP_IS_PUBLIC
GLOBAL struct MUI_CustomClass * 
	CreateYGRGroupCustomClass(struct Library * base);

#endif /* !YGRGROUP_IS_PUBLIC */
#endif /* YGRGROUP_MCC_PRIV_H */
