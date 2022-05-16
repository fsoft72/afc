#include "../../circular_list.h"
int afc_circular_list_debug ( CircularList * cl)
{
        CircularListNode *p;

        p=cl->pointer;
        printf( " elements: %d \n",cl->count );
        printf( "  element |  previous |      next | data pointer \n" );
        do
        {
                printf( " %8x |", (int)cl->pointer );
                printf( "  %8x |", (int)cl->pointer->prev );
                printf( "  %8x |", (int)cl->pointer->next );
                printf("  %8x\n", (int)afc_circular_list_obj ( cl ) ) ;
        afc_circular_list_next ( cl );
        }while(p!=cl->pointer);
        return ( AFC_ERR_NO_ERROR );
}

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
	
	for ( t = 0; t < 3; t ++ )
	{
		i = afc_circular_list_next ( cl );
		print_res ( "afc_circular_list_next()", ( void * )  (t % 10)  , i, 0 );
	}
	for ( t = 3; t < 6; t ++ )
	{
		print_res("afc_circular_list_del()", (void *)  t , afc_circular_list_del ( cl ), 0);
	}

	print_row();

	for ( t	= 6; t < 10; t ++ )
	{
	
		print_res ( "afc_circular_list_next()", (void *) t, afc_circular_list_next( cl ), 0 );
	}
	print_row();
	print_res ( "cl->count", (void *) 7, cl->count, 0 );	
	print_row();
	print_res( "afc_circular_list_next()", (void *) 0, afc_circular_list_next ( cl ), 0 ); 	
	print_row();
	for ( t	= 0; t < 1; t ++ )
	{
	
		print_res ( "afc_circular_list_del()", (void *) ( t + 1 ), afc_circular_list_del( cl ), 0 );
	}
	print_res( "afc_circular_list_del()", (void *) 5, afc_circular_list_del( cl ), 0);
			
	for ( t	= 6; t < 10; t ++ )
	{
	
		print_res ( "afc_circular_list_del()", (void *) ( t ), afc_circular_list_del( cl ), 0 );
	}
	/*while( ( i = afc_circular_list_del ( cl ) ) )
	{
		
		t = t++ % 10;
		print_res ( "afc_circular_list_del()", (void *) t, i, 0);
	}*/
	print_row();
		
	print_res ( "afc_circular_list_del()", (void *) 0, afc_circular_list_del( cl ), 0);
	
	print_res ( "cl->count", (void *) 0, cl->count, 0 );	
	print_summary ();

	afc_circular_list_delete ( cl );
	afc_delete ( afc );
	
	return ( 0 );
}
