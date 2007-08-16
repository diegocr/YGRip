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


GLOBAL Object * BalanceObj( ULONG ObjectID );
GLOBAL Object * ButtonObj( STRPTR label, LONG FixWidth, STRPTR ShortHelp );
GLOBAL Object * StringObj( STRPTR ShortHelp, ULONG ObjectID, ULONG String_MaxLen );
GLOBAL Object * IntegerStringObj( STRPTR ShortHelp, ULONG ObjectID, ULONG String_MaxLen );
GLOBAL Object * CheckMarkObj( STRPTR ShortHelp, ULONG ObjectID );
GLOBAL Object * CycleObj( char **array, STRPTR ShortHelp, ULONG ObjectID );
GLOBAL Object * PopaslObj( STRPTR Contents, STRPTR ASLText, BOOL drawers, ULONG ObjectID);
GLOBAL Object * SliderObj( LONG min, LONG max, LONG Current, STRPTR ShortHelp, ULONG ObjectID);

