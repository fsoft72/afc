/* 
 * Advanced Foundation Classes
 * Copyright (C) 2000/2004  Fabio Rotondo 
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
	TITLE:     RegExp
	VERSION:   2.00
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
@endnode
*/

#include "regexp.h"

static const char class_name[] = "RegExp";

static int afc_regexp_internal_replace ( RegExp * regexp, char * replace );

// {{{ docs
/*
@node quote
	*The 'Internet' cannot be removed from your desktop, would you like to delete the 'Internet' now?*

		MS Windows '95 
@endnode

@node intro
RegExp will help you create regular expressions inside your code.
@endnode
// }}}
// {{{ afc_regexp_new ()
@node afc_regexp_new

		 NAME: afc_regexp_new	()	 - Initializes a new afc_regexp instance.

			 SYNOPSIS: RegExp * afc_regexp_new ( void )

		DESCRIPTION: This function initializes a new afc_regexp instance.

		INPUT: - NONE

	RESULTS: a valid inizialized afc_regexp structure. NULL in case of errors.

			 SEE ALSO: - afc_regexp_delete()

@endnode
*/
RegExp * afc_regexp_new ()
{
TRY ( RegExp * )

	RegExp * regexp = ( RegExp * ) afc_malloc ( sizeof ( struct afc_regexp ) );

	if ( regexp == NULL ) RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "regexp", NULL );
	regexp->magic = AFC_REGEXP_MAGIC;

	if ( ( regexp->buffer = afc_string_new ( AFC_REGEXP_BUFFER ) ) == NULL ) RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "buffer", NULL );
	if ( ( regexp->ovector = afc_malloc ( sizeof ( int ) * AFC_REGEXP_MAX_OSIZE ) ) == NULL ) RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "ovector", NULL );

	RETURN ( regexp );

EXCEPT
	afc_regexp_delete ( regexp );

FINALLY

ENDTRY
}
// }}}
// {{{ afc_regexp_delete ( re )
/*
@node afc_regexp_delete

		 NAME: afc_regexp_delete ( regexp )	- Disposes a valid regexp instance.

			 SYNOPSIS: int afc_regexp_delete (RegExp * regexp)

		DESCRIPTION: This function frees an already alloc'd afc_regexp structure.

		INPUT: - regexp	- Pointer to a valid afc_regexp class.

	RESULTS: should be AFC_ERR_NO_ERROR

		NOTES: - this method calls: afc_regexp_clear()

			 SEE ALSO: - afc_regexp_new()
		 - afc_regexp_clear()
@endnode
*/
int _afc_regexp_delete ( RegExp * regexp ) 
{
	int afc_res; 

	if ( ( afc_res = afc_regexp_clear ( regexp ) ) != AFC_ERR_NO_ERROR ) return ( afc_res );

	afc_string_delete ( regexp->str );
	afc_string_delete ( regexp->buffer );
	afc_free ( regexp->ovector );

	afc_free ( regexp );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_regexp_clear ( re )
/*
@node afc_regexp_clear

		 NAME: afc_regexp_clear ( regexp )	- Clears all stored data

			 SYNOPSIS:	int afc_regexp_clear ( RegExp * regexp )

		DESCRIPTION: Use this command to clear all stored data in the current regexp instance.

		INPUT: - regexp		- Pointer to a valid afc_regexp instance.

	RESULTS: should be AFC_ERR_NO_ERROR
		
			 SEE ALSO: - afc_regexp_delete()
		 
@endnode
*/
int afc_regexp_clear ( RegExp * regexp ) 
{
	if ( regexp == NULL ) return ( AFC_LOG_FAST ( AFC_ERR_NULL_POINTER ) );
 
	if ( regexp->magic != AFC_REGEXP_MAGIC ) return ( AFC_LOG_FAST ( AFC_ERR_INVALID_POINTER ) );

	if ( regexp->str	) afc_string_delete ( regexp->str );

	if ( regexp->pattern )	afc_free ( regexp->pattern );
	if ( regexp->hints ) 	afc_free ( regexp->hints );	

	regexp->str		= NULL;
	regexp->ready		= FALSE;
	regexp->matches 	= 0;
	regexp->pattern		= NULL;
	regexp->hints		= NULL;

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_regexp_compile ( re, exp )
/*
@node afc_regexp_compile

		 NAME: afc_regexp_compile ( regexp, exp ) - Compiles the Regular Expression for the matches

			 SYNOPSIS: int afc_regexp_compile ( RegExp * regexp, const char * str )

		DESCRIPTION: This function prepares the Regular Expression for the next afc_regexp_match() calls.

		INPUT: - regexp		- Pointer to a valid afc_regexp class.
		       - exp		- The Regular Expression to compile

	RESULTS: should be AFC_ERR_NO_ERROR

		NOTES: - You *must* call this function at least once or the afc_regexp_match() will not work.

			 SEE ALSO: - afc_regexp_match()
@endnode
*/
int afc_regexp_compile ( RegExp * regexp, const char * str )
{
	char buf [ 1024 ];
	const char * error;
	int 	errpos;
	
	afc_regexp_clear ( regexp );

	regexp->pattern = pcre_compile ( str, regexp->options, & error, & errpos, NULL );

	if ( regexp->pattern == NULL ) 
	{
		sprintf ( buf, "Error in expression. Offset: %d: %s", errpos, error );
		return ( AFC_LOG ( AFC_LOG_ERROR, AFC_REGEXP_ERR_COMPILING, buf, str ) );
	}

	regexp->hints	= pcre_study ( regexp->pattern, 0, & error );
	if ( error != NULL )
	{
		sprintf ( buf, "Error while studying regexp: %s", error );
		return ( AFC_LOG ( AFC_LOG_ERROR, AFC_REGEXP_ERR_COMPILING, error, str ) );
	}
 
	regexp->ready = TRUE;
 
	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_regexp_match ( re, str, startpos )
/*
@node afc_regexp_match

		 NAME: afc_regexp_match ( regexp, str, startpos ) - Matches the str with the current Regular Expression

			 SYNOPSIS: int afc_regexp_match ( RegExp * regexp, const char * str, const int startpos )

		DESCRIPTION: This function applies the current Regular Expression (created with afc_regexp_compile () )
		 to the string provided.

		INPUT: - regexp	- Pointer to a valid afc_regexp class.
		 - str		 - The string to match againts the current Regular Expression

	RESULTS: - AFC_ERR_NO_ERROR - match succeded
		 - AFC_REGEXP_ERR_NO_MATCH	- String didn't match the pattern

		NOTES: - This function can handle NULL strings

	 SEE ALSO: - afc_regexp_compile()
		 - afc_regexp_get_sub_string()
	
@endnode
*/
int afc_regexp_match ( RegExp * regexp, const char * str, const int startpos )
{
	if ( ( str == NULL ) || ( strlen ( str ) == 0 ) ) return ( 1 );		// No string provided for the match
	if ( regexp->ready == FALSE ) return ( AFC_LOG ( AFC_LOG_WARNING, AFC_REGEXP_ERR_NOT_READY, "RegExp not ready. Missing compile()?", NULL ) );


	if ( regexp->str ) afc_string_delete ( regexp->str );
	if ( ( regexp->str = afc_string_dup ( str ) ) == NULL ) return ( AFC_LOG_FAST ( AFC_ERR_NO_MEMORY ) );

	regexp->matches = pcre_exec ( regexp->pattern, regexp->hints, str, strlen ( str ), startpos, regexp->options, regexp->ovector, AFC_REGEXP_MAX_OSIZE );

	if ( regexp->matches <= 0 ) return ( AFC_REGEXP_ERR_NO_MATCH );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_regexp_get_pos ( re, pos, coords )
/*
@node afc_regexp_get_pos

		 NAME: afc_regexp_get_pos ( regexp, pos, coords ) - Returns the offsets of the string matching the RE

			 SYNOPSIS: int afc_regexp_get_pos ( RegExp * regexp, int pos, struct afc_regexp_pos * coords )

		DESCRIPTION: This function returns the relative coordinates (start/end) of the particular sub string match.

		INPUT: - regexp	- Pointer to a valid afc_regexp class.
		 - pos		 - The substring ordinal position you want to know the coords of.
		 - coords	- A pointer to an already allocated afc_regexp_pos structure.

	RESULTS: should be AFC_ERR_NO_ERROR

		NOTES: - in case the desired sub string has not been matched, the values contained in the coords
			 structure will be -1.

			 SEE ALSO: - afc_regexp_get_sub_string()
	
@endnode
*/
int afc_regexp_get_pos ( RegExp * regexp, int pos, struct afc_regexp_pos * retval )
{
	if ( regexp->matches <= 0 ) return ( AFC_LOG ( AFC_LOG_NOTICE, AFC_REGEXP_ERR_NO_MATCH, "No match for current expression", NULL ) );
	if ( pos > regexp->matches ) return ( AFC_LOG ( AFC_LOG_NOTICE, AFC_REGEXP_ERR_OUT_OF_BOUNDS, "Queried result out of bounds", NULL ) );

	retval->start = regexp->ovector [ pos * 2 ];
	retval->end   = regexp->ovector [ ( pos * 2 ) + 1 ];

	return ( AFC_ERR_NO_ERROR ); 
}
// }}}
// {{{ afc_regexp_get_sub_string ( re, dest, pos )
/*
@node afc_regexp_get_sub_string

		 NAME: afc_regexp_get_sub_string ( regexp, dest, pos ) - Returns the coords of a substring match.

			 SYNOPSIS: int afc_regexp_sub_string ( RegExp * regexp, char * dest, int pos )

		DESCRIPTION: This function returns the relative coordinates (start/end) of the particular sub string match.

		INPUT: - regexp		- Pointer to a valid afc_regexp class.
		       - dest			- Destination string. It *MUST* be an AFC afc_string_new.
		 - pos			- The substring ordinal position you want to know the coords of.

	RESULTS: should be AFC_ERR_NO_ERROR

		NOTES: - in case the desired sub string has not been matched, the values contained in the coords
			 structure will be -1.

			 SEE ALSO: - afc_regexp_get_pos()
	
@endnode
*/
int afc_regexp_get_sub_string ( RegExp * regexp, char * dest, int pos )
{
	struct afc_regexp_pos coords;
	int res, len;

	if ( ( res = afc_regexp_get_pos ( regexp, pos, &coords ) ) != AFC_ERR_NO_ERROR ) return ( res );

	if ( ( coords.start == -1 ) || ( coords.start >= coords.end ) )
	{
		afc_string_copy ( dest, "", ALL );
		return ( AFC_ERR_NO_ERROR ); 
	} 

	//printf("Start: %d\n", coords.start );
	//printf("End	: %d\n", coords.end );

	len = coords.end - coords.start;

	afc_string_copy ( dest, regexp->str + coords.start, len );

	return ( AFC_ERR_NO_ERROR );
} 
// }}}
// {{{ afc_regexp_set_buffer ( re, size )
/*
@node afc_regexp_set_buffer

		 NAME: afc_regexp_set_buffer()		 - Sets buffer for replace operations

		 SYNOPSIS: int afc_regexp_set_buffer ( RegExp * regexp, int size )

		DESCRIPTION: Sets the buffer for all the afc_regexp_replace() operation.
	               At startup, the buffer is set to a default value, but it can
	               be changed at any time using this method.

		INPUT: - regexp		- Pointer to a valid afc_regexp class.
	               - size			- New dimension (in bytes) of the buffer

	RESULTS: should be AFC_ERR_NO_ERROR

			 SEE ALSO: - afc_regexp_replace()

@endnode
*/
int afc_regexp_set_buffer ( RegExp * regexp, int size ) 
{
	if ( regexp->buffer ) afc_string_delete ( regexp->buffer );

	if ( ( regexp->buffer = afc_string_new ( size ) ) == NULL ) 
		return ( AFC_LOG_FAST ( AFC_ERR_NO_MEMORY ) );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_regexp_replace ( re, dest, str, str_re, replace, replace_all )
/*
@node afc_regexp_replace

		 NAME: afc_regexp_replace ()		 - Replaces a string using RE patterns

	 SYNOPSIS: int afc_regexp_replace ( RegExp * regexp, char * dest, char * src, char * str_re, char * replace, short replace_all )

	DESCRIPTION: With this method you can replace a string or a part of it using Regular Expression as pattern matching.
	               Rules are almost the same as the sed command or the vi editor. Only, it does not have sense use the
	               "^" operator with the replace_all set to TRUE.

	INPUT: - regexp	- Pointer to a valid afc_regexp class.
               - dest		- Destination string where the result of substitution will be saved.
                              This  string MUST be an AFC afc_string_new.
		 - src      		- Source string
	               - str_re	- afc_string_new defining the RE used as search pattern 
		 - replace			- afc_string_new containing the replace mask
		 - replace_all	- Flag TRUE/FALSE. If TRUE the replacement will take place in all possible places of the string,
	                                if FALSE the replace will take place only once.

	RESULTS: should be AFC_ERR_NO_ERROR

			 SEE ALSO: - afc_regexp_compute_replace_size()
@endnode
*/
int afc_regexp_replace ( RegExp * regexp, char * dest, char * str, char * str_re, char * replace, short replace_all )
{
	int err;
	struct afc_regexp_pos coords;
	char * buf;
	int    start_pos = 0, new_start_pos;

	if ( ( err = afc_regexp_compile ( regexp, str_re ) ) != AFC_ERR_NO_ERROR ) return ( err );

	if ( ( buf = afc_string_new ( afc_string_max ( dest ) ) ) == NULL ) return ( AFC_ERR_NO_MEMORY );

	afc_string_copy ( buf, str, ALL );
	afc_string_copy ( regexp->buffer, replace, ALL );

	start_pos = 0;
	regexp->replaces = 0;
	while ( ( afc_regexp_match ( regexp, buf,  start_pos ) ) == AFC_ERR_NO_ERROR ) 
	{
		if ( ( err = afc_regexp_get_pos        ( regexp, 0, &coords ) )           != AFC_ERR_NO_ERROR ) return ( err );
		if ( ( err = afc_regexp_internal_replace ( regexp, regexp->buffer ) )     != AFC_ERR_NO_ERROR ) return ( err );

		afc_string_clear ( dest );

	  	//coords.start += start_pos;
	  	//coords.end   += start_pos;

		if ( coords.start > 0 ) afc_string_copy ( dest, buf, coords.start );
		afc_string_add ( dest, regexp->buffer, ALL );
	  	new_start_pos = strlen ( dest );

		afc_string_add ( dest, buf + coords.end, ALL );

		// afc_string_reset_len ( dest );

	  	afc_string_copy ( buf, dest, ALL );

	  	if ( replace_all == FALSE ) break;

	  	start_pos = new_start_pos;
		regexp->replaces ++;
	}
	
	afc_string_delete ( buf );	

	if ( regexp->replaces == 0 ) afc_string_copy ( dest, str, ALL );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_regexp_compute_replace_size ( regexp, str, str_re, replace, replace_all )
/*
@node afc_regexp_compute_replace_size

		 NAME: afc_regexp_compute_replace_size () - Compute the final size of a string replaced

	 SYNOPSIS: int afc_regexp_compute_replace_size ( RegExp * regexp, char * src, char * str_re, char * replace, short replace_all )

	DESCRIPTION: With this method you can compute the size of the string you'll obtain by calling afc_regexp_replace() method.
		     This is useful to set the buffer size when you suspect it will be no big enought to store all the replaced string.

	INPUT:   - regexp	- Pointer to a valid afc_regexp class.
		 - src      		- Source string
	         - str_re	- afc_string_new defining the RE used as search pattern 
		 - replace	- afc_string_new containing the replace mask
		 - replace_all	- Flag TRUE/FALSE. If TRUE the replacement will take place in all possible places of the string,
	                          if FALSE the replace will take place only once.

	RESULTS: the size of the replace string or -1 in case of any error (tipically: RegExp compilation error).

	SEE ALSO: - afc_regexp_replace()
@endnode
*/
int afc_regexp_compute_replace_size ( RegExp * regexp, char * str, char * str_re, char * replace, short replace_all )
{
	int err;
	struct afc_regexp_pos coords;
	unsigned int    start_pos = 0, len = 0;

	if ( ( err = afc_regexp_compile ( regexp, str_re ) ) != AFC_ERR_NO_ERROR ) return ( -1 );

	afc_string_copy ( regexp->buffer, replace, ALL );

	while ( ( afc_regexp_match ( regexp, str,  start_pos ) ) == AFC_ERR_NO_ERROR ) 
	{

		if ( ( err = afc_regexp_get_pos ( regexp, 0, &coords ) )           != AFC_ERR_NO_ERROR ) return ( -1 );
		if ( ( err = afc_regexp_internal_replace ( regexp, regexp->buffer ) )     != AFC_ERR_NO_ERROR ) return ( -1 );

		//coords.start += start_pos;
		//coords.end   += start_pos;

		len    	+= strlen ( regexp->buffer );

	  	start_pos = coords.end;

		if ( replace_all == false ) break;
	}

	if ( start_pos < strlen ( str ) ) 
		len += strlen ( str )  - start_pos;

	return ( len );
}
// }}}
// {{{ afc_regexp_set_options ( re, options )
int afc_regexp_set_options ( RegExp * regexp, const int options )
{
	regexp->options = options;

	return ( AFC_ERR_NO_ERROR );
}
// }}}


/*  --------------------------------------------------------------------------------------------------
	  INTERNAL FUNCTIONS
--------------------------------------------------------------------------------------------------  */
// {{{ afc_regexp_internal_replace ( re, replace )
static int afc_regexp_internal_replace ( RegExp * regexp, char * replace ) 
{
	RegExp * tmp_regex = afc_regexp_new ();
	int err;
	RegExpPos coords;
	char * buf, num[4];

	if ( tmp_regex == NULL ) return ( AFC_LOG_FAST_INFO ( AFC_ERR_NO_MEMORY, "tmp_regex" ) );

	if ( ( buf = afc_string_new ( afc_string_max ( regexp->buffer ) ) ) == NULL ) 
	{
		afc_regexp_delete ( tmp_regex );

		return ( AFC_LOG_FAST_INFO ( AFC_ERR_NO_MEMORY, "buf" ) );
	}

	if ( ( err = afc_regexp_set_buffer ( tmp_regex, afc_string_max ( regexp->buffer ) ) )  != AFC_ERR_NO_ERROR )
	{
		afc_string_delete ( buf );
		afc_regexp_delete ( tmp_regex );
	  	return ( err );
	}

	if ( ( err = afc_regexp_compile   ( tmp_regex, "\\\\[0-9]"  ) ) != AFC_ERR_NO_ERROR )
	{
		afc_string_delete ( buf );
		afc_regexp_delete ( tmp_regex );
	  	return ( err );
	}

	while ( ( afc_regexp_match ( tmp_regex, replace, 0 ) ) == AFC_ERR_NO_ERROR )
	{
		if ( ( err = afc_regexp_get_pos ( tmp_regex, 0, &coords ) ) != AFC_ERR_NO_ERROR )
		{
			afc_string_delete ( buf );
			afc_regexp_delete ( tmp_regex );
	  	return ( err );
		}

		afc_string_copy ( tmp_regex->buffer, "", ALL );

		if ( coords.start > 0 ) afc_string_copy ( tmp_regex->buffer, replace, coords.start );

		strncpy ( num, replace + coords.start+1 , coords.end - ( coords.start + 1 ) );
		num[coords.end - ( coords.start + 1 )] = 0;
		afc_regexp_get_sub_string (	regexp, buf, atoi(num) );

		strcat ( tmp_regex->buffer, buf ); 
		strcat ( tmp_regex->buffer, replace + coords.end );

		afc_string_copy ( replace, tmp_regex->buffer, ALL );
	}
	
	afc_string_delete ( buf );
	afc_regexp_delete ( tmp_regex );
	return ( AFC_ERR_NO_ERROR );	
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
int get_field ( RegExp * regex, int f )
{
	char buf [ 255 ];
	int res;

	printf ( "Getting: %d\n", f );
	
	if ( ( res = afc_regexp_get_sub_string ( regex, buf, f ) ) != AFC_ERR_NO_ERROR )
	{
		printf ( " -- Error: %d\n", res - AFC_REGEXP_BASE );
	} else {
		printf ( " -- Str: %s\n", buf );
	}

	return ( 0 );
}


int main ()
{
	RegExp * regex;
	char * dest;

	if ( ( regex = afc_regexp_new () ) == NULL )
	{
		printf ( "Error\n");
		return ( 1 );
	}

/*
	printf("Set: %d\n", afc_regexp_set_expression ( regex, "\\\\[0-9]", FALSE, FALSE ));


	printf("Match: %d\n", afc_regexp_match ( regex, "pippo \\1 Sei \\2" ));

	get_field ( regex, 0 );
	get_field ( regex, 1 );
	get_field ( regex, 2 );
	get_field ( regex, 3 );
*/

	dest=afc_string_new(255);

	//afc_regexp_replace ( gt->regex, tmp, str, "\\\\[^~]*\\~", "", TRUE );
	//afc_regexp_replace ( gt->regex, str, tmp, "<[^>]*>", "", TRUE );

	printf ( "Replace: %d\n", afc_regexp_replace ( regex, dest, "<rifNorm>\\i~Costituzione art. 134", "\\\\[^~]*\\~", "", TRUE ) - AFC_REGEXP_BASE);
/*
	printf ( "Replace: %d\n", afc_regexp_replace ( regex, dest, dest, "<[^>]*>", "", TRUE ) - AFC_REGEXP_BASE);

	printf ( "Replaced: %s ( %d times )\n", dest, regex->replaces );
	printf ( "-------------------------\n");
	printf ( "Replace: %d\n", afc_regexp_replace ( regex, dest, "aaa bbb aaa", "\\(aa*\\)", "\\1", TRUE ) - AFC_REGEXP_BASE);
	printf ( "Replaced: %s ( %d times )\n", dest, regex->replaces );
	*/
	
	printf("Delete: %d\n", afc_regexp_delete ( regex ) ); 

	return ( 0 );
}
// }}}
#endif

