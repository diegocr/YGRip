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


#include <proto/exec.h>
#include "util.h"

#define INITIAL_BUFSIZE		65536
#define ADDTIONAL_BUFSIZE	768

#ifndef VA_COPY
# define VA_COPY	__va_copy
#endif

#define MEMFLAGS	MEMF_PUBLIC

/****************************************************************************/

String * string_new( long size )
{
	String * str;
	
	if(size <= 0)
		size = INITIAL_BUFSIZE;
	
	if((str = AllocMem(sizeof(*str), MEMFLAGS)))
	{
		if((str->str = AllocMem(str->size = size, MEMFLAGS)))
		{
			str->str[str->len = 0] = 0;
		}
		else {
			FreeMem( str, sizeof(*str));
		}
	}
	
	return str;
}

/****************************************************************************/

void string_free( String * s )
{
	if(s != NULL)
	{
		if(s->str != NULL)
			FreeMem( s->str, s->size );
		
		FreeMem( s, sizeof(*s));
	}
}

/****************************************************************************/

BOOL string_resize(String * s, StringLong lentocopy)
{
	if(s->len + lentocopy >= s->size)
	{
		StringChar new_str;
		StringLong new_size;
		
		new_size = s->size + lentocopy + ADDTIONAL_BUFSIZE;
		
		if(!(new_str = AllocMem( new_size, MEMFLAGS )))
			return FALSE;
		
		memcpy( new_str, s->str, s->len );
		new_str[s->len] = 0;
		
		FreeMem( s->str, s->size );
		
		s->str  = new_str;
		s->len  = s->len;
		s->size = new_size;
	}
	
	return TRUE;
}

/****************************************************************************/

int string_append(String *s, StringChar str, StringLong len)
{
	if(((long)len) <= 0)
		len = strlen(str);
	
	if(((long)len) <= 0)
		return -1;
	
	if(!string_resize( s, len ))
		return -1;
	
	memcpy (s->str + s->len, str, len);
	s->len += len;
	
	s->str[s->len] = 0;
	
	return len;
}

/****************************************************************************/

int string_appendvf(String *sobj, const char *fmt, va_list args)
{
	va_list args_cpy;
	int written = 0;
	ULONG fmt_len;

	if (!sobj)
		return 0;
	
	fmt_len = strlen(fmt);
	if(fmt_len < 3)
		fmt_len = 3;
	
	/* loop until we've got a buffer big enough to write this damned
	 * string, or until we can no longer make the buffer any bigger */
	do {
		if((sobj->size - sobj->len) > fmt_len)
		{
			long maxlen = ((sobj->size - sobj->len) - 2);
			
			/*
			 * We have to make a copy of the va_list because we may pass this
			 * point multiple times. Note that simply calling va_start again is
			 * not a good idea since the va_start/va_end should be in the same
			 * stack frame because of some obscure implementations.
			 */
			VA_COPY (args_cpy, args);
			written = VSNPrintf( sobj->str + sobj->len, maxlen, fmt, args_cpy );
			va_end (args_cpy);
			
			/*
			 * Some implementations use -1 to indicate an inability to write
			 * the complete buffer, some return a value equal to the number
			 * of bytes needed (excluding NUL) to perform the operation.
			 */
			if(written > -1 && written < maxlen)
				break;
		}
		
		/* if we got here, we don't have enough memory in the underlying buffer
		 * to fit this fmt buffer */
		if(string_resize (sobj, sobj->size * 2) == FALSE)
			return -1;
		
	} while(1);
	
	/* calculate the new string length */
	sobj->len += written;
//	sobj->str [sobj->len] = 0;
	
	/* return the number of bytes added by this call, not the total */
	return written;
}

/****************************************************************************/

int string_appendf(String *sobj, const char *fmt, ... )
{
	va_list args;
	int rc;
	
	va_start (args, fmt);
	rc = string_appendvf (sobj, fmt, args);
	va_end (args);
	
	return(rc);
}

/****************************************************************************/

