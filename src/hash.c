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
	TITLE:     Hash
	VERSION:   1.30
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
@endnode

@node quote
*Nothing is faster than the speed of light... To prove this to yourself, try opening the refrigerator door before the light comes on.*

	Anonymous
@endnode

@node history
	- 1.30	- Added afc_hash_before_first() function
@endnode

@node intro
Hash is a class that's able to handle hash tables. Hash tables are particulary useful for ultra speed
items lookups. To each element in the table, you can assign an id (the so called /hash_value/) that will help
find quickly the element in the table.

Internally, elements are sorted by their /hash_value/ and accessed using a dicothomic search algo.

Main features of this class are:

- High search speed.
- Simple interface.

To inizialize a new instance, simply call afc_hash_new(), and to destroy it, call the afc_hash_delete().
To add a new value in the hash table, use the afc_hash_add() method. To get back a value use the afc_hash_find() method.
@endnode
*/
// }}}

#include "hash.h"

static const char class_name[] = "Hash";

static int afc_hash_internal_sort(const void *hd1, const void *hd2);

// {{{ struct afc_hash * afc_hash_new ()
/*
@node afc_hash_new

			 NAME: afc_hash_new ()    - Initializes a new afc_hash instance.

		 SYNOPSIS: struct afc_hash * afc_hash_new ( )

	  DESCRIPTION: This function initializes a new afc_hash instance.

			INPUT: NONE

		  RESULTS: a valid inizialized afc_hash structure. NULL in case of errors.

		 SEE ALSO: - afc_hash_delete()

@endnode
*/
struct afc_hash *afc_hash_new()
{
	TRY(Hash *)

	struct afc_hash *hash = (struct afc_hash *)afc_malloc(sizeof(struct afc_hash));

	if (hash == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "hash", NULL);

	hash->magic = AFC_HASH_MAGIC;

	if ((hash->am = afc_array_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "am", NULL);

	RETURN(hash);

	EXCEPT
	afc_hash_delete(hash);

	FINALLY

	ENDTRY
}
// }}}
// {{{ int afc_hash_delete ( struct afc_hash * hash )
/*
@node afc_hash_delete

			 NAME: afc_hash_delete ( hash )  - Disposes a valid hash instance.

		 SYNOPSIS: int afc_hash_delete (struct afc_hash * hash)

	  DESCRIPTION: This function frees an already alloc'd afc_hash structure.

			INPUT: - hash  - Pointer to a valid afc_hash class.

		  RESULTS: should be AFC_ERR_NO_ERROR

			NOTES: - this method calls: afc_hash_clear()

		 SEE ALSO: - afc_hash_new()
				   - afc_hash_clear()
@endnode
*/
int _afc_hash_delete(struct afc_hash *hash)
{
	int afc_res;

	if ((afc_res = afc_hash_clear(hash)) != AFC_ERR_NO_ERROR)
		return afc_res;

	afc_array_delete(hash->am);
	afc_free(hash);

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ int afc_hash_clear ( struct afc_hash * hash )
/*
@node afc_hash_clear

			 NAME: afc_hash_clear ( hash )  - Clears all stored data

		 SYNOPSIS: int afc_hash_clear ( struct afc_hash * hash)

	  DESCRIPTION: Use this command to clear all stored data in the current hash instance.

			INPUT: - hash    - Pointer to a valid afc_hash instance.

		  RESULTS: should be AFC_ERR_NO_ERROR

		 SEE ALSO: - afc_hash_delete()

@endnode

*/
int afc_hash_clear(struct afc_hash *hash)
{
	HashData *hd;

	if (hash == NULL)
		return AFC_LOG_FAST(AFC_ERR_NULL_POINTER);

	if (hash->magic != AFC_HASH_MAGIC)
		return AFC_LOG_FAST(AFC_ERR_INVALID_POINTER);

	if (hash->am)
	{
		hd = (HashData *)afc_array_first(hash->am);

		while (hd)
		{
			if (hash->func_clear)
				hash->func_clear(hash, hd->data);

			afc_free(hd);
			hd = (HashData *)afc_array_next(hash->am);
		}

		afc_array_clear(hash->am);
	}

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ int afc_hash_add ( Hash * hm, unsigned long long hash_value, void * data )
/*
@node afc_hash_add

			 NAME: afc_hash_add ( hash, hash_value, data )  - Add a new data to the hash table

		 SYNOPSIS: int afc_hash_add ( Hash * hash, unsigned long long hash_value, void * data )

	  DESCRIPTION: This function adds a new data element to the hash table.

			INPUT: - hash  - Pointer to a valid afc_hash class.
		 - hash_value   - Value key for the data being added
		 - data         - Data to add to the hash table

		  RESULTS: should be AFC_ERR_NO_ERROR

		 SEE ALSO: - afc_hash_find()
				   - afc_hash_del()
@endnode
*/
int afc_hash_add(Hash *hm, unsigned long int hash_value, void *data)
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
// {{{ void * afc_hash_find ( Hash * hm, unsigned long long hash_value )
/*
@node afc_hash_find

			 NAME: afc_hash_find ( hash, hash_value )  - Returns the data associated to the given hash_value

		 SYNOPSIS: void * afc_hash_find ( Hash * hash, unsigned long long hash_value )

	  DESCRIPTION: This function returns the data associated to the given hash_value. If the hash_value cannot be found,
				   you'll get a NULL return value.

			INPUT: - hash  - Pointer to a valid afc_hash class.
		 - hash_value   - Value key for the data being added

		  RESULTS: - a pointer containing the value of the desired data, or NULL if the hash_value is not present in the
					 Hash table.

		NOTES: - It is possible to have more than one single element associated to a given hash_value, and Hash
					 will simply return the first it finds, and not particulary the very first.

		 SEE ALSO: - afc_hash_add()
@endnode
*/
void *afc_hash_find(Hash *hm, unsigned long int hash_value)
{
	HashData *hd;
	register int max, pos, min = 0;

	// if ( afc_array_is_empty ( hm->am ) ) return ( NULL );
	if (hm->am->num_items == 0)
		return (NULL);

	if (hm->am->is_sorted == FALSE)
		afc_array_sort(hm->am, afc_hash_internal_sort);

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
// {{{ HashData * afc_hash_del ( Hash * hm )
/*
@node afc_hash_del

			 NAME: afc_hash_del ( hash )  - Deletes the current element

		 SYNOPSIS: HashData * afc_hash_del ( Hash * hash )

	  DESCRIPTION: This function removes the current element from the Hash list.

			INPUT: - hash  - Pointer to a valid afc_hash class.

		  RESULTS: - The value of the next element in the Hash Table. NULL if the Hash Table is empty.

		 SEE ALSO: - afc_hash_add()
@endnode
*/
void *afc_hash_del(Hash *hm)
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
// {{{ HashData * afc_hash_item ( Hash * hm, int item )
/*
@node afc_hash_item

			 NAME: afc_hash_item ( hash, item )  - Returns the item-th element

		 SYNOPSIS: HashData * afc_hash_item ( Hash * hash, int item )

	  DESCRIPTION: This function returns the /item-th/ item. Since this function accesses the Hash table like it
				   was an array, it should be considered a very low-level function and it is useless in standard applications.

			INPUT:  - hash  - Pointer to a valid afc_hash class.
			- item         - The ordinal number of the item you want.

		  RESULTS: - a pointer to the next HashData structure. If NULL, then the Hash is now empty, or
					 there was no current object.

		 SEE ALSO: - afc_hash_find()
		 - afc_array_item()

@endnode
*/
/*
HashData * afc_hash_item ( Hash * hm, int item )
{
	return ( ( HashData * ) afc_array_item ( hm->am, item ) );
}
*/
// }}}
// {{{ void * afc_hash_first ( Hash * hm )
/*
@node afc_hash_first

			 NAME: afc_hash_first ( hm )  - Returns the first element in the hash

		 SYNOPSIS: void * afc_hash_first ( Hash * hash )

	  DESCRIPTION: This function returns the first element in the hash array.

			INPUT: - hash    - Pointer to a valid Hash instance.

		  RESULTS: - Returns the value of the first element in the Hash table. NULL if the hash table is empty.

		NOTE:  - Remember that the elements inside the hash table are *sorted* using the hash value.
			 So the first element you'll with this function would almost certainly be different from the
			 first element you've previously added with the afc_hash_add() function.

		 SEE ALSO: - afc_hash_add()
				   - afc_hash_del()
				   - afc_hash_next()
@endnode
*/
void *afc_hash_first(Hash *hm)
{
	HashData *hd;

	hd = afc_array_first(hm->am);
	if (hd == NULL)
		return (NULL);

	return (hd->data);
}
// }}}
// {{{ afc_hash_next ( hm )
/*
@node afc_hash_next

			 NAME: afc_hash_next ( hm )  - Returns the next element in the hash table

		 SYNOPSIS: void * afc_hash_next ( Hash * hash )

	  DESCRIPTION: This function returns the next element in the hash table.

			INPUT: - hash    - Pointer to a valid Hash instance.

		  RESULTS: - Returns the value of the next element in the Hash table. NULL if the hash table is empty or you
			 are trying to go over the last element.

		NOTE:  - Remember that the elements inside the hash table are *sorted* using the hash value.
			 The order you can get them back using afc_hash_first(), afc_hash_next() and
			 so on may not reflect the same insertion order you have followed with afc_hash_add().

		 SEE ALSO: - afc_hash_add()
				   - afc_hash_del()
				   - afc_hash_first()
				   - afc_hash_prev()
@endnode
*/
void *afc_hash_next(Hash *hm)
{
	HashData *hd;

	hd = afc_array_next(hm->am);
	if (hd == NULL)
		return (NULL);

	return (hd->data);
}
// }}}
// {{{ afc_hash_last ( hm )
/*
@node afc_hash_last

			 NAME: afc_hash_last ( hm )  - Returns the last element in the hash table

		 SYNOPSIS: void * afc_hash_last ( Hash * hash )

	  DESCRIPTION: This function returns the last element in the hash table.

			INPUT: - hash    - Pointer to a valid Hash instance.

		  RESULTS: - Returns the value of the last element in the Hash table. NULL if the hash table is empty.

		NOTE:  - Remember that the elements inside the hash table are *sorted* using the hash value.
			 The order you can get them back using afc_hash_first(), afc_hash_next() and
			 so on may not reflect the same insertion order you have followed with afc_hash_add().

		 SEE ALSO: - afc_hash_add()
				   - afc_hash_del()
				   - afc_hash_first()
				   - afc_hash_prev()
@endnode
*/
void *afc_hash_last(Hash *hm)
{
	HashData *hd;

	hd = afc_array_last(hm->am);
	if (hd == NULL)
		return (NULL);

	return (hd->data);
}
// }}}
// {{{ afc_hash_prev ( hm )
/*
@node afc_hash_prev

			 NAME: afc_hash_prev ( hm )  - Returns the previous element in the hash table

		 SYNOPSIS: void * afc_hash_prev ( Hash * hash )

	  DESCRIPTION: This function returns the next element in the hash table.

			INPUT: - hash    - Pointer to a valid Hash instance.

		  RESULTS: - Returns the value of the previous element in the Hash table. NULL if the hash table is empty, or
			 if you are trying to go over the first element.

		NOTE:  - Remember that the elements inside the hash table are *sorted* using the hash value.
			 The order you can get them back using afc_hash_first(), afc_hash_next() and
			 so on may not reflect the same insertion order you have followed with afc_hash_add().

		 SEE ALSO: - afc_hash_add()
				   - afc_hash_del()
				   - afc_hash_first()
				   - afc_hash_next()
@endnode
*/

void *afc_hash_prev(Hash *hm)
{
	HashData *hd;

	hd = afc_array_prev(hm->am);
	if (hd == NULL)
		return (NULL);

	return (hd->data);
}
// }}}
// {{{ int afc_hash_set_clear_func ( Hash * am, int ( * func ) ( Hash * hm, void * )  )
/*
@node afc_hash_set_clear_func

	   NAME: afc_hash_set_clear_func(am, func) - Sets the clear func

   SYNOPSIS: int afc_hash_set_clear_func (Hash * am, int ( *func ) ( void * ) )

	  SINCE: 1.10

DESCRIPTION: Use this command to set a clear function. The function will be called each time an
		 item is being deleted from the list with afc_hash_del() or afc_hash_clear().
		 To remove this function, pass a NULL value as function pointer.

	  INPUT: - am	- Pointer to a valid Hash class.
		 - func	- Function to be called in clearing operations.

	RESULTS: AFC_ERR_NO_ERROR

   SEE ALSO: - afc_hash_del()
		 - afc_hash_clear()
@endnode
*/
/*
int afc_hash_set_clear_func ( Hash * am, int ( *func ) ( Hash * hm, void * ) )
{
	am->func_clear = func;

	return ( AFC_ERR_NO_ERROR );
}
*/
// }}}
// {{{ int afc_hash_for_each ( Hash * hm, int ( * func ) ( Hash * am, void *, void *  ), void * info  )
/*
@node afc_hash_set_for_each

	   NAME: afc_hash_set_for_each(hm, func) - Sets the clear func

   SYNOPSIS: int afc_hash_set_for_each (Hash * hm, int ( *func ) ( Hash * am, int pos, void * v, void * info ), void * info )

	  SINCE: 1.10

DESCRIPTION: Use this command to traverse all elements inside the Hash.
		 The function called must return AFC_ERR_NO_ERROR when the traverse succeded and
		 a valid error otherwise. In case of error, the traverse is stopped and the error
		 returned.

	  INPUT: - hm	- Pointer to a valid Hash class.
		 - func	- Function to be called in the traverse.
			  Function prototype: int func ( Hash * am, int pos, void * v, void * info );
		 - info	- Additional param passed to the /func/ being called.

	RESULTS: AFC_ERR_NO_ERROR

   SEE ALSO: - afc_hash_first()
		 - afc_hash_next()
@endnode
*/
int afc_hash_for_each(Hash *hm, int (*func)(Hash *am, int pos, void *v, void *info), void *info)
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
// {{{ int afc_hash_set_custom_sort ( Hash * am, void ( * f ) ( void * base, size_t n, size_t size, int ( *com ) ( const void *, const void *) ) )
/*
@node afc_hash_set_clear_func

	   NAME: afc_hash_set_clear_func(am, func) - Sets the clear func

   SYNOPSIS: int afc_hash_set_custom_sort ( Hash * am, void ( * func )  ( void * base, size_t nmemb, size_t size, int ( *compar ) ( const void *, const void *) ) )

	  SINCE: 1.20

DESCRIPTION: Use this command to set a sort routine different to glibc qsort. You should use this function only if you
		 know what you are doing and your glibc implementation of qsort is buggy.

	  INPUT: - am	- Pointer to a valid Hash class.
		 - func	- Function to be called in sort operations.

	RESULTS: AFC_ERR_NO_ERROR
@endnode
*/
/*
int afc_hash_set_custom_sort ( Hash * hm, void ( * func )  ( void * base, size_t nmemb, size_t size, int ( *compar ) ( const void *, const void *) ) )
{
	hm->am->custom_sort = func;

	return ( AFC_ERR_NO_ERROR );
}
*/
// }}}
// {{{ afc_hash_before_first ( hm ) ***************
/*
int afc_hash_before_first ( Hash * hm )
{
	return ( afc_array_before_first ( hm->am ) );
}
*/
// }}}

/* =====================================================================================
	 INTERNAL FUNCTIONS
===================================================================================== */
// {{{ afc_hash_internal_sort ( hd1, hd2 )
static int afc_hash_internal_sort(const void *hd1, const void *hd2)
{
	unsigned long int v1 = ((HashData *)(*(HashData **)hd1))->hash_value;
	unsigned long int v2 = ((HashData *)(*(HashData **)hd2))->hash_value;

	if (v1 > v2)
		return (1);
	return (v1 < v2 ? -1 : 0);
}
// }}}

#ifdef TEST_HASH
int main()
{
	Hash *hm = afc_hash_new();

	afc_hash_add(hm, 1, afc_string_dup("Ciao Fabio"));
	afc_hash_add(hm, 2, afc_string_dup("Ciao Pippo"));

	printf("1: %s\n", afc_hash_find(hm, 1));
	printf("2: %s\n", afc_hash_find(hm, 2));

	afc_hash_del(hm);

	return (0);
}
#endif
