/*
 * Advanced Foundation Classes
 * Copyright (C) 2000/2025  Fabio Rotondo
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

// {{{ docs
/*
@config
	TITLE:     HashMaster
	VERSION:   1.30
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
@endnode

@node quote
*Nothing is faster than the speed of light... To prove this to yourself, try opening the refrigerator door before the light comes on.*

	Anonymous
@endnode

@node history
	- 1.30	- Added afc_hash_master_before_first() function
@endnode

@node intro
HashMaster is a class that's able to handle hash tables. Hash tables are particulary useful for ultra speed
items lookups. To each element in the table, you can assign an id (the so called /hash_value/) that will help
find quickly the element in the table.

Internally, elements are sorted by their /hash_value/ and accessed using a dicothomic search algo.

Main features of this class are:

- High search speed.
- Simple interface.

To inizialize a new instance, simply call afc_hash_master_new(), and to destroy it, call the afc_hash_master_delete().
To add a new value in the hash table, use the afc_hash_master_add() method. To get back a value use the afc_hash_master_find() method.
@endnode
*/
// }}}

#include "hash_master.h"

static const char class_name[] = "HashMaster";

static int afc_hash_master_internal_sort(const void *hd1, const void *hd2);

// {{{ struct afc_hash_master * afc_hash_master_new ()
/*
@node afc_hash_master_new

			 NAME: afc_hash_master_new ()    - Initializes a new afc_hash_master instance.

		 SYNOPSIS: struct afc_hash_master * afc_hash_master_new ( )

	  DESCRIPTION: This function initializes a new afc_hash_master instance.

			INPUT: NONE

		  RESULTS: a valid inizialized afc_hash_master structure. NULL in case of errors.

		 SEE ALSO: - afc_hash_master_delete()

@endnode
*/
struct afc_hash_master *afc_hash_master_new()
{
	TRY(HashMaster *)

	struct afc_hash_master *hash_master = (struct afc_hash_master *)afc_malloc(sizeof(struct afc_hash_master));

	if (hash_master == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "hash", NULL);

	hash_master->magic = AFC_HASH_MASTER_MAGIC;

	if ((hash_master->am = afc_array_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "am", NULL);

	RETURN(hash_master);

	EXCEPT
	afc_hash_master_delete(hash_master);

	FINALLY

	ENDTRY
}
// }}}
// {{{ int afc_hash_master_delete ( struct afc_hash_master * hash_master )
/*
@node afc_hash_master_delete

			 NAME: afc_hash_master_delete ( hash_master )  - Disposes a valid hash_master instance.

		 SYNOPSIS: int afc_hash_master_delete (struct afc_hash_master * hash_master)

	  DESCRIPTION: This function frees an already alloc'd afc_hash_master structure.

			INPUT: - hash_master  - Pointer to a valid afc_hash_master class.

		  RESULTS: should be AFC_ERR_NO_ERROR

			NOTES: - this method calls: afc_hash_master_clear()

		 SEE ALSO: - afc_hash_master_new()
				   - afc_hash_master_clear()
@endnode
*/
int _afc_hash_master_delete(struct afc_hash_master *hash_master)
{
	int afc_res;

	if ((afc_res = afc_hash_master_clear(hash_master)) != AFC_ERR_NO_ERROR)
		return afc_res;

	afc_array_delete(hash_master->am);
	afc_free(hash_master);

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ int afc_hash_master_clear ( struct afc_hash_master * hash_master )
/*
@node afc_hash_master_clear

			 NAME: afc_hash_master_clear ( hash_master )  - Clears all stored data

		 SYNOPSIS: int afc_hash_master_clear ( struct afc_hash_master * hash_master)

	  DESCRIPTION: Use this command to clear all stored data in the current hash_master instance.

			INPUT: - hash_master    - Pointer to a valid afc_hash_master instance.

		  RESULTS: should be AFC_ERR_NO_ERROR

		 SEE ALSO: - afc_hash_master_delete()

@endnode

*/
int afc_hash_master_clear(struct afc_hash_master *hash_master)
{
	HashData *hd;

	if (hash_master == NULL)
		return AFC_LOG_FAST(AFC_ERR_NULL_POINTER);

	if (hash_master->magic != AFC_HASH_MASTER_MAGIC)
		return AFC_LOG_FAST(AFC_ERR_INVALID_POINTER);

	if (hash_master->am)
	{
		hd = (HashData *)afc_array_first(hash_master->am);

		while (hd)
		{
			if (hash_master->func_clear)
				hash_master->func_clear(hash_master, hd->data);

			afc_free(hd);
			hd = (HashData *)afc_array_next(hash_master->am);
		}

		afc_array_clear(hash_master->am);
	}

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ int afc_hash_master_add ( HashMaster * hm, unsigned long long hash_value, void * data )
/*
@node afc_hash_master_add

			 NAME: afc_hash_master_add ( hash_master, hash_value, data )  - Add a new data to the hash table

		 SYNOPSIS: int afc_hash_master_add ( HashMaster * hash_master, unsigned long long hash_value, void * data )

	  DESCRIPTION: This function adds a new data element to the hash table.

			INPUT: - hash_master  - Pointer to a valid afc_hash_master class.
		 - hash_value   - Value key for the data being added
		 - data         - Data to add to the hash table

		  RESULTS: should be AFC_ERR_NO_ERROR

		 SEE ALSO: - afc_hash_master_find()
				   - afc_hash_master_del()
@endnode
*/
int afc_hash_master_add(HashMaster *hm, unsigned long int hash_value, void *data)
{
	HashData *hd = (HashData *)afc_malloc(sizeof(HashData));

	if (hd == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NO_MEMORY));

	hd->hash_value = hash_value;
	hd->data = data;

	afc_array_add(hm->am, hd, AFC_ARRAY_ADD_TAIL);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ void * afc_hash_master_find ( HashMaster * hm, unsigned long long hash_value )
/*
@node afc_hash_master_find

			 NAME: afc_hash_master_find ( hash_master, hash_value )  - Returns the data associated to the given hash_value

		 SYNOPSIS: void * afc_hash_master_find ( HashMaster * hash_master, unsigned long long hash_value )

	  DESCRIPTION: This function returns the data associated to the given hash_value. If the hash_value cannot be found,
				   you'll get a NULL return value.

			INPUT: - hash_master  - Pointer to a valid afc_hash_master class.
		 - hash_value   - Value key for the data being added

		  RESULTS: - a pointer containing the value of the desired data, or NULL if the hash_value is not present in the
					 HashMaster table.

		NOTES: - It is possible to have more than one single element associated to a given hash_value, and HashMaster
					 will simply return the first it finds, and not particulary the very first.

		 SEE ALSO: - afc_hash_master_add()
@endnode
*/
void *afc_hash_master_find(HashMaster *hm, unsigned long int hash_value)
{
	HashData *hd;
	register int max, pos, min = 0;

	// if ( afc_array_is_empty ( hm->am ) ) return ( NULL );
	if (hm->am->num_items == 0)
		return (NULL);

	if (hm->am->is_sorted == FALSE)
		afc_array_sort(hm->am, afc_hash_master_internal_sort);

	// max = afc_array_len ( hm->am );
	max = hm->am->num_items;
	pos = max / 2;

	while (pos != -1)
	{
		if ((hd = (HashData *)afc_array_item(hm->am, pos)) == NULL)
			return (NULL);

		if (hd->hash_value == hash_value)
			return (hd->data);

		if (hd->hash_value < hash_value)
			min = pos + 1;
		else
			max = pos - 1;

		if (min > max)
			return (NULL);

		pos = (max - min) / 2 + min;
	}

	return (NULL);
}
// }}}
// {{{ HashData * afc_hash_master_del ( HashMaster * hm )
/*
@node afc_hash_master_del

			 NAME: afc_hash_master_del ( hash_master )  - Deletes the current element

		 SYNOPSIS: HashData * afc_hash_master_del ( HashMaster * hash_master )

	  DESCRIPTION: This function removes the current element from the HashMaster list.

			INPUT: - hash_master  - Pointer to a valid afc_hash_master class.

		  RESULTS: - The value of the next element in the Hash Table. NULL if the Hash Table is empty.

		 SEE ALSO: - afc_hash_master_add()
@endnode
*/
void *afc_hash_master_del(HashMaster *hm)
{
	HashData *hd;

	if ((hd = afc_array_obj(hm->am)) == NULL)
		return (NULL); // If there is no current object, return NULL

	if (hm->func_clear)
		hm->func_clear(hm, hd->data); // Free associated data (if clear func exists)

	afc_free(hd); // Free the HashData added to the Array

	hd = afc_array_del(hm->am);
	return (hd ? hd->data : NULL);
}
// }}}
// {{{ HashData * afc_hash_master_item ( HashMaster * hm, int item )
/*
@node afc_hash_master_item

			 NAME: afc_hash_master_item ( hash_master, item )  - Returns the item-th element

		 SYNOPSIS: HashData * afc_hash_master_item ( HashMaster * hash_master, int item )

	  DESCRIPTION: This function returns the /item-th/ item. Since this function accesses the HashMaster table like it
				   was an array, it should be considered a very low-level function and it is useless in standard applications.

			INPUT:  - hash_master  - Pointer to a valid afc_hash_master class.
			- item         - The ordinal number of the item you want.

		  RESULTS: - a pointer to the next HashData structure. If NULL, then the HashMaster is now empty, or
					 there was no current object.

		 SEE ALSO: - afc_hash_master_find()
		 - afc_array_item()

@endnode
*/
/*
HashData * afc_hash_master_item ( HashMaster * hm, int item )
{
	return ( ( HashData * ) afc_array_item ( hm->am, item ) );
}
*/
// }}}
// {{{ void * afc_hash_master_first ( HashMaster * hm )
/*
@node afc_hash_master_first

			 NAME: afc_hash_master_first ( hm )  - Returns the first element in the hash

		 SYNOPSIS: void * afc_hash_master_first ( HashMaster * hash )

	  DESCRIPTION: This function returns the first element in the hash array.

			INPUT: - hash    - Pointer to a valid HashMaster instance.

		  RESULTS: - Returns the value of the first element in the Hash table. NULL if the hash table is empty.

		NOTE:  - Remember that the elements inside the hash table are *sorted* using the hash value.
			 So the first element you'll with this function would almost certainly be different from the
			 first element you've previously added with the afc_hash_master_add() function.

		 SEE ALSO: - afc_hash_master_add()
				   - afc_hash_master_del()
				   - afc_hash_master_next()
@endnode
*/
void *afc_hash_master_first(HashMaster *hm)
{
	HashData *hd;

	hd = afc_array_first(hm->am);
	if (hd == NULL)
		return (NULL);

	return (hd->data);
}
// }}}
// {{{ afc_hash_master_next ( hm )
/*
@node afc_hash_master_next

			 NAME: afc_hash_master_next ( hm )  - Returns the next element in the hash table

		 SYNOPSIS: void * afc_hash_master_next ( HashMaster * hash )

	  DESCRIPTION: This function returns the next element in the hash table.

			INPUT: - hash    - Pointer to a valid HashMaster instance.

		  RESULTS: - Returns the value of the next element in the Hash table. NULL if the hash table is empty or you
			 are trying to go over the last element.

		NOTE:  - Remember that the elements inside the hash table are *sorted* using the hash value.
			 The order you can get them back using afc_hash_master_first(), afc_hash_master_next() and
			 so on may not reflect the same insertion order you have followed with afc_hash_master_add().

		 SEE ALSO: - afc_hash_master_add()
				   - afc_hash_master_del()
				   - afc_hash_master_first()
				   - afc_hash_master_prev()
@endnode
*/
void *afc_hash_master_next(HashMaster *hm)
{
	HashData *hd;

	hd = afc_array_next(hm->am);
	if (hd == NULL)
		return (NULL);

	return (hd->data);
}
// }}}
// {{{ afc_hash_master_last ( hm )
/*
@node afc_hash_master_last

			 NAME: afc_hash_master_last ( hm )  - Returns the last element in the hash table

		 SYNOPSIS: void * afc_hash_master_last ( HashMaster * hash )

	  DESCRIPTION: This function returns the last element in the hash table.

			INPUT: - hash    - Pointer to a valid HashMaster instance.

		  RESULTS: - Returns the value of the last element in the Hash table. NULL if the hash table is empty.

		NOTE:  - Remember that the elements inside the hash table are *sorted* using the hash value.
			 The order you can get them back using afc_hash_master_first(), afc_hash_master_next() and
			 so on may not reflect the same insertion order you have followed with afc_hash_master_add().

		 SEE ALSO: - afc_hash_master_add()
				   - afc_hash_master_del()
				   - afc_hash_master_first()
				   - afc_hash_master_prev()
@endnode
*/
void *afc_hash_master_last(HashMaster *hm)
{
	HashData *hd;

	hd = afc_array_last(hm->am);
	if (hd == NULL)
		return (NULL);

	return (hd->data);
}
// }}}
// {{{ afc_hash_master_prev ( hm )
/*
@node afc_hash_master_prev

			 NAME: afc_hash_master_prev ( hm )  - Returns the previous element in the hash table

		 SYNOPSIS: void * afc_hash_master_prev ( HashMaster * hash )

	  DESCRIPTION: This function returns the next element in the hash table.

			INPUT: - hash    - Pointer to a valid HashMaster instance.

		  RESULTS: - Returns the value of the previous element in the Hash table. NULL if the hash table is empty, or
			 if you are trying to go over the first element.

		NOTE:  - Remember that the elements inside the hash table are *sorted* using the hash value.
			 The order you can get them back using afc_hash_master_first(), afc_hash_master_next() and
			 so on may not reflect the same insertion order you have followed with afc_hash_master_add().

		 SEE ALSO: - afc_hash_master_add()
				   - afc_hash_master_del()
				   - afc_hash_master_first()
				   - afc_hash_master_next()
@endnode
*/

void *afc_hash_master_prev(HashMaster *hm)
{
	HashData *hd;

	hd = afc_array_prev(hm->am);
	if (hd == NULL)
		return (NULL);

	return (hd->data);
}
// }}}
// {{{ int afc_hash_master_set_clear_func ( HashMaster * am, int ( * func ) ( HashMaster * hm, void * )  )
/*
@node afc_hash_master_set_clear_func

	   NAME: afc_hash_master_set_clear_func(am, func) - Sets the clear func

   SYNOPSIS: int afc_hash_master_set_clear_func (HashMaster * am, int ( *func ) ( void * ) )

	  SINCE: 1.10

DESCRIPTION: Use this command to set a clear function. The function will be called each time an
		 item is being deleted from the list with afc_hash_master_del() or afc_hash_master_clear().
		 To remove this function, pass a NULL value as function pointer.

	  INPUT: - am	- Pointer to a valid HashMaster class.
		 - func	- Function to be called in clearing operations.

	RESULTS: AFC_ERR_NO_ERROR

   SEE ALSO: - afc_hash_master_del()
		 - afc_hash_master_clear()
@endnode
*/
/*
int afc_hash_master_set_clear_func ( HashMaster * am, int ( *func ) ( HashMaster * hm, void * ) )
{
	am->func_clear = func;

	return ( AFC_ERR_NO_ERROR );
}
*/
// }}}
// {{{ int afc_hash_master_for_each ( HashMaster * hm, int ( * func ) ( HashMaster * am, void *, void *  ), void * info  )
/*
@node afc_hash_master_set_for_each

	   NAME: afc_hash_master_set_for_each(hm, func) - Sets the clear func

   SYNOPSIS: int afc_hash_master_set_for_each (HashMaster * hm, int ( *func ) ( HashMaster * am, int pos, void * v, void * info ), void * info )

	  SINCE: 1.10

DESCRIPTION: Use this command to traverse all elements inside the HashMaster.
		 The function called must return AFC_ERR_NO_ERROR when the traverse succeded and
		 a valid error otherwise. In case of error, the traverse is stopped and the error
		 returned.

	  INPUT: - hm	- Pointer to a valid HashMaster class.
		 - func	- Function to be called in the traverse.
			  Function prototype: int func ( HashMaster * am, int pos, void * v, void * info );
		 - info	- Additional param passed to the /func/ being called.

	RESULTS: AFC_ERR_NO_ERROR

   SEE ALSO: - afc_hash_master_first()
		 - afc_hash_master_next()
@endnode
*/
int afc_hash_master_for_each(HashMaster *hm, int (*func)(HashMaster *am, int pos, void *v, void *info), void *info)
{
	int res, t = 0;
	HashData *hd;

	hd = (HashData *)afc_array_first(hm->am);
	while (hd)
	{
		if ((res = func(hm, t++, hd->data, info)) != AFC_ERR_NO_ERROR)
			return (res);
		hd = afc_array_next(hm->am);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ int afc_hash_master_set_custom_sort ( HashMaster * am, void ( * f ) ( void * base, size_t n, size_t size, int ( *com ) ( const void *, const void *) ) )
/*
@node afc_hash_master_set_clear_func

	   NAME: afc_hash_master_set_clear_func(am, func) - Sets the clear func

   SYNOPSIS: int afc_hash_master_set_custom_sort ( HashMaster * am, void ( * func )  ( void * base, size_t nmemb, size_t size, int ( *compar ) ( const void *, const void *) ) )

	  SINCE: 1.20

DESCRIPTION: Use this command to set a sort routine different to glibc qsort. You should use this function only if you
		 know what you are doing and your glibc implementation of qsort is buggy.

	  INPUT: - am	- Pointer to a valid HashMaster class.
		 - func	- Function to be called in sort operations.

	RESULTS: AFC_ERR_NO_ERROR
@endnode
*/
/*
int afc_hash_master_set_custom_sort ( HashMaster * hm, void ( * func )  ( void * base, size_t nmemb, size_t size, int ( *compar ) ( const void *, const void *) ) )
{
	hm->am->custom_sort = func;

	return ( AFC_ERR_NO_ERROR );
}
*/
// }}}
// {{{ afc_hash_master_before_first ( hm ) ***************
/*
int afc_hash_master_before_first ( HashMaster * hm )
{
	return ( afc_array_before_first ( hm->am ) );
}
*/
// }}}

/* =====================================================================================
	 INTERNAL FUNCTIONS
===================================================================================== */
// {{{ afc_hash_master_internal_sort ( hd1, hd2 )
static int afc_hash_master_internal_sort(const void *hd1, const void *hd2)
{
	unsigned long int v1 = ((HashData *)(*(HashData **)hd1))->hash_value;
	unsigned long int v2 = ((HashData *)(*(HashData **)hd2))->hash_value;

	if (v1 > v2)
		return (1);
	return (v1 < v2 ? -1 : 0);
}
// }}}

#ifdef TEST_HASH_MASTER
int main()
{
	HashMaster *hm = afc_hash_master_new();

	afc_hash_master_add(hm, 1, afc_string_dup("Ciao Fabio"));
	afc_hash_master_add(hm, 2, afc_string_dup("Ciao Pippo"));

	printf("1: %s\n", afc_hash_master_find(hm, 1));
	printf("2: %s\n", afc_hash_master_find(hm, 2));

	afc_hash_master_del(hm);

	return (0);
}
#endif
