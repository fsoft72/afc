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

#include "array_master.h"

/*
@config
	TITLE:     ArrayMaster
	VERSION:   1.30
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
@endnode
*/

// {{{ docs
/*
@node quote
	*Who are you going to believe, me or your own eyes?*

		Groucho Marx
@endnode

@node intro

ArrayMaster is a class that handles arrays, like the name said. But it has some more advantages
to standard arrays:

- It automatically enlarges array size when new space is needed 
- It offers some APIs to handle the array like it was a double linked list, infact
  you can browse throught the array with calls like afc_array_master_first(), 
  afc_array_master_next(), afc_array_master_prev() and so on. 
- It offers a customizable, lightspeed fast sort routine. 
  See afc_array_master_sort() for more info 

Like all AFC classes, you can instance a new  ArrayMaster by calling afc_array_master_new (), and free it with afc_array_master_delete ().

To add elements to the array, use afc_array_master_add (); to delete all elements call afc_array_master_clear(), 
and to delete just one of them there is afc_array_master_del() .

@endnode

@node history
	- 1.30:		ADD: afc_array_master_before_first()	function
	- 1.20:	 	ADD: afc_array_master_set_custom_sort () function
@endnode
*/
// }}}


static const char class_name[] = "Array Master";

static int afc_array_master_internal_double_array ( ArrayMaster * );
static int afc_array_master_internal_insert ( ArrayMaster *, void * );
#ifdef MINGW
static void quick_sort ( void * base, size_t num_items, size_t width, int ( *compare ) ( const void *, const void *) );
static void rqsort ( char * low, char * high, size_t width, int ( *compare ) ( const void *, const void *) );
#endif

// {{{ ArrayMaster * afc_array_master_new ()   
/*
@node afc_array_master_new

	         NAME: afc_array_master_new ()    - Initializes a new afc_array_master instance.

	     SYNOPSIS: ArrayMaster * afc_array_master_new ( )

	  DESCRIPTION: This function initializes a new afc_array_master instance.

	        INPUT: NONE

	      RESULTS: a valid inizialized afc_array_master structure. NULL in case of errors.

	  NOTES: - this function calls afc_array_master_init() to initialize the array.
	                 The array will be initialized to handle a default amount of items, defined
	                 with the value stored in *AFC_ARRAY_MASTER_DEFAULT_ITEMS*.
			 This default value should be enought for almost all applications. Anyway, keep
	                 in mind that the array dimension will automatically doubled each time you are 
	                 adding the (array_size + 1)th element.

	     SEE ALSO: - afc_array_master_delete()
		 - afc_array_master_init()

@endnode
*/
ArrayMaster * _afc_array_master_new ( const char * file, const char * func, const unsigned int line  )
{
TRY ( ArrayMaster * )

	ArrayMaster * array_master = _afc_malloc ( sizeof ( struct afc_array_master ), file, func, line );

	if ( array_master == NULL ) RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "array_master", NULL );

	array_master->magic = AFC_ARRAY_MASTER_MAGIC;

	if ( ( array_master->mem = _afc_malloc ( ( sizeof ( void * ) ) * AFC_ARRAY_MASTER_DEFAULT_ITEMS, file, func, line ) ) == NULL ) 
		RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "mem", NULL );

	array_master->current_pos = 0;
	array_master->max_items   = AFC_ARRAY_MASTER_DEFAULT_ITEMS;

#ifndef MINGW
	array_master->custom_sort = qsort;
#else
	array_master->custom_sort = quick_sort;
#endif

	RETURN ( array_master );
EXCEPT
	afc_array_master_delete ( array_master );

FINALLY

ENDTRY
}
// }}}
// {{{ int afc_array_master_delete ( ArrayMaster * array_master ) 
/*
@node afc_array_master_delete

	         NAME: afc_array_master_delete ( array_master )  - Disposes a valid array_master instance.

	     SYNOPSIS: int afc_array_master_delete (ArrayMaster * array_master)

	  DESCRIPTION: This function frees an already alloc'd afc_array_master structure.

	        INPUT: - array_master  - Pointer to a valid afc_array_master class.

	      RESULTS: should be AFC_ERR_NO_ERROR

	        NOTES: - this method calls: afc_array_master_clear()

	     SEE ALSO: - afc_array_master_new()
	               - afc_array_master_clear()
@endnode

*/
int _afc_array_master_delete ( ArrayMaster * array_master ) 
{
	int afc_res; 

	if ( ( afc_res = afc_array_master_clear ( array_master ) ) != AFC_ERR_NO_ERROR ) return afc_res;

	afc_free ( array_master->mem );
	afc_free ( array_master );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ int afc_array_master_clear ( ArrayMaster * array_master ) 
/*
@node afc_array_master_clear

	         NAME: afc_array_master_clear ( array_master )  - Clears all stored data

	     SYNOPSIS: int afc_array_master_clear ( ArrayMaster * array_master)

	  DESCRIPTION: Use this command to clear all stored data in the current array_master instance.
		       If the clear function has been set, items are also cleared from memory.

	        INPUT: - array_master    - Pointer to a valid afc_array_master instance.

	      RESULTS: should be AFC_ERR_NO_ERROR
	              
	     SEE ALSO: - afc_array_master_delete()
		       - afc_array_master_set_clear_func()
	               
@endnode
*/
int afc_array_master_clear ( ArrayMaster * am ) 
{
	unsigned int t;

	if ( am == NULL ) return AFC_LOG_FAST ( AFC_ERR_NULL_POINTER );
 
	if ( am->magic != AFC_ARRAY_MASTER_MAGIC ) return AFC_LOG_FAST ( AFC_ERR_INVALID_POINTER );

	if ( ( am->num_items ) &&  ( am->func_clear ) )
		for ( t = 0; t < am->num_items; t ++ ) am->func_clear ( am->mem [ t ] );

	am->num_items = 0;
	am->current_pos = 0;

	/* Custom Clean-up code should go here */

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ int afc_array_master_init ( ArrayMaster * am, unsigned long int size )
/*
@node afc_array_master_init

	         NAME: afc_array_master_init ( array_master, size )  - Inits the Array size

	     SYNOPSIS: int afc_array_master_init ( ArrayMaster * array_master, unsigned long int size )

	  DESCRIPTION: Use this command to set a new initail dimension for the array.

	        INPUT: - array_master    - Pointer to a valid afc_array_master instance.
	               - size            - New array dimension.

	      RESULTS: - should be AFC_ERR_NO_ERROR
	              
		NOTES: - The actual use of memory can be computed using this algo: (size * (sizeof ( void * ) ) ).
	                 On x86 machines, the sizeof ( void * ) is equal to 4 bytes, so setting the array dimension
	                 to, let's say, 1000 items, will eat up 4k of memory. 

	     SEE ALSO: 
@endnode
*/
int afc_array_master_init ( ArrayMaster * am, unsigned long int size )
{
	if ( ( am->mem = afc_realloc ( am->mem, sizeof ( void * ) * size ) ) == NULL ) return ( AFC_ERR_NO_MEMORY );

	am->max_items    = size;
	am->current_pos  = 0;
	am->num_items    = 0;

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ int afc_array_master_add ( ArrayMaster * am, void * data, int mode )
/*
@node afc_array_master_add

	         NAME: afc_array_master_add ( array_master, data, mode )  - Adds a new element to the array

	     SYNOPSIS: int afc_array_master_add ( ArrayMaster * array_master, void * data, int mode )

	  DESCRIPTION: This function adds a new element to the array. If the array is full and you are trying a new
	               element that is after the (internal) size limit of the array, the array will double its size
	               (without loosing any data) and the element will be added.

	        INPUT: - array_master    - Pointer to a valid afc_array_master instance.
	               - data            - Data to add to the array.
	               - mode            - Inserting method. Valid values are:
	                                   + AFC_ARRAY_MASTER_ADD_TAIL - This is the fastest method of all. Element will be
	                                                                 added as the last element in the array.
	                                   + AFC_ARRAY_MASTER_ADD_HEAD - This is the slowest method of all. Element will be 
	                                                                 added as the first element of the array.
	                                   + AFC_ARRAY_MASTER_ADD_HERE - This method is quite slow (depending on the current 
	                                                                 position in the array). It will add the data after the
	                                                                 current array position.

	      RESULTS: - should be AFC_ERR_NO_ERROR
	              
	     SEE ALSO: - afc_array_master_del()
	               - afc_array_master_item()
@endnode
*/
int afc_array_master_add ( ArrayMaster * am, void * data, int mode )
{
	switch ( mode ) 
	{
		case AFC_ARRAY_MASTER_ADD_TAIL:			// To add in as the LAST element
			am->mem [ am->num_items ] = data;       // use am->num_items (number of items in list)
			am->current_pos = am->num_items;        // Set current pos to am->num_items 
			break;
		case AFC_ARRAY_MASTER_ADD_HEAD:			// To add in the HEAD of the array
			am->current_pos = 0;				// Set current_pos to 0 (first element)
			afc_array_master_internal_insert ( am, data ); // Call internal_insert()
			break;
		case AFC_ARRAY_MASTER_ADD_HERE:				// To add HERE (in the middle of the array)
			am->current_pos++;						// Increment current pos of 1 (we add AFTER the current item)
			afc_array_master_internal_insert ( am, data );	// call internal_insert()
			break;
	}

	am->num_items ++;				// Increment num_items

	am->is_sorted = FALSE;

	if ( am->num_items == am->max_items ) 
		return ( afc_array_master_internal_double_array ( am ) );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ void * afc_array_master_item ( ArrayMaster * am, unsigned long item )
/*
@node afc_array_master_item

	         NAME: afc_array_master_item ( array_master, item )  - Returns the desired element in the Array

	     SYNOPSIS: void * afc_array_master_item ( ArrayMaster * array_master, unsigned long item )

	  DESCRIPTION: This function returns the desired element from the array. If the item number you are specifying is
	               out of array bounds, you will get a NULL value.

	        INPUT: - array_master    - Pointer to a valid afc_array_master instance.
	               - item            - Ordinal value of the item in the array (starting from 0)

	      RESULTS: - should return the desired value, or NULL if the desired element is out of bounds.
	              
	     SEE ALSO: - afc_array_master_add()
	               - afc_array_master_del()
		       - afc_array_master_first()
	               - afc_array_master_next()
@endnode
*/
void * afc_array_master_item ( ArrayMaster * am, unsigned long item )
{
	if ( item >= am->num_items  ) return NULL;

	return ( am->mem [ am->current_pos = item ] );
}
// }}}
// {{{ void * afc_array_master_first ( ArrayMaster * am )
/*
@node afc_array_master_first

	         NAME: afc_array_master_first ( array_master )  - Returns the first element in the array

	     SYNOPSIS: void * afc_array_master_first ( ArrayMaster * array_master )

	  DESCRIPTION: This function returns the first element in the array.

	        INPUT: - array_master    - Pointer to a valid afc_array_master instance.

	      RESULTS: - should return the desired value, or NULL if the desired element is out of bounds.
	              
	     SEE ALSO: - afc_array_master_add()
	               - afc_array_master_del()
	               - afc_array_master_next()
@endnode
*/
void * afc_array_master_first ( ArrayMaster * am )
{
	am->before_first = FALSE;

	if ( am->num_items <= 0 ) return ( NULL );

	am->current_pos = 0;
	return ( am->mem [ 0 ] );
}
// }}}
// {{{ void * afc_array_master_next ( ArrayMaster * am )
/*
@node afc_array_master_next

	         NAME: afc_array_master_next ( array_master )  - Returns the next element in the array

	     SYNOPSIS: void * afc_array_master_next ( ArrayMaster * array_master )

	  DESCRIPTION: This function returns the next element in the array.

	        INPUT: - array_master    - Pointer to a valid afc_array_master instance.

	      RESULTS: - should return the desired value, or NULL if the desired element is out of bounds (ie. you have reached
	                 the last element)
			
	              
	     SEE ALSO: - afc_array_master_add()
	               - afc_array_master_del()
	               - afc_array_master_prev()
@endnode
*/
void * afc_array_master_next ( ArrayMaster * am )
{
	if ( am->before_first ) return ( afc_array_master_first ( am ) );

	if ( am->current_pos <  ( am->num_items -1 ) ) 
		return ( am->mem [ ++ am->current_pos ] );
		
	return ( NULL );
}
// }}}
// {{{ void * afc_array_master_prev ( ArrayMaster * am )
/*
@node afc_array_master_prev

	         NAME: afc_array_master_prev ( array_master )  - Returns the previous element in the array

	     SYNOPSIS: void * afc_array_master_prev ( ArrayMaster * array_master )

	  DESCRIPTION: This function returns the previous element in the array.

	        INPUT: - array_master    - Pointer to a valid afc_array_master instance.

	      RESULTS: - should return the desired value, or NULL if the desired element is out of bounds (ie. you have reached
	                 the first element)
	              
	     SEE ALSO: - afc_array_master_add()
	               - afc_array_master_del()
	               - afc_array_master_next()
@endnode
*/
void * afc_array_master_prev ( ArrayMaster * am )
{
	if ( am->current_pos > 0 ) return ( am->mem [ -- am->current_pos ] );

	return ( NULL );
}
// }}}
// {{{ void * afc_array_master_last ( ArrayMaster * am )
/*
@node afc_array_master_last

	         NAME: afc_array_master_last ( array_master )  - Returns the last element in the array

	     SYNOPSIS: void * afc_array_master_last ( ArrayMaster * array_master )

	  DESCRIPTION: This function returns the last element in the array.

	        INPUT: - array_master    - Pointer to a valid afc_array_master instance.

	      RESULTS: - should return the desired value, or NULL if the desired element is out of bounds.
	              
	     SEE ALSO: - afc_array_master_add()
	               - afc_array_master_del()
	               - afc_array_master_next()
		 - afc_array_master_first()
		 - afc_array_master_prev()
@endnode
*/
void * afc_array_master_last ( ArrayMaster * am )
{
	if ( am->num_items == 0 ) return ( NULL );

	am->current_pos = am->num_items -1;

	return ( am->mem [ am->current_pos ] );
}
// }}}
// {{{ void * afc_array_master_obj ( ArrayMaster * am )
/*
@node afc_array_master_obj

	         NAME: afc_array_master_obj ( array_master )  - Returns the current element in the array

	     SYNOPSIS: void * afc_array_master_obj ( ArrayMaster * array_master )

	  DESCRIPTION: This function returns the current element pointed in the array.

	        INPUT: - array_master    - Pointer to a valid afc_array_master instance.

	      RESULTS: - should return the desired value, or NULL if the desired element is out of bounds 
	                 (ie.  the array is empty )
	              
	     SEE ALSO: - afc_array_master_add()
	               - afc_array_master_del()
	               - afc_array_master_next()
@endnode
*/
void * afc_array_master_obj ( ArrayMaster * am )
{
	if ( am->num_items == 0 ) return ( NULL );

	return ( am->mem [ am->current_pos ] );
}
// }}}
// {{{ short afc_array_master_is_first ( ArrayMaster * am )
/*
@node afc_array_master_is_first

	         NAME: afc_array_master_is_first ( array_master )  - Checks if you are on the first element

	     SYNOPSIS: short afc_array_master_is_first ( ArrayMaster * array_master )

	  DESCRIPTION: This function checks if you are on the first element of the Array Master.

	        INPUT: - array_master    - Pointer to a valid afc_array_master instance.

	      RESULTS: - TRUE  - You are on the first element.
	               - FALSE - You are not on the first element.
	              
	     SEE ALSO: - afc_array_master_is_last()
		 - afc_array_master_is_empty()
@endnode
*/
short afc_array_master_is_first ( ArrayMaster * am )
{
	return ( am->current_pos == 0 );
}
// }}}
// {{{ short afc_array_master_is_last ( ArrayMaster * am )
/*
@node afc_array_master_is_last

	         NAME: afc_array_master_is_last ( array_master )  - Checks if you are on the last element

	     SYNOPSIS: short afc_array_master_is_last ( ArrayMaster * array_master )

	  DESCRIPTION: This function checks if you are on the last element of the Array Master.

	        INPUT: - array_master    - Pointer to a valid afc_array_master instance.

	      RESULTS: - TRUE  - You are on the last element.
	               - FALSE - You are not on the last element.
	              
	     SEE ALSO: - afc_array_master_is_first()
		 - afc_array_master_is_empty()
@endnode
*/
short afc_array_master_is_last ( ArrayMaster * am )
{
	return ( am->current_pos == ( am->num_items - 1 ) );
}
// }}}
// {{{ short afc_array_master_is_empty ( ArrayMaster * am )
/*
@node afc_array_master_is_empty

	         NAME: afc_array_master_is_empty ( array_master )  - Checks if the array is empty

	     SYNOPSIS: short afc_array_master_is_last ( ArrayMaster * array_master )

	  DESCRIPTION: This function checks if the array is empty or not.

	        INPUT: - array_master    - Pointer to a valid afc_array_master instance.

	      RESULTS: - TRUE  - The array is empty.
	               - FALSE - The array is not empty.
	              
	     SEE ALSO: - afc_array_master_is_first()
		 - afc_array_master_is_empty()
@endnode
*/
short afc_array_master_is_empty ( ArrayMaster * am )
{
	return ( am->num_items == 0 );
}
// }}}
// {{{ void * afc_array_master_del ( ArrayMaster * am )
/*
@node afc_array_master_del

	         NAME: afc_array_master_del ( array_master )  - Deletes the current element from the array

	     SYNOPSIS: void * afc_array_master_del ( ArrayMaster * array_master )

	  DESCRIPTION: This function deletes the current element in the array. Like you were using a list, 
	               all other elements are shift to fill the gap, so ie. if you are deleteing element
	               number 10, the 11th element will become the 10th and so on.
		       If the clear function has been set, the item is also cleared from memory.

	        INPUT: - array_master    - Pointer to a valid afc_array_master instance.

	      RESULTS: - a valid pointer to the data contained in the next avaible element, or NULL if array
	                 is now empty.
	              
	     SEE ALSO: 	- afc_array_master_add()
			- afc_array_master_set_clear_func()
@endnode
*/
void * afc_array_master_del ( ArrayMaster * am )
{
	void * dest;
	void * src;
	size_t size;

	if ( am->num_items == 0 ) return ( NULL );

	if ( am->func_clear )						// If the clear_func exists
		am->func_clear ( am->mem [ am->current_pos ] );		// Delete the item from memory

	if ( am->current_pos == ( am->num_items - 1 ) ) 		// If I enter here, then I am pointing on the last element of the array
	{
		am->num_items --;					// The array is resized -1
		am->current_pos = am->num_items-1;			// and we'll point (again!) on the last element 

		if ( am->num_items == 0 ) return ( NULL );		// If the array is now empty, we should return NULL

		return ( am->mem [ am->current_pos ] );			// Else the current element (last) will be returned
	}

	dest = &am->mem [ am->current_pos ];
	src  = &am->mem [ am->current_pos + 1 ];

	size = ( am->num_items - ( am->current_pos ) ) * ( sizeof ( void * ) ) ;

	memcpy ( dest, src, size );	// Flawfinder: ignore

	am->num_items --;

	if ( am->current_pos >= am->num_items ) am->current_pos = am->num_items-1;

	am->is_sorted = FALSE;

	if ( am->num_items <= 0 ) return ( NULL );

	return ( am->mem [ am->current_pos ] );
}
// }}}
// {{{ void * afc_array_master_sort ( ArrayMaster * am, int ( *comp ) ( const void *, const void * ) ) 
/*
@node afc_array_master_sort

	         NAME: afc_array_master_sort ( array_master, comp_routine )  - Sorts the elements in the array

	     SYNOPSIS: void * afc_array_master_sort ( ArrayMaster * array_master, int (*comp) ( const void *, const void * ) )

	  DESCRIPTION: This function sorts all elements in the array using the provided comparison function.
		 The comparison function must return an integer less  than,
	     		 equal  to,  or  greater than zero if the first argument is
	     		 considered to be respectively  less  than,  equal  to,  or
	     		 greater than the second.  If two members compare as equal,
	     		 their order in the sorted array is undefined.

	        INPUT: - array_master    - Pointer to a valid afc_array_master instance.
		 - comp            - The comparison function.

	      RESULTS: - a valid pointer to the  (new) first element in the array.
	              
	     SEE ALSO: 
@endnode
*/
void * afc_array_master_sort ( ArrayMaster * am, int ( *comp ) ( const void *, const void * ) ) 
{
	am->custom_sort ( am->mem, am->num_items, sizeof ( void * * ), comp );
	am->current_pos = 0;
	am->is_sorted = TRUE;

	return ( am->mem [ 0 ] );
}
// }}}
// {{{ unsigned long int afc_array_master_pos ( ArrayMaster * am )
/*
@node afc_array_master_pos

	         NAME: afc_array_master_pos ( array_master )  - Returns the current ordinal position

	     SYNOPSIS: void * afc_array_master_pos ( ArrayMaster * array_master )

	  DESCRIPTION: This function returns the current ordinal position of the currently selected element 
	               in the array.

	        INPUT: - array_master    - Pointer to a valid afc_array_master instance.

	      RESULTS: - The ordinal value of the current pos.
	              
	     SEE ALSO: 
@endnode
*/
/*
unsigned long int afc_array_master_current_pos ( ArrayMaster * am )
{
	return ( am->current_pos );
}
*/
// }}}
// {{{ unsigned long int afc_array_master_len ( ArrayMaster * am )
/*
@node afc_array_master_len

		 NAME: afc_array_master_len(am) - Returns the number of items in list

			 SYNOPSIS: unsigned long afc_array_master_len (ArrayMaster * am)

		DESCRIPTION: Use this command to know how many items are added to the list.

		INPUT: - am	- Pointer to a valid ArrayMaster class.

	RESULTS: items	- Number of items in this list.

			 SEE ALSO: - afc_array_master_add()
		 - afc_array_master_del()
@endnode
*/
unsigned long int afc_array_master_len ( ArrayMaster * am )
{
	return ( am ? am->num_items : 0 );
}
// }}}
// {{{ int afc_array_master_set_clear_func ( ArrayMaster * am, int ( * func ) ( void * )  )
/*
@node afc_array_master_set_clear_func

       NAME: afc_array_master_set_clear_func(am, func) - Sets the clear func

   SYNOPSIS: int afc_array_master_set_clear_func (ArrayMaster * am, int ( *func ) ( void * ) )

      SINCE: 1.10

DESCRIPTION: Use this command to set a clear function. The function will be called each time an
	     item is being deleted from the list with afc_array_master_del() or afc_array_master_clear().
	     To remove this function, pass a NULL value as function pointer.

      INPUT: - am	- Pointer to a valid ArrayMaster class.
	     - func	- Function to be called in clearing operations.

    RESULTS: AFC_ERR_NO_ERROR

   SEE ALSO: - afc_array_master_del()
	     - afc_array_master_clear()
@endnode
*/
int afc_array_master_set_clear_func ( ArrayMaster * am, int ( *func ) ( void * ) )
{
	am->func_clear = func;

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ int afc_array_master_for_each ( ArrayMaster * am, int ( * func ) ( ArrayMaster * am, void *, void *  ), void * info  )
/*
@node afc_array_master_set_clear_func

       NAME: afc_array_master_set_clear_func(am, func) - Sets the clear func

   SYNOPSIS: int afc_array_master_set_clear_func (ArrayMaster * am, int ( *func ) ( ArrayMaster * am, int pos, void * v, void * info ) )

      SINCE: 1.10

DESCRIPTION: Use this command to traverse all elements inside the ArrayMaster.
	     The function called must return AFC_ERR_NO_ERROR when the traverse succeded and
	     a valid error otherwise. In case of error, the traverse is stopped and the error
	     returned.

      INPUT: - am	- Pointer to a valid ArrayMaster class.
	     - func	- Function to be called in the traverse.
			  Function prototype: int func ( ArrayMaster * am, int pos, void * v, void * info );
	     - info	- Additional param passed to the /func/ being called.

    RESULTS: AFC_ERR_NO_ERROR

   SEE ALSO: - afc_array_master_first()
	     - afc_array_master_next()
@endnode
*/
int afc_array_master_for_each ( ArrayMaster * am, int ( *func ) ( ArrayMaster * am, int pos, void * v, void * info ), void * info )
{
	unsigned int t;
	int res;

	for ( t = 0; t < am->num_items; t ++ )
		if ( ( res = func ( am, t, am->mem [ t ], info ) ) != AFC_ERR_NO_ERROR ) return ( res );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ int afc_array_master_set_custom_sort ( ArrayMaster * am, void ( * f ) ( void * base, size_t n, size_t size, int ( *com ) ( const void *, const void *) ) )
/*
@node afc_array_master_set_clear_func

       NAME: afc_array_master_set_clear_func(am, func) - Sets the clear func

   SYNOPSIS: int afc_array_master_set_custom_sort ( ArrayMaster * am, void ( * func )  ( void * base, size_t nmemb, size_t size, int ( *compar ) ( const void *, const void *) ) )

      SINCE: 1.20

DESCRIPTION: Use this command to set a sort routine different to glibc qsort. You should use this function only if you
	     know what you are doing and your glibc implementation of qsort is buggy.

      INPUT: - am	- Pointer to a valid ArrayMaster class.
	     - func	- Function to be called in sort operations.

    RESULTS: AFC_ERR_NO_ERROR
@endnode
*/
int afc_array_master_set_custom_sort ( ArrayMaster * am, void ( * func )  ( void * base, size_t nmemb, size_t size, int ( *compar ) ( const void *, const void *) ) )
{
	am->custom_sort = func;

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_array_master_before_first ( am ) **************
int afc_array_master_before_first ( ArrayMaster * am )
{
	am->before_first = TRUE;

	return ( AFC_ERR_NO_ERROR );
}
// }}}

/* ===============================================================================================================
	INTERNAL FUNCTIONS
=============================================================================================================== */
// {{{ int afc_array_master_internal_double_array ( ArrayMaster * am )
static int afc_array_master_internal_double_array ( ArrayMaster * am )
{
	unsigned long int m = ( ( sizeof ( void * ) ) * ( am->max_items  * 2 ) );

	if ( ( am->mem = afc_realloc ( am->mem, m ) ) == NULL ) return ( AFC_LOG_FAST ( AFC_ERR_NO_MEMORY ) );

	am->max_items = am->max_items * 2;
	
	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ int afc_array_master_internal_insert ( ArrayMaster * am, void * data )
static int afc_array_master_internal_insert ( ArrayMaster * am, void * data )
{
	register unsigned long t;

	for ( t = am->num_items; t > am->current_pos; t--)
		am->mem[ t ] = am->mem[ t-1 ];

	am->mem [ am->current_pos ] = data;

	return ( AFC_ERR_NO_ERROR );
}
// }}}
#ifdef MINGW
// {{{ rqsort
/*	Perform a quick sort on an array starting at base.  The
	array is nel elements large and width is the size of a single
	element in bytes.  Compare is a pointer to a comparison routine
	which will be passed pointers to two elements of the array.  It
	should return a negative number if the left-most argument is
	less than the rightmost, 0 if the two arguments are equal, a
	positive number if the left argument is greater than the right. 
	(That is, it acts like a "subtract" operator.) If compare is 0
	then the default comparison routine, argvcmp (which sorts an
	argv-like array of pointers to strings), is used.
*/
static void quick_sort ( void * base, size_t num_items, size_t width, int ( *compare ) ( const void *, const void *) )
{
	if ( num_items > 1 ) rqsort ( base, base + ( ( num_items - 1 ) * width ), width, compare );
}

static void rqsort ( char * low, char * high, size_t width, int ( *compare ) ( const void *, const void *) )
{
	char * pivot, * base;
	static char * a, * b;	//	Used for exchanges, will not need to retain	
	static int tmp, i;	//	their values during the recursion so they can be static	
	
	base = low;
	pivot = high;	
	high -= width;

	do { 
		while ( low < high && ( *compare ) ( low, pivot ) <= 0 )  low += width;
		
		while ( low < high && ( *compare ) ( high, pivot) >= 0 ) high -= width;
		
		if( low < high )	// exchange low & high 
		{
			for ( b = low, a = high, i = width ; --i >= 0 ; )
			{
				tmp = * b;	// Exchange *low and *high 
				* b++ = * a;
				* a++ = tmp;
			}
		}
	} while ( low < high );
	
	if ( low < pivot && ( *compare ) ( low, pivot ) > 0 )
		for ( b = low, a = pivot, i = width ; --i >= 0 ; )
		{
			tmp = * b ;	// Exchange *low and *pivot 
			* b++ = * a;
			* a++ = tmp;
		}

	low += width;
	if ( high - base < pivot - low )
	{
		if ( low < pivot ) rqsort ( low, pivot, width, compare );
		if ( base < high ) rqsort ( base, high, width, compare );
	} else {
		if ( base < high ) rqsort ( base, high, width, compare );
		if ( low < pivot ) rqsort ( low, pivot, width, compare );
	}
}
// }}}
#endif

#ifdef TEST_CLASS
// {{{ TEST_CLASS
int my_sort ( const void * v1, const void * v2 )
{
	char * c1 =  ( char * ) ( * ( char * *) v1 );
	char * c2 =  ( char * ) ( * ( char * *) v2 );

	// printf ( "COMP: %s - %s\n", c1, c2 );
 
	return ( strcmp ( c1,  c2 ) ) ;
}

void do_switch ( const void * v1, const void * v2 )
{
	char * c1 = ( char * ) ( * ( char * * ) v1 );
	char * c2 =  ( char * ) ( * ( char * *) v2 );
	char * tmp;

	tmp = c1;

	*c2 = *c1;
	*c1 = *tmp;
}


int dump_all ( ArrayMaster * am )
{
	int i = afc_array_master_len ( am );
	int t;

	printf ( "Dumping: %d items \n\n" , i );
	for ( t = 0; t< i; t++ )
		printf ( "Item: %d - %s\n", t, ( char * ) afc_array_master_item ( am, t ) );

	printf ( "--------------------------------------\n" );

	return ( AFC_ERR_NO_ERROR );
}

int dump_list ( ArrayMaster * am )
{
	char * s;
	int i=0;

	printf ( "List dumping ... \n");
	s = afc_array_master_first ( am );
	while ( s )
	{
		printf ( "Item: %d - %s\n", i++, s );
		s = afc_array_master_next ( am );
	}

	printf ( "--------------------------------------\n" );
	return ( AFC_ERR_NO_ERROR );
}

int dump_list_reverse ( ArrayMaster * am )
{
	char * s;
	int i=0;

	printf ( "REVERSE List dumping...\n" );
	s = afc_array_master_last ( am );
	while ( s )
	{
		printf ( "Item: %d - %s\n", i++, s );
		s = afc_array_master_prev ( am );
	}
	printf ( "--------------------------------------\n" );

	return ( AFC_ERR_NO_ERROR );
}

int clear_func ( void * v )
{
	printf ( "Deleting: %s\n", ( char * ) v );

	afc_string_delete ( v );

	return ( AFC_ERR_NO_ERROR );
}

void custom_sort ( void * base, size_t nmemb, size_t size, int ( *compar ) ( const void *, const void *) )
{
	unsigned int j, i;
	char * pos2, * pos1;
	char   temp [ 5 ];

 	for ( i = 0; i < nmemb-1; i++ ) 
	{ 
		for ( j = 1; j < nmemb-i; j++ ) 
		{ 
			pos1 = ( j - 1 ) * size + base;
			pos2 = j * size + base;

			if ( compar (  pos1, pos2 ) > 0 )
			{
				memcpy ( & temp, pos1, 4 );
				memcpy ( pos1, pos2, 4 );
				memcpy ( pos2, &temp, 4 );
			}
		}
	}
}

#define ITEMS  20

int main ( void )
{
	AFC * afc = afc_new ();
	ArrayMaster * am = afc_array_master_new ( );
	int t = 0;
	char * buf;

	afc_array_master_init ( am, ITEMS + 1 );

	afc_array_master_set_custom_sort ( am, custom_sort );

	printf ( "Creating %d elements...\n", ITEMS );
	for ( t = ITEMS ; t>0 ; t-- )
	{
		buf = ( char * ) afc_malloc ( 15 );
		sprintf ( buf, "%4.4d", t );	// Flawfinder: ignore
	  afc_array_master_add ( am, buf, AFC_ARRAY_MASTER_ADD_TAIL );
	}
	printf ( "     DONE!\n" );
	
	printf ( "Sorting... \n" );
	afc_array_master_sort ( am, my_sort );
	printf ( "     DONE!\n" );

	dump_all ( am );

	for ( t = 0; t<ITEMS; t++ )
	{
		buf = afc_array_master_item ( am, t );
		afc_free ( buf ) ;
	}
	
	afc_array_master_clear ( am );

	dump_all ( am );

	afc_array_master_add ( am, afc_string_dup ( "first" ), AFC_ARRAY_MASTER_ADD_TAIL );
	afc_array_master_add ( am, afc_string_dup ( "second" ), AFC_ARRAY_MASTER_ADD_TAIL );
	afc_array_master_add ( am, afc_string_dup ( "third" ), AFC_ARRAY_MASTER_ADD_TAIL );
	afc_array_master_add ( am, afc_string_dup ( "last" ), AFC_ARRAY_MASTER_ADD_TAIL );

	dump_list ( am );

	printf ( "Delete last item...\n");
	afc_array_master_del ( am );  // Delete the last element

	dump_list ( am );

	printf ( "Delete first item...\n");
	afc_array_master_first ( am );
	afc_array_master_del ( am );		// Delete the first element
	dump_list ( am );

	printf ( "Delete succ element...\n" );
	afc_array_master_next ( am );
	afc_array_master_del ( am );
	dump_list ( am );


	printf ( "Clearing all the Array...\n" );
	afc_array_master_clear ( am );	
	dump_list ( am );

	printf ( "Adding all 4 elements again...\n" );
	afc_array_master_add ( am, afc_string_dup ( "first" ), AFC_ARRAY_MASTER_ADD_TAIL );
	afc_array_master_add ( am, afc_string_dup ( "second" ), AFC_ARRAY_MASTER_ADD_TAIL );
	afc_array_master_add ( am, afc_string_dup ( "third" ), AFC_ARRAY_MASTER_ADD_TAIL );
	afc_array_master_add ( am, afc_string_dup ( "last" ), AFC_ARRAY_MASTER_ADD_TAIL );
	dump_list ( am );

	dump_list_reverse ( am );
	
	printf ( "Deleting item: \"first\"...\n" );
	afc_array_master_del ( am );
	dump_list_reverse ( am );

	afc_array_master_set_clear_func ( am, clear_func );

	afc_array_master_delete ( am );

	afc_delete ( afc );

	return ( 0 );
}
// }}}
#endif
