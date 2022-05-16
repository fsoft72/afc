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

	test_header ();

	re = afc_regexp_new ();

	print_res ( "set_expression()",    AFC_ERR_NO_ERROR, ( void * ) afc_regexp_compile ( re, "test" ), 0 );
	print_res ( "match ( \"test\" )",  AFC_ERR_NO_ERROR, ( void * ) afc_regexp_match ( re, "this is a test", 0 ), 0 );
	print_res ( "match ( \"dump\" )",  ( void * ) AFC_REGEXP_ERR_NO_MATCH, ( void * ) afc_regexp_match ( re, "dump", 0 ), 0 );

	print_row ();

	print_res ( "set_expression(\"[A-Z][a-z]*\" )", AFC_ERR_NO_ERROR, ( void * ) afc_regexp_compile ( re, "[A-Z][a-z]*" ), 0 );
	print_res ( "match ( \"Fabio\" )",              AFC_ERR_NO_ERROR, ( void * ) afc_regexp_match ( re, "Fabio", 0 ), 0 );
	print_res ( "match ( \"fabio\" )",              ( void * ) AFC_REGEXP_ERR_NO_MATCH,  ( void * ) afc_regexp_match ( re, "fabio", 0 ), 0 );
	print_row ();

	print_res ( "replace ()", AFC_ERR_NO_ERROR, ( void * ) afc_regexp_replace ( re, dest, "aabbcc", "(.*)bb(.*)", "\\2bb\\1", TRUE ), 0  );
	print_res ( "... result", ( void * ) 0, ( void * ) strcmp ( dest, "ccbbaa" ), 0 );

	print_res ( "replace ()", AFC_ERR_NO_ERROR, ( void * ) afc_regexp_replace ( re, dest, "aabbcc", "(.*)zz(.*)", "\\2xx\\1", TRUE ), 0  );
	print_res ( "... result", ( void * ) 0, ( void * ) strcmp ( dest, "aabbcc" ), 0 );

	print_row ();
	print_summary ();

	afc_regexp_delete ( re );
	afc_string_delete ( dest );

	afc_delete ( afc );

	return ( 0 );
}

