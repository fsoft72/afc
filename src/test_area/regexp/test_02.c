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
#include <stdlib.h>

#include "../test_utils.h"


RegExp * re;

int main ()
{
	AFC * afc = afc_new ();
	char * dest = afc_string_new ( 100 );

	if ( afc == NULL ) return ( 1 );

	// test_header ();

	re = afc_regexp_new ();

/*
	afc_regexp_compute_replace_size ( re, "aabbcc", "\\(.*\\)bb\\(.*\\)", "\\2bb\\1", TRUE );

	afc_regexp_compute_replace_size ( re, "aabbcc", "a", "cccc", TRUE );

	afc_regexp_compute_replace_size ( re, "aabbcc", "\\(.*\\)bbcc", "abc", TRUE );
*/

	afc_regexp_compile ( re, "\\b([0-9][0-9][0-9][0-9])-([01][0-9])-([0-3][0-9])\\b" );
	afc_regexp_match ( re, "the current date is 2004-04-17 and it is a good period.", 0 );

	afc_regexp_get_sub_string ( re, dest, 1 );
	printf ( "Year: %s\n", dest );

	afc_regexp_get_sub_string ( re, dest, 2 );
	printf ( "Month: %s\n", dest );

	afc_regexp_get_sub_string ( re, dest, 3 );
	printf ( "Day: %s\n", dest );

	afc_regexp_get_sub_string ( re, dest, 0 );
	printf ( "Full: %s\n", dest );

	afc_regexp_delete ( re );

	afc_delete ( afc );

	return ( 0 );
}

