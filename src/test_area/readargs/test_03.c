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

int main ( int argc, char * argv [] )
{
	AFC * afc = afc_new ();
	ReadArgs * rdarg = afc_readargs_new ( );
	NodeMaster * nm;
	char * s;

	afc_readargs_parse ( rdarg, "MULTI/M", "a b c d e \"f c\" \"\" aaa \" \"" );
	afc_readargs_parse ( rdarg, "MULTI/M", "a b c d e \"f c\" \"\" aaa \" \"" );
	afc_readargs_parse ( rdarg, "MULTI/M", "a" );
	afc_readargs_parse ( rdarg, "MULTI/M", "b" );
	afc_readargs_parse ( rdarg, "MULTI/M", "c" );
	afc_readargs_parse ( rdarg, "MULTI/M", "d" );

	afc_readargs_delete ( rdarg );
	afc_delete ( afc );

	exit ( 0 );
}
