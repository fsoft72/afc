/*
 * Advanced Foundation Classes
 * Copyright (C) 2000/2025  Fabio Rotondo
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
@config
	TITLE:     AFC String
	VERSION:   1.01
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
	AUTHOR:	   Massimo Tantignone - tanti@intercon.it
@endnode

@node history
	1.01	- FIX:	small bug in `afc_string_temp`_ when a non AFC string were passed as parameter.
@endnode
*/

#include "string.h"

#include <unistd.h>

#define STRING_MAX(str) (str ? ((unsigned long)(*((unsigned long *)(str - sizeof(unsigned long) * 2)) - 1)) : 0L)

#ifdef MINGW
static const char dir_sep = '\\';
#else
static const char dir_sep = '/';
#endif

// {{{ docs
/*
@node quote
	*Last night I stayed up late playing poker with Tarot cards. I got a full house and four people died.*

		Stephen Wright
@endnode

@node intro
This is a suite of functions to create, manipulate and dispose strings.
As you should already know, strings are a bad animal for C programmers, sice they are
just a bunch of bytes in memory where no bound control is made during manipulation and
that can lead to the worse (and most hidden) segmentation fault problems.

If you use inside your programs these functions and what we have called /AFC/ /Strings/,
you will not suffer for these problems. Every function for manipulating strings does bound
checking and integrity checking, so if you copy "supercalifragilisticespiralidous" into a string
10 bytes long, you'll not damage external memory, but just have back the first 10 chars of the word.

But there's more. /AFC/ /Strings/ offer a bunch of optimized calls, like afc_string_len(), that is
light year faster than the standard C strlen function, and afc_string_max() that let's you know how
many chars can be handled by the provided string.

The most important thing is that /AFC/ /Strings/ are backward compatible with standard C strins (array of chars),
so you can pass a standard /AFC/ /String/ to functions like printf, sprintf and so on without problems.

NOTE:
	Please, note that where an /AFC/ /String/ is needed, you /must/ provide an /AFC/ /String/ and not just a C string,
	or you'll get into troubles. As we mentioned before, /AFC/ /Strings/ can (and should) be used as standard C strings
	in all function calls, like printf or sprintf, but the opposite is /never/ true.
	So keep in mind: where you need an /AFC/ /String/, you need an /AFC/ /String/. Where you need a C string,
	you can provide both a C string or an /AFC/ /String/.

/AFC/ /Strings/ also offer /advanced/ manipulation functions, like afc_string_radix() used to convert a number
to a given base (up to base64 is supported); afc_string_hash() that is able to generate an hash value for the provided
string or afc_string_pattern_match() that is able to match the given string against a standard pattern match string to
see if the pattern is ok.

To create a new /AFC/ /String/ you use afc_string_new(), then you can manipulate it with standard AFC functions like
afc_string_copy(), afc_string_left(), afc_string_right() or afc_string_make(). When you have finished with a string,
remember to call afc_string_delete() to free the memory associated with it.
@endnode
*/
// }}}

// {{{ afc_string_new ( max_chars )
/*
@node afc_string_new

	NAME: afc_string_new ( numchars ) - Allocates a string numchars long

	SYNOPSIS: char * afc_string_new ( unsigned long numchars )

 DESCRIPTION: This command allocates a standard Amiga string.
			This string structure has been mutuated from
			AmigaE's EString, so from now on we will call them
			AFC Strings.

			These kind of strings have several advantages such as:

			- Bound checking - you'll not generate any memory problem by doing a afc_string_copy() with a string signed longer than the one allocated.

			- Faster afc_string_len - the afc_string_len() command is light years faster than the standard strlen.

		 INPUT: - numchars		- Number of chars to assign to the string.

		RESULT: - a pointer to the starting memory area.

			- NULL means that an error occurred (usually: no memory to allocate string).

			NOTE: - A string allocated using afc_string_new() _MUST_ be freed using afc_string_delete()

	SEE ALSO: - afc_string_delete()
			- afc_string_copy()
			- afc_string_max()
			- afc_string_len()

@endnode
*/

char *_afc_string_new(unsigned long numchars, const char *file, const char *func, const unsigned int line)
{
	unsigned long *location;
	char *str;

	if ((str = _afc_malloc(numchars + 1 + (sizeof(unsigned long) * 2), file, func, line)) == NULL)
		return NULL;

	// if ( ( str = afc_malloc ( numchars + 1 + ( sizeof ( unsigned long ) * 2 ) ) ) == NULL ) return ( NULL );

	location = (unsigned long *)str;

	// printf ( "String NEW: (%d) (%x)\n", ( numchars + 1 + ( sizeof ( unsigned long ) * 2 ) ), ( int ) str );

	location[0] = numchars + 1;
	location[1] = 0L;

	return (str + (sizeof(unsigned long) * 2));
}
// }}}
// {{{ afc_string_delete ( str )
/*
@node afc_string_delete

			NAME: afc_string_delete(string) - Remove the string from memory

	SYNOPSIS: char *  afc_string_delete( char * string)

		 DESCRIPTION: This command deallocates a string created using afc_string_new().

		 INPUT: - string - AFC string to free.

		RESULT: - the string is freed from memory and a NULL is returned. This value is useful if you want to set
				the string pointer to NULL in a shot.

			NOTE: - afc_string_delete() can handle NULL pointers.

			- afc_string_delete() is THE ONLY WAY to correctly deallocate strings
			  created with afc_string_new().

	SEE ALSO: - afc_string_new()

@endnode
*/

char *_afc_string_delete(char *location)
{
	if (location == NULL)
		return (NULL);

	location = location - (sizeof(unsigned long) * 2);
	afc_free(location);

	return (NULL);
}
// }}}
// {{{ afc_string_max ( str )
/*
@node afc_string_max

			NAME: afc_string_max(string) - Returns the maximum size of the given string

	SYNOPSIS: unsigned long afc_string_max ( cosnt char * string)

		 DESCRIPTION: This function returns the maximum number of chars that a string
			created using afc_string_new() can handle.

		 INPUT: - string				- afc_string_new to examine.

		RESULT: - The maximum number of chars storable inside the passed string.

			NOTE: - afc_string_max() can handle NULL pointers.

	SEE ALSO: - afc_string_new()
			- afc_string_len()
			- afc_string_copy()

@endnode
*/

unsigned long afc_string_max(const char *str)
{
	return (str ? ((unsigned long)(*((unsigned long *)(str - sizeof(unsigned long) * 2)) - 1)) : 0L);
}
// }}}
// {{{ afc_string_len ( str )
/*
@node afc_string_len

			NAME: afc_string_len(string) - Returns the actual string length

	SYNOPSIS: unsigned long afc_string_len( const char * string)

		 DESCRIPTION: This function returns the actual string length.

		 INPUT: - string				- afc_string_new to examine.

		RESULT: - Actual string length.

			NOTE: - afc_string_len() can handle NULL pointers.

			- Even if you can use the standard C strlen command, we suggest to use our afc_string_len(): it is faster.

			- afc_string_len() works only with AFC string_news.

	SEE ALSO: - afc_string_new()
			- afc_string_max()

@endnode
*/

unsigned long afc_string_len(const char *str)
{
	return (str ? ((unsigned long)(*((unsigned long *)(str - sizeof(unsigned long))))) : 0L);
}
// }}}
// {{{ afc_string_copy ( dest, src, len )
/*
@node afc_string_copy

			NAME: afc_string_copy(deststring, sourcestring, len ) - Copy a string inside an AFC string_new

	SYNOPSIS: char * afc_string_copy(char * deststring, const char * sourcestring, unsigned long len)

		 DESCRIPTION: Use this function to copy a string inside an AFC string_new.
			This afc_string_copy() is advanced considering the usual C strcpy
			command: it does bound checking so you don't have to do any
			control before copying.

		 INPUT: - deststring		- Destination string. This string MUST be an AFC string_new
					created using afc_string_new()

			- sourcestring	- Source string. This string can be anything, a static
					string, a AFC string_new, a memory area...

			- len		 - Here you can specify
					how many chars to copy. Pass the value *ALL* to copy the whole string.

		RESULT: - The resulting string.

			NOTE: afc_string_copy() can handle NULL pointers.

	SEE ALSO: - afc_string_add()

@endnode
*/

char *afc_string_copy(char *dest, const char *source, unsigned long len)
{
	unsigned long m, srclen;

	if (!(dest && source))
		return (NULL);

	srclen = strlen(source);

	m = afc_string_max(dest);

	if ((long)len == ALL)
		len = srclen;

	if (len > m)
		len = m;
	if (len > srclen)
		len = srclen;

	*((unsigned long *)(dest - sizeof(unsigned long))) = len;

	for (m = 0; m < len; m++)
		*(dest++) = *(source++);
	*dest = '\0';

	return (dest);
}
// }}}
// {{{ afc_string_clear ( str )
/*
@node afc_string_clear

			NAME: afc_string_clear ( string ) - Empty the contents of a string

	SYNOPSIS: char * afc_string_clear ( char * str )

		 DESCRIPTION: This function deletes all the contents of an AFC string.
					It is similar to /afc_string_copy(str,"",ALL)/ but faster.

		 INPUT: - string		- The string to clear. MUST be an AFC string_new, created using the afc_string_new() command.

		RESULT: - str	 - Resulting string.

			NOTE: - This function can handle NULL pointers.

	SEE ALSO: - afc_string_new()
			- afc_string_copy()
			- afc_string_add()

@endnode
*/
char *afc_string_clear(char *dest)
{
	if (dest == NULL)
		return (NULL);

	*((unsigned long *)(dest - sizeof(unsigned long))) = 0;
	*dest = '\0';

	return (dest);
}
// }}}
// {{{ afc_string_mid ( dest, src, from, num_chars )
/*
@node afc_string_mid

			NAME: afc_string_mid(dest, source, fromchar, numchars) - Copies a part of source string in dest string

	SYNOPSIS: char * afc_string_mid(char * dest, const char * source, unsigned long fromchar, unsigned long numchars)

		 DESCRIPTION: This function copies a part of source string into dest string.

		 INPUT: - dest		- The destination string. MUST be an AFC string_new, created
					using the afc_string_new() command.

			- source				- Source string.

			- fromchar			- Starting char for the copy.

			- numchars			- Number of chars to copy.

		RESULT: - str		 - Resulting string.

			NOTE: - afc_string_mid() can handle NULL pointers.

	SEE ALSO: - afc_string_new()
			- afc_string_copy()

@endnode
*/
char *afc_string_mid(char *dest, const char *src, unsigned long fromchar, unsigned long numchars)
{
	unsigned long len;

	if (!src)
		return (NULL);

	len = strlen(src);

	if (fromchar > len)
		return (NULL);

	if ((fromchar + numchars) > len)
		numchars = len - fromchar;

	return (afc_string_copy(dest, src + fromchar, numchars));
}
// }}}
// {{{ afc_string_comp ( str1, str2, chars )
/*
@node afc_string_comp

			NAME: afc_string_comp(str1, str2, numchars ) - Compares two strings

	SYNOPSIS: signed long afc_string_comp( const char * str1, const char * str2, unsigned long numchars)

		 DESCRIPTION: This function compares two strings and returns a value.

		 INPUT: - str1              - First string to compare
			- str2              - Second string to compare
			- numchars		- How many chars to compare before quitting.
					If you pass *ALL*, that means that the whole
									strings will be compared

		RESULT: -  a value < 0	 means	str1>str2
		  -  a value > 0	 means	str2>str1
			-  a value == 0	 means	str1==str2

	SEE ALSO:
@endnode
*/
signed long afc_string_comp(const char *s1, const char *s2, long chars)
{
	char *str1, *str2;
	long c = 0;

	if (chars != ALL)
		chars--;
	else
		chars = 0;

	for (str1 = (char *)s1, str2 = (char *)s2; (*str1 == *str2) && (*str1 && *str2) && ((chars == 0 || (long)c++ < chars)); str1++, str2++)
		;

	return (-(*str1 - *str2));
}
// }}}
// {{{ afc_string_upper ( str )
/*
@node afc_string_upper

			NAME: afc_string_upper(string) - Converts a string in upper case chars

	SYNOPSIS: char * afc_string_upper( char * string)

		 DESCRIPTION: This function converts a string in all upper case chars.
			Please note that this is a /in/ /place/ substitution. The
			  provided string will be modified with all uppercase chars.

		 INPUT: - string				- AFC string to convert.

		RESULT: - a pointer to the all uppercase chars string.

			NOTE: - This function can handle NULL pointers.

	SEE ALSO: - afc_string_lower()

@endnode
*/
char *afc_string_upper(register char *s)
{
	char *x;

	if ((x = s) == NULL)
		return (NULL);

	for (; *s; s++)
		*s = toupper(*s);

	return (x);
}
// }}}
// {{{ afc_string_lower ( str )
/*
@node afc_string_lower

			NAME: afc_string_lower(string) - Converts a string in lower case chars

	SYNOPSIS: char * afc_string_lower( char * string)

		 DESCRIPTION: This function converts a string in all lower case chars.
			Please note that this is a /in/ /place/ substitution. The
				provided string will be modified with all lowercase chars.

		 INPUT: - string				- AFC string to convert.

		RESULT: - a pointer to the all uppercase chars string.

			NOTE: - This function can handle NULL pointers.

	SEE ALSO: - afc_string_upper()

@endnode
*/
char *afc_string_lower(register char *s)
{
	char *x;

	if ((x = s) == NULL)
		return (NULL);

	for (; *s; s++)
		*s = tolower(*s);

	return (x);
}
// }}}
// {{{ afc_string_trim ( str )
/*
@node afc_string_trim

			NAME: afc_string_trim(string) - Removes all blank chars from string

	SYNOPSIS: char * afc_string_trim ( char * string )

		 DESCRIPTION: This function removes all blank chars from both start and end
				string. Blank chars are: space, tab, new line (10), carriage return (13).
			Please note that this is a /in/ /place/ substitution. The
				provided string will be modified.

		 INPUT: - string				- AFC string to convert.

		RESULT: - a pointer to the trimmed string.

			NOTE: - This function can handle NULL pointers.

	SEE ALSO:

@endnode
*/
char *afc_string_trim(char *s)
{
	char *x;
	int y;

	if (s == NULL)
		return (NULL);

	x = s;
	while ((x[0] == ' ') || (x[0] == 9))
	{
		x++;
	}

	y = afc_string_len(s);

	while ((s[y] == ' ') || (s[y] == 9) || (s[y] == 0) || (s[y] == 10) || (s[y] == 13))
		s[y--] = 0;

	afc_string_copy(s, x, ALL);

	return (s);
}
// }}}
// {{{ afc_string_instr ( str, match, start_pos )
/*
@node afc_string_instr

			NAME: afc_string_instr ( string, match, startpos ) - Search for a matching string inside a string

	SYNOPSIS: char * afc_string_instr ( const char * string, const char * match, unsigned long startpos )

		 DESCRIPTION: This function searches for the string /match/ inside the string /string/.
				It is possible to specify a starting point to search from inside the string
				/string/ by providing a value != 0 in /startpos/.

		 INPUT: - string				- AFC string to convert.
			- match		- string to match inside the /string/
			- startpos			- The first char in /string/ that could match.

		RESULT: - a pointer to the matched string or NULL if matching string was not found.

			NOTE: - This function can handle NULL pointers.

	SEE ALSO:

@endnode
*/
char *afc_string_instr(const char *str, const char *match, unsigned long startpos)
{

	if (str == NULL)
		return NULL;
	if (match == NULL)
		return NULL;

	if (startpos > afc_string_len(str))
		return NULL;

	return ((char *)(strstr(str + startpos, match)));
}
// }}}
// {{{ afc_string_left ( dest, src, len )
/*
@node afc_string_left

			NAME: afc_string_left ( dest, src, len ) - Copies the leftmost len chars from src to string

	SYNOPSIS: char * afc_string_left ( char * dest, const char * src, long len )

		 DESCRIPTION: This function copies the leftmost /len/ characters from /src/ into /string/.
				This function is just provided for completeness, since it has the same function
				like afc_string_copy().

		 INPUT: - dest				- AFC destination string.
			- src 		- The source string. It doesn't necessarily need to be an AFC string.
			- len     			- Number of chars to copy into /string/

		RESULT: - a pointer to the string /string/.

			NOTE: - This function can handle NULL pointers.

	SEE ALSO: - afc_string_right()
			- afc_string_mid()
			- afc_string_copy()
@endnode
*/
char *afc_string_left(char *dest, const char *src, long len)
{
	return (afc_string_copy(dest, src, len));
}
// }}}
// {{{ afc_string_right ( dest, src, len )
/*
@node afc_string_right

			NAME: afc_string_right ( string, src, len ) - Copies the leftmost len chars from src to string

	SYNOPSIS: char * afc_string_right ( char * string, const char * src, long len )

		 DESCRIPTION: This function copies the rightmlost /len/ characters from /src/ into /string/.

		 INPUT: - string				- AFC string to convert.
			- src 		- The source string. It doesn't necessarily need to be an AFC string.
			- len     			- Number of chars to copy into /string/

		RESULT: - a pointer to the string /string/.

			NOTE: - This function can handle NULL pointers.

	SEE ALSO: - afc_string_left()
			- afc_string_mid()
			- afc_string_copy()
@endnode
*/
char *afc_string_right(char *dest, const char *src, long len)
{
	unsigned long l;

	l = strlen(src);

	if ((long)l < len)
		len = l;

	return (afc_string_copy(dest, (src + l) - len, ALL));
}
// }}}
// {{{ afc_string_reset_len ( str )
/*
@node afc_string_reset_len

			NAME: afc_string_reset_len ( string ) - Resets internal string len

	SYNOPSIS: unsigned long afc_string_reset_len ( const char * string )

DESCRIPTION: This function is low-level. It is needed to update internal AFC string information when the AFC string
				 is not manipulated by AFC string functions, but rather with standard string functions (like those in
				 string.h). After you have used one function that manipulates an AFC string like it was a standard buffer,
				 you must call /afc_string_reset_len/ to update internal string len rappresentation.

		 INPUT: - string				- AFC string to convert.

		RESULT: - the real AFC string len

			NOTE: - This function can handle NULL pointers.

	SEE ALSO:

@endnode
*/
unsigned long afc_string_reset_len(const char *str)
{
	unsigned long l;
	// char * s;

	if (str == NULL)
		return (0);

	*((unsigned long *)(str - sizeof(unsigned long))) = (l = strlen(str));

	return (l);
}
// }}}
// {{{ afc_string_radix ( str, number, base )
/*
@node afc_string_radix

			NAME: afc_string_radix ( string, number, base ) - Rappresents a number using the given base

	SYNOPSIS: int afc_string_radix ( char * string, long number, int base )

		 DESCRIPTION: This function converts the passed /number/ into a string in the given /base/ and copies
					  the result in /string/. You can generate string rappresenting a number with base ranging
					  from 1 to 64.

		 INPUT: - string				- Destination AFC string.
			- number				- The number to convert.
			- base    			- The new base. It can range from 1 to 64.

		RESULT: - 0 - The convertion worked properely
			- non zero - Something went wrong.

			NOTE: - This function can handle NULL pointers.
				- These are the chars used in the base: "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_@"
				please note that, since the function uses both lower and upper chars, case /does/ matter.

	SEE ALSO:

@endnode
*/
int afc_string_radix(char *dest, long n, int radix)
{
	char hexn[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_@";
	char buf[1024]; // Flawfinder: ignore
	long q = labs(n);
	int r = 0;

	afc_string_copy(dest, "", ALL);

	if (radix > 64)
		return (-1);

	while (1)
	{
		r = q % radix;

		snprintf(buf, 1024, "%c%s", hexn[r], dest); // Flawfinder: ignore
		afc_string_copy(dest, buf, ALL);

		q = (q - r) / radix;

		if (q == 0)
			break;
	}

	if (n < 0)
	{
		snprintf(buf, 1024, "-%s", dest);
		afc_string_copy(dest, buf, ALL);
	}

	return (0);
}
// }}}
// {{{ afc_string_hash ( string, turbolence )
/*
@node afc_string_hash

			NAME: afc_string_hash ( string, turbolence ) - Creates an hash value for the string

	SYNOPSIS: unsigned long int afc_string_hash ( register unsigned char * string, unsigned long int turbolence )

		 DESCRIPTION: This function generates and hash value for the given /string/. An hash value is a (very long)
					  number that tries to generate unique id (values) for one string. The /turbolence/ value is used
					  to add some randomness to the given number. Please, remember that changing the /turbolence/ causes
					  a different hash value to be generated. If you want always the same hash value for a specific /string/,
					  you should always use the same /turbolence/

		 INPUT: - string				- String to be hashed.
			- turbolence		- The starting value randomness

		RESULT: - 0 - The convertion worked properely
			- non zero - Something went wrong.

			NOTE: - This function can handle NULL pointers.

	SEE ALSO:

	 CREDITS: Original code by Bob Jenkins

@endnode
*/
unsigned long int afc_string_hash(register const unsigned char *k, register unsigned long int turbolence)
{
	register unsigned long int a, b, c, len, length;

	if (k == NULL)
		return (0);

	length = strlen((char *)k);

	/* Set up the internal state */
	len = length;
	a = b = 0x9e3779b9; /* the golden ratio; an arbitrary value */
	c = turbolence;		/* An init turbolence value */

	/*---------------------------------------- handle most of the key */
	while (len >= 12)
	{
		a += (k[0] + ((unsigned long int)k[1] << 8) + ((unsigned long int)k[2] << 16) + ((unsigned long int)k[3] << 24));
		b += (k[4] + ((unsigned long int)k[5] << 8) + ((unsigned long int)k[6] << 16) + ((unsigned long int)k[7] << 24));
		c += (k[8] + ((unsigned long int)k[9] << 8) + ((unsigned long int)k[10] << 16) + ((unsigned long int)k[11] << 24));
		afc_tools_internal_mix(a, b, c);
		k += 12;
		len -= 12;
	}

	/*------------------------------------- handle the last 11 bytes */
	c += length;
	switch (len) /* all the case statements fall through */
	{
	case 11:
		c += ((unsigned long int)k[10] << 24);
		// fall through
	case 10:
		c += ((unsigned long int)k[9] << 16);
		// fall through
	case 9:
		c += ((unsigned long int)k[8] << 8);
		/* the first byte of c is reserved for the length */
		// fall through
	case 8:
		b += ((unsigned long int)k[7] << 24);
		// fall through
	case 7:
		b += ((unsigned long int)k[6] << 16);
		// fall through
	case 6:
		b += ((unsigned long int)k[5] << 8);
		// fall through
	case 5:
		b += k[4];
		// fall through
	case 4:
		a += ((unsigned long int)k[3] << 24);
		// fall through
	case 3:
		a += ((unsigned long int)k[2] << 16);
		// fall through
	case 2:
		a += ((unsigned long int)k[1] << 8);
		// fall through
	case 1:
		a += k[0];
		/* case 0: nothing left to add */
	}
	afc_tools_internal_mix(a, b, c);
	/*-------------------------------------------- report the result */

	return c;
}
// }}}
// {{{ afc_string_dup ( str )
/*
@node afc_string_dup

			NAME: afc_string_dup ( string ) - Creates a new string with the same contents of the given one

	SYNOPSIS: char * afc_string_dup ( const char * string )

		 DESCRIPTION: This function allocates a new AFC string and copies the contents of the provided /string/ into it,
					  It is a shorthand for afc_string_new() and afc_string_copy().

		 INPUT: - string				- String to be copied. It doesn't necessarily be an AFC string.

		RESULT: - a new AFC string or NULL in case of errors (usually: no memory).

			NOTE: - This function can handle NULL pointers.
			- This is a new AFC string, like the ones usually created using afc_string_new(), so you have to
				  explicitely deallocate it when you have finished, using the standard afc_string_delete() function.

	SEE ALSO: - afc_string_new ()
				- afc_string_copy()

@endnode
*/
char *_afc_string_dup(const char *str, const char *file, const char *func, const unsigned int line)
{
	char *s;
	unsigned long l;

	if (str == NULL)
		return (NULL);

	l = strlen(str);

	if (l == 0)
		return (NULL);

	if ((s = _afc_string_new(l, file, func, line)) == NULL)
		return (NULL);

	afc_string_copy(s, str, ALL);

	return (s);
}
// }}}
// {{{ afc_string_make ( str, fmt, ... )
/*
@node afc_string_make

			NAME: afc_string_make ( string, fmt, ... ) - Copies data into string using the given fmt and args

	SYNOPSIS: char * afc_string_make ( char * string, const char * fmt, ... )

		 DESCRIPTION: This function is a safe version of the standard sprintf. It does just the same things,
					  but with bound checking, so you are pretty sure that the passed args will not be too big.

		 INPUT: - string				- Destination AFC string.
			- fmt                 - Format string. It uses the same syntax of printf or sprintf.
			- ...                 - All the params needed.

		RESULT: - the string just produced.

			NOTE: - This function can handle NULL pointers.

	SEE ALSO: - afc_string_new ()
				- afc_string_copy()

@endnode
*/
char *afc_string_make(char *dest, const char *fmt, ...)
{
	va_list ap;

	if ((dest == NULL) || (fmt == NULL))
		return (NULL);

	va_start(ap, fmt);

	vsnprintf(dest, afc_string_max(dest) + 1, fmt, ap); // Flawfinder: ignore

	va_end(ap);

	afc_string_reset_len(dest);

	return (dest);
}
// }}}
// {{{ afc_string_fget ( dest, file )
/*
@node afc_string_fget

			NAME: afc_string_fget ( deststring, file ) - Get a line of text from a file

	SYNOPSIS: char * afc_string_fget ( char * deststring, FILE * file )

		 DESCRIPTION: Use this function to read a line of text from a file.
			This function is advanced considering the usual C /fgets/
			command: it does bound checking so you don't have to do any
			control before reading from the file.

		 INPUT: - deststring		- Destination string. This string MUST be an AFC string
					created using afc_string_new ()

			- file		- A valid file handler where to get the string from.

		RESULT: - The resulting string or NULL if end of file has been encountered.

			NOTE: - this function can handle NULL pointers

	SEE ALSO:

@endnode
*/
char *afc_string_fget(char *dest, FILE *fh)
{
	if ((dest == NULL) || (fh == NULL))
		return (NULL);

	afc_string_clear(dest);

	if (fgets(dest, afc_string_max(dest), fh) == NULL)
		return (NULL);

	afc_string_reset_len(dest);

	return (dest);
}
// }}}
// {{{ afc_string_add ( dest, source, len )
/*
@node afc_string_add

			NAME: afc_string_add (deststring, sourcestring, len ) - Add a string to the current one

	SYNOPSIS: char * afc_string_add ( char * deststring, const char * sourcestring, unsigned long len)

		 DESCRIPTION: Use this function to append a string at the end of another.
			This function is advanced considering the usual C strcat
			command: it does bound checking so you don't have to do any
			control before appending.

		 INPUT: - deststring		- Destination string. This string MUST be an AFC string
					created using afc_string_new ()

			- sourcestring	- Source string. This string can be anything, a static
					string, an AFC string , a memory area...

			- len		 - Here you can specify how many chars to copy. Pass the value /ALL/ to copy the whole string.

		RESULT: - The resulting string.

			NOTE: - this function can handle NULL pointers

	SEE ALSO: - afc_string_copy()

@endnode
*/
char *afc_string_add(char *dest, const char *source, unsigned long len)
{
	unsigned long m, srclen, clen;

	if (!(dest && source))
		return (NULL);

	srclen = strlen(source);

	clen = afc_string_len(dest);
	m = afc_string_max(dest) - clen;

	if ((long)len == ALL)
		len = srclen;

	if (len > m)
		len = m;
	if (len > srclen)
		len = srclen;

	*((unsigned long *)(dest - sizeof(unsigned long))) = clen + len;

	dest += clen;

	for (m = 0; m < len; m++)
		*(dest++) = *(source++);
	*dest = '\0';

	return (dest);
}
// }}}
// {{{ afc_string_temp ( path )
/*
@node afc_string_temp

		NAME: afc_string_temp ( path ) - Creates a temporary file name

	SYNOPSIS: char * afc_string_temp ( const char * path )

	 DESCRIPTION: This function creates a new string containing an unique name, not avaible
		  in the given path when checking. This is useful when you want to create,
		  for example, some temporary files and want to be sure an unique name is used.
		  In the /path/ you should also specify a /prefix/ for the temporary file being
		  created. If for example, you want to create a temp file in "/tmp" with the prefix
		  "temp", you should write "/tmp/temp" and the temp file name you'll get back will
		  be something like "/tmp/tempVt6Akr".


	   INPUT: - path		- The path in the filesystem to check against unique name.

	  RESULT: - A *new* AFC string, that you'll need to free with afc_string_delete() when you're done.
		  - A NULL value means an error occurred.

		NOTE: - To avoid race conditions, this function actually creates the file in the specified path.
			This should not be a problem, since you are going to create that file anyway and you have the
			permissions to do that.
		  - this function can handle NULL pointers
			  - remember to free the string using afc_string_delete() when you have finished with it.

	SEE ALSO: - afc_string_delete()
		  - stdio.h/tempnam call.
		  - stdlib.h/mkstemp call.

@endnode
*/
char *afc_string_temp(const char *path)
{
	char *tmp;
	int fd;
#ifdef MINGW
	char *name;
#endif
	char *p;

	if (path == NULL)
		path = "/tmp/afc";

#ifdef MINGW
	p = afc_string_dirname(path);

	/* tempnam adds "file" prefix to the file name */
	tmp = afc_string_new(afc_string_len(p) + 11);
#else
	p = (char *)path;
	tmp = afc_string_new(strlen(p) + 7);
#endif

	if (tmp == NULL)
		return (NULL);

	afc_string_make(tmp, "%sXXXXXX", path);

#ifdef MINGW
	if ((name = tempnam(p, NULL)) == NULL)
	{
		afc_string_delete(p);
		afc_string_delete(tmp);
		return (NULL);
	}

	afc_string_copy(tmp, name, ALL);

	afc_string_delete(p);
	free(name);

	if ((fd = open(tmp, O_CREAT | O_EXCL)) == -1)
#else
	if ((fd = mkstemp(tmp)) == -1)
#endif
	{
		fprintf(stderr, "afc_string_temp() error: %s\n", strerror(errno));
		afc_string_delete(tmp);
		return (NULL);
	}

	close(fd); // Close the file descriptor

	return (tmp);
}
// }}}
// {{{ afc_string_resize_copy ( dest, str )
/*
@node afc_string_resize_copy

		NAME: afc_string_resize_copy ( dest, str ) - Copies a string resizing the dest buffer

	SYNOPSIS: char * afc_string_resize_copy ( char * * dest, char * str )

	 DESCRIPTION: This function copies the full content of string /str/ inside the /dest/ AFC String.
		  If the memory alloc'd for the /dest/ string is not enough, a new string is allocated in place
		  of the original /dest/ AFC String and the /str/ is copied.

	   INPUT: - dest		- The destination string where to copy the /str/ data.
		  - str			- The string to be copied into /dest/.

	  RESULT: - The /str/ will be copied inside /dest/.

		NOTE: - /dest/ is a pointer of pointer. You have to call this function by referencing the /dest/ string
		   with the "&" operator. Eg. afc_string_resize_copy ( &dest, "hello world" );

	SEE ALSO: - afc_string_copy()

@endnode
*/
char *afc_string_resize_copy(char **dest, const char *str)
{
	char *str_new;
	unsigned int max;

	max = afc_string_max(*dest);

	if ((strlen(str) + afc_string_len(*dest)) > (max - 3))
	{
		str_new = afc_string_new(max * 2);
		afc_string_copy(str_new, *dest, ALL);

		afc_string_delete(*dest);
		*dest = str_new;
	}
	else
		afc_string_copy(*dest, str, ALL);

	return (*dest);
}
// }}}
// {{{ afc_string_resize_add ( dest, str )
char *afc_string_resize_add(char **dest, const char *str)
{
	char *str_new;
	unsigned int max;

	max = afc_string_max(*dest);

	if ((strlen(str) + afc_string_len(*dest)) > (max - 3))
	{
		if ((str_new = afc_string_new((strlen(str) + afc_string_len(*dest)) * 2)) == NULL)
			return (NULL);

		afc_string_copy(str_new, *dest, ALL);

		afc_string_delete(*dest);
		*dest = str_new;
	}

	afc_string_add(*dest, str, ALL);

	return (*dest);
}
// }}}

// {{{ afc_string_dirname ( path )
char *afc_string_dirname(const char *path)
{
	char *dest = NULL;
	char *x;

	// If path is not defined, do nothing.
	if (path == NULL)
		return (NULL);

	// Search for the last "/" in the file_name
	x = strrchr(path, dir_sep);

	// if we don't find it, simply copy all the path
	if (x == NULL)
		dest = afc_string_dup(path);
	else
	{
		// ... else we have to create a new string and
		// copy the chars we are interested in
		dest = afc_string_new((x - path));
		afc_string_copy(dest, path, ALL);
	}

	return (dest);
}
// }}}
// {{{ afc_string_basename ( path )
char *afc_string_basename(const char *path)
{
	char *dest = NULL;
	char *x;

	// If path is not defined, do nothing.
	if (path == NULL)
		return (NULL);

	// Search for the last "/" in the file_name
	x = strrchr(path, dir_sep);

	// if we don't find it, simply copy all the path
	if (x == NULL)
		dest = afc_string_dup(path);
	else
	{
		// ... else we have to create a new string and
		// copy the chars we are interested in
		dest = afc_string_dup(x + 1);
	}

	return (dest);
}
// }}}

int _seems_utf8(const char *str)
{
	int len = strlen(str);
	int i, n, j;
	unsigned char c;

	for (i = 0; i < len; i++)
	{
		c = str[i];

		if (c < 0x80)
			n = 0;
		else if ((c & 0xE0) == 0xC0)
			n = 1;
		else if ((c & 0xF0) == 0xE0)
			n = 2;
		else if ((c & 0xF8) == 0xF0)
			n = 3;
		else if ((c & 0xFC) == 0xF8)
			n = 4;
		else if ((c & 0xFE) == 0xFC)
			n = 5;
		else
			return FALSE;

		for (j = 0; j < n; j++)
		{
			if ((++i == len) || ((str[i] & 0xC0) != 0x80))
				return FALSE;
		}
	}

	return TRUE;
}

char *afc_string_utf8_to_latin1(const char *utf8)
{
	if (!utf8 || !strlen(utf8))
		return afc_string_new(1);

	if (!_seems_utf8(utf8))
	{
		return afc_string_dup(utf8);
	}

	unsigned char *s = afc_malloc(strlen(utf8) + 20);
	unsigned int pos = 0;
	unsigned int len = strlen(utf8);
	unsigned char c1, c2, iso;
	unsigned int xpos = 0;
	char *res;

	while (pos < len)
	{
		c1 = utf8[pos++];

		if (c1 <= 0x7F)
		{
			s[xpos++] = c1;
		}
		else if (c1 >= 0xC0 && c1 <= 0xC7)
		{
			if (pos == len)
			{
				_afc_dprintf("%s::%s - ERROR: wrong string length", __FILE__, __FUNCTION__);
				return NULL;
			}

			c2 = utf8[pos++];

			iso = ((c1 & 0x07) << 6) | (c2 & 0x3F);

			if (iso <= 0x7F)
			{
				_afc_dprintf("%s::%s - ERROR: Sequence longer than needed", __FILE__, __FUNCTION__);
				return NULL;
			}

			s[xpos++] = iso;
		}
	}

	s[xpos] = '\0';

	res = afc_string_dup((char *)s);
	afc_free(s);

	return res;
}

#ifndef MINGW
// {{{ afc_string_pattern_match ( str, pattern, no_case )
/*
@node afc_string_pattern_match

			NAME: afc_string_pattern_match ( string, pattern, no_case ) - Use pattern matching against this string

	SYNOPSIS: int afc_string_pattern_match ( const char * string, const char * pattern, short no_case )

		 DESCRIPTION: This function tests if the given /string/ correctly matches the provided /pattern/ using
					  the standard system pattern matching algo. It can be made case insensitive by passing TRUE
					  as /no_case/ value.

		 INPUT: - string				- AFC string to convert.
			- pattern 			- The pattern string. It doesn't necessarily need to be an AFC string.
			- no_case  			- Flag T/F. If TRUE, the pattern matching is done case insensitive.

		RESULT: - 0 - The /string/ matches /pattern/.
			- non zero - The /string/ does /not/ match /pattern/ or NULL pointers passed.

			NOTE: - This function can handle NULL pointers.

	SEE ALSO:

@endnode
*/
int afc_string_pattern_match(const char *str, const char *pattern, short nocase)
{
	int res;
	char *s, *patt;

	if ((str == NULL) || (pattern == NULL))
		return (-1);

	if (nocase)
	{
		if ((s = afc_string_dup(str)) == NULL)
			return (-1);
		afc_string_upper(s);

		if ((patt = afc_string_dup(pattern)) == NULL)
		{
			afc_string_delete(s);
			return (-1);
		}

		afc_string_upper(patt);

		res = fnmatch(patt, s, 0);

		afc_string_delete(patt);
		afc_string_delete(s);
	}
	else
		res = fnmatch(pattern, str, 0);

	return (res);
}
// }}}
#endif

// {{{ afc_string_char_at ( str, index )
/*
@node afc_string_char_at

			NAME: afc_string_char_at ( str, index ) - Returns the character at the specified index

	SYNOPSIS: char afc_string_char_at ( const char * str, long index )

		 DESCRIPTION: Returns the character at the specified index. Accepts negative integers to count back from the last string character.

		 INPUT: - str				- The string.
			- index 			- The index of the character to return.

		RESULT: - The character at the specified index, or '\0' if out of range.

	SEE ALSO:

@endnode
*/
char afc_string_char_at(const char *str, long index)
{
	unsigned long len;

	if (str == NULL)
		return '\0';

	len = afc_string_len(str);

	if (index < 0)
		index = len + index;

	if (index < 0 || index >= (long)len)
		return '\0';

	return str[index];
}
// }}}

// {{{ afc_string_starts_with ( str, search, position )
/*
@node afc_string_starts_with

			NAME: afc_string_starts_with ( str, search, position ) - Checks if string starts with search string

	SYNOPSIS: int afc_string_starts_with ( const char * str, const char * search, unsigned long position )

		 DESCRIPTION: Determines whether a string begins with the characters of a specified string.

		 INPUT: - str				- The string to check.
			- search 			- The characters to be searched for at the start of this string.
			- position 			- The position in this string at which to begin searching for searchString. Defaults to 0.

		RESULT: - 1 if the string begins with the characters of the search string, 0 otherwise.

	SEE ALSO: - afc_string_ends_with()

@endnode
*/
int afc_string_starts_with(const char *str, const char *search, unsigned long position)
{
	unsigned long len, search_len;

	if (str == NULL || search == NULL)
		return 0;

	len = afc_string_len(str);
	search_len = strlen(search);

	if (position > len)
		return 0;

	if (search_len > len - position)
		return 0;

	return (strncmp(str + position, search, search_len) == 0);
}
// }}}

// {{{ afc_string_ends_with ( str, search, length )
/*
@node afc_string_ends_with

			NAME: afc_string_ends_with ( str, search, length ) - Checks if string ends with search string

	SYNOPSIS: int afc_string_ends_with ( const char * str, const char * search, unsigned long length )

		 DESCRIPTION: Determines whether a string ends with the characters of a specified string.

		 INPUT: - str				- The string to check.
			- search 			- The characters to be searched for at the end of this string.
			- length 			- If provided, it is used as the length of str. Defaults to str.length.

		RESULT: - 1 if the string ends with the characters of the search string, 0 otherwise.

	SEE ALSO: - afc_string_starts_with()

@endnode
*/
int afc_string_ends_with(const char *str, const char *search, unsigned long length)
{
	unsigned long len, search_len;

	if (str == NULL || search == NULL)
		return 0;

	len = afc_string_len(str);
	search_len = strlen(search);

	if ((long)length == ALL || length > len)
		length = len;

	if (search_len > length)
		return 0;

	return (strncmp(str + length - search_len, search, search_len) == 0);
}
// }}}

// {{{ afc_string_repeat ( dest, str, count )
/*
@node afc_string_repeat

			NAME: afc_string_repeat ( dest, str, count ) - Returns a new string containing the specified number of copies of the given string

	SYNOPSIS: char * afc_string_repeat ( char * dest, const char * str, unsigned long count )

		 DESCRIPTION: Returns a new string containing the specified number of copies of the given string.

		 INPUT: - dest				- The destination string.
			- str 				- The string to repeat.
			- count 			- The number of times to repeat the string.

		RESULT: - The destination string.

	SEE ALSO:

@endnode
*/
char *afc_string_repeat(char *dest, const char *str, unsigned long count)
{
	unsigned long i;

	if (dest == NULL)
		return NULL;

	afc_string_clear(dest);

	if (str == NULL || count == 0)
		return dest;

	for (i = 0; i < count; i++)
	{
		afc_string_add(dest, str, ALL);
	}

	return dest;
}
// }}}

// {{{ afc_string_replace ( dest, str, pattern, replacement )
/*
@node afc_string_replace

			NAME: afc_string_replace ( dest, str, pattern, replacement ) - Replaces the first occurrence of pattern with replacement

	SYNOPSIS: char * afc_string_replace ( char * dest, const char * str, const char * pattern, const char * replacement )

		 DESCRIPTION: Returns a new string with the first match of a pattern replaced by a replacement.

		 INPUT: - dest				- The destination string.
			- str 				- The source string.
			- pattern 			- The string pattern to replace.
			- replacement 			- The replacement string.

		RESULT: - The destination string.

	SEE ALSO: - afc_string_replace_all()

@endnode
*/
char *afc_string_replace(char *dest, const char *str, const char *pattern, const char *replacement)
{
	char *pos;
	unsigned long offset;

	if (dest == NULL)
		return NULL;

	if (str == NULL)
	{
		afc_string_clear(dest);
		return dest;
	}

	if (pattern == NULL || replacement == NULL)
	{
		afc_string_copy(dest, str, ALL);
		return dest;
	}

	pos = strstr(str, pattern);

	if (pos == NULL)
	{
		afc_string_copy(dest, str, ALL);
		return dest;
	}

	offset = pos - str;

	afc_string_copy(dest, str, offset);
	afc_string_add(dest, replacement, ALL);
	afc_string_add(dest, pos + strlen(pattern), ALL);

	return dest;
}
// }}}

// {{{ afc_string_replace_all ( dest, str, pattern, replacement )
/*
@node afc_string_replace_all

			NAME: afc_string_replace_all ( dest, str, pattern, replacement ) - Replaces all occurrences of pattern with replacement

	SYNOPSIS: char * afc_string_replace_all ( char * dest, const char * str, const char * pattern, const char * replacement )

		 DESCRIPTION: Returns a new string with all matches of a pattern replaced by a replacement.

		 INPUT: - dest				- The destination string.
			- str 				- The source string.
			- pattern 			- The string pattern to replace.
			- replacement 			- The replacement string.

		RESULT: - The destination string.

	SEE ALSO: - afc_string_replace()

@endnode
*/
char *afc_string_replace_all(char *dest, const char *str, const char *pattern, const char *replacement)
{
	const char *current_pos = str;
	char *next_pos;
	unsigned long pattern_len;

	if (dest == NULL)
		return NULL;

	afc_string_clear(dest);

	if (str == NULL)
		return dest;

	if (pattern == NULL || replacement == NULL || *pattern == '\0')
	{
		afc_string_copy(dest, str, ALL);
		return dest;
	}

	pattern_len = strlen(pattern);

	while ((next_pos = strstr(current_pos, pattern)) != NULL)
	{
		afc_string_add(dest, current_pos, next_pos - current_pos);
		afc_string_add(dest, replacement, ALL);
		current_pos = next_pos + pattern_len;
	}

	afc_string_add(dest, current_pos, ALL);

	return dest;
}
// }}}

// {{{ afc_string_pad_start ( dest, str, targetLength, padString )
/*
@node afc_string_pad_start

			NAME: afc_string_pad_start ( dest, str, targetLength, padString ) - Pads the current string with a given string from the start

	SYNOPSIS: char * afc_string_pad_start ( char * dest, const char * str, unsigned long targetLength, const char * padString )

		 DESCRIPTION: Pads the current string with a given string (repeated, if needed) so that the resulting string reaches a given length. Padding is applied from the start.

		 INPUT: - dest				- The destination string.
			- str 				- The source string.
			- targetLength 			- The length of the resulting string once the current string has been padded.
			- padString 			- The string to pad the current string with. Defaults to " ".

		RESULT: - The destination string.

	SEE ALSO: - afc_string_pad_end()

@endnode
*/
char *afc_string_pad_start(char *dest, const char *str, unsigned long targetLength, const char *padString)
{
	unsigned long str_len, pad_len, padding_needed, i;
	const char *pad = (padString != NULL) ? padString : " ";

	if (dest == NULL)
		return NULL;

	if (str == NULL)
		str = "";

	str_len = afc_string_len(str);

	if (str_len >= targetLength)
	{
		afc_string_copy(dest, str, ALL);
		return dest;
	}

	padding_needed = targetLength - str_len;
	pad_len = strlen(pad);

	afc_string_clear(dest);

	for (i = 0; i < padding_needed; i++)
	{
		// Add one char at a time from pad string
		char c[2] = {pad[i % pad_len], '\0'};
		afc_string_add(dest, c, ALL);
	}

	afc_string_add(dest, str, ALL);

	return dest;
}
// }}}

// {{{ afc_string_pad_end ( dest, str, targetLength, padString )
/*
@node afc_string_pad_end

			NAME: afc_string_pad_end ( dest, str, targetLength, padString ) - Pads the current string with a given string from the end

	SYNOPSIS: char * afc_string_pad_end ( char * dest, const char * str, unsigned long targetLength, const char * padString )

		 DESCRIPTION: Pads the current string with a given string (repeated, if needed) so that the resulting string reaches a given length. Padding is applied from the end.

		 INPUT: - dest				- The destination string.
			- str 				- The source string.
			- targetLength 			- The length of the resulting string once the current string has been padded.
			- padString 			- The string to pad the current string with. Defaults to " ".

		RESULT: - The destination string.

	SEE ALSO: - afc_string_pad_start()

@endnode
*/
char *afc_string_pad_end(char *dest, const char *str, unsigned long targetLength, const char *padString)
{
	unsigned long str_len, pad_len, padding_needed, i;
	const char *pad = (padString != NULL) ? padString : " ";

	if (dest == NULL)
		return NULL;

	if (str == NULL)
		str = "";

	str_len = afc_string_len(str);

	afc_string_copy(dest, str, ALL);

	if (str_len >= targetLength)
		return dest;

	padding_needed = targetLength - str_len;
	pad_len = strlen(pad);

	for (i = 0; i < padding_needed; i++)
	{
		// Add one char at a time from pad string
		char c[2] = {pad[i % pad_len], '\0'};
		afc_string_add(dest, c, ALL);
	}

	return dest;
}
// }}}

// {{{ afc_string_slice ( dest, str, beginIndex, endIndex )
/*
@node afc_string_slice

			NAME: afc_string_slice ( dest, str, beginIndex, endIndex ) - Extracts a section of a string

	SYNOPSIS: char * afc_string_slice ( char * dest, const char * str, long beginIndex, long endIndex )

		 DESCRIPTION: Extracts a section of a string and returns it as a new string (in dest), without modifying the original string.

		 INPUT: - dest				- The destination string.
			- str 				- The source string.
			- beginIndex 			- The zero-based index at which to begin extraction. If negative, it is treated as strLength + beginIndex.
			- endIndex 			- The zero-based index before which to end extraction. The character at this index will not be included. If negative, it is treated as strLength + endIndex. To extract to the end of the string, pass a value >= string length (e.g. LONG_MAX).

		RESULT: - The destination string.

	SEE ALSO: - afc_string_mid()

@endnode
*/
char *afc_string_slice(char *dest, const char *str, long beginIndex, long endIndex)
{
	unsigned long len;
	long start, end;

	if (dest == NULL)
		return NULL;

	afc_string_clear(dest);

	if (str == NULL)
		return dest;

	len = afc_string_len(str);
	start = beginIndex;
	end = endIndex;

	// Handle negative start
	if (start < 0)
	{
		start = len + start;
	}

	// Clamp start
	if (start < 0)
		start = 0;
	if (start >= (long)len)
		return dest; // Empty

	// Handle negative end
	if (end < 0)
	{
		end = len + end;
	}

	// Clamp end
	if (end > (long)len)
		end = len;
	if (end < 0)
		end = 0;

	if (end <= start)
		return dest; // Empty

	return afc_string_copy(dest, str + start, end - start);
}
// }}}

// {{{ afc_string_index_of ( str, search, fromIndex )
/*
@node afc_string_index_of

			NAME: afc_string_index_of ( str, search, fromIndex ) - Returns the index of the first occurrence of the specified value

	SYNOPSIS: long afc_string_index_of ( const char * str, const char * search, long fromIndex )

		 DESCRIPTION: Returns the index of the first occurrence of the specified value, or -1 if not found.

		 INPUT: - str				- The string to search in.
			- search 			- The string to search for.
			- fromIndex 			- The index to start the search from.

		RESULT: - The index of the first occurrence, or -1 if not found.

	SEE ALSO: - afc_string_last_index_of()

@endnode
*/
long afc_string_index_of(const char *str, const char *search, long fromIndex)
{
	long len;
	char *p;

	if (str == NULL || search == NULL)
		return -1;

	len = afc_string_len(str);

	if (fromIndex < 0)
		fromIndex = 0;

	if (fromIndex >= len)
		return -1;

	p = strstr(str + fromIndex, search);

	if (p)
		return (long)(p - str);

	return -1;
}
// }}}

// {{{ afc_string_last_index_of ( str, search, fromIndex )
/*
@node afc_string_last_index_of

			NAME: afc_string_last_index_of ( str, search, fromIndex ) - Returns the index of the last occurrence of the specified value

	SYNOPSIS: long afc_string_last_index_of ( const char * str, const char * search, long fromIndex )

		 DESCRIPTION: Returns the index of the last occurrence of the specified value, searching backwards from fromIndex.

		 INPUT: - str				- The string to search in.
			- search 			- The string to search for.
			- fromIndex 			- The index to start the search backwards from.

		RESULT: - The index of the last occurrence, or -1 if not found.

	SEE ALSO: - afc_string_index_of()

@endnode
*/
long afc_string_last_index_of(const char *str, const char *search, long fromIndex)
{
	long len, search_len, i;

	if (str == NULL || search == NULL)
		return -1;

	len = afc_string_len(str);
	search_len = strlen(search);

	if (search_len == 0)
		return (fromIndex >= len) ? len : fromIndex;

	if (fromIndex < 0)
		return -1; // JS behavior: if fromIndex < 0, treated as 0, but since we search backwards from 0, only match at 0 is possible if searchLen is 0, else -1. Actually JS treats negative as 0.

	if (fromIndex >= len)
		fromIndex = len;

	// Start searching from min(fromIndex, len - searchLen)
	i = fromIndex;
	if (i > len - (long)search_len)
		i = len - (long)search_len;

	for (; i >= 0; i--)
	{
		if (strncmp(str + i, search, search_len) == 0)
			return i;
	}

	return -1;
}
// }}}

// {{{ afc_string_trim_start ( str )
/*
@node afc_string_trim_start

			NAME: afc_string_trim_start ( str ) - Removes whitespace from the beginning of a string

	SYNOPSIS: char * afc_string_trim_start ( char * str )

		 DESCRIPTION: Removes whitespace from the beginning of a string. Modifies the string in place.

		 INPUT: - str				- The string to trim.

		RESULT: - The trimmed string.

	SEE ALSO: - afc_string_trim_end()

@endnode
*/
char *afc_string_trim_start(char *str)
{
	char *p;

	if (str == NULL)
		return NULL;

	p = str;
	while (isspace((unsigned char)*p))
		p++;

	if (p != str)
	{
		afc_string_copy(str, p, ALL);
	}

	return str;
}
// }}}

// {{{ afc_string_trim_end ( str )
/*
@node afc_string_trim_end

			NAME: afc_string_trim_end ( str ) - Removes whitespace from the end of a string

	SYNOPSIS: char * afc_string_trim_end ( char * str )

		 DESCRIPTION: Removes whitespace from the end of a string. Modifies the string in place.

		 INPUT: - str				- The string to trim.

		RESULT: - The trimmed string.

	SEE ALSO: - afc_string_trim_start()

@endnode
*/
char *afc_string_trim_end(char *str)
{
	long len;
	char *p;

	if (str == NULL)
		return NULL;

	len = afc_string_len(str);
	if (len == 0)
		return str;

	p = str + len - 1;
	while (p >= str && isspace((unsigned char)*p))
		p--;

	*(p + 1) = '\0';
	afc_string_reset_len(str);

	return str;
}
// }}}

// {{{ afc_string_from_char_code ( code )
/*
@node afc_string_from_char_code

			NAME: afc_string_from_char_code ( code ) - Returns a string created from the specified UTF-16 code unit

	SYNOPSIS: char * afc_string_from_char_code ( int code )

		 DESCRIPTION: Returns a string created from the specified char code.

		 INPUT: - code				- The char code.

		RESULT: - A new AFC string containing the character.

	SEE ALSO:

@endnode
*/
char *afc_string_from_char_code(int code)
{
	char *s = afc_string_new(1);
	if (s)
	{
		s[0] = (char)code;
		s[1] = '\0';
		afc_string_reset_len(s);
	}
	return s;
}
// }}}

#ifdef TEST_CLASS
// {{{ test
/*
int main(void)
{
	 char * m, *n, *dup;

	 m = afc_string_new(25);
	 n = afc_string_new(25);

	 printf("afc_string_new Max: %ld\n",afc_string_max(m));
	 printf("afc_string_new: \"%s\"\n",afc_string_copy(m,"Ciao Mamma", ALL));
	 printf("afc_string_new: \"%s\", Len: %ld\n",m,afc_string_len(m));

	 afc_string_mid(n,m,5,10);
	 printf("%s - Len: %ld\n",n, afc_string_len(n));

	 afc_string_copy(m, "AAAB", ALL);
	 afc_string_copy(n, "AAAA", ALL);

	 printf("Compare: %ld\n", afc_string_comp(m,n, ALL));
	 printf("Compare: %ld\n", afc_string_comp(m,n,3));

	 afc_string_copy(m, "ciao mamma!", ALL);
	 printf("%s\n", m);
	 afc_string_upper(m);
	 printf("%s\n", m);
	 afc_string_lower(m);
	 printf("%s\n", m);

	 afc_string_copy(m, "	 	ciao mamma!		", ALL);
	 afc_string_trim(m);
	 printf("-%s-\n", m);
	 afc_string_copy(m, "	 			", ALL);
	 afc_string_trim(m);
	 printf("-%s- %ld\n", m, afc_string_len(m));

	 afc_string_copy( m, "super califragilistichespiralidoso anche se da dire pu� sembrare spaventoso", ALL);
	 printf("-%s- %ld\n", m, afc_string_len(m));

	 printf("%s\n", afc_string_instr(m, "fragi", 0));

	 afc_string_left ( n, m, 5);
	 printf("-%s- %ld\n", n, afc_string_len(n));

	 afc_string_right ( n, "ciao mamma", 4);
	 printf("-%s- %ld\n", n, afc_string_len(n));

	dup = afc_string_dup ( n );
	printf("Duplicate of %s is %s\n", n, dup );

		afc_string_make ( n, "%s 123 %d", "ciao", 321 );

	printf ( "Make: %s\n", n );


	 afc_string_delete(dup);
	 afc_string_delete(m);
	 afc_string_delete(n);

	 return ( 0 );
}
*/
// }}}
#endif
