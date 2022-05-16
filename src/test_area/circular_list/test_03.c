#include "../../circular_list.h"

int main ()
{
	AFC * afc = afc_new ();
	CircularList * cl;
	int t;
	void * i;

	cl = afc_circular_list_new ();

	test_header ();
	

	for ( t = 0; t < 10; t ++ )
		afc_circular_list_add ( cl, ( void * ) t );

	for ( t = 0; t < 20; t ++ )
	{
		i = afc_circular_list_next ( cl );
		print_res ( "afc_circular_list_next()", ( void * )  ( t % 10 ) , i, 0 );
	}
	print_row();

	//delete last element	
	print_res( "afc_circular_list_del()", (void *) 0, afc_circular_list_del ( cl ), 0 ); 	
	print_row();
	
	for ( t = 0; t < 18; t ++ )
	{
		i = afc_circular_list_next ( cl );
		print_res ( "afc_circular_list_next()", ( void * )  ( ( t + 1 ) % 9 ) , i, 0 );
	}

	print_row();
	
	//delete first element
	print_res( "afc_circular_list_del()", (void *) 1, afc_circular_list_del ( cl ), 0 ); 	
	print_row();
		
	for ( t = 2; t < 9; t ++ )
	{
		i = afc_circular_list_next ( cl );
		print_res ( "afc_circular_list_next()", ( void * )  (  t  % 9 ) , i, 0 );
	}	
	print_row();
		
	for ( t = 9; t < 15; t ++ )
		afc_circular_list_add( cl, (void *) t );
		
	print_row();

	//add another element	
	afc_circular_list_add( cl, 0 );
	print_row();

	//before last elemnt added	
	afc_circular_list_prev( cl );	
	for ( t = 0; t < 30; t ++ )
	{
		i = afc_circular_list_next ( cl );
		print_res ( "afc_circular_list_next()", ( void * ) ( t % 15 ) , i, 0 );
	}
	afc_circular_list_del( cl );
	afc_circular_list_add ( cl, ( void * ) t );
	print_summary ();

	afc_circular_list_delete ( cl );
	afc_delete ( afc );
	
	return ( 0 );
}
