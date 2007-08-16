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


#include <SDI_mui.h>
#include <proto/muimaster.h>
#include <proto/asl.h>
#include <MUI/BetterString_mcc.h>
#include <MUI/BetterBalance_mcc.h>

Object * BalanceObj( ULONG ObjectID )
{
	return BetterBalanceObject,
		MUIA_ObjectID, ObjectID,
	End;
}

Object * ButtonObj( STRPTR label, LONG FixWidth, STRPTR ShortHelp )
{
	Object *obj;
	
	if((obj = MUI_MakeObject(MUIO_Button, label )))
	{
		SetAttrs(obj,
			MUIA_CycleChain , 1,
			MUIA_FixWidth   , FixWidth,
			MUIA_ShortHelp  , (ULONG) ShortHelp,
		TAG_DONE);
	}
	
	return(obj);
}

Object * StringObj( STRPTR ShortHelp, ULONG ObjectID, ULONG String_MaxLen )
{
	return BetterStringObject,
		StringFrame,
		MUIA_String_MaxLen	, String_MaxLen,
		MUIA_String_AdvanceOnCR	, TRUE,
		MUIA_CycleChain		, TRUE,
		MUIA_ShortHelp		, (ULONG)ShortHelp,
		MUIA_ObjectID		, ObjectID,
	End;
}

Object * IntegerStringObj( STRPTR ShortHelp, ULONG ObjectID, ULONG String_MaxLen )
{
	Object * obj;
	
	if((obj = StringObj( ShortHelp, ObjectID, String_MaxLen )))
	{
		SetAttrs( obj,
			MUIA_String_Integer	, 0,
			MUIA_String_Accept	, (ULONG)"0123456789",
		TAG_DONE);
	}
	
	return obj;
}

Object * CheckMarkObj( STRPTR ShortHelp, ULONG ObjectID )
{
	Object *obj;
	
	if((obj = MUI_MakeObject(MUIO_Checkmark, NULL)))
	{
		SetAttrs(obj,
			MUIA_CycleChain	, TRUE,
			MUIA_ShortHelp	, (ULONG)ShortHelp,
			MUIA_ObjectID	, ObjectID,
		TAG_DONE);
	}
	
	return(obj);
}

Object * CycleObj( char **array, STRPTR ShortHelp, ULONG ObjectID )
{
	Object *obj;
	
	if((obj = MUI_MakeObject(MUIO_Cycle, NULL, array)))
	{
		SetAttrs(obj,
			MUIA_CycleChain	, TRUE,
			MUIA_ShortHelp	, (ULONG) ShortHelp,
			MUIA_ObjectID	, ObjectID,
		TAG_DONE);
	}
	
	return(obj);
}

Object * PopaslObj( STRPTR Contents, STRPTR ASLText, BOOL drawers, ULONG ObjectID)
{
	return PopaslObject,
		MUIA_Popstring_String, StringObject, StringFrame,
			MUIA_CycleChain			, TRUE,
			MUIA_String_AdvanceOnCR		, TRUE,
			MUIA_String_MaxLen		, 1024,
			MUIA_String_Contents		, Contents,
			MUIA_ObjectID			, ObjectID,
		End,
		MUIA_Popstring_Button, PopButton(drawers ? MUII_PopDrawer : MUII_PopFile),
		ASLFR_TitleText, ASLText,
		ASLFR_DrawersOnly, drawers,
	End;
}

Object * SliderObj( LONG min, LONG max, LONG Current, STRPTR ShortHelp, ULONG ObjectID)
{
	Object *obj;
	
	if((obj = MUI_MakeObject(MUIO_Slider, NULL, min, max)))
	{
		SetAttrs(obj,
			MUIA_CycleChain		, TRUE,
			MUIA_ShortHelp		, (ULONG) ShortHelp,
			MUIA_Slider_Level	, Current,
			MUIA_ObjectID		, ObjectID,
		TAG_DONE);
	}
	
	return(obj);
}

