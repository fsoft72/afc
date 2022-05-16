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
#include "test_utils.h"

static int total_test = 0;
static int total_ok   = 0;
static int total_ko   = 0;

void test_header ()
{ 
	printf ( "| ACTION                         | EXPECTED         | RESULT\n" );
	printf ( "+--------------------------------+------------------+------------------------------------------\n" ); 
}

void print_row ()
{
	printf ( "+--------------------------------+------------------+------------------------------------------\n" ); 
}

void print_summary ()
{
	print_row ();
	printf ( "| Total Tests:                   | %5.5d            |\n", total_test );
	printf ( "| Total OK:                      | %5.5d            |\n", total_ok );
	printf ( "| Total Failed:                  | %5.5d            |\n", total_ko );
	print_row ();

}

void print_res ( char * action, void * expected, void * result, int mode )
{
	total_test ++;

	if ( mode == 0 )
	{
		printf ( "| %30.30s | %16d | %d", action, ( int ) expected, ( int ) result );

		if ( ( int ) expected == ( int ) result )
		{
			printf ( "%10.10s\n", "OK" );
			total_ok ++;
		} else {
			printf ( "%10.10s\n", "FAILED" );
			total_ko ++;
		}

	} else {
		printf ( "| %30.30s | %16.16s | %s", action, ( char * ) expected, ( char * ) result );

		if ( expected == NULL ) expected = "(null)";
		if ( result   == NULL ) result = "(null)";

		if ( strcmp ( ( char * ) expected, ( char * ) result ) == 0 )
		{
			printf ( "%10.10s\n", "OK" );
			total_ok ++;
		} else {
			printf ( "%10.10s (%s)\n", "FAILED", ( char * ) result );
			total_ko ++;
		}
	}
	
}

#ifdef TEST_CLASS
int main ()
{
	test_header ();
	print_res ( "ciao", 0, 0, 0 );
	print_res ( "mamma", "ciao", "mondo", 1 );
	print_res ( "mamma", "ciao", "ciao", 1 );
	print_res ( "mamma", "ciaoasapsoapsoaps opao spao spa ospa ospa:w ", "mondo", 1 );
	return ( 0 );
}
#endif 
