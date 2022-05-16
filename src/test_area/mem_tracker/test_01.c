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
#include "../../mem_tracker.h"

int main ()
{
	AFC * afc = afc_new ();
	MemTracker * mt = afc_mem_tracker_new ();
	void * m;
	int t;

	for ( t = 0; t < 1000; t ++ )
	{
		printf ( "Item: %d\n", t );
		m = afc_mem_tracker_malloc ( mt, 20, __FILE__, __FUNCTION__, __LINE__ );
		if ( t % 2 ) afc_mem_tracker_free ( mt, m );
	}
		
	afc_mem_tracker_delete ( mt );
	afc_delete ( afc );
	
	return ( 0 );
}

