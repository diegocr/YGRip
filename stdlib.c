/*
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2006 by Olaf Barthel <olsen (at) sourcery.han.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the name of Olaf Barthel nor the names of contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <proto/exec.h>
#include <proto/dos.h>
#include <SDI_compiler.h>
#include "util.h"
#include "debug.h"

/****************************************************************************/

/* Address is neither aligned to a word or long word boundary. */
#define IS_UNALIGNED(a) ((((unsigned long)(a)) & 1) != 0)

/* Address is aligned to a word boundary, but not to a long
   word boundary. */
#define IS_SHORT_ALIGNED(a) ((((unsigned long)(a)) & 3) == 2)

/* Address is aligned to a long word boundary. For an 68030 and beyond the
   alignment does not matter. */
#if defined(M68020) || defined(_M68020) || defined(mc68020) || defined(__mc68020)
#define IS_LONG_ALIGNED(a) (1)
#else
#define IS_LONG_ALIGNED(a) ((((unsigned long)(a)) & 3) == 0)
#endif /* M68020 */

/****************************************************************************/

struct RawDoFmtStream {
	
	STRPTR Buffer;
	LONG Size;
};

static void RawDoFmtChar( REG(d0,UBYTE c), REG(a3,struct RawDoFmtStream *s))
{
	if(s->Size > 0)
	{
		*(s->Buffer)++ = c;
		 (s->Size)--;
		
		if(s->Size == 1)
		{
			*(s->Buffer)	= '\0';
			  s->Size	= 0;
		}
	}
}

LONG VSNPrintf(STRPTR outbuf, LONG size, CONST_STRPTR fmt, _BSD_VA_LIST_ args)
{
	long rc = 0;
	
	if((size > (long)sizeof(char)) && (outbuf != NULL))
	{
		struct RawDoFmtStream s;
		
		s.Buffer = outbuf;
		s.Size	 = size;
		
		RawDoFmt( fmt, (APTR)args, (void (*)())RawDoFmtChar, (APTR)&s);
		
		if((rc = ( size - s.Size )) != size)
			--rc;
	}
	
	return(rc);
}

LONG SNPrintf( STRPTR outbuf, LONG size, CONST_STRPTR fmt, ... )
{
	va_list args;
	long rc;
	
	va_start (args, fmt);
	rc = VSNPrintf( outbuf, size, fmt, args );
	va_end(args);
	
	return(rc);
}

/****************************************************************************/

INLINE VOID __memcpy(unsigned char * to,unsigned char * from,size_t len)
{
	/* The setup below is intended to speed up copying larger
	 * memory blocks. This can be very elaborate and should not be
	 * done unless a payoff can be expected.
	 */
	if(len > 4 * sizeof(long))
	{
		/* Try to align both source and destination to an even address. */
		if(IS_UNALIGNED(to) && IS_UNALIGNED(from))
		{
			(*to++) = (*from++);
			len--;
		}

		/* Try to align both source and destination to addresses which are
		 * multiples of four.
		 */
		if(len >= sizeof(short) && IS_SHORT_ALIGNED(to) && IS_SHORT_ALIGNED(from))
		{
			(*to++) = (*from++);
			(*to++) = (*from++);

			len -= sizeof(short);
		}

		/* If both source and destination are aligned to addresses which are
		 * multiples of four and there is still enough data left to be copied,
		 * try to move it in larger chunks.
		 */
		if(len >= sizeof(long) && IS_LONG_ALIGNED(to) && IS_LONG_ALIGNED(from))
		{
			unsigned long * _to		= (unsigned long *)to;
			unsigned long * _from	= (unsigned long *)from;

			/* An unrolled transfer loop, which shifts 32 bytes per iteration. */
			while(len >= 8 * sizeof(long))
			{
				/* The following should translate into load/store
				   opcodes which encode the access offsets (0..7)
				   into the respective displacement values. This
				   should help the PowerPC by avoiding pipeline
				   stalls (removing the postincrement on the address
				   will do that) but has no noticeable impact on the
				   68k platform (I checked). */

				_to[0] = _from[0];
				_to[1] = _from[1];
				_to[2] = _from[2];
				_to[3] = _from[3];
				_to[4] = _from[4];
				_to[5] = _from[5];
				_to[6] = _from[6];
				_to[7] = _from[7];

				_to		+= 8;
				_from	+= 8;

				/*
				(*_to++) = (*_from++);
				(*_to++) = (*_from++);
				(*_to++) = (*_from++);
				(*_to++) = (*_from++);
				(*_to++) = (*_from++);
				(*_to++) = (*_from++);
				(*_to++) = (*_from++);
				(*_to++) = (*_from++);
				*/

				len -= 8 * sizeof(long);
			}

			/* Try to mop up any small amounts of data still in need of
			 * copying...
			 */
			while(len >= sizeof(long))
			{
				(*_to++) = (*_from++);

				len -= sizeof(long);
			}		

			to		= (char *)_to;
			from	= (char *)_from;
		}
	}

	/* If there's anything left, copy the rest. */
	while(len-- > 0)
		(*to++) = (*from++);
}

/****************************************************************************/
/* This is ugly: GCC 2.95.x assumes that 'unsigned long' is used in the built-in
   memcmp/memcpy/memset functions instead of 'size_t'. This can produce warnings
   where none are necessary. */
#if defined(__GNUC__) && (__GNUC__ < 3)
void *
memcpy(void *dst, const void *src, unsigned long len)
#else
void *
memcpy(void *dst, const void *src, size_t len)
#endif /* __GNUC__ && __GNUC__ < 3 */
{
	void * result = dst;

	if(!( (len == 0) || (dst != NULL && src != NULL && (int)len > 0) )) {
	//	errno = EFAULT;
		goto out;
	}
	
	if(len > 0 && dst != src)
	{
		char * to = dst;
		const char * from = src;

		/* The two memory regions may not overlap. */
		if(!((to) >= (from)+len || (from) >= (to  )+len))
		{
			DBG(" ********* FAILED ASSERTION\n");
			goto out;
		}

		#if 0
		{
			while(len-- > 0)
				(*to++) = (*from++);
		}
		#else
		{
			__memcpy((unsigned char *)to,(unsigned char *)from,len);
		}
		#endif
	}
	
out:
	return(result);
}

/****************************************************************************/

INLINE VOID __memmove(unsigned char * to,unsigned char * from,size_t len)
{
	if(from < to && to < from + len)
	{
		to		+= len;
		from	+= len;

		/* The setup below is intended to speed up copying larger
		 * memory blocks. This can be very elaborate and should not be
		 * done unless a payoff can be expected.
		 */
		if(len > 4 * sizeof(long))
		{
			size_t distance;

			/* Try to align both source and destination to an even address. */
			if(IS_UNALIGNED(to) && IS_UNALIGNED(from))
			{
				(*--to) = (*--from);
				len--;
			}

			/* Try to align both source and destination to addresses which are
			 * multiples of four.
			 */
			if(len >= sizeof(short) && IS_SHORT_ALIGNED(to) && IS_SHORT_ALIGNED(from))
			{
				(*--to) = (*--from);
				(*--to) = (*--from);

				len -= sizeof(short);
			}

			/* Check the distance between source and destination. If it's shorter
			 * than a long word, don't dive into the copying routine below since
			 * the overlapping copying may clobber data.
			 */
			distance = (size_t)(to - from);

			/* If both source and destination are aligned to addresses which are
			 * multiples of four and there is still enough data left to be copied,
			 * try to move it in larger chunks.
			 */
			if(distance >= sizeof(long) && len >= sizeof(long) && IS_LONG_ALIGNED(to) && IS_LONG_ALIGNED(from))
			{
				unsigned long * _to		= (unsigned long *)to;
				unsigned long * _from	= (unsigned long *)from;

				/* An unrolled transfer loop, which shifts 32 bytes per iteration. */
				while(len >= 8 * sizeof(long))
				{
					/* The following should translate into load/store
					   opcodes which encode the access offsets (-1..-8)
					   into the respective displacement values. This
					   should help the PowerPC by avoiding pipeline
					   stalls (removing the predecrement on the address
					   will do that) but has no noticeable impact on the
					   68k platform (I checked). */

					_to[-1] = _from[-1];
					_to[-2] = _from[-2];
					_to[-3] = _from[-3];
					_to[-4] = _from[-4];
					_to[-5] = _from[-5];
					_to[-6] = _from[-6];
					_to[-7] = _from[-7];
					_to[-8] = _from[-8];

					_to		-= 8;
					_from	-= 8;

					/*
					(*--_to) = (*--_from);
					(*--_to) = (*--_from);
					(*--_to) = (*--_from);
					(*--_to) = (*--_from);
					(*--_to) = (*--_from);
					(*--_to) = (*--_from);
					(*--_to) = (*--_from);
					(*--_to) = (*--_from);
					*/

					len -= 8 * sizeof(long);
				}

				/* Try to mop up any small amounts of data still in need of
				 * copying...
				 */
				while(len >= sizeof(long))
				{
					(*--_to) = (*--_from);

					len -= sizeof(long);
				}		

				to		= (char *)_to;
				from	= (char *)_from;
			}
		}

		/* If there's anything left, copy the rest. */
		while(len-- > 0)
			(*--to) = (*--from);
	}
	else
	{
		/* The setup below is intended to speed up copying larger
		 * memory blocks. This can be very elaborate and should not be
		 * done unless a payoff can be expected.
		 */
		if(len > 4 * sizeof(long))
		{
			size_t distance;

			/* Try to align both source and destination to an even address. */
			if(IS_UNALIGNED(to) && IS_UNALIGNED(from))
			{
				(*to++) = (*from++);
				len--;
			}

			/* Try to align both source and destination to addresses which are
			 * multiples of four.
			 */
			if(len >= sizeof(short) && IS_SHORT_ALIGNED(to) && IS_SHORT_ALIGNED(from))
			{
				(*to++) = (*from++);
				(*to++) = (*from++);

				len -= sizeof(short);
			}

			/* Check the distance between source and destination. If it's shorter
			 * than a long word, don't dive into the copying routine below since
			 * the overlapping copying may clobber data.
			 */
			if(to >= from)
				distance = (size_t)(to - from);
			else
				distance = (size_t)(from - to);

			/* If both source and destination are aligned to addresses which are
			 * multiples of four and there is still enough data left to be copied,
			 * try to move it in larger chunks.
			 */
			if(distance >= sizeof(long) && len >= sizeof(long) && IS_LONG_ALIGNED(to) && IS_LONG_ALIGNED(from))
			{
				unsigned long * _to		= (unsigned long *)to;
				unsigned long * _from	= (unsigned long *)from;

				/* An unrolled transfer loop, which shifts 32 bytes per iteration. */
				while(len >= 8 * sizeof(long))
				{
					/* The following should translate into load/store
					   opcodes which encode the access offsets (0..7)
					   into the respective displacement values. This
					   should help the PowerPC by avoiding pipeline
					   stalls (removing the postincrement on the address
					   will do that) but has no noticeable impact on the
					   68k platform (I checked). */

					_to[0] = _from[0];
					_to[1] = _from[1];
					_to[2] = _from[2];
					_to[3] = _from[3];
					_to[4] = _from[4];
					_to[5] = _from[5];
					_to[6] = _from[6];
					_to[7] = _from[7];

					_to		+= 8;
					_from	+= 8;

					/*
					(*_to++) = (*_from++);
					(*_to++) = (*_from++);
					(*_to++) = (*_from++);
					(*_to++) = (*_from++);
					(*_to++) = (*_from++);
					(*_to++) = (*_from++);
					(*_to++) = (*_from++);
					(*_to++) = (*_from++);
					*/

					len -= 8 * sizeof(long);
				}

				/* Try to mop up any small amounts of data still in need of
				 * copying...
				 */
				while(len >= sizeof(long))
				{
					(*_to++) = (*_from++);

					len -= sizeof(long);
				}		

				to		= (char *)_to;
				from	= (char *)_from;
			}
		}

		/* If there's anything left, copy the rest. */
		while(len-- > 0)
			(*to++) = (*from++);
	}
}

/****************************************************************************/

void * memmove(void *dest, const void * src, size_t len)
{
	void * result = dest;

	if(!( (len == 0) || (dest != NULL && src != NULL && (int)len > 0) )) {
	//	errno = EFAULT;
		goto out;
	}

	if(len > 0 && dest != src)
	{
		char * to = dest;
		const char * from = src;

		#if 0
		{
			if(from < to && to < from + len)
			{
				to		+= len;
				from	+= len;

				while(len-- > 0)
					(*--to) = (*--from);
			}
			else
			{
				while(len-- > 0)
					(*to++) = (*from++);
			}
		}
		#else
		{
			__memmove((unsigned char *)to,(unsigned char *)from,len);
		}
		#endif
	}

 out:

	return(result);
}

/****************************************************************************/

INLINE VOID __memset(unsigned char * to,unsigned char value,size_t len)
{
	/* The setup below is intended to speed up changing larger
	 * memory blocks. This can be very elaborate and should not be
	 * done unless a payoff can be expected.
	 */
	if(len > 4 * sizeof(long))
	{
		if(IS_UNALIGNED(to))
		{
			(*to++) = value;
			len--;
		}

		if(len >= sizeof(short) && IS_SHORT_ALIGNED(to))
		{
			(*to++) = value;
			(*to++) = value;

			len -= sizeof(short);
		}

		if(len >= sizeof(long) && IS_LONG_ALIGNED(to))
		{
			unsigned long *	_to		= (unsigned long *)to;
			unsigned long	_value	= value;

			_value |= (_value <<  8);
			_value |= (_value << 16);

			while(len >= 8 * sizeof(long))
			{
				/* The following should translate into load/store
				   opcodes which encode the access offsets (0..7)
				   into the respective displacement values. This
				   should help the PowerPC by avoiding pipeline
				   stalls (removing the postincrement on the address
				   will do that) but has no noticeable impact on the
				   68k platform (I checked). */

				_to[0] = _value;
				_to[1] = _value;
				_to[2] = _value;
				_to[3] = _value;
				_to[4] = _value;
				_to[5] = _value;
				_to[6] = _value;
				_to[7] = _value;

				_to += 8;

				/*
				(*_to++) = _value;
				(*_to++) = _value;
				(*_to++) = _value;
				(*_to++) = _value;
				(*_to++) = _value;
				(*_to++) = _value;
				(*_to++) = _value;
				(*_to++) = _value;
				*/

				len -= 8 * sizeof(long);
			}

			while(len >= sizeof(long))
			{
				(*_to++) = _value;

				len -= sizeof(long);
			}		

			to = (char *)_to;
		}
	}

	while(len-- > 0)
		(*to++) = value;
}

/****************************************************************************/
/* This is ugly: GCC 2.95.x assumes that 'unsigned long' is used in the built-in
   memcmp/memcpy/memset functions instead of 'size_t'. This can produce warnings
   where none are necessary. */
#if defined(__GNUC__) && (__GNUC__ < 3)
void *
memset(void *ptr, int val, unsigned long len)
#else
void *
memset(void *ptr, int val, size_t len)
#endif /* __GNUC__ && __GNUC__ < 3 */
{
	void * result = ptr;
	unsigned char * m = ptr;

	if(!( (len == 0) || (ptr != NULL && (int)len > 0) )) {
	//	errno = EFAULT;
		goto out;
	}

	#if 0
	{
		while(len-- > 0)
			(*m++) = val;
	}
	#else
	{
		__memset(m,(unsigned char)(val & 255),len);
	}
	#endif

 out:

	return(result);
}

/****************************************************************************/

void bcopy(const void *src,void *dest,size_t len)
{
	if( (len == 0) || (src != NULL && dest != NULL && (int)len > 0))
		memmove(dest,src,len);
}

/****************************************************************************/

void bzero(void *b,size_t n)
{
	memset( b, '\0', n );
}

/****************************************************************************/

size_t strlen(const char *string)
{ const char *s=string;
  
  if(!(string && *string))
  	return 0;
  
  do;while(*s++); return ~(string-s);
}

/****************************************************************************/

time_t time(time_t *tloc)
{ struct DateStamp t;
  time_t ti;
  DateStamp(&t); /* Get timestamp */
  ti=((t.ds_Days+2922)*1440+t.ds_Minute)*60+t.ds_Tick/TICKS_PER_SECOND;
  if(tloc!=NULL)
    *tloc=ti;
  return ti;
}

/****************************************************************************/

char *strstr(const char *s1,const char *s2)
{ const char *c1,*c2;

  do {
    c1 = s1; c2 = s2;
    while(*c1 && *c1==*c2) {
      c1++; c2++;
    }
    if (!*c2)
      return (char *)s1;
  } while(*s1++);
  return (char *)0;
}

/****************************************************************************/

char *strchr(const char *s,int c)
{
  while (*s!=(char)c)
    if (!*s++)
      { s = (char *)0; break; }
  return (char *)s;
}

/****************************************************************************/

int strncmp(const char *s1,const char *s2,size_t n)
{ unsigned char *p1=(unsigned char *)s1,*p2=(unsigned char *)s2;
  unsigned long r,c;

  if ((r=n))
    do;while(r=*p1++,c=*p2++,!(r-=c) && (char)c && --n);
  return r;
}

/****************************************************************************/

/****************************************************************************/

/****************************************************************************/

