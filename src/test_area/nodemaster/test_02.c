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
#include "../test_utils.h"


void add_three ( NodeMaster * am )
{
	afc_nodemaster_add ( am, (void * )1, AFC_NODEMASTER_ADD_TAIL );
	afc_nodemaster_add ( am, (void * )2, AFC_NODEMASTER_ADD_TAIL );
	afc_nodemaster_add ( am, (void * )3, AFC_NODEMASTER_ADD_TAIL );
}

int main ()
{
	AFC	* afc;
	NodeMaster * am;

	if ( ( afc = afc_new () ) == NULL ) exit ( 1 );

	if ( ( am = afc_nodemaster_new ( ) ) == NULL ) exit ( 1 );

	add_three ( am );

	test_header ();

	print_res ( "AFC Base", afc, __internal_afc_base, 0 );

	print_row ();

	print_res ( "first", ( void * ) 1 , afc_nodemaster_first ( am ) ,0);
	print_res ( "del", ( void * ) 2 , afc_nodemaster_del ( am ) ,0);
	print_res ( "del", ( void * ) 3 , afc_nodemaster_del ( am ) ,0);
	print_res ( "del", ( void * ) 0 , afc_nodemaster_del ( am ) ,0);
	print_res ( "first", ( void * ) 0 , afc_nodemaster_first ( am ) ,0);
	print_res ( "empty", ( void * ) 1 , ( void * ) ( int ) afc_nodemaster_is_empty ( am ) ,0);

	print_row ();

	add_three ( am );

	print_res ( "item(1)", ( void * ) 2 , afc_nodemaster_item ( am, 1 ) ,0);
	print_res ( "del", ( void * ) 3 , afc_nodemaster_del ( am ) ,0);
	print_res ( "del", ( void * ) 1 , afc_nodemaster_del ( am ) ,0);
	print_res ( "del", ( void * ) 0 , afc_nodemaster_del ( am ) ,0);
	print_res ( "empty", ( void * ) 1 , ( void * ) ( int ) afc_nodemaster_is_empty ( am ) ,0);

	print_row ();

	add_three ( am );

	print_res ( "last", ( void * ) 3 , afc_nodemaster_last ( am ) ,0);
	print_res ( "del", ( void * ) 2 , afc_nodemaster_del ( am ) ,0);
	print_res ( "del", ( void * ) 1 , afc_nodemaster_del ( am ) ,0);
	print_res ( "del", ( void * ) 0 , afc_nodemaster_del ( am ) ,0);
	print_res ( "empty", ( void * ) 1 , ( void * ) ( int ) afc_nodemaster_is_empty ( am ) ,0);
	
	print_row ();

	add_three ( am );

	print_res ( "clear", ( void * ) AFC_ERR_NO_ERROR, ( void * ) afc_nodemaster_clear ( am ), 0 );
	print_res ( "empty", ( void * ) 1 , ( void * ) ( int ) afc_nodemaster_is_empty ( am ) ,0);

	print_summary ();

	afc_nodemaster_clear ( am );

	printf ( "Empty: %d\n", afc_nodemaster_is_empty ( am ) );
	afc_nodemaster_clear ( am );
	printf ( "Empty: %d\n", afc_nodemaster_is_empty ( am ) );
	afc_nodemaster_clear ( am );
	printf ( "Empty: %d\n", afc_nodemaster_is_empty ( am ) );
	afc_nodemaster_clear ( am );
	

	afc_nodemaster_delete ( am );

	afc_delete ( afc );

	return ( 0 );
}
