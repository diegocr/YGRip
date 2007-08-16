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


GLOBAL int SetActiveGroup(struct ClassData * data, STRPTR YahooGroup );

GLOBAL int LoadIndex(struct ClassData * data);
GLOBAL int InsertIndex(struct ClassData * data);

GLOBAL void FreeIndex( struct ClassData * data, struct GroupFolder ** grp );
GLOBAL void FreeGroupFolder(struct ClassData * data, struct GroupFolder ** grp );
GLOBAL void ExpungeIndex( struct ClassData * data, struct GroupFolder ** grp );

GLOBAL int SaveIndexToDisk( struct ClassData * data, struct GroupFolder ** grp );
GLOBAL int MoveIndexToDisk( struct ClassData * data, struct GroupFolder ** grp );

GLOBAL BOOL SameGroupFolder( struct ClassData * data, BOOL lock );
//GLOBAL int InsertNewIndex( struct ClassData * data, STRPTR from, STRPTR subj, STRPTR date );
GLOBAL int SaveMessage( struct ClassData * data, STRPTR Message, ULONG msgnum );

GLOBAL BOOL IsNewGroup(struct ClassData * data, STRPTR YahooGroup );

GLOBAL void LeftListInsert( struct ClassData * data, STRPTR IndexFile );
GLOBAL BOOL LeftListSelect( struct ClassData * data, STRPTR YahooGroup);

GLOBAL int MakeMailFile(struct GroupFolder * folder, STRPTR outbuf, LONG outlen, ULONG MsgNum, BOOL makeDir );

GLOBAL VOID ClearReaderMail( struct ClassData * data );
GLOBAL VOID SetReaderMail( struct ClassData * data, STRPTR header, STRPTR Mail );

