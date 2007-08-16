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


#include "inet.h"
#include "YGRGroup_mcc_priv.h"
#include "util.h"
#include "debug.h"
#include "stringsep.h"
#include "http.h"
#include "subtask.h"

/****************************************************************************/

static String * realloc_cookie(String * cookie)
{
	long ulen = 256;
	String * str;
	
	if(cookie)
	{
		if(cookie->len > ulen)
			ulen = cookie->len;
		
		string_free(cookie);
	}
	
	DBG("Restoring cookie with length %ld bytes...\n", ulen );
	
	str = string_new( ulen );
	
	DBG_ASSERT(str != NULL);
	
	return str;
}

/****************************************************************************/

static BOOL restore_cookie(struct ClassData * data, HTTP * http)
{
	if(!(http->cookie = realloc_cookie(http->cookie)))
		return FALSE;
	
	if(string_append( http->cookie, data->yahoo.cookie, -1) < 0)
		return FALSE;
	
	DBG(" ***** COOKIE RESTORED: \"%s\"...\n", http->cookie->str );
	
	return TRUE;
}

/****************************************************************************/

static void __remove_cookie_chunk(
	struct ClassData * data, HTTP * http, STRPTR chunk, long chunklen )
{
	if(!(http->cookie))
		return;
	
	if(strstr( http->cookie->str, chunk ))
	{
		char *cookiestr;
		String * nc;
		
		if(!(cookiestr = strndup(http->cookie->str,http->cookie->len)))
		{
			DBG(" +++++ -- strndup FAILED\a\n");
			return;
		}
		
		if((nc = realloc_cookie(http->cookie)))
		{
			char *cstr = cookiestr, *key;
			
			http->cookie = nc;
			DBG("Removing cookie chunk(s) \"%s\"...\n", chunk );
			
			while((key = _strsep( &cstr, " ")))
			{
				if(Strnicmp( key, chunk, chunklen ))
					string_append(http->cookie, key, -1);
			}
		}
		free(cookiestr);
	}
}

/****************************************************************************/

static char *read_http_headers(struct ClassData * data, HTTP * http )
{
	char *__sdata, *sdata, *left, *location = NULL;
	long headlen;
	
//	if(!APPEND_COOKIE && (http->rcode != 302))	return NULL;
	
	headlen = FindPos( http->rdata->str, "\r\n\r\n");
	
	DBG("Reading HTTP Headers, length = %ld bytes\n", headlen );
	if( headlen == 0 )
		return NULL;
	
	sdata = __sdata = strndup(http->rdata->str, headlen);
	
	while((left = _strsep( &sdata, "\r\n")))
	{
		char *key;
		
		if(!(key = _strsep( &left, ": "))) break;
		
		if(!Stricmp( key, "Set-Cookie"))
		{
			char *cookie = left, *ckey;
			
			DBG("Got Set-Cookie field...\n");
			
			if(http->advert_count)
			{ // THATS OK ?
				__remove_cookie_chunk(data, http,"interrupt",9);
			}
			
			while((ckey = _strsep( &cookie, " ")))
			{
				if(!Strnicmp( ckey, "path", 4) || 
				   !Strnicmp( ckey, "domain", 6)) continue;
				
				if(!Strnicmp( ckey, "tz=y=", 5))
					__remove_cookie_chunk(data, http,"tz=y=", 5);
				
				DBG("Adding Cookie \"%s\"...\n", ckey );
				
				string_appendf( http->cookie, " %s", ckey );
			}
			
			if(http->cookie->str[http->cookie->len-1] != ';')
				string_appendf( http->cookie, ";", 1 );
		}
		else if((http->rcode == 302) && !Stricmp( key, "location"))
		{
			location = strdup(left);
		}
	}
	free(__sdata);
	
	DBG("ENDED, new location = \"%s\"\n\n", location );
	
	return location;
}

/****************************************************************************/

static long Connect(unsigned long ip, long port, struct Library * SocketBase)
{
	long sockfd;
	struct sockaddr_in server;
	
	if((((long)ip) <= 0)
		|| !((port > 0) && (port < 65535)))
			return -1;
	
	if ((sockfd = socket (AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0)
		return sockfd;
	
	memset (&server, 0, sizeof (struct sockaddr_in));
	server.sin_family      = AF_INET;
	server.sin_port        = port;//htons (port);
	server.sin_addr.s_addr = __IPADDR(ip);
	
	if(0==connect (sockfd, (struct sockaddr *)&server, sizeof (struct sockaddr)))
		return sockfd; // connection succeed;
	
	CloseSocket( sockfd );
	return -1;
}

/****************************************************************************/

#ifdef SYS_TIME_H /* AMITCP "compatible_timeval" */
# define DECLARE_TIMEVAL_TIMEOUT( seconds, micros )	\
	struct timeval timeout = {{seconds},{micros}}
#else
# if defined(_SYS_TIME_H_) || defined(DEVICES_TIMER_H)
#  define DECLARE_TIMEVAL_TIMEOUT( seconds, micros )	\
	struct timeval timeout = { seconds, micros }
# else
#  error struct timeval isnt know
# endif
#endif

STATIC LONG Receive(long sock, void *buf, long len, long flags, ULONG TimeOut, struct Library * SocketBase)
{
	fd_set rdfs;
	LONG sel, result = 0;
	
	DECLARE_TIMEVAL_TIMEOUT(TimeOut,10);
	
	FD_ZERO(&rdfs);
	FD_SET(sock, &rdfs);
	
	if((sel = WaitSelect( sock+1, &rdfs, NULL, NULL, &timeout, NULL)))
	{
		result = recv( sock, buf, len , flags );
		
		#ifdef DEBUG
		if(result < 0) {
			DBG(" +++ SOCKET ERROR %ld\n\a", Errno());
		}
		#endif
	}
	
	return( result );
}

/****************************************************************************/

String *GetURL( struct ClassData * data, HTTP * http, STRPTR url, long *error,
	ULONG *bytes_received,ULONG *bytes_written, struct Library *SocketBase)
{
	STRPTR host, path, furl = url;
	int maxlen, ulen = strlen(url);
	struct YGRProxy * proxy;
	STATIC UBYTE sockbuf[SUBTASK_SOCKBUFLEN];
	
	int MAX_HOSTLEN	= 256;
	int MAX_PATHLEN	= MAX(ulen,512);
	
	HIBERNATION(FALSE);
	
	DBG(" +++ GETTING URL \"%s\"\n", url);
	
	if(!Strnicmp( url, "http://", 7))
		url += 7;
	
	free(http->urlhost);
	free(http->urlpath);
	host = http->urlhost = malloc(MAX_HOSTLEN);
	path = http->urlpath = malloc(MAX_PATHLEN);
	
	if(!(host && path)) {
		(*error) = ERR_NOMEM;
		return NULL;
	}
	
	maxlen = MAX_HOSTLEN;
	do {
		*host++ = *url++;
		
	} while( *url && *url != '/' && *url != ':' && --maxlen > 0);
	
	if(maxlen <= 0) {
		(*error) = ERR_OVERFLOW;
		return NULL;
	}
	
	// TODO if(*url == ':')
	
	maxlen = MAX_PATHLEN;
	do {
		*path++ = *url++;
		
	} while( *url && --maxlen > 0);
	
	if(maxlen <= 0) {
		(*error) = ERR_OVERFLOW;
		return NULL;
	}
	
	*path = 0;
	*host = 0;
	
	http->sockfd = -1;
	DBG(" ++++ Connecting to \"%s\"...\n", http->urlhost );
	
	if(http->last_proxy_used == NULL)
		http->last_proxy_used = data->proxy;
	
	for( proxy = http->last_proxy_used ; proxy ; proxy = proxy->next )
	{
		DBG(" ++++ Proxy connection to 0x%08lx:%lu...", proxy->ip,(long) proxy->port );
		
		if(proxy->banned)
		{
			time_t now = time(NULL);
			
			/* if the ban was 30 minutes ago try to connect */
			if((now - proxy->tstamp) < PROXY_BAN_TIMEOUT)
			{
				DBG("Proxy %08lx is banned...\n", proxy->ip );
				continue;
			}
			
			proxy->banned	= FALSE;
			proxy->tstamp	= now;
		}
		
		http->sockfd = Connect( proxy->ip, proxy->port, SocketBase );
		
		if(http->sockfd != -1)
		{
			DBG(" ++++ connection to proxy succed\n");
			break;
		}
		else {
			DBG(" ---- Proxy connection ERROR\n");
		}
	}
	
	http->last_proxy_used = proxy;
	
	if(http->sockfd == -1)
	{
		ULONG hostip;
		
		hostip = DNSCacheEntry(http->urlhost,error,data,SocketBase);
		
		if( ! hostip )
		{
			DBG_ASSERT((*error) != ERR_NOERROR);
			
			HIBERNATION(40);
			return NULL;
		}
		
		http->sockfd = Connect( hostip, 80, SocketBase );
	}
	
	DBG_ASSERT(http->sockfd < 0x9000); // LAME sanity check...
	
	if((http->sockfd == -1) || (http->sockfd > 0x9000))
	{
		DBG("*** Direct connection failed !?\n");
		
		HIBERNATION(20);
		(*error) = ERR_CONNECT;
		
		return NULL;
	}
	
	DBG_VALUE(http->sockfd);
	DBG("Sending request...\n");
	
	free(http->request);
	maxlen = ((strlen(furl)*2)+strlen(http->urlhost)+((http->cookie != NULL) ? http->cookie->len:0)+1024);
	
	DBG("Allocated buffer for request is %ld bytes\n", maxlen );
	
	if(!(http->request = malloc( maxlen+2 )))
	{
		(*error) = ERR_NOMEM;
		CloseSocket(http->sockfd);
		http->sockfd = -1;
		return NULL;
	}
	
	maxlen = SNPrintf( http->request, maxlen,
		"GET %s HTTP/1.0\r\n"
		"User-Agent: Mozilla/5.0; (compatible; YGRMUI 1.0 (c)2006 Diego Casorran; AmigaOS)\r\n"
		"Host: %s\r\n"
		"Referer: %s\r\n"
		"Cookie: %s\r\n\r\n", 
		((http->last_proxy_used != NULL) ? furl : http->urlpath),
		http->urlhost,
		((http->referer != NULL) ? (char *)http->referer : "http://yahoo.com/"),
		((http->cookie != NULL) ? (char *)http->cookie->str : "dummy") );
	
	DBG("Formatted request length is %ld (strlen=%ld) bytes\n", maxlen, strlen(http->request));
	DBG_STRING(http->request);
	
	if(send( http->sockfd, http->request, maxlen,0) != maxlen)
	{
		DBG(" ++++++ SEND FAILED ++++++++\n");
		(*error) = ERR_BSD;
		CloseSocket(http->sockfd);
		http->sockfd = -1;
		return NULL;
	}
	
	(*bytes_written) += maxlen;
	
	if(http->rdata)
		string_free( http->rdata );
	
	if(!(http->rdata = string_new( sizeof(sockbuf)+64 )))
	{
		DBG("string_new() FAILED !!!!!!\n");
		(*error) = ERR_NOMEM;
		CloseSocket(http->sockfd);
		http->sockfd = -1;
		return NULL;
	}
	
	free(http->referer);
	http->referer = strdup( furl );
	
	#ifdef DEBUG
	DBG("Receiving page...\n");
//	Delay(20);
//	#warning remove delays
	#endif
	
	while((maxlen = Receive( http->sockfd, sockbuf, sizeof(sockbuf)-1, MSG_WAITALL, data->recvTimeout, SocketBase))>0)
	{
		DBG("Received %ld bytes for the %ld bytes buffer length\n", maxlen, sizeof(sockbuf)-1);
		
		if(string_append( http->rdata, sockbuf, maxlen ) < 0)
		{
			DBG("string_append() FAILED !!!, out of memory !?\n");
			(*error) = ERR_NOMEM;
			CloseSocket(http->sockfd);
			http->sockfd = -1;
			return NULL;
		}
	}
	
	(*bytes_received) += http->rdata->len;
	
	DBG("Got %ld bytes from server (so strlen = %ld)\n", http->rdata->len, strlen(http->rdata->str));
	hexdump( http->rdata->str, 0, 128 );
	WriteDebugFile( "sockbuf", http->rdata->str, http->rdata->len );
	
	CloseSocket(http->sockfd);
	http->sockfd = -1;
	
	if(!(http->rdata->str && *http->rdata->str))
	{
		DBG(" **** NO DATA RECEIVED AT ALL **** errno: %s\n\a", ErrorString(ERR_BSD));
		HIBERNATION(10);
		(*error) = ERR_TIMEOUT;
		return NULL;
	}
	
	if(StrToLong( &http->rdata->str[9], &http->rcode ) == -1)
	{
		DBG(" WTF!!!!!!!!! +++++++ StrToLong FAILED ++++++\n");
		http->rcode = 503;
	}
	
	DBG("\nServer returned code %ld\n", http->rcode);
	
	
	free(http->location);
	
	DBG("Reading headers...\n");
	
	if((http->location = read_http_headers( data, http )))
	{
		DBG(" ++++++++++ Found 302, redirecting....\n\n");
		return GetURL(data,http,http->location,error,bytes_received,bytes_written,SocketBase);
	}
	
	DBG("Checking results...\n");
	//DBG_ASSERT(strlen(http->rdata->str) == http->rdata->len);
	
	if(http->rcode == 400)
	{
		(*error) = ERR_INTERNAL;
		InfoText("Server returned error #%ld!", http->rcode );
		DisplayBeep(NULL);
		Delay( 100 );
		return NULL;
	}
	else
	if(strstr( http->rdata->str, "Yahoo! Groups is an advertising supported service."))
	{
		if(++http->advert_count < 4)
		{
			DBG(" ++++++++++ Got ADVERTISEMENT page, re-fetching....\n");
			
			return GetURL(data,http,http->CurrentURL,error,bytes_received,bytes_written,SocketBase);
		}
		else {
			DBG(" ++++++++++ Got ADVERTISEMENT page, hey!, got TOO MANY, reinitiliazing...\n");
		}
	}
	else
	if(strstr( http->rdata->str, "Your browser is not accepting our cookies"))
	{
		(*error) = ERR_INTERNAL;
		InfoText("Your browser is not accepting our cookies");
		DisplayBeep(NULL);
		Delay( 100 );
		return NULL;
	}
	else
	if(strstr( http->rdata->str, "/config/login?."))
	{
		DBG("\nA request for authentification was received!, please insert/change your Cookie!\n\n\a");
		(*error) = ERR_COOKIE;
		return NULL;
	}
	else
	if((http->rcode == 999) || strstr( http->rdata->str, "This page is currently unavailable"))
	{
		char *str = (char *) http->rdata->str, *_str, *current_host;
		
		DBG("uh-oh, entering hibernation mode?....:\n");
		
		while(*str && !(*(str+1) == 'U' && *str == '\n')) *str++ = '\0';
		
		_str = str;  while(*_str && *_str != '<') _str++; *_str = '\0';
		
		DBG(" >>>>>>>>>>>>>>>>>>>>>>>>>>>> %s\n\n", str);
		InfoText( str );
		DisplayBeep(NULL);
		
		free(http->referer);
		http->referer = NULL;
		
		if( proxy )
		{
			proxy->banned = TRUE;
			proxy->tstamp = time(NULL);
			
			HIBERNATION(3);
		}
		else
		{
			/* MY own IP gets banned!... */
			
			HIBERNATION(190); /* seconds */
		}
#if 0		
		deb("According with the above error message from Yahoo! website,\n"
		    "heither: really the site can't be accessed OR the IP/Host\n"
		    "address \"%s\" has been BANNED!...\n\n", current_host);
#endif		
		return NULL;
	}
	else if(http->rcode == 200)	return http->rdata;
	
	
	DBG("undefined result (%ld), reinitiliazing...\n", http->rcode);
	
	if(!restore_cookie(data,http))
	{
		DBG(" restore_cookie() FAILED !!!! \n\a");
		(*error) = ERR_NOMEM;
		return NULL;
	}
	
	//ACCEPT_COOKIE;
	#warning que pasa con ACCEPT_COOKIE; ??
	http->advert_count = 0;
	
	Delay( 3 * TICKS_PER_SECOND );
	
	return GetURL(data,http,http->CurrentURL,error,bytes_received,bytes_written,SocketBase);
}


/****************************************************************************/
/****************************************************************************/


