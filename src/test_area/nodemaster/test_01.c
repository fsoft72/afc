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

int elements = 0;

NodeMaster * am;

void add_element ( )
{
	printf ( "Adding a new Element...\n ");

	afc_nodemaster_add ( am, ( void * ) 1, AFC_NODEMASTER_ADD_TAIL );
	elements ++;

	printf ( "Elements: %d - AM Elements: %lu\n", elements, afc_nodemaster_len ( am ) );
}

void del_element ( )
{
	int num;

	if ( elements == 0 ) return;

	num = random() % elements;

	printf ( "Deleting an Element...%d\n ", num);

	afc_nodemaster_item ( am, num );
	afc_nodemaster_del ( am );
	elements --;

	printf ( "Elements: %d - AM Elements: %lu\n", elements, afc_nodemaster_len ( am ) );
}

int main ()
{
	AFC * afc = afc_new ();
	int t;
	int n;
	int i;

	if ( afc == NULL ) return ( 1 );

	am = afc_nodemaster_new ( );

	for ( t = 0; t < 100000; t++ )
	{
		n = ( int ) random ();
		i = n % 2;
		
		if ( i == 0 )
			add_element ();
		else
			del_element ();
	}

	afc_nodemaster_delete ( am );

	afc_delete ( afc );

	return ( 0 );
}

