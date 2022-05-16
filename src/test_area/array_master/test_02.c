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

AFC	* afc;
ArrayMaster * am;

void add_three ()
{
	afc_array_master_add ( am, (void * )1, AFC_ARRAY_MASTER_ADD_TAIL );
	afc_array_master_add ( am, (void * )2, AFC_ARRAY_MASTER_ADD_TAIL );
	afc_array_master_add ( am, (void * )3, AFC_ARRAY_MASTER_ADD_TAIL );
}

int main ()
{
	if ( ( afc = afc_new () ) == NULL ) exit ( 1 );

	if ( ( am = afc_array_master_new ( ) ) == NULL ) exit ( 1 );

	add_three ();

	test_header ();

	print_res ( "AFC Base", afc, __internal_afc_base, 0 );

	print_row ();

	print_res ( "first", ( void * ) 1 , afc_array_master_first ( am ) ,0);
	print_res ( "del", ( void * ) 2 , afc_array_master_del ( am ) ,0);
	print_res ( "del", ( void * ) 3 , afc_array_master_del ( am ) ,0);
	print_res ( "del", ( void * ) 0 , afc_array_master_del ( am ) ,0);
	print_res ( "first", ( void * ) 0 , afc_array_master_first ( am ) ,0);
	print_res ( "empty", ( void * ) 1 , ( void * ) ( int ) afc_array_master_is_empty ( am ) ,0);

	print_row ();

	add_three();

	print_res ( "item(1)", ( void * ) 2 , afc_array_master_item ( am, 1 ) ,0);
	print_res ( "del", ( void * ) 3 , afc_array_master_del ( am ) ,0);
	print_res ( "del", ( void * ) 1 , afc_array_master_del ( am ) ,0);
	print_res ( "del", ( void * ) 0 , afc_array_master_del ( am ) ,0);
	print_res ( "empty", ( void * ) 1 , ( void * ) ( int ) afc_array_master_is_empty ( am ) ,0);

	print_row ();

	add_three();

	print_res ( "last", ( void * ) 3 , afc_array_master_last ( am ) ,0);
	print_res ( "del", ( void * ) 2 , afc_array_master_del ( am ) ,0);
	print_res ( "del", ( void * ) 1 , afc_array_master_del ( am ) ,0);
	print_res ( "del", ( void * ) 0 , afc_array_master_del ( am ) ,0);
	print_res ( "empty", ( void * ) 1 , ( void * ) ( int ) afc_array_master_is_empty ( am ) ,0);
	
	print_row ();
	print_summary ();

	afc_array_master_delete ( am );

	afc_delete ( afc );

	return ( 0 );
}
