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
#include "../../base.h"
#include "../../array_master.h"
#include "../../dictionary.h"

int clear_func ( void * data )
{
	printf ( "FREE: %x\n", ( int ) data );
	if ( data ) afc_free ( data );
	return ( AFC_ERR_NO_ERROR );
}

int main ()
{
	AFC * afc = afc_new ();
	ArrayMaster * am;
	Dictionary * d;
	int t;
	
	afc_track_mallocs ( afc );
	afc_set_tags ( afc, 	AFC_TAG_LOG_LEVEL, 	AFC_LOG_WARNING, 
				AFC_TAG_SHOW_MALLOCS, 	TRUE,
				AFC_TAG_SHOW_FREES,	TRUE,
		       AFC_TAG_END );

	am = afc_array_master_new ();
	d  = afc_dictionary_new ();
	afc_array_master_set_clear_func ( am, clear_func );

	for ( t = 0; t < 10; t ++ )
	{
		if ( ! ( t % 2 ) )
			afc_array_master_add ( am, afc_malloc ( 10 ), AFC_ARRAY_MASTER_ADD_TAIL );
		else
			afc_array_master_del ( am );
	}

	printf ( "FINISHED!\n" );
	afc_delete ( afc );
	
	return ( 0 );
}

