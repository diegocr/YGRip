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

/*:ts=4 */
#include <proto/exec.h>
#include "html.h"

#define strlen		__strlen
#define strncmp		__strncmp

static __inline unsigned long strlen(const char *string)
{ const char *s=string;
  do;while(*s++); return ~(string-s);
}
static __inline int strncmp(const char *s1,const char *s2,unsigned long n)
{ unsigned char *p1=(unsigned char *)s1,*p2=(unsigned char *)s2;
  unsigned long r=n,c;
  do;while(r=*p1++,c=*p2++,!(r-=c) && (char)c && --n);
  return r;
}


unsigned char * HTMLToText( unsigned char * htmlstring )
{
	register int nb = 0;
	register BOOL flag = TRUE;
	register unsigned char * buf = htmlstring;
	
	for( ; *(buf+nb) ; nb++ )
	{
		if(flag && *(buf+nb) == '<')
			flag = FALSE;
		else if(!flag && *(buf+nb) == '>')
			flag = TRUE;
		else if(flag)
		{
			BOOL copy = TRUE;
			
			if(*(buf+nb) == '&' && *(buf+nb+1) != ' ')
			{
				if(*(buf+nb+1) == '#')
				{
					register long bit = 0, count = 0;
					register unsigned char * ptr = &buf[nb+2];
					
					while(*ptr && (*ptr >= '0' && *ptr <= '9'))
					{
						bit += *ptr++ - '0';
						if(*ptr && (*ptr >= '0' && *ptr <= '9')) bit *= 10;
						count++;
					}
					
					// esto debe ser algo como "&#169;" (bit = 169)
					if(*ptr == ';' && bit < 256)
					{
						nb += ++count;
						*buf++ = (unsigned char) bit;
						copy = FALSE;
					}
				}
				else
				{
					register int c = -1;
					
					while(htmlcodes[++c] != NULL)
					{
						int hlen = strlen(htmlcodes[c]);
						
						if(!strncmp( buf+nb, htmlcodes[c], hlen))
						{
							*buf++ = asciicodes[c];
							nb += hlen -2;
							copy = FALSE;
							break;
						}
					}
				}
			}
			
			if( copy )
				*buf++ = *(buf+nb--);
		}
	}
	
	*buf = 0;
	
	return htmlstring;
}



#ifdef TEST
#include <proto/dos.h>

int main(int argc, char * argv[] )
{
	BPTR fd;
	
	if((fd = Open( argv[1], MODE_OLDFILE)))
	{
		char * buf;
		
		if((buf = AllocMem( 65536, MEMF_ANY)))
		{
			long read = Read( fd, buf, 65535 );
			
			Printf("Readed %ld bytes from %s file\n", read, argv[1] );
			
			PutStr("\nConverted HTMLToText follow:\n\n");
			PutStr( HTMLToText( buf ));
			
			FreeMem( buf, 65536 );
		}
		
		Close( fd );
	}
	
	return 0;
}

#endif

