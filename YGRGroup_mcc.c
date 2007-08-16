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
 * :ts=4
 */
#include "YGRGroup_mcc_priv.h"
#include <MUI/NListview_mcc.h>
#include <MUI/InfoText_mcc.h>
#include <MUI/textinput_mcc.h>
#include <MUI/Busy_mcc.h>
#include "util.h"
#include "debug.h"
#include "muiobjs.h"
#include "subtask.h"
#include "mails.h"
#include "stringsep.h"

/***************************************************************************/

#define CLASS		"YGRGroup.mcc"
#define VERSION		1
#define REVISION	0
#define BUILD_DATE	"04.07.2006"
#define SUPERCLASS	MUIC_Group
#define INSTDATA	ClassData

#if YGRGROUP_IS_PUBLIC
# include "mccheader.c"
#endif

#define DEFAULT_YAHOOGROUPS_HOSTNAME	"groups.yahoo.com"

/***************************************************************************/
//------------------------------------------------------ NList -------------

struct NList_Entry
{
	ULONG num;
	STRPTR from;
	STRPTR subj;
	STRPTR date;
};

//---------------------------------------------- NList Construct Hook ------
#if 0
HOOKPROTONO( ConstructFunc, APTR, struct NList_ConstructMessage * msg )
{
	struct ClassData *data = hook->h_Data;
	struct NList_Entry * nle, * entry = (struct NList_Entry *) msg->entry;
	
	#ifdef DEBUG
	if(!data || data->MagicID != magicid)
	{
		DBG(" **** WARNING: wrong DATA !\n");
		return 0;
	}
	#endif
	
	if((nle = AllocPooled( data->mempool, sizeof(*nle))))
	{
		nle->num = entry->num;
		nle->from = uchar_dup( data, entry->from );
		nle->subj = uchar_dup( data, entry->subj );
		nle->date = uchar_dup( data, entry->date );
	}
	return (APTR) nle;
}
MakeStaticHook( ConstructHook, ConstructFunc );
#endif

//---------------------------------------------- NList Destruct Hook -------
#if 0
HOOKPROTONO( DestructFunc, VOID, struct NList_DestructMessage * msg )
{
	struct ClassData *data = hook->h_Data;
	struct NList_Entry * entry = (struct NList_Entry *) msg->entry;
	
	#ifdef DEBUG
	if(!data || data->MagicID != magicid)
	{
		DBG(" **** WARNING: wrong DATA !\n");
		return;
	}
	#endif
	
	if((data->disposing == FALSE) && (data->mempool != NULL) && (entry != NULL))
	{
		uchar_free(data,entry->from);
		uchar_free(data,entry->subj);
		uchar_free(data,entry->date);
		
		FreePooled(data->mempool, entry, sizeof(struct NList_Entry));
	}
}
MakeStaticHook( DestructHook, DestructFunc );
#endif

//---------------------------------------------- NList Display Hook --------

HOOKPROTONO( DisplayFunc, long, struct NList_DisplayMessage * msg )
{
	struct ClassData *data = hook->h_Data;
	struct YGRDBIndex * entry = (struct YGRDBIndex *) msg->entry;
	
	#ifdef DEBUG
	if(!data || data->MagicID != magicid)
	{
		DBG(" **** WARNING: wrong DATA !\n");
		return 0;
	}
	#endif
	
	if( entry == NULL )
	{
		msg->strings[0] = "Num";
		msg->strings[1] = "Sender";
		msg->strings[2] = "Subject";
		msg->strings[3] = "Date";
	}
	else {
		SNPrintf(data->numbuf, sizeof(data->numbuf)-1, 
			"\033r%ld", entry->msgnum );
		
		msg->strings[0] = data->numbuf;
		msg->strings[1] = entry->from;
		msg->strings[2] = entry->subj;
		msg->strings[3] = entry->date;
	}
	
	return 0;
}
MakeStaticHook( DisplayHook, DisplayFunc );

//---------------------------------------------- NList Compare Hook --------

STATIC LONG __cmp2col( struct ClassData *data,
	struct YGRDBIndex * e1, struct YGRDBIndex * e2, ULONG column )
{
	switch (column)
	{
		default:
		case 0:	return e1->msgnum - e2->msgnum;
		case 1:	return Stricmp( e1->from, e2->from );
		case 2:	return Stricmp( e1->subj, e2->subj );
		case 3:	return Stricmp( e1->date, e2->date );
	}
}

HOOKPROTONO( CompareFunc, LONG, struct NList_CompareMessage * msg )
{
	struct ClassData *data = hook->h_Data;
	struct YGRDBIndex *ead1, *ead2;
	ULONG col1, col2;
	LONG result;

	ead1 = (struct YGRDBIndex *)msg->entry1;
	ead2 = (struct YGRDBIndex *)msg->entry2;
	col1 = msg->sort_type & MUIV_NList_TitleMark_ColMask;
	col2 = msg->sort_type2 & MUIV_NList_TitleMark2_ColMask;

	if (msg->sort_type == (LONG)MUIV_NList_SortType_None) return 0;

	if (msg->sort_type & MUIV_NList_TitleMark_TypeMask) {
		result = __cmp2col(data, ead2, ead1, col1);
	} else {
		result = __cmp2col(data, ead1, ead2, col1);
	}

	if (result != 0 || col1 == col2) return result;

	if (msg->sort_type2 & MUIV_NList_TitleMark2_TypeMask) {
		result = __cmp2col(data, ead2, ead1, col2);
	} else {
		result = __cmp2col(data, ead1, ead2, col2);
	}

	return result;
}
MakeStaticHook( CompareHook, CompareFunc );



/***************************************************************************/


INLINE ULONG YGRGroup_new(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct ClassData *data, * tdata;
	APTR mempool;
	
	static char * RegisterTitles[] = {
		"Messages", "Settings", "Proxys", "EOMTAGS", "Reader", NULL,
	};
	static char * EOMCriteria[] = {
		"What is found first descending the list",
		"What is found first at the message",
		NULL
	};
	
	if(!(mempool = CreatePool(MEMF_PUBLIC|MEMF_CLEAR, 4096, 1024 )))
		return 0;
	
	if(!(data = tdata = AllocPooled( mempool, sizeof(struct ClassData)))
		|| !(data->sem = __create_semaphore()) || !(data->semalloc = __create_semaphore()))
	{
		if(data->sem)
			__delete_semaphore(data->sem);
		DeletePool( mempool );
		return 0;
	}
	
	data->MagicID = magicid;
	data->mempool = mempool;
	
	data->RegisterTitles = RegisterTitles;
	data->RegisterFrame  = GetTagData( MUIA_YGRGroup_RegisterFrame, TRUE, msg->ops_AttrList );
	
	obj = (Object *)DoSuperNew(cl,obj,
		MUIA_Background, (long)"2:ffffffff,ffffffff,ffffffff",
		Child, HGroup,
			Child, VSpace(0),
			Child, YahooGroupsLogo ( ),
			Child, HVSpace,
		End,
		Child, VGroup,
			Child, HGroup,
				Child, data->info = InfoTextObject,
					MUIA_InfoText_ExpirationPeriod, 54000,
				End,
			End,
			Child, data->page = PageGroup, ReadListFrame,
				MUIA_InnerBottom,            2,
				MUIA_InnerLeft,              4,
				MUIA_InnerRight,             2,
				MUIA_InnerTop,               2,
				MUIA_Background, MUII_TextBack,
				Child, HGroup,
					Child, Label2("Group:"),
					Child, data->group = StringObj("Yahoo! Group Name", MAKE_ID('y','g','r','p'), 254),
					Child, BalanceObj(MAKE_ID('b','a','l','1')),
					Child, Label2("First MsgNum:"),
					Child, data->fmsgnum = IntegerStringObj("Message Number to start receiving from", MAKE_ID('f','m','s','g'), 16),
					Child, BalanceObj(MAKE_ID('b','a','l','2')),
					Child, Label2("Last MsgNum:"),
					Child, data->lmsgnum = IntegerStringObj("Last message number to get (leave 0 to get all)", MAKE_ID('l','m','s','g'), 16),
//					Child, BalanceObj(MAKE_ID('','','','')),
#if 0
					Child, Label2(""),
					Child, StringObj(NULL, MAKE_ID('','','',''), 64),
					Child, BalanceObj(MAKE_ID('','','','')),
#endif
					Child, data->startstask = ButtonObj("Start", 30, NULL),
				End,
				Child, HGroup,
					Child, BusyObject,
						MUIA_Weight, 20,
						MUIA_Busy_Speed, MUIV_Busy_Speed_User,
					End,
					Child, data->gauge = GaugeObject, GaugeFrame,
					//	MUIA_InnerRight,     8,
						MUIA_Gauge_Horiz,    TRUE,
					//	MUIA_Gauge_InfoText, "\033rreceiving message %ld of %ld...",
						MUIA_Background,     MUII_TextBack,
					End,
					Child, data->stopstask = ButtonObj("Stop", 30, NULL),
				End,
			End,
			Child, HGroup,
			//	InputListFrame,
				GroupSpacing(0),
				Child, NListviewObject,
					MUIA_HorizWeight, 30,
					MUIA_NListview_Horiz_ScrollBar, MUIV_NListview_HSB_FullAuto,
					MUIA_NListview_NList, data->leftlist = NListObject,
						MUIA_NList_ConstructHook, MUIV_NList_ConstructHook_String,
						MUIA_NList_DestructHook,  MUIV_NList_DestructHook_String,
					End,
				End,
				Child, data->lrbar = BalanceObj(MAKE_ID('Y','G','R','B')),
				Child, RegisterObject,
					MUIA_Register_Frame, data->RegisterFrame,
					MUIA_Register_Titles, data->RegisterTitles,
					
					/* MESSAGES */
					Child, NListviewObject,
						MUIA_NListview_Horiz_ScrollBar, MUIV_NListview_HSB_FullAuto,
						MUIA_NListview_NList, data->rightlist = NListObject,
							MUIA_NList_AutoVisible,		TRUE,
							MUIA_NList_TitleSeparator,	TRUE,
							MUIA_NList_Title,		TRUE,
							MUIA_NList_Input,		TRUE,
						//	MUIA_NList_EntryValueDependent, TRUE,
							MUIA_NList_DefaultObjectOnClick,TRUE,
							MUIA_NList_MinColSortable,	0,
							MUIA_NList_Imports,		MUIV_NList_Imports_All,
							MUIA_NList_Exports,		MUIV_NList_Exports_All,
							MUIA_NList_Format,		",,,,",
						End,
					End,
					
					
					/* SETTINGS */
					Child, ScrollgroupObject,	// main
					//	InnerSpacing(4,3),
						MUIA_Scrollgroup_Contents, VGroupV,
							Child, ColGroup(2), GroupFrame,
								MUIA_Background, MUII_GroupBack,
								Child, Label2("Hostname:"),
								Child, data->hostname = StringObj("Yahoo! Groups's hostname",MAKE_ID('h','o','s','t'),254),
								Child, Label2("Cookie:\n\n\n\n"),
								Child, data->cookie = TextinputscrollObject,
									StringFrame,
									MUIA_CycleChain, 1,
									MUIA_Textinput_Multiline, TRUE,
									MUIA_ObjectID, MAKE_ID('c','o','o','k'),
								End,
								Child, Label2("Timeout:"),
								Child, data->RecvTimeoutObj = SliderObj( 1, 88, 10, "receiver timeout (in seconds)", MAKE_ID('r','e','c','v')),
							End,
							Child, HVSpace,
							Child, ColGroup(2), GroupFrame,
								MUIA_Background, MUII_GroupBack,
								Child, Label2("Rules:\n\n\n\n"),
								Child, data->RulesObj = TextinputscrollObject,
									StringFrame,
									MUIA_CycleChain, 1,
									MUIA_Textinput_Multiline, TRUE,
									MUIA_ObjectID, MAKE_ID('r','u','l','e'),
								End,
							End,
							Child, HVSpace,
						End,
					End,
					
					
					/* PROXYS */
					Child, VGroup,
						Child, TextObject,
							TextFrame,
							MUIA_Background, MUII_TextBack,
							MUIA_Text_PreParse, "\033c\033b",
							MUIA_Text_Contents, "Proxys list\nformat = <hostname>[:<port>]",
						End,
						Child, NListviewObject,
							MUIA_NListview_Horiz_ScrollBar, MUIV_NListview_HSB_FullAuto,
							MUIA_NListview_NList, data->proxys = NListObject,
								MUIA_NList_ConstructHook, MUIV_NList_ConstructHook_String,
								MUIA_NList_DestructHook,  MUIV_NList_DestructHook_String,
							End,
						End,
						Child, ColGroup(2),
							Child, Label2("Cache:"),
							Child, data->proxycache = PopaslObj(NULL, NULL,FALSE,MAKE_ID('p','r','o','x')),
							Child, Label2("Proxy:"),
							Child, HGroup,
								Child, data->proxyentry = StringObj( NULL, MAKE_ID('p','r','x','e'), 255),
								Child, data->proxyadd = ButtonObj( "Add", 6, NULL),
								Child, data->proxydel = ButtonObj( "Delete Selected", 1, NULL),
							End,
						End,
					End,
					
					
					/* END OF MESSAGE TAGS */
					Child, VGroup,
						Child, TextObject,
							TextFrame,
							MUIA_Background, MUII_TextBack,
							MUIA_Text_PreParse, "\033c\033b",
							MUIA_Text_Contents, "End Of Message Tags\nno wildcards supported yet",
						End,
						Child, NListviewObject,
							MUIA_NListview_Horiz_ScrollBar, MUIV_NListview_HSB_FullAuto,
							MUIA_NListview_NList, data->eomtags = NListObject,
								MUIA_NList_ConstructHook, MUIV_NList_ConstructHook_String,
								MUIA_NList_DestructHook,  MUIV_NList_DestructHook_String,
							End,
						End,
						Child, ColGroup(2),
							Child, Label2("Cache:"),
							Child, data->eomcache = PopaslObj(NULL, NULL,FALSE,MAKE_ID('e','o','m','c')),
							Child, Label2("Entry:"),
							Child, HGroup,
								Child, data->eomentry = StringObj( NULL, MAKE_ID('e','o','m','e'), 255),
								Child, data->eomadd = ButtonObj( "Add", 6, NULL),
								Child, data->eomdel = ButtonObj( "Delete Selected", 1, NULL),
							End,
							Child, Label1("Criteria"),
							Child, data->eomcriteria = CycleObj( EOMCriteria, NULL, MAKE_ID('e','o','m','m')),
						End,
					End,
					
					
					/* READER */
					Child, VGroup,
						Child, HGroup, TextFrame,
							MUIA_Weight, 40,
							MUIA_Background, MUII_TextBack,
							InnerSpacing(2,2),
							Child, data->readhead = TextinputObject,
								MUIA_Textinput_Multiline, TRUE,
								MUIA_Textinput_NoInput, TRUE,
							End,
							Child, VGroup,
								MUIA_InnerBottom,0,
								MUIA_InnerLeft,  0,
								MUIA_InnerRight, 2,
								MUIA_InnerTop,   1,
								Child, RowGroup(2),
									GroupSpacing(0),
									Child, data->readprev = PopButton(MUII_ArrowUp),
									Child, data->readnext = PopButton(MUII_ArrowDown),
								End,
								Child, HVSpace,
							End,
						End,
						Child, data->readlist = TextinputscrollObject,
							ReadListFrame,
							MUIA_Textinput_Multiline, TRUE,
							MUIA_Textinput_NoInput, TRUE,
							MUIA_Textinput_WordWrap, TRUE,
						End,
					End,
				End,
			End,
		End,
	TAG_MORE, msg->ops_AttrList);
	
	if(!obj || !(data = INST_DATA(cl,obj)))
	{
		DeletePool( mempool );
		return 0;
	}
	
	CopyMem( tdata, data, sizeof(struct ClassData));
	FreePooled( data->mempool, tdata, sizeof(struct ClassData));
	
//	InitHook( &(data->construct_hook), ConstructHook, data );
//	InitHook( &(data->destruct_hook),  DestructHook,  data );
	InitHook( &(data->display_hook),   DisplayHook,   data );
	InitHook( &(data->compare_hook),   CompareHook,   data );
	
	SetAttrs( data->rightlist,
//		MUIA_NList_ConstructHook2,  (ULONG) &data->construct_hook,
//		MUIA_NList_DestructHook2,   (ULONG) &data->destruct_hook,
		MUIA_NList_DisplayHook2,    (ULONG) &data->display_hook,
		MUIA_NList_CompareHook2,    (ULONG) &data->compare_hook,
	TAG_DONE);
	
	
	data->apptask = FindTask(NULL);
	
	SetAttrs( obj,
		MUIA_YGRGroup_SubTaskName,		(ULONG) data->apptask->tc_Node.ln_Name,
		MUIA_YGRGroup_SubTaskPri,		(ULONG) data->apptask->tc_Node.ln_Pri,
		MUIA_YGRGroup_ProxysCacheFile,	(ULONG) "PROGDIR:.proxys.txt",
		MUIA_YGRGroup_EOMTagsCacheFile,	(ULONG) "PROGDIR:.eomtags.txt",
		MUIA_YGRGroup_YahooGroupsHost,	(ULONG) DEFAULT_YAHOOGROUPS_HOSTNAME,
		MUIA_YGRGroup_MailsFolder,		(ULONG) "PROGDIR:.cache",
		MUIA_YGRGroup_RecvTimeout,		 10,
	TAG_DONE );
	
	SetAttrsA( data->selfobj = obj, msg->ops_AttrList );
	
	#define obj data->rightlist
	
	DoMethod( obj, MUIM_Notify, MUIA_NList_TitleClick,  MUIV_EveryTime, obj, 4, MUIM_NList_Sort3, MUIV_TriggerValue, MUIV_NList_SortTypeAdd_2Values, MUIV_NList_Sort3_SortType_Both);
	DoMethod( obj, MUIM_Notify, MUIA_NList_TitleClick2, MUIV_EveryTime, obj, 4, MUIM_NList_Sort3, MUIV_TriggerValue, MUIV_NList_SortTypeAdd_2Values, MUIV_NList_Sort3_SortType_2);
	DoMethod( obj, MUIM_Notify, MUIA_NList_SortType,    MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_NList_TitleMark,MUIV_TriggerValue);
	DoMethod( obj, MUIM_Notify, MUIA_NList_SortType2,   MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_NList_TitleMark2,MUIV_TriggerValue);
	
	#undef obj
	
	DoMethod( data->startstask, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 2, MUIM_YGRGroup_StartSubTask, MUIV_YGRGroup_StartSubTask_User );
	
	DoMethod( data->stopstask, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MUIM_YGRGroup_KillSubTask );
	
	DoMethod( data->rightlist, MUIM_Notify, MUIA_NList_DoubleClick,
		MUIV_EveryTime, obj, 1, MUIM_YGRGroup_GoToReader );
	DoMethod( data->leftlist, MUIM_Notify, MUIA_NList_Active,
		MUIV_EveryTime, obj, 1, MUIM_YGRGroup_GroupSelected );
	
	DoMethod( data->eomadd, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MUIM_YGRGroup_AddEOMTagEntry );
	DoMethod( data->eomdel, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MUIM_YGRGroup_DelEOMTagEntry );
	
	DoMethod( data->proxyadd, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MUIM_YGRGroup_AddProxyEntry );
	DoMethod( data->proxydel, MUIM_Notify, MUIA_Pressed, FALSE,
		obj, 1, MUIM_YGRGroup_DelProxyEntry );
	
	DoMethod( data->eomtags, MUIM_Notify, MUIA_NList_Active, MUIV_EveryTime,
		data->eomdel, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue );
	
	DoMethod( data->proxys, MUIM_Notify, MUIA_NList_Active, MUIV_EveryTime,
		data->proxydel, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue );
	
	DoMethod( data->RecvTimeoutObj, MUIM_Notify, MUIA_Slider_Level, MUIV_EveryTime,
		MUIV_Notify_Self, 3, MUIM_WriteLong, MUIV_TriggerValue, &data->recvTimeout );
	
	set( data->proxydel, MUIA_Disabled, TRUE );
	set( data->eomdel,   MUIA_Disabled, TRUE );
	
	DBG("\n\tCreated new object for task 0x%08lx, "
		"obj = 0x%08lx, data = 0x%08lx\n\n", data->apptask, obj, data );
	
	return((ULONG)obj);
}

/***************************************************************************/

INLINE ULONG YGRGroup_dispose(struct IClass *cl,Object *obj,Msg msg)
{
	struct ClassData *data = INST_DATA(cl,obj);
	
	data->disposing = TRUE;
	
	DBG_POINTER(data->subtask.task);
	
	if(data->subtask.task != NULL)
	{
		//Signal( data->subtask.task, SIGBREAKF_CTRL_C );
		DoMethod( obj, MUIM_YGRGroup_KillSubTask );
		
		ObtainSemaphore(data->sem);
		ReleaseSemaphore(data->sem);
		
		ExpungeIndex( data, &(data->subtask.folder));
	}
	
	// da igual si es el mismo que data->subtask.folder, ->New se pone a cero
	ExpungeIndex( data, &(data->ActiveGroup));
	
	SaveSingleNListContents( data->eomtags, data->eomtagscachefile );
	SaveSingleNListContents( data->proxys, data->proxyscachefile );
	
	DoSuperMethodA(cl, obj, msg);
	
	__delete_semaphore(data->sem);
	__delete_semaphore(data->semalloc);
	
	DeletePool( data->mempool );
	data->mempool = NULL;
	data->MagicID = NULL;
	
#ifdef _PROTO_ICON_H
	if(dobj != NULL)
	{
		FreeDiskObject( dobj );
	}
#endif
	
	return TRUE;
}

/***************************************************************************/

#define NUM_OF_BUFS	40

INLINE BOOL ReadMailsFolder( struct ClassData *data )
{
	register struct ExAllControl * eaControl;
	register struct ExAllData    * eaBuffer, * eaData;
	register LONG more, eaBuffSize = NUM_OF_BUFS * sizeof(struct ExAllData);
	register BPTR lock;
	BOOL success = FALSE;
	
	if((lock = Lock(data->MailsFolder, SHARED_LOCK)))
	{
		struct FileInfoBlock fib;
		
		success = (Examine(lock, &fib) && (fib.fib_DirEntryType > 0));
		
		if(success)
		{
			success = FALSE;
			
			eaControl = (struct ExAllControl *) AllocDosObject(DOS_EXALLCONTROL, NULL);
			
			if( eaControl != NULL )
			{
				eaControl->eac_LastKey = 0;
				eaControl->eac_MatchString = (UBYTE *) NULL;
				eaControl->eac_MatchFunc = (struct Hook *) NULL;
				
				eaBuffer = (struct ExAllData *) AllocMem( eaBuffSize, MEMF_ANY );
				
				if( eaBuffer != NULL)
				{
					BOOL FirstEntry = TRUE;
					
					success = TRUE;
					
					do {
						
						more = ExAll( lock, eaBuffer, eaBuffSize, ED_NAME, eaControl);
						
						if( (!more) && (IoErr() != ERROR_NO_MORE_ENTRIES) )
						{
							DBG("ExAll failed abnormally!\n");
							success = FALSE;
							break;
						}
						
						if (eaControl->eac_Entries == 0)
						{
							DBG("no more entries\n");
							continue;
						}
						
						eaData = (struct ExAllData *) eaBuffer;
						
						while( eaData != NULL )
						{
							long filelen = strlen(eaData->ed_Name), pfx = strlen(IDX_EXT);
							
							if(filelen > pfx)
							{
								if(!Stricmp(eaData->ed_Name + filelen - pfx, IDX_EXT))
								{
									if(FirstEntry) {
										DoMethod(data->leftlist, MUIM_NList_Clear );
										set( data->leftlist, MUIA_NList_Quiet, TRUE );
										FirstEntry = FALSE;
									}
									
									LeftListInsert( data, eaData->ed_Name );
								}
							}
							
							eaData = eaData->ed_Next;
						}
						
					} while(more);
					
					FreeMem( eaBuffer, eaBuffSize );
				}
				
				if(success)
					set( data->leftlist, MUIA_NList_Quiet, FALSE );
				
				FreeDosObject(DOS_EXALLCONTROL, eaControl);
			}
		}
		
		UnLock(lock);
	}
	
	data->LastError = IoErr();
	
	return(success);
}

/***************************************************************************/

INLINE STRPTR Cookie(struct ClassData * data, STRPTR utag)
{
	/* try to fix a user-given cookie, removing and completing parts */
	
	UBYTE buf[4096], * s=utag, * d=buf;
	long maxlen = sizeof(buf)-1;
	
	do {
		switch(*s)
		{
			case '\n':
			case '\r':
			case '\t':
				if((maxlen -= 2) <= 0)
					break;
				
				*d++ = ';';
				*d++ = ' ';
				
				while(*s && ((*(s+1) == ' ') || (*(s+1) == '\n') || (*(s+1) == '\r') || (*(s+1) == '\t'))) ++s;
				
				break;
			
			case ' ':
				if(*(s-1) != ';')
				{
					/* cookie parts must be separated by "; ", and this seems
					 * not the case here, fix it */
					
					if((maxlen -= 2) <= 0)
						break;
					
					*d++ = ';';
					*d++ = ' ';
					
					while(*s && *(s+1) == 0x20) ++s;
					
					break;
				}
			default:
				*d++ = *s;	--maxlen; break;
		}
	} while(maxlen > 0 && *++s);
	
	if(maxlen <= 0)
	{
		InfoText("buffer overflow filling cookie!");
		return NULL;
	}
	
	*d = 0;
	return strdup( buf );
}

/***************************************************************************/

#define NUM_RULES	1

INLINE LONG FillRules(struct ClassData * data)
{
	char * rules = (char *)xget( data->RulesObj, MUIA_Textinput_Contents);
	char * ptr = rules, * r ;
	int x;
	
	if(!(ptr && *ptr))	return -1;
	while(*ptr && (*ptr == ' ' || *ptr == '\n' || *ptr == '\t')) ptr++;
	if(!(ptr && *ptr))	return -2;
	
	if(data->rules)
	{
		for( x = 0 ; data->rules[x] ; x++ )
			free( data->rules[x] );
		free(data->rules);
	}
	
	if(!(data->rules = calloc(NUM_RULES+1,sizeof(void *))))
		return -3;
	
	x = 0;
	while((r = _strsep( &ptr, "\n")))
	{
		if(!(r && *r) || *r == ';')	continue;
		while(*r && (*r == ' ' || *r == '\n' || *r == '\t')) r++;
		if(!(r && *r))	continue;
		
		if(x == NUM_RULES)
			break;
		
		if(!(data->rules[x++] = (APTR) strdup( r )))
			return -5;
		
		DBG_STRING(data->rules[x-1]);
	}
	
	DBG_ASSERT(x == NUM_RULES);
	
	return 0;
}

/***************************************************************************/

INLINE ULONG YGRGroup_set(struct IClass *cl, Object *obj, Msg msg)
{
	register struct ClassData * data = INST_DATA(cl,obj);
	struct TagItem *tag, *tags = (((struct opSet *)msg)->ops_AttrList);
	
	while(( tag = (struct TagItem *) NextTagItem( &tags )))
	{
		ULONG tidata = tag->ti_Data;
		
		switch (tag->ti_Tag)
		{
			case MUIA_YGRGroup_YahooGroupsHost:
			{
				uchar_free( data, data->yahoo.hostname );
				
				data->yahoo.hostname = uchar_dup( data, (STRPTR) tidata );
				
				set( data->hostname, MUIA_String_Contents, (ULONG) data->yahoo.hostname );
				
			}	break;
			
			case MUIA_YGRGroup_YahooGroup:
			{
				uchar_free( data, data->yahoo.group);
				
				data->yahoo.group = uchar_dup( data, (STRPTR)tidata);
				
				set( data->group, MUIA_String_Contents, (ULONG) data->yahoo.group );
				
			}	break;
			
			case MUIA_YGRGroup_FirstMsgNum:
				data->yahoo.fmsgnum = tidata;
				break;
			
			case MUIA_YGRGroup_LastMsgNum:
				if((data->yahoo.lmsgnum = tidata) == 0)
					data->yahoo.lmsgnum = (ULONG) -1;
				break;
			
			case MUIA_YGRGroup_YahooCookie:
			{
				if(data->yahoo.cookie)
					free(data->yahoo.cookie);
				
				data->yahoo.cookie = Cookie(data,(STRPTR)tidata);
				
			}	break;
			
			case MUIA_YGRGroup_MailsFolder:
			{
				STRPTR folder;
				
				data->LastError = ERR_NOMEM;
				
				folder = uchar_dup(data, (STRPTR)tidata);
				DBG_STRING(folder);
				
				if(folder != NULL)
				{
					int flen = strlen(folder);
					
					if(*((char *)&folder[flen]) != '/')
					{
						/* an extra byte is available on the allocated buffer.. */
						
						*((char *)&folder[flen])	=  '/';
						*((char *)&folder[flen+1])	= '\0';
					}
					
					data->LastError = MakeDir( folder );
				}
				
				if(data->LastError == ERR_NOERROR)
				{
					uchar_free(data, data->MailsFolder);
					
					data->MailsFolder = (STRPTR) folder;
				}
				else if(data->MailsFolder == NULL)
				{
					/* fall back to use RAM:, damn */
					
					data->MailsFolder = uchar_dup(data, "RAM:");
				}
				
				DBG_STRING(data->MailsFolder);
				//hexdump( data->MailsFolder, 0, strlen(data->MailsFolder)+16);
				
				DoMethod( obj, MUIM_YGRGroup_ReadMailsFolder );
				
			}	break;
			
			case MUIA_YGRGroup_ShowLeftList:
				data->showleftlist = (tidata ? TRUE : FALSE);
				DBG_VALUE(data->showleftlist);
				
				if(data->showleftlist == FALSE)
				{
					set( data->leftlist, MUIA_ShowMe, FALSE );
					set( data->lrbar, MUIA_ShowMe, FALSE );
				}
				break;
			
			case MUIA_YGRGroup_SubTaskPri:
				data->subtask.pri = (long) tidata;
				DBG_VALUE(data->subtask.pri);
				break;
			
			case MUIA_YGRGroup_SubTaskName:
			{
				STRPTR subtaskname;
				
				subtaskname = uchar_dup(data, (STRPTR)tidata);
				DBG_STRING(subtaskname);
				
				if(subtaskname != NULL)
				{
					uchar_free(data, data->subtask.name);
					
					data->subtask.name = subtaskname;
				}
			}	break;
			
			case MUIA_YGRGroup_ProxysCacheFile:
			{
				STRPTR proxyscachefile;
				
				proxyscachefile = uchar_dup(data, (STRPTR)tidata);
				DBG_STRING(proxyscachefile);
				
				if(proxyscachefile != NULL)
				{
					uchar_free(data, data->proxyscachefile );
					
					data->proxyscachefile = (STRPTR) proxyscachefile;
					
					LoadSingleNListContents( data->proxys, data->proxyscachefile );
				}
				
				set( data->proxycache, MUIA_String_Contents,
					(ULONG) proxyscachefile );
				
			}	break;
			
			case MUIA_YGRGroup_EOMTagsCacheFile:
			{
				STRPTR eomtagscachefile;
				
				eomtagscachefile = uchar_dup(data, (STRPTR)tidata);
				DBG_STRING(eomtagscachefile);
				
				if(eomtagscachefile != NULL)
				{
					uchar_free(data, data->eomtagscachefile );
					
					data->eomtagscachefile = (STRPTR) eomtagscachefile;
					
					LoadSingleNListContents( data->eomtags, data->eomtagscachefile );
				}
				
				set( data->eomcache, MUIA_String_Contents,
					(ULONG) eomtagscachefile );
				
			}	break;
			
			case MUIA_YGRGroup_RecvTimeout:
				data->recvTimeout = tidata;
				break;
			
		}
	}
	
	return(DoSuperMethodA(cl, obj, msg));
}

/***************************************************************************/

INLINE ULONG YGRGroup_get(struct IClass *cl, Object *obj, struct opGet * msg)
{
	struct ClassData *data = INST_DATA( cl, obj);
	
	#define STORE *(msg->opg_Storage)
	
	switch(msg->opg_AttrID)
	{
		case MUIA_YGRGroup_YahooGroupsHost:
			STORE = (ULONG) data->yahoo.hostname;
			return 1;
		
		case MUIA_YGRGroup_YahooGroup:
			STORE = (ULONG) data->yahoo.group;
			return 1;
		
		case MUIA_YGRGroup_FirstMsgNum:
			STORE = data->yahoo.fmsgnum;
			return 1;
		
		case MUIA_YGRGroup_LastMsgNum:
			STORE = data->yahoo.lmsgnum;
			return 1;
		
		case MUIA_YGRGroup_YahooCookie:
			STORE = (ULONG) data->yahoo.cookie;
			return 1;
		
		case MUIA_YGRGroup_MailsFolder:
			STORE = (ULONG) data->MailsFolder;
			return 1;
		
		case MUIA_YGRGroup_ShowLeftList:
			STORE = data->showleftlist;
			return 1;
		
		case MUIA_YGRGroup_LeftListObj:
			STORE = (ULONG) data->leftlist;
			return 1;
		
		case MUIA_YGRGroup_RightListObj:
			STORE = (ULONG) data->rightlist;
			return 1;
		
		case MUIA_YGRGroup_MemPool:
			STORE = (ULONG) &(data->mempool);
			return 1;
		
		case MUIA_YGRGroup_SubTaskPri:
			STORE = (ULONG) data->subtask.pri;
			return 1;
		
		case MUIA_YGRGroup_SubTaskName:
			STORE = (ULONG) data->subtask.name;
			return 1;
		
		case MUIA_YGRGroup_SubTaskTask:
			STORE = (ULONG) data->subtask.task;
			return 1;
		
		case MUIA_YGRGroup_RegisterFrame:
			STORE = data->RegisterFrame;
			return 1;
		
		case MUIA_YGRGroup_ProxysCacheFile:
			STORE = (ULONG) data->proxyscachefile;
			return 1;
		
		case MUIA_YGRGroup_EOMTagsCacheFile:
			STORE = (ULONG) data->eomtagscachefile;
			return 1;
		
		case MUIA_YGRGroup_ProxyList:
			STORE = (ULONG) data->proxy;
			return 1;
		
		case MUIA_YGRGroup_EOMTagsList:
			STORE = (ULONG) data->eomtag;
			return 1;
		
		case MUIA_YGRGroup_LastError:
			STORE = data->LastError;
			return 1;
		
		case MUIA_YGRGroup_LastErrorString:
			STORE = (ULONG) ErrorString( data->LastError );
			return 1;
		
		case MUIA_YGRGroup_RecvTimeout:
			STORE = data->recvTimeout;
			return 1;
	}
	return(DoSuperMethodA(cl, obj, (Msg)msg));
}

/***************************************************************************/

INLINE ULONG YGRGroup_setup(struct IClass *cl, Object *obj, Msg msg)
{
	register struct ClassData * data = INST_DATA(cl,obj);
	STATIC BOOL FirstSetup = TRUE;
	
	if(!DoSuperMethodA(cl, obj, msg))
		return FALSE;
	
	data->appobj = _app(obj);
	
	DBG("Application Object for Application Task 0x%08lx is 0x%08lx\n",
		data->apptask, data->appobj );
	
	if(data->yahoo.group != NULL)
		set( data->group, MUIA_String_Contents, data->yahoo.group );
	if(data->yahoo.cookie != NULL && FirstSetup)
		set( data->cookie, MUIA_Textinput_Contents, data->yahoo.cookie );
	
	FirstSetup = FALSE;
	
#ifdef _PROTO_ICON_H
	if(dobj == NULL)
	{
		if((dobj = GetDiskObject("YGRMUI")))
			set(_app(obj), MUIA_Application_DiskObject, dobj );
	}
#endif
	
	return TRUE;
}

/***************************************************************************/

INLINE ULONG YGRGroup_cleanup(struct IClass *cl, Object *obj, Msg msg)
{
	register struct ClassData * data = INST_DATA(cl,obj);
	
	return(DoSuperMethodA(cl, obj, msg));
}

/***************************************************************************/

INLINE ULONG YGRGroup_askminmax(struct IClass *cl, Object *obj,struct MUIP_AskMinMax *msg)
{
	register struct ClassData * data = INST_DATA(cl,obj);
	
	DoSuperMethodA(cl, obj,(Msg) msg);
	
	return 0;
}

/***************************************************************************/

INLINE ULONG YGRGroup_show(struct IClass *cl, Object *obj, Msg msg)
{
	register struct ClassData * data = INST_DATA(cl,obj);
	
	return(DoSuperMethodA(cl, obj, msg));
}

/***************************************************************************/

INLINE ULONG YGRGroup_hide(struct IClass *cl, Object *obj, Msg msg)
{
	register struct ClassData * data = INST_DATA(cl,obj);
	
	return(DoSuperMethodA(cl, obj, msg));
}

/***************************************************************************/

INLINE ULONG YGRGroup_draw(struct IClass *cl, Object *obj,struct MUIP_Draw *msg)
{
	register struct ClassData * data = INST_DATA(cl,obj);
	
	DoSuperMethodA(cl, obj,(Msg) msg);
	
	return 0;
}

/***************************************************************************/

#if 0
INLINE ULONG YGRGroup_(struct IClass *cl, Object *obj, Msg msg)
{
	register struct ClassData * data = INST_DATA(cl,obj);
	
	return(DoSuperMethodA(cl, obj, msg));
}
#endif

/***************************************************************************/

DISPATCHERPROTO(_Dispatcher)
{
	switch(msg->MethodID) {
		
		case OM_NEW:     return( YGRGroup_new     (cl, obj, (APTR)msg));
		case OM_DISPOSE: return( YGRGroup_dispose (cl, obj, (APTR)msg));
		case OM_SET:     return( YGRGroup_set     (cl, obj, (APTR)msg));
		case OM_GET:     return( YGRGroup_get     (cl, obj, (APTR)msg));
		
		case MUIM_Setup:
			return( YGRGroup_setup     (cl, obj, (APTR)msg));
		
		//------------------------------------------------------- +++++
		
		case MUIM_YGRGroup_StartSubTask:
		{
			register struct ClassData * data = INST_DATA(cl,obj);
			ULONG mode = (ULONG) MARG1;
			BOOL isNewGroup;
			
			DBG("MUIM_YGRGroup_LaunchSubTask\n");
			
			data->LastError = ERR_NOERROR;
			
			if(data->subtask.task != NULL)
			{
				InfoText("I can't manage multiple subtasks (yet)");
				return 1;
			}
			
			switch( mode )
			{
				case MUIV_YGRGroup_StartSubTask_User:
				{
					STRPTR tmp;
					
					if(!((tmp = _GStr(data->group)) && *tmp))
					{
						InfoText("You must provide a valid Yahoo! Group.");
						return 2;
					}
					set( obj, MUIA_YGRGroup_YahooGroup, (ULONG) tmp );
					
					if(!((tmp = (STRPTR)xget(data->cookie, MUIA_Textinput_Contents)) && *tmp))
					{
						InfoText("The Cookie string is missing!");
						return 3;
					}
					set( obj, MUIA_YGRGroup_YahooCookie, (ULONG) tmp );
					
					if(!((tmp = _GStr(data->hostname)) && *tmp))
					{
						tmp = DEFAULT_YAHOOGROUPS_HOSTNAME;
					}
					set( obj, MUIA_YGRGroup_YahooGroupsHost, (ULONG) tmp );
					
					
					if((data->yahoo.group == NULL) || (data->yahoo.cookie == NULL)
						|| (data->yahoo.hostname == NULL))
					{
						Delay( 25 ); // half second, may there was a previous msg...
						InfoTextError(data->LastError = ERR_NOMEM);
						return 4;
					}
					
					set(obj, MUIA_YGRGroup_FirstMsgNum, xget( data->fmsgnum,MUIA_String_Integer));
					set(obj, MUIA_YGRGroup_LastMsgNum, xget( data->lmsgnum,MUIA_String_Integer));
					
				}	break;
				
				case MUIV_YGRGroup_StartSubTask_Auto:
					break;
				
				default:
					InfoText("wrong start mode used!");
					return 5;
			}
			
			data->yahoo.ip = Resolver( data->yahoo.hostname );
			
			if((data->yahoo.ip == (ULONG) -1) || !data->yahoo.ip)
			{
				InfoText("Cannot resolve Yahoo! Groups's hostname \"%s\"!",
					data->yahoo.hostname );
				
				data->LastError = ERR_RESOLVER;
				return 6;
			}
			
			if(FillRules( data ))
			{
				InfoText("damn, no rules provided...\n");
				return 7;
			}
			
			LoadEOMTags( data );
			LoadProxyList( data );
			
			DBG_STRING(data->yahoo.cookie);
			DBG_STRING(data->yahoo.group);
			DBG_VALUE(data->yahoo.fmsgnum);
			DBG_VALUE(data->yahoo.lmsgnum);
			
			// check if this is a new yahoo group to download msgs from
			isNewGroup = IsNewGroup( data, data->yahoo.group );
			
			if((data->ActiveGroup == NULL) || isNewGroup)
			{
				data->LastError = SetActiveGroup(data, data->yahoo.group);
				if(data->LastError != ERR_NOERROR)
				{
					InfoText("Error %s index: %s", "setting", ErrorString(data->LastError));
					return 9;
				}
				
				if( isNewGroup )
				{
					LeftListInsert( data, data->yahoo.group );
				}
				LeftListSelect( data, data->yahoo.group );
			}
			
			data->window = _window(obj);
			DBG_POINTER(data->window);
			
			if(subtask( data ) == FALSE)
			{
				InfoText("Cannot create subtask!, hmm..thats very bad...");
				data->LastError = ERR_INTERNAL;
				return 10;
			}
			
			InfoText("My subtask is contacting Yahoo, please wait...");
			set( data->page, MUIA_Group_ActivePage, 1 );
			
		}	return FALSE; /* no errors */
		
		
		//------------------------------------------------------- +++++
		
		case MUIM_YGRGroup_GroupSelected:
		{
			register struct ClassData * data = INST_DATA(cl,obj);
			STRPTR entry = NULL;
			BOOL error = FALSE;
			int ret;
			
			DBG("MUIM_YGRGroup_GroupSelected\n");
			
			DoMethod( data->leftlist, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &entry );
			if(!(entry && *entry))
			{
				DBG("WTF, no active entry !?\a\n");
				InfoTextError(data->LastError = ERR_INTERNAL);
				return FALSE;
			}
			
			if((data->ActiveGroup != NULL)
				&& !Stricmp( entry, data->ActiveGroup->YahooGroup))
			{
				DBG("same group selected\n");
				return TRUE;
			}
			
			set( obj, MUIA_YGRGroup_YahooGroup, (ULONG) entry );
			if(data->yahoo.group == NULL)
			{
				InfoTextError(ERR_NOMEM);
				return FALSE;
			}
			
			
			ObtainSemaphore(data->sem);
			
			error = TRUE;
			
			if((data->subtask.folder != NULL) && 
				!Stricmp( entry, data->subtask.folder->YahooGroup))
			{
				/* El grupo selecionado es el que esta descargando la subtask,
				 * (ha sido re-selecionado) asi pues solo que hay que actualizar
				 * el puntero en data->ActiveGroup */
				
				data->ActiveGroup = data->subtask.folder;
				
				error = FALSE;
			}
			else if((ret = SetActiveGroup(data, data->yahoo.group)) != ERR_NOERROR)
			{
				InfoText("Error %s index: %s", "selecting", ErrorString(ret));
			}
			else if((ret = LoadIndex(data)) != ERR_NOERROR)
			{
				InfoText("Error %s index: %s", "loading", ErrorString(ret));
			}
			else error = FALSE;
			
			if( error == FALSE )
			{
				if((ret = InsertIndex(data)) != ERR_NOERROR)
				{
					InfoText("Error %s index: %s", "inserting", ErrorString(ret));
				}
			}
			else
			{
				set( data->fmsgnum, MUIA_String_Integer, data->ActiveGroup->Total );
			}
			
			ReleaseSemaphore(data->sem);
			
			if( error )
			{
				DisplayBeep(NULL);
				return FALSE;
			}
			
			DBG("Messages from %s Index loaded and inserted succesfuly\n\n", entry);
			
		}	return TRUE;
		
		
		//------------------------------------------------------- +++++
		
		case MUIM_YGRGroup_KillSubTask:
		{
			register struct ClassData * data = INST_DATA(cl,obj);
			
			InfoText("Breaking SubTask, please wait...");
			
			Signal( data->subtask.task, SIGBREAKF_CTRL_C );
			Signal( data->subtask.task, SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D );
			
			Delay( 25 );
			// XXX: comprueba que no puede haber pifias
			
		}	return TRUE;
		
		
		//------------------------------------------------------- +++++
		
		case MUIM_YGRGroup_SubTaskStopped:
		{
			register struct ClassData * data = INST_DATA(cl,obj);
			ULONG error = (ULONG) MARG1;
			
			DBG("MUIM_YGRGroup_SubTaskStopped\n");
			
			if(SameGroupFolder( data, FALSE ) == FALSE)
			{
				/* Si el grupo que esta activo y del que acabo de pillar
				 * mensages no son los mismos salva este ultimo a disco
				 */
				
				ExpungeIndex( data, &(data->subtask.folder));
			}
			
			if(error != ERR_NOERROR)
			{
				InfoText("SubTask finished with error %ld: %s",
						error, ErrorString(error));
			}
			else {
				InfoText("SubTask finished succesfuly.");
			}
			
			Signal( data->subtask.task, SIGBREAKF_CTRL_E );
			ObtainSemaphore(data->sem);
			
			data->subtask.task   = NULL;
			data->subtask.folder = NULL;
			
			set( data->page, MUIA_Group_ActivePage, 0 );
			ReleaseSemaphore(data->sem);
			
			DisplayBeep(NULL);
			
		}	return TRUE;
		
		
		//------------------------------------------------------- +++++
		
		case MUIM_YGRGroup_AddEOMTagEntry:
		case MUIM_YGRGroup_AddProxyEntry:
		{
			register struct ClassData * data = INST_DATA(cl,obj);
			Object * string = NULL, * list = NULL;
			STRPTR entry;
			
			switch(msg->MethodID)
			{
				case MUIM_YGRGroup_AddEOMTagEntry:
					string = data->eomentry;
					list = data->eomtags;
					break;
				case MUIM_YGRGroup_AddProxyEntry:
					string = data->proxyentry;
					list = data->proxys;
					break;
			}
			
			if(list == NULL) // cannot happen..
				return FALSE;
			
			entry = _GStr( string );
			
			if(entry && *entry)
			{
				DoMethod( list, MUIM_NList_InsertSingle, 
					entry, MUIV_NList_Insert_Bottom );
				
				set( string, MUIA_String_Contents, NULL );
			}
		}	return TRUE;
		
		
		//------------------------------------------------------- +++++
		
		case MUIM_YGRGroup_DelEOMTagEntry:
		case MUIM_YGRGroup_DelProxyEntry:
		{
			register struct ClassData * data = INST_DATA(cl,obj);
			Object *list = NULL;
			
			switch(msg->MethodID)
			{
				case MUIM_YGRGroup_DelEOMTagEntry:
					list = data->eomtags;
					break;
				case MUIM_YGRGroup_DelProxyEntry:
					list = data->proxys;
					break;
			}
			
			if(list == NULL) // cannot happen..
				return FALSE;
			
			DoMethod( list, MUIM_NList_Remove, MUIV_NList_Remove_Active);
			set( list, MUIA_NList_Active, MUIV_NList_Active_Off );
			
		}	return TRUE;
		
		
		//------------------------------------------------------- +++++
		
		case MUIM_YGRGroup_ReadMailsFolder:
		{
			struct ClassData * data = INST_DATA(cl,obj);
			BOOL error = TRUE;
			
			if(data->MailsFolder != NULL)
			{
				error = !ReadMailsFolder( data );
			}
			
			if( error )
			{
				InfoText("uh-oh, error reading mails folder: %s",
					ErrorString(data->LastError));
			}
		}	return TRUE;
		
		//------------------------------------------------------- +++++
		
		case MUIM_YGRGroup_GoToReader:
		{
			struct ClassData * data = INST_DATA(cl,obj);
			struct YGRDBIndex * entry = NULL;
			
			DoMethod( data->rightlist, MUIM_NList_GetEntry,
				MUIV_NList_GetEntry_Active, &entry );
			
			data->reader.MailNum = entry->msgnum - 1;
			
			DoMethod( obj, MUIM_YGRGroup_ReaderNext );
			
		}	return TRUE;
		
		//------------------------------------------------------- +++++
		
		case MUIM_YGRGroup_ReaderNext:
		case MUIM_YGRGroup_ReaderPrev:
		{
			struct ClassData * data = INST_DATA(cl,obj);
			long msgnum, msize;
			UBYTE file[1024], * mail;
			
			if(data->reader.MailNum == -1)
			{
				DisplayBeep(NULL);
				return FALSE;
			}
			
			ObtainSemaphore(data->sem);
			
			if(msg->MethodID == MUIM_YGRGroup_ReaderNext)
					msgnum = ++data->reader.MailNum;
			else	msgnum = --data->reader.MailNum;
			
			data->LastError = MakeMailFile( data->ActiveGroup, file,sizeof(file)-1,msgnum,FALSE);
			
			if(data->LastError == ERR_NOERROR)
				data->LastError = FileToMem(data,file,&mail,&msize);
			
			if(data->LastError != ERR_NOERROR)
			{
				InfoTextErrorMsg("error loading mail file: %s", data->LastError );
			}
			else
			{
				if(data->reader.Mail)
					free(data->reader.Mail);
				
				data->reader.Mail		= mail;
				data->reader.MailSize	= msize;
				
				data->reader.HeaderPos = FindPos( data->reader.Mail, "\n\n");
				data->reader.Mail[data->reader.HeaderPos-1] = 0;
				
				SetReaderMail( data, 
					&data->reader.Mail[FindPos(data->reader.Mail,"\n")],
					&data->reader.Mail[data->reader.HeaderPos] );
			}
			
			ReleaseSemaphore(data->sem);
			
		}	return TRUE;
		
		//------------------------------------------------------- +++++
		
		//------------------------------------------------------- +++++
		
#if 0
		case MUIM_Cleanup:
			return( YGRGroup_cleanup   (cl, obj, (APTR)msg));
		
		case MUIM_AskMinMax:
			return( YGRGroup_askminmax (cl, obj, (APTR)msg));
		
		case MUIM_Show:
			return( YGRGroup_show      (cl, obj, (APTR)msg));
		
		case MUIM_Hide:
			return( YGRGroup_hide      (cl, obj, (APTR)msg));
		
		case MUIM_Draw:
			return( YGRGroup_draw      (cl, obj, (APTR)msg));
#endif
	}
	return(DoSuperMethodA(cl,obj,msg));
}

/***************************************************************************/

#if !YGRGROUP_IS_PUBLIC

struct MUI_CustomClass * CreateYGRGroupCustomClass(struct Library * base)
{
	struct MUI_CustomClass * ThisClass;
	
	ThisClass = MUI_CreateCustomClass(base, SUPERCLASS, NULL, sizeof(struct INSTDATA), ENTRY(_Dispatcher));
	
	return ThisClass;
}
#endif

