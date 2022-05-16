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
	i = afc_circular_list_obj( cl );
	for ( t = 1; t < 1000; t ++ )
		afc_circular_list_add ( cl, ( void * ) t );

	i = afc_circular_list_next ( cl );
	printf ( "Deve essere 1: %d\n", ( int ) i );

	for ( t = 1; t < 20; t ++ ) afc_circular_list_next ( cl );

	for ( t = 0; t < 980; t ++ ) afc_circular_list_del ( cl );

	printf ( "Elemeneti: %d\n", cl->count );

	while ( ( i = afc_circular_list_del ( cl ) ) )
	{
		printf ( "%d", ( int ) i );
		//afc_circular_list_debug ( cl );
		printf ( " Elementi : %d\n", cl->count );
	}
	afc_circular_list_delete ( cl );
	afc_delete ( afc );
	
	return ( 0 );
}
