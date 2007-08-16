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


/** $Id: muiface.c,v 0.1 2006/07/04 03:42:46 diegocr Exp $
 **/


#include "YGRGroup_mcc_priv.h"

/***************************************************************************/

struct ExecBase       *SysBase       = NULL;
struct UtilityBase    *UtilityBase   = NULL;
struct DosLibrary     *DOSBase       = NULL;
struct GfxBase        *GfxBase       = NULL;
struct IntuitionBase  *IntuitionBase = NULL;
struct Library        *MUIMasterBase = NULL;

#if !YGRGROUP_IS_PUBLIC
struct MUI_CustomClass * YGRGroupClass = NULL;
# undef YGRGroupObject
# define YGRGroupObject	NewObject( YGRGroupClass->mcc_Class, NULL
#endif

STATIC STRPTR ApplicationData[] = {
	"YGRMUI",					/* title / base */
	"$VER: YGRMUI 1.0 (02.07.2006)\r\n",		/* Version */
	"©2006, Diego Casorran",			/* Copyright */
	"YahooGroupsRipper Magic User Interface",	/* Description */
	NULL
};

STATIC void __request(const char *msg);
/***************************************************************************/

int __main( void )
{
	Object * app = NULL, * window;
	BOOL success = FALSE;
	
	if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME,MUIMASTER_VMIN)))
	{
		__request("This program requires MUI!");
		goto done;
	}
	
	#if !YGRGROUP_IS_PUBLIC
	if(!(YGRGroupClass = CreateYGRGroupCustomClass(NULL)))
	{
		__request("Cannot create custom class...");
		goto done;
	}
	
	UtilityBase   = (APTR)YGRGroupClass->mcc_UtilityBase;
	DOSBase       = (APTR)YGRGroupClass->mcc_DOSBase;
	GfxBase       = (APTR)YGRGroupClass->mcc_GfxBase;
	IntuitionBase = (APTR)YGRGroupClass->mcc_IntuitionBase;
	#else
	# error fixme, openlibs
	#endif
	
	app = ApplicationObject,
		MUIA_Application_Title      , ApplicationData[0],
		MUIA_Application_Version    , ApplicationData[1],
		MUIA_Application_Copyright  , ApplicationData[2],
		MUIA_Application_Author     , ApplicationData[2] + 7,
		MUIA_Application_Description, ApplicationData[3],
		MUIA_Application_Base       , ApplicationData[0],
		
		SubWindow, window = WindowObject,
		//	MUIA_Window_Title, ApplicationData[3],
			MUIA_Window_ID, MAKE_ID('M','A','I','N'),
			WindowContents, YGRGroupObject, End,
		End,
	End;

	if( ! app ) {
		__request("Failed to create Application!");
		goto done;
	}
	
	DoMethod(window,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
		app,2,MUIM_Application_ReturnID,MUIV_Application_ReturnID_Quit);
	
	DoMethod( app, MUIM_Application_Load, MUIV_Application_Load_ENVARC );
	
	set(window,MUIA_Window_Open,TRUE);
	
	if(xget( window, MUIA_Window_Open))
	{
		ULONG sigs = 0;

		while(DoMethod(app,MUIM_Application_NewInput,&sigs) != (ULONG)MUIV_Application_ReturnID_Quit)
		{
			if (sigs)
			{
				sigs = Wait(sigs | SIGBREAKF_CTRL_C);
				if (sigs & SIGBREAKF_CTRL_C) break;
			}
		}
		
		set(window,MUIA_Window_Open,FALSE);
		DoMethod( app, MUIM_Application_Save, MUIV_Application_Save_ENVARC );
		
		success = TRUE;
	}
	
done:
	if(app)
		MUI_DisposeObject(app);
	if(YGRGroupClass)
		MUI_DeleteCustomClass( YGRGroupClass );
	if (MUIMasterBase)
		CloseLibrary(MUIMasterBase);
	
	return((int)success);
}

/***************************************************************************/

static void __request(const char *msg)
{
	APTR IntuitionBase, SysBase=*(APTR *)4L;
	static struct IntuiText body = { 0,0,0, 15,5, NULL, NULL, NULL };
	static struct IntuiText   ok = { 0,0,0,  6,3, NULL, "Ok", NULL };
	
	if(((struct Process *)FindTask(NULL))->pr_WindowPtr != (APTR)-1L)
	{
		if((IntuitionBase=OpenLibrary("intuition.library",0)))
		{
			body.IText = (UBYTE *)msg;
			AutoRequest(NULL,&body,NULL,&ok,0,0,640,72);
			CloseLibrary(IntuitionBase);
		}
	}
}

/***************************************************************************/

