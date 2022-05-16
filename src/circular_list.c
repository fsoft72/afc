#include "circular_list.h"

/*
@config
	TITLE:   CircularList
	VERSION: 1.0
	AUTHOR:  Fabrizio Pastore - pastorefabrizio@libero.it
	AUTHOR:  Fabio Rotondo - fabio@rotondo.it
@endnode
*/

// {{{ docs
/*
@node quote
	*Put the quote here*

	Quote Author
@endnode

@node intro
	CircularList documentation introduction should go here.
	Use the reST syntax to create docs.
@endnode
*/
// }}}

static const char class_name[] = "CircularList";
static CircularListNode * afc_circular_list_int_create_node( void );

// {{{ afc_circular_list_new ()   
/*
@node afc_circular_list_new

                 NAME: afc_circular_list_new ()    - Initializes a new CircularList instance.

             SYNOPSIS: CircularList * afc_circular_list_new ()

          DESCRIPTION: This function initializes a new CircularList instance.

                INPUT: NONE

              RESULTS: a valid inizialized CircularList instance. NULL in case of errors.

             SEE ALSO: - afc_circular_list_delete()
                       - afc_circular_list_clear()
@endnode
*/

CircularList * afc_circular_list_new ( void )
{
TRY ( CircularList * )

        CircularList * cl = afc_malloc ( sizeof ( CircularList ) );

        if ( cl == NULL ) RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "cl", NULL );
        cl->magic = AFC_CIRCULAR_LIST_MAGIC;
	cl->pointer = NULL;
	cl->count = 0;
	// cl->max_elems = max;

        RETURN ( cl );

EXCEPT
        afc_circular_list_delete ( cl );

FINALLY

ENDTRY
}
// }}}
// {{{ afc_circular_list_init ( cl, max_elems )
/*
@node afc_circular_list_init

                 NAME: afc_circular_list_init ( cl, max_elems ) - initialize the list

             SYNOPSIS: int afc_circular_list_init ( CircularList * cl , int max_elems )

          DESCRIPTION: This method set the max number of elements of the list 

                INPUT: - cl   - Pointer to a *valid* CircularList instance.
		       - max_elems - Int indicating the max number of elements

              RESULTS: - AFC_ERR_NO_ERROR on success.

             SEE ALSO:
 
@endnode
*/
void * afc_circular_list_init ( CircularList * cl, int max_elems )
{
	cl->max_elems = max_elems;
	return ( AFC_ERR_NO_ERROR ); 
}
// }}}
// {{{ afc_circular_list_delete ( cl )
/*
@node afc_circular_list_delete 

                 NAME: afc_circular_list_delete ( cl ) - Dispose a CircularList instance.

             SYNOPSIS: int afc_circular_list_delete ( CircularList * cl )

          DESCRIPTION: Use this method to delete an object's instance.

                INPUT: - cl - Pointer to a *valid* CircularList instance.

              RESULTS: - AFC_ERR_NO_ERROR on success.

             SEE ALSO: afc_circular_list_new()
@endnode
*/
int _afc_circular_list_delete ( CircularList * cl )
{
        int res;

        if ( ( res = afc_circular_list_clear ( cl ) ) != AFC_ERR_NO_ERROR ) return ( res );

        afc_free ( cl );

        return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_circular_list_clear ( cl )
/*
@node afc_circular_list_clear 

                 NAME: afc_circular_list_clear ( cl ) - Frees all unused memory

             SYNOPSIS: int afc_circular_list_clear ( CircularList * cl )

          DESCRIPTION: This method clears all data inside the CircularList instance, 
                       except the main classes. 

                INPUT: - cl   - Pointer to a *valid* CircularList instance.

              RESULTS: - AFC_ERR_NO_ERROR on success.

             SEE ALSO: - afc_circular_list_new()
		       - afc_circular_list_delete()
@endnode
*/
int afc_circular_list_clear ( CircularList * cl )
{
        char *p;
	if ( cl == NULL ) return ( AFC_LOG_FAST ( AFC_ERR_NULL_POINTER ) );
        if ( cl->magic != AFC_CIRCULAR_LIST_MAGIC ) return ( AFC_LOG_FAST ( AFC_ERR_INVALID_POINTER ) );
	while( ( p = afc_circular_list_del ( cl ) ) != NULL );
        return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ int afc_circular_list_set_clear_func ( CircularList * cl, int ( * func ) ( void * )  )
/*
@node afc_circular_list_set_clear_func

       NAME: afc_circular_list_set_clear_func( cl, func) - Sets the clear func

   SYNOPSIS: int afc_circular_list_set_clear_func (CircularList *cl, int ( *func ) ( void * ) )

      SINCE: 1.10

DESCRIPTION: Use this command to set a clear function. The function will be called each time an
	     item is being deleted from the list with afc_circular_list_del() or afc_circular_list_clear().
	     To remove this function, pass a NULL value as function pointer.

      INPUT: - cl	- Pointer to a valid Circular List
	     - func	- Function to be called in clearing operations.

    RESULTS: AFC_ERR_NO_ERROR

   SEE ALSO: - afc_circular_list_del()
	     - afc_circular_list_clear()
@endnode
*/
int afc_circular_list_set_clear_func ( CircularList * cl, int ( *func ) ( void * ) )
{
	cl->func_clear = func;

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_circular_list_add ( cl, data )
/*
@node afc_circular_list_add 

                 NAME: afc_circular_list_add ( cl, data ) - Add an element to the list

             SYNOPSIS: int afc_circular_list_add ( CircularList * cl, void * data )

          DESCRIPTION: This method adds an element to the circular list.

                INPUT: - cl   	- Pointer to a *valid* CircularList instance.
		       - data	- Data to be added to the list.

              RESULTS: - AFC_ERR_NO_ERROR on success.

             SEE ALSO:
 
@endnode
*/
int afc_circular_list_add ( CircularList * cl, void * data )
{
TRY ( int )
	CircularListNode * p;

	//check if has alredy max elems
	if ( cl->max_elems != 0 && cl->count == cl->max_elems ) 
		RAISE_RC ( AFC_LOG_ERROR, AFC_CIRCULAR_LIST_ERR_MAX_ELEMS, "Max elems limit reached", NULL , AFC_CIRCULAR_LIST_ERR_MAX_ELEMS );
	
	//check if first
	if ( cl->pointer == NULL )
	{
		//allocate memory
		if ( ( cl->pointer = afc_circular_list_int_create_node() ) == NULL )
        		RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "cl->pointer", AFC_ERR_NO_MEMORY );
		cl->pointer->next = cl->pointer;
		//set itself as his previous node
		cl->pointer->prev = cl->pointer;
	}else{
		//allocate memory
		if ( ( p = afc_circular_list_int_create_node() ) == NULL )
        		RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "p", AFC_ERR_NO_MEMORY );
		
		//current pointer successor become new node successor
		p->next = cl->pointer->next;
		
		//current pointer become new node parent
		p->prev = cl->pointer;
		
		//new node become his successor's preceding node
		p->next->prev=p;
		
		// new node become current pointer's successor
		cl->pointer->next = p;
		
		// new node become pointed
		cl->pointer = p;
	}

	//check if all went right
	if ( cl->pointer == NULL ) RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "cl", AFC_ERR_NO_MEMORY );
	
	//set data
	cl->pointer->data = data;
	
	//increase counter of elements
	cl->count ++;

	RETURN ( AFC_ERR_NO_ERROR );
EXCEPT

FINALLY

ENDTRY
}
// }}}
// {{{ afc_circular_list_prev ( cl )
/*
@node afc_circular_list_prev

                 NAME: afc_circular_list_prev ( cl ) - return the previou element

             SYNOPSIS: int afc_circular_list_prev ( CircularList * cl )

          DESCRIPTION: This method set the pointer to the previous element in the list and returns the pointer

                INPUT: - cl   - Pointer to a *valid* CircularList instance.

              RESULTS: - AFC_ERR_NO_ERROR on success.

             SEE ALSO:
 
@endnode
*/
void * afc_circular_list_prev ( CircularList * cl )
{
	cl->pointer = cl->pointer->prev;
	return ( cl->pointer->data );
}
// }}}
// {{{ afc_circular_list_next ( cl )
/*
@node afc_circular_list_next

                 NAME: afc_circular_list_next ( cl ) - return the next element

             SYNOPSIS: int afc_circular_list_next ( CircularList * cl )

          DESCRIPTION: This method set the pointer to the next element in the list and returns the pointer

                INPUT: - cl   - Pointer to a *valid* CircularList instance.

              RESULTS: - AFC_ERR_NO_ERROR on success.

             SEE ALSO:
 
@endnode
*/
void * afc_circular_list_next ( CircularList * cl )
{
	cl->pointer = cl->pointer->next;
	return ( cl->pointer->data );
}
// }}}
// {{{ afc_circular_list_obj ( cl )
/*
@node afc_circular_list_obj

                 NAME: afc_circular_list_obj ( cl ) - return the current element 

             SYNOPSIS: void * afc_circular_list_obj ( CircularList * cl )

          DESCRIPTION: This method return the object currently pointed 

                INPUT: - cl   - Pointer to a *valid* CircularList instance.

              RESULTS: - AFC_ERR_NO_ERROR on success.

             SEE ALSO:
 
@endnode
*/
// }}}
// {{{ afc_circular_list_del ( cl )
/*
@node afc_circular_list_del

                 NAME: void * afc_circular_list_del ( cl )

             SYNOPSIS: void * afc_circular_list_del ( CircularList * cl )

          DESCRIPTION: This method deletes pointed object 

                INPUT: - cl   - Pointer to a *valid* CircularList instance.

              RESULTS: - Next element data on success or NULL if cl is void

             SEE ALSO:
 
@endnode
*/
void * afc_circular_list_del ( CircularList * cl )
{
	CircularListNode * n;
	CircularListNode * old;

	//check if cl has elements
	if ( cl->count == 0 ) return ( NULL );
	
	//remember current element
	old = cl->pointer;

	//set auxiliary pointer for  new current pointer
	n = cl->pointer->next;

	//set auxiliarys previous node
	n->prev=cl->pointer->prev;
		
	//clear node data
	if ( cl->func_clear ) cl->func_clear ( old->data );

	//frees memory
	afc_free ( old );
	
	cl->pointer = n;

	//current pointer is his successor preceding item
	cl->pointer->prev->next = cl->pointer;	
	
	//decrease counter
	cl->count --;	
	
	//check if cl has elements
	if ( cl->count == 0 ) {
		cl->pointer = NULL;
		return ( NULL );
	}
	

	return ( n->data );
}
// }}}

/* ===============================================================================================================
	INTERNAL FUNCTIONS
=============================================================================================================== */
// {{{ afc_circular_list_int_create_node ( void )
static  CircularListNode * afc_circular_list_int_create_node( void )
{
TRY ( CircularListNode * )
	
	CircularListNode * p;
	if ( ( p = afc_malloc ( sizeof ( CircularList ) ) ) == NULL )
        	RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "cl", NULL );

	RETURN ( p );
EXCEPT

FINALLY

ENDTRY
}
// }}}

// {{{ TEST_CLASS
#ifdef TEST_CLASS
int clear_function ( void *p )
{
	afc_string_delete ( p );
	return ( AFC_ERR_NO_ERROR );
}

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
int main ( int argc, char * argv [] )
{
TRY ( int )
        AFC * afc;
        CircularList * cl;
	char * s;
        int i;
	afc = afc_new ();
        afc_track_mallocs ( afc );
        afc_set_tags ( afc, AFC_TAG_LOG_LEVEL, AFC_LOG_WARNING,
			AFC_TAG_SHOW_MALLOCS, TRUE,
			 AFC_TAG_END );
	
        cl = afc_circular_list_new ( );
	afc_circular_list_init ( cl, 2 );
	s = afc_circular_list_obj ( cl );	
	afc_circular_list_set_clear_func ( cl, clear_function );
	for( i = 0; i<5; i++)
	{

		if ( ( s = afc_string_new ( 15 ) ) == NULL )
			RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "s", AFC_ERR_NO_MEMORY );
		afc_string_make (s,"stringa %d",i); 
		afc_circular_list_add ( cl, s);
	}
	afc_circular_list_del ( cl );
	afc_circular_list_del ( cl );

	afc_circular_list_debug ( cl );
	
	afc_circular_list_delete ( cl );
        afc_delete ( afc );
        RETURN  ( 0 );
EXCEPT

FINALLY

ENDTRY
}
#endif
// }}}

