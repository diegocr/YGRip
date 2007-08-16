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


#include <exec/types.h>
#include <libraries/mui.h>
#include "logo.h"

GLOBAL Object *MUI_NewObject( STRPTR, Tag, ...);

Object * YahooGroupsLogo ( VOID )
{
	Object * obj;
	
	obj = BodychunkObject,
		MUIA_Group_Spacing         , 0,
		MUIA_FixWidth              , LOGO_WIDTH ,
		MUIA_FixHeight             , LOGO_HEIGHT,
		MUIA_Bitmap_Width          , LOGO_WIDTH ,
		MUIA_Bitmap_Height         , LOGO_HEIGHT,
		MUIA_Bodychunk_Depth       , LOGO_DEPTH ,
		MUIA_Bodychunk_Body        , (UBYTE *) logo_body,
		MUIA_Bodychunk_Compression , LOGO_COMPRESSION,
		MUIA_Bodychunk_Masking     , LOGO_MASKING,
		MUIA_Bitmap_SourceColors   , (ULONG *) logo_colors,
		MUIA_Bitmap_Transparent    , 0,
		MUIA_Bitmap_Precision      , PRECISION_EXACT,
	End;
	
	return obj;
}
