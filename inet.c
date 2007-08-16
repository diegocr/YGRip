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

UBYTE __bsdsocketname[] = "bsdsocket.library";

/****************************************************************************/

struct DNSCache
{
	struct DNSCache * next;
	
	STRPTR hostname;
	ULONG expire;
	struct in_addr addr;
	int8_t zero_pad[8]; // erm...
};
STATIC struct DNSCache * dnsc = NULL;

ULONG DNSCacheEntry( STRPTR hostname, long * error, 
	struct ClassData * data, struct Library * SocketBase )
{
	time_t now = time(NULL);
	struct DNSCache * ptr;
	BOOL update = FALSE;
	struct hostent * hent;
	
	DBG("Resolving hostname \"%s\"...\n", hostname ); Delay(4);
	#warning remove delays...
	
	*error = ERR_NOERROR;
	for( ptr = dnsc ; ptr ; ptr = ptr->next )
	{
		if(!Stricmp( hostname, ptr->hostname ))
		{
			DBG(" +++ Found cached hostname \"%s\"...\n", hostname );
			
			if(ptr->expire > (ULONG)now)
				goto done;
			
			DBG("...which hostname has expired !\n");
			update = TRUE;
			break;
		}
	}
	
	if(!(hent = gethostbyname( hostname )))
	{
		DBG(" +++++++++++ cannot resolve hostname \"%s\" !!\n", hostname );
		*error = ERR_RESOLVER;
		return 0;
	}
	
	if( update == FALSE )
	{
		DBG(" +++ Caching hostname \"%s\"...\n", hostname );
		
		ptr = malloc(sizeof(struct DNSCache));
		
		if(ptr == NULL)
		{
			*error = ERR_NOMEM;
			return 0;
		}
		
		ptr->hostname = strdup( hostname );
		
		if(!(ptr->hostname && *ptr->hostname))
		{
			free( ptr );
			*error = ERR_NOMEM;
			return 0;
		}
	}
	
	// expire in two minutes...
	ptr->expire = (ULONG)now + 120;
	
	DBG_VALUE(ptr->expire); Delay(8);
	
#if 0
	// damn!, "LONG READ/WRITE" bug
	ptr->ip = (ULONG) ((struct in_addr *)(hent->h_addr))->s_addr;
#else
	DBG_ASSERT(sizeof(struct in_addr) == hent->h_length);
	
	memcpy( &ptr->addr, hent->h_addr, hent->h_length );
#endif
	
	if( update == FALSE )
	{
		ptr->next = dnsc;
		dnsc = ptr;
	}
	
done:
	DBG("Resolved hostname IP = 0x%08lx\n",__IPADDR(ptr->addr.s_addr)); Delay(4);
	
	return((ULONG)__IPADDR(ptr->addr.s_addr));
}

void FreeDNSCache(struct ClassData * data)
{
	struct DNSCache * ptr, * next;
	
	for( ptr = dnsc ; ptr ; ptr = next )
	{
		next = ptr->next;
		
		free( ptr->hostname );
		free( ptr );
	}
	
	dnsc = NULL;
}

/****************************************************************************/

ULONG Resolver(STRPTR hostname)
{
	struct Library * SocketBase = NULL;
	
	ULONG ip = (ULONG) -1; // <- bsdsocket cannot be opened
	
	DBG("Resolving \"%s\" hostname...\n", hostname);
	
	if((SocketBase = OpenLibrary(__bsdsocketname,4)))
	{
		struct hostent * hent;
		
		ip = 0; // <- bsdsocket opened, host not resolved
		
		if((hent = gethostbyname( hostname )))
		{
#if 0
			/* hostname resolved succesfuly */
			ip = ((struct in_addr *)(hent->h_addr))->s_addr;
#else
			struct in_addr addr;
			
			memcpy( &addr, hent->h_addr, hent->h_length );
			
			ip = (ULONG)__IPADDR(addr.s_addr);
#endif
		}
		
		CloseLibrary( SocketBase );
	}
	
	DBG("Resolved ULONG's ip: 0x%08lx\n", ip );
	
	return(ip);
}

/****************************************************************************/
