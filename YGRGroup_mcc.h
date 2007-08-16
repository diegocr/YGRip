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

/**
 * $Id: YGRGroup_mcc.h,v 0.1 2006/07/04 01:55:03 diegocr Exp $
 */

#ifndef YGRGROUP_MCC_H
#define YGRGROUP_MCC_H

#define MUIC_YGRGroup	"YGRGroup.mcc"
#define YGRGroupObject	MUI_NewObject(MUIC_YGRGroup

enum {
	__YGRGroup_TAGBASE = (TAG_USER | (65432 << 16)),
	
	/* Attributes */
	
	MUIA_YGRGroup_YahooGroupsHost,	//  [ISG.] STRPTR
	MUIA_YGRGroup_YahooGroup,	//  [ISG.] STRPTR
	MUIA_YGRGroup_FirstMsgNum,	//  [ISG.] ULONG
	MUIA_YGRGroup_LastMsgNum,	//  [ISG.] ULONG
	MUIA_YGRGroup_YahooCookie,	//  [ISG.] STRPTR
	MUIA_YGRGroup_MailsFolder,	//  [ISG.] STRPTR
	MUIA_YGRGroup_ShowLeftList,	//  [ISG.] BOOL
	MUIA_YGRGroup_LeftListObj,	//  [..G.] Object *
	MUIA_YGRGroup_RightListObj,	//  [..G.] Object *
	MUIA_YGRGroup_MemPool,		//  [..G.] APTR *
	MUIA_YGRGroup_SubTaskTask,	//  [..G.] struct Task *
	MUIA_YGRGroup_SubTaskName,	//  [ISG.] STRPTR
	MUIA_YGRGroup_SubTaskPri,	//  [ISG.] LONG
	MUIA_YGRGroup_RegisterFrame,	//  [I.G.] BOOL
	MUIA_YGRGroup_ProxysCacheFile,	//  [ISG.] STRPTR
	MUIA_YGRGroup_ProxyList,	//  [..G.] struct YGRProxy *
	MUIA_YGRGroup_EOMTagsCacheFile,	//  [ISG.] STRPTR
	MUIA_YGRGroup_EOMTagsList,	//  [..G.] struct YGREOMTags *
	MUIA_YGRGroup_LastError,	//  [..G.] LONG
	MUIA_YGRGroup_LastErrorString,	//  [..G.] STRPTR
	MUIA_YGRGroup_RecvTimeout,	//  [ISG.] ULONG
//	MUIA_YGRGroup_		//  [ISG.] 
	
	/* Methods */
	
	MUIM_YGRGroup_StartSubTask,
	MUIM_YGRGroup_KillSubTask,
	
	
	/* Values */
	
	MUIV_YGRGroup_StartSubTask_Auto,
	MUIV_YGRGroup_StartSubTask_User,
	
};

/****************************************************************************/

struct YGRProxy
{
	struct YGRProxy * next;
	
	unsigned long ip;	/* the hostname is resolved and the ip filled
				   here, if the hostname cannot be resolved it
				   is just ignored and not added */
	unsigned short port;	/* Proxy port to connect to */
	char banned;		/* BOOLean, was the proxy banned?... */
	unsigned long tstamp;	/* timestamp of last connection */
};

/****************************************************************************/

struct YGREOMTags
{
	struct YGREOMTags * next;
	
	unsigned char * tag;	/* 255 max length */
};

/****************************************************************************/

#define YGRFROM_MAXLEN	200
#define YGRSUBJ_MAXLEN	250
#define YGRDATE_MAXLEN	50

struct YGRDBIndex 
{
	struct YGRDBIndex * next;
	
	unsigned long msgnum;
	unsigned char from[YGRFROM_MAXLEN];
	unsigned char subj[YGRSUBJ_MAXLEN];
	unsigned char date[YGRDATE_MAXLEN];
	APTR udata;
};

#endif /* YGRGROUP_MCC_H */
