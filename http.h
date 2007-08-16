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


#define PROXY_BAN_TIMEOUT	600	/* 10 minutos */

#define HIBERNATION( timeout )					\
	do {							\
		http->hibernation = (timeout ? TRUE : FALSE);	\
		http->hiberTime   = timeout; /* seconds */	\
								\
	} while(0)

typedef struct
{
	long sockfd;
	
	UBYTE CurrentURL[4096]; // tooo enough (should be)
	STRPTR urlhost;
	STRPTR urlpath;
	STRPTR request;
	STRPTR referer;
	STRPTR location;
	
	String * cookie;
	String * rdata;
	
	long rcode;		/* http return code from server, 200, 302, etc */
	long advert_count;	/* number of advertesiments we received fetching a single page/msg */
	
	BOOL hibernation;	/* this gets TRUE when our IP address (or all proxys) gets banned */
	long hiberTime;		/* time to wait before retrying to fetch msgs in the hope of our ban disappareaded */
	
	struct YGRProxy * last_proxy_used;
	
} HTTP;


GLOBAL 
String *GetURL( struct ClassData * data, HTTP * http, STRPTR url, long *error,
	ULONG *bytes_received,ULONG *bytes_written, struct Library *SocketBase);

