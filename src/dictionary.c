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

#include "dictionary.h"

// {{{ intro doc
/*
@config
	TITLE:     Dictionary
	VERSION:   1.30
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
@endnode

@node quote
*I'd like to share a revelation that I've had, during my time here. It came to me when I tried to classify your*
*species. I realized that you're not actually mammals. Every mammal on this planet instinctively develops a natural equilibrium*
*with the surrounding environment, but you humans do not. You move to an area, and you multiply, and multiply, until every*
*natural resource is consumed. The only way you can survive is to spread to another area. There is another organism on this*
*planet that follows the same pattern. A virus. Human beings are a disease, a cancer of this planet, you are a plague, and we*
*are the cure.*

Agent Smith, *The Matrix*
@endnode

@node history
	- 1.30	- Added afc_dictionary_before_first() function
@endnode

@node intro
Dictionary is an HashTable class extension specialized in items where the key is a string.
Main features of this class are:

- Incredibly fast search routine.
- Offers APIs to browse all values in the Dictionary like it was a double linked list.
  See functions like afc_dictionary_first(), afc_dictionary_next() and such.

To inizialize a new instance, simply call afc_dictionary_new(), and to destroy it, call the
afc_dictionary_delete().

To set new values in the dictionary, call afc_dictionary_set(), to delete a specified item, call afc_dictionary_del_item()
and to get a key value, call afc_dictionary_get().


@endnode
*/
// }}}

static const char class_name[] = "Dictionary";
static DictionaryData *afc_dictionary_internal_find(Dictionary *dict, const char *key);

// {{{ afc_dictionary_key_new ()
/*
@node afc_dictionary_new

			 NAME: afc_dictionary_new ()    - Initializes a new afc_dictionary instance.

		 SYNOPSIS: struct afc_dictionary * afc_dictionary_new ()

	  DESCRIPTION: This function initializes a new afc_dictionary instance.

			INPUT: NONE

		  RESULTS: a valid inizialized afc_dictionary structure. NULL in case of errors.

		 SEE ALSO: - afc_dictionary_delete()

@endnode
*/
struct afc_dictionary *afc_dictionary_new()
{
	TRY(Dictionary *)

	Dictionary *dictionary = afc_malloc(sizeof(struct afc_dictionary));

	if (dictionary == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "dictionary", NULL);

	dictionary->magic = AFC_DICTIONARY_MAGIC;

	if ((dictionary->hash = afc_hash_master_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "hash", NULL);

	RETURN(dictionary);

	EXCEPT
	afc_dictionary_delete(dictionary);

	FINALLY

	ENDTRY
}
// }}}
// {{{ afc_dictionary_delete ( dict )
/*
@node afc_dictionary_delete

			 NAME: afc_dictionary_delete ( dictionary )  - Disposes a valid dictionary instance.

		 SYNOPSIS: int afc_dictionary_delete (struct afc_dictionary * dictionary)

	  DESCRIPTION: This function frees an already alloc'd afc_dictionary structure.

			INPUT: - dictionary  - Pointer to a valid afc_dictionary class.

		  RESULTS: should be AFC_ERR_NO_ERROR

			NOTES: - this method calls: afc_dictionary_clear()

		 SEE ALSO: - afc_dictionary_new()
				   - afc_dictionary_clear()
@endnode
*/
int _afc_dictionary_delete(struct afc_dictionary *dictionary)
{
	int afc_res;

	if ((afc_res = afc_dictionary_clear(dictionary)) != AFC_ERR_NO_ERROR)
		return (afc_res);

	afc_hash_master_delete(dictionary->hash);
	afc_free(dictionary);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dictionary_clear ( dict )
/*
@node afc_dictionary_clear

			 NAME: afc_dictionary_clear ( dictionary )  - Clears all stored data

		 SYNOPSIS: int afc_dictionary_clear ( struct afc_dictionary * dictionary)

	  DESCRIPTION: Use this function to clear all stored data in the current dictionary instance.

			INPUT: - dictionary    - Pointer to a valid afc_dictionary instance.

		  RESULTS: should be AFC_ERR_NO_ERROR

		 SEE ALSO: - afc_dictionary_delete()

@endnode

*/
int afc_dictionary_clear(struct afc_dictionary *dictionary)
{
	DictionaryData *ddata;
	HashData *hd;

	if (dictionary == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));
	if (dictionary->magic != AFC_DICTIONARY_MAGIC)
		return (AFC_LOG_FAST(AFC_ERR_INVALID_POINTER));

	if (dictionary->hash)
	{
		afc_array_before_first(dictionary->hash->am); // Move through all Array items
		while ((hd = afc_array_next(dictionary->hash->am)))
		{
			ddata = hd->data; // Get the DictionaryData inside

			if (dictionary->func_clear)
				dictionary->func_clear(ddata->value);

			afc_string_delete(ddata->key); // Dispose the Dictionary Item key
			afc_free(ddata);			   // Free the dictionary Item
		}
		afc_hash_master_clear(dictionary->hash); // Clears the HashMaster
	}

	dictionary->curr_data = NULL; // Set no current data

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dictionary_set ( dict, key, data )
/*
@node afc_dictionary_set

			 NAME: afc_dictionary_set ( dictionary, key, data )  - Sets a key in the dictionary

		 SYNOPSIS: int afc_dictionary_set ( struct afc_dictionary * dictionary, const char * key, void * data )

	  DESCRIPTION: Use this function to add a new key to the current dictionary. This method works in two different ways:

				   1. If the passed key is already set in the dictionary, the new data will replace the existing one
				   2. If the key is not set yet, a new key is created and inserted inside the dictionary, and the data will be set.

			INPUT: 	- dictionary    - Pointer to a valid afc_dictionary instance.
			- key 		- afc_string_new containing the key of the item in the dictionary.
					- data         	- Data to assign to this key.

		  RESULTS: should be AFC_ERR_NO_ERROR

		 SEE ALSO: 	- afc_dictionary_get()
			- afc_dictionary_get_default()
					- afc_dictionary_del()
			- afc_dictionary_del_item()
@endnode
*/
int afc_dictionary_set(Dictionary *dict, const char *key, void *data)
{
	DictionaryData *ddata = NULL;

	if (dict == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));
	if (dict->magic != AFC_DICTIONARY_MAGIC)
		return (AFC_LOG_FAST(AFC_ERR_INVALID_POINTER));

	if (!dict->skip_find)
		ddata = afc_dictionary_internal_find(dict, key); // First of all, we look for the key

	if (ddata == NULL)
	{
		if (data == NULL)
			return (AFC_ERR_NO_ERROR); // If the key does not exists and data is NULL,
									   // Simply exit without adding a NULL key

		ddata = (DictionaryData *)afc_malloc(sizeof(DictionaryData)); // If the key is not found, we create a new DictionaryData

		if (ddata == NULL)
			return (AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "DictionaryData"));

		if ((ddata->key = afc_string_dup(key)) == NULL) // Duplicate the key
		{
			afc_free(ddata);
			return (AFC_ERR_NO_MEMORY);
		}

		if (afc_hash_master_add(dict->hash, afc_string_hash((unsigned char *)key, afc_string_len(ddata->key)), ddata) != AFC_ERR_NO_ERROR) // Add the new entry in the Dictionary
		{
			afc_string_delete(ddata->key);
			afc_free(ddata);
			return (AFC_LOG(AFC_LOG_ERROR, AFC_DICTIONARY_ERR_HASHING, "Error during Hashing of this key", key));
		}
	}
	else
	{
		// If the ddata already exists, we have to call the func_clear (if set) before
		// resetting its value.

		if (dict->func_clear)
			dict->func_clear(ddata->value);
		ddata->value = NULL;
	}

	if (data != NULL) // If there is a data value, then we add it
	{
		ddata->value = data;
		dict->curr_data = ddata;
	}
	else
		afc_dictionary_del(dict); // Else we delete the current item

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dictionary_get ( dict, key )
/*
@node afc_dictionary_get

			 NAME: afc_dictionary_get ( dictionary, key )  - Retrieves the data of a specified key

		 SYNOPSIS: void * afc_dictionary_get ( struct afc_dictionary * dictionary, const char * key )

	  DESCRIPTION: Use this function to get the data stored in a key.

			INPUT: 	- dictionary    - Pointer to a valid afc_dictionary instance.
			- key 		- afc_string_new containing the key of the item in the dictionary.

		  RESULTS: the value binded to the key, or NULL if the key cannot be found.

		 SEE ALSO: 	- afc_dictionary_set()
					- afc_dictionary_del()
			- afc_dictionary_del_item()
@endnode
*/
void *afc_dictionary_get(Dictionary *dict, const char *key)
{
	DictionaryData *ddata;

	if (dict == NULL)
	{
		AFC_LOG_FAST(AFC_ERR_NULL_POINTER);
		return (NULL);
	}
	if (dict->magic != AFC_DICTIONARY_MAGIC)
	{
		AFC_LOG_FAST(AFC_ERR_INVALID_POINTER);
		return (NULL);
	}

	ddata = afc_dictionary_internal_find(dict, key);

	if (ddata == NULL)
		return (NULL);

	return (ddata->value);
}
// }}}
// {{{ afc_dictionary_get_default ( dict, key, def_val )
/*
@node afc_dictionary_get_default

			 NAME: afc_dictionary_get_default ( dictionary, key, def_val )  - Retrieves the data of a specified key

		 SYNOPSIS: void * afc_dictionary_set ( struct afc_dictionary * dictionary, const char * key, void * def_val )

	  DESCRIPTION: Use this function to get the data stored in a key. If the key cannot be found, the default value
			passed in will be returned.

			INPUT: 	- dictionary    - Pointer to a valid afc_dictionary instance.
			- key 		- afc_string_new containing the key of the item in the dictionary.
			- def_val	- Default value returned when the key is not present in the dictionary.

		  RESULTS: the value binded to the key, or the default value.

		 SEE ALSO: 	- afc_dictionary_get()
					- afc_dictionary_del()
			- afc_dictionary_del_item()
@endnode
*/
void *afc_dictionary_get_default(Dictionary *dict, const char *key, void *def_val)
{
	void *s;

	s = afc_dictionary_get(dict, key);

	if (s == NULL)
		return (def_val);

	return (s);
}
// }}}
// {{{ afc_dictionary_first ( dict )
/*
@node afc_dictionary_first

			 NAME: afc_dictionary_first ( dictionary )  - Returns the first element in the Dictionary

		 SYNOPSIS: void * afc_dictionary_first ( struct afc_dictionary * dictionary )

	  DESCRIPTION: Use this function to get the first element in the Dictionary instance.
				   Please, keep in mind that items inside an Hash Table are stored in random
				   order, so the first element may not be the first you have inserted.

			INPUT: - dictionary    - Pointer to a valid afc_dictionary instance.

		  RESULTS: the value desired or NULL if the Dictionary is empty.

		 SEE ALSO: - afc_dictionary_next()
				   - afc_dictionary_prev()
@endnode
*/
void *afc_dictionary_first(Dictionary *dict)
{
	HashData *hmdata = (HashData *)afc_array_first(dict->hash->am);

	if (hmdata == NULL)
		return (NULL);

	dict->curr_data = hmdata->data;

	if (dict->curr_data == NULL)
		return (NULL);

	return (dict->curr_data->value);
}
// }}}
// {{{ afc_dictionary_next ( dict )
/*
@node afc_dictionary_next

			 NAME: afc_dictionary_next ( dictionary )  - Returns the next element in the Dictionary

		 SYNOPSIS: void * afc_dictionary_next ( struct afc_dictionary * dictionary )

	  DESCRIPTION: Use this function to get the next element in the Dictionary instance.
				   Please, keep in mind that items inside an Hash Table are stored in random
				   order, so the first element may not be the first you have inserted.

			INPUT: - dictionary    - Pointer to a valid afc_dictionary instance.

		  RESULTS: the value desired or NULL if the Dictionary is empty or you were already pointing to the last item.

		 SEE ALSO: - afc_dictionary_first()
				   - afc_dictionary_prev()
@endnode
*/
void *afc_dictionary_next(Dictionary *dict)
{
	HashData *hmdata = (HashData *)afc_array_next(dict->hash->am);

	if (hmdata == NULL)
		return (NULL);

	dict->curr_data = hmdata->data;

	if (dict->curr_data == NULL)
		return (NULL);

	return (dict->curr_data->value);
}
// }}}
// {{{ afc_dictionary_prev ( dict )
/*
@node afc_dictionary_prev

			 NAME: afc_dictionary_prev ( dictionary )  - Returns the previous element in the Dictionary

		 SYNOPSIS: void * afc_dictionary_prev ( struct afc_dictionary * dictionary )

	  DESCRIPTION: Use this function to get the previous element in the Dictionary instance.
				   Please, keep in mind that items inside an Hash Table are stored in random
				   order, so the first element may not be the first you have inserted.

			INPUT: - dictionary    - Pointer to a valid afc_dictionary instance.

		  RESULTS: the value desired or NULL if the Dictionary is empty or you are already pointing on the first item.

		 SEE ALSO: - afc_dictionary_next()
				   - afc_dictionary_first()
@endnode
*/
void *afc_dictionary_prev(Dictionary *dict)
{
	HashData *hmdata = (HashData *)afc_array_prev(dict->hash->am);

	if (hmdata == NULL)
		return (NULL);

	dict->curr_data = hmdata->data;

	if (dict->curr_data == NULL)
		return (NULL);

	return (dict->curr_data->value);
}
// }}}
// {{{ afc_dictionary_del ( dict )
/*
@node afc_dictionary_del

			 NAME: afc_dictionary_del ( dictionary )  - Deletes the current element

		 SYNOPSIS: void * afc_dictionary_del ( struct afc_dictionary * dictionary )

	  DESCRIPTION: Use this function to delete the element you are currently pointing to.

			INPUT: - dictionary    - Pointer to a valid afc_dictionary instance.

		  RESULTS: the value of the next avaible item or NULL if the Dictionary is empty.

		 SEE ALSO: - afc_dictionary_del_item()
				   - afc_dictionary_set()
@endnode
*/
void *afc_dictionary_del(Dictionary *dict)
{
	if (dict->curr_data == NULL)
		return (AFC_ERR_NO_ERROR); // If no item is set, simply exit

	if (dict->func_clear)
		dict->func_clear(dict->curr_data->value); // Clear the item from memory

	afc_string_delete(dict->curr_data->key); // Free the Item KEY
	afc_free(dict->curr_data);				 // Free current item data

	dict->curr_data = afc_hash_master_del(dict->hash); // Remove from the Array Master

	if (dict->curr_data == NULL)
		return (NULL); // Check against NULL

	return (dict->curr_data->value); // And return the next item data
}
// }}}
// {{{ afc_dictionary_del_item ( dict, key )
/*
@node afc_dictionary_del_item

			 NAME: afc_dictionary_del_item ( dictionary, key )  - Deletes the item provieded

		 SYNOPSIS: int afc_dictionary_del_item ( struct afc_dictionary * dictionary, char * key )

	  DESCRIPTION: Use this function to delete the specified element.

			INPUT: - dictionary    - Pointer to a valid afc_dictionary instance.
				   - key           - Key of the item to delete

		  RESULTS: should be AFC_ERR_NO_ERROR, or a return value if something went wrong.

		 SEE ALSO: - afc_dictionary_del()
				   - afc_dictionary_set()
@endnode
*/
int afc_dictionary_del_item(Dictionary *dict, const char *key)
{
	DictionaryData *ddata = afc_dictionary_internal_find(dict, key);

	if (ddata == NULL)
		return (AFC_LOG(AFC_LOG_WARNING, AFC_DICTIONARY_ERR_NOT_FOUND, "Not found in dictionary", key));

	afc_dictionary_del(dict);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dictionary_obj ( dict )
/*
@node afc_dictionary_obj

			 NAME: afc_dictionary_obj ( dictionary )  - Returns the current Dictionary item value

		 SYNOPSIS: int afc_dictionary_obj ( struct afc_dictionary * dictionary )

	  DESCRIPTION: Use this function to get the current item value.

			INPUT: - dictionary    - Pointer to a valid afc_dictionary instance.

		  RESULTS: the value of the current avaible item or NULL if the Dictionary is empty.

		 SEE ALSO: - afc_dictionary_get_key()
				   - afc_dictionary_first()
				   - afc_dictionary_next()
				   - afc_dictionary_prev()
@endnode
*/
/*
DictionaryData * afc_dictionary_obj ( Dictionary * dict )
{
	return ( dict->curr_data->value );
}
*/
// }}}
// {{{ afc_dictionary_len ( dict )
/*
@node afc_dictionary_len

			 NAME: afc_dictionary_len ( dictionary )  - Returns the number of keys in the Dictionary

		 SYNOPSIS: int afc_dictionary_len ( Dictionary * dictionary )

	  DESCRIPTION: Use this function to get the number of keys present in the Dictionary.

			INPUT: - dictionary    - Pointer to a valid afc_dictionary instance.

		  RESULTS: the number of keys.

		 SEE ALSO: - afc_dictionary_first()
				   - afc_dictionary_next()
				   - afc_dictionary_prev()
			   - afc_array_len()
@endnode
*/
/*
unsigned long int afc_dictionary_len ( Dictionary * dict )
{
	return ( afc_array_len ( dict->hash->am ) );
}
*/
// }}}
// {{{ afc_dictionary_has_key ( dict, key )
/*
@node afc_dictionary_has_key

			 NAME: afc_dictionary_has_key ( dictionary, key )  - Looks for a specified key in the Dictionary

		 SYNOPSIS: BOOL afc_dictionary_has_key ( struct afc_dictionary * dictionary, char * key )

	  DESCRIPTION: Use this function to know whether a named key is actually present inside the Dictionary.

			INPUT: - dictionary    - Pointer to a valid afc_dictionary instance.
				   - key           - Key to look for

		  RESULTS: - TRUE - The ey is actually present in the Dictionary
				   - FALSE - The key is not present in the Dictionary

		 SEE ALSO: - afc_dictionary_get()
				   - afc_dictionary_set()
				   - afc_dictionary_del_item()
@endnode
*/
BOOL afc_dictionary_has_key(Dictionary *dict, const char *key)
{
	if (afc_dictionary_internal_find(dict, key) != NULL)
		return (TRUE);

	return (FALSE);
}
// }}}
// {{{ afc_dictionary_get_key ( dict )
/*
@node afc_dictionary_get_key

			 NAME: afc_dictionary_get_key ( dictionary )  - Returns the current key name

		 SYNOPSIS: char * afc_dictionary_get_key ( struct afc_dictionary * dictionary )

	  DESCRIPTION: Use this function to know the current key name.

			INPUT: - dictionary    - Pointer to a valid afc_dictionary instance.

		  RESULTS: The current key name or NULL if dictionary is empty.

		 SEE ALSO: - afc_dictionary_get()
				   - afc_dictionary_set()
				   - afc_dictionary_del_item()
@endnode
*/
/*
char * afc_dictionary_get_key ( Dictionary * dict )
{
	if ( dict->curr_data == NULL ) return ( NULL );

	return ( dict->curr_data->key );
}
*/
// }}}
// {{{ afc_dictionary_find_key ( dict )
/*
@node afc_dictionary_find_key

			 NAME: afc_dictionary_find_key ( dictionary, data )  - Returns the key binded to a given data

		 SYNOPSIS: char * afc_dictionary_find_key ( struct afc_dictionary * dictionary, void * data )

	  DESCRIPTION: Use this function to know the current key name.

			INPUT: - dictionary    - Pointer to a valid afc_dictionary instance.

		  RESULTS: The current key name or NULL if dictionary is empty.

		 SEE ALSO: - afc_dictionary_get()
				   - afc_dictionary_set()
				   - afc_dictionary_del_item()
@endnode
*/
char *afc_dictionary_find_key(Dictionary *dict, void *data)
{
	void *val;

	val = afc_dictionary_first(dict);

	while (val)
	{
		if (val == data)
			return (dict->curr_data->key);

		val = afc_dictionary_next(dict);
	}

	return (NULL);
}
// }}}
// {{{ afc_dictionary_set_clear_func ( Dictionary * dict, int ( * func ) ( void * )  )
/*
@node afc_dictionary_set_clear_func

	   NAME: afc_dictionary_set_clear_func(dict, func) - Sets the clear func

   SYNOPSIS: int afc_dictionary_set_clear_func (Dictionary * dict, int ( *func ) ( void * ) )

	  SINCE: 1.20

DESCRIPTION: Use this command to set a clear function. The function will be called each time an
		 item is being deleted from the list with afc_dictionary_del() or afc_dictionary_clear().
		 To remove this function, pass a NULL value as function pointer.

	  INPUT: - dict	- Pointer to a valid Dictionary class.
		 - func	- Function to be called in clearing operations.

	RESULTS: AFC_ERR_NO_ERROR

   SEE ALSO: - afc_dictionary_del()
		 - afc_dictionary_clear()
@endnode
*/
/*
int afc_dictionary_set_clear_func ( Dictionary * am, int ( *func ) ( void * ) )
{
	am->func_clear = func;

	return ( AFC_ERR_NO_ERROR );
}
*/
// }}}
// {{{ afc_dictionary_for_each ( Dictionary * dict, int ( * func ) ( Dictionary * am, void *, void *  ), void * info  )
/*
@node afc_dictionary_set_clear_func

	   NAME: afc_dictionary_set_clear_func(dict, func) - Sets the clear func

   SYNOPSIS: int afc_dictionary_set_clear_func (Dictionary * dict, int ( *func ) ( Dictionary * dict, int pos, void * v, void * info ) )

	  SINCE: 1.10

DESCRIPTION: Use this command to traverse all elements inside the Dictionary.
		 The function called must return AFC_ERR_NO_ERROR when the traverse succeded and
		 a valid error otherwise. In case of error, the traverse is stopped and the error
		 returned.

	  INPUT: - dict	- Pointer to a valid Dictionary class.
		 - func	- Function to be called in the traverse.
			  Function prototype: int func ( Dictionary * dict, int pos, void * v, void * info );
		 - info	- Additional param passed to the /func/ being called.

	RESULTS: AFC_ERR_NO_ERROR

   SEE ALSO: - afc_dictionary_first()
		 - afc_dictionary_next()
@endnode
*/
int afc_dictionary_for_each(Dictionary *dict, int (*func)(Dictionary *am, int pos, void *v, void *info), void *info)
{
	int t = 0;
	int res;
	HashData *hd;
	DictionaryData *ddata;

	hd = afc_hash_master_first(dict->hash);
	while (hd)
	{
		ddata = hd->data;
		if ((res = func(dict, t++, ddata->value, info)) != AFC_ERR_NO_ERROR)
			return (res);

		hd = afc_hash_master_next(dict->hash);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ int afc_dictionary_set_custom_sort ( Dictionary * am, void ( * f ) ( void * base, size_t n, size_t size, int ( *com ) ( const void *, const void *) ) )
/*
@node afc_dictionary_set_clear_func

	   NAME: afc_dictionary_set_clear_func(am, func) - Sets the clear func

   SYNOPSIS: int afc_dictionary_set_custom_sort ( Dictionary * am, void ( * func )  ( void * base, size_t nmemb, size_t size, int ( *compar ) ( const void *, const void *) ) )

	  SINCE: 1.21

DESCRIPTION: Use this command to set a sort routine different to glibc qsort. You should use this function only if you
		 know what you are doing and your glibc implementation of qsort is buggy.

	  INPUT: - am	- Pointer to a valid Dictionary class.
		 - func	- Function to be called in sort operations.

	RESULTS: AFC_ERR_NO_ERROR
@endnode
*/
/*
int afc_dictionary_set_custom_sort ( Dictionary * dict, void ( * func )  ( void * base, size_t nmemb, size_t size, int ( *compar ) ( const void *, const void *) ) )
{
	dict->hash->am->custom_sort = func;

	return ( AFC_ERR_NO_ERROR );
}
*/
// }}}
// {{{ afc_dictionary_before_first ( d ) ************
// int afc_dictionary_before_first ( Dictionary * d ) { return ( afc_hash_master_before_first ( d->hash ) ); }
// }}}

/* ==================================================================================================================
	INTERNAL FUNCTIONS
================================================================================================================== */
// {{{ afc_dictionary_internal_find ( dict, key )
static DictionaryData *afc_dictionary_internal_find(Dictionary *dict, const char *key)
{
	DictionaryData *ddata;

	ddata = (DictionaryData *)afc_hash_master_find(dict->hash, afc_string_hash((unsigned char *)key, strlen(key)));
	dict->curr_data = ddata;

	return ddata;
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
void dump_all(Dictionary *dict)
{
	char *s;
	printf("Avaible Keys: \n");

	s = (char *)afc_dictionary_first(dict);
	while (s)
	{
		printf("  Key: %s - Val: %s\n", afc_dictionary_get_key(dict), s);
		s = (char *)afc_dictionary_next(dict);
	}
	printf(" -------------------------------- \n");
}

int main()
{
	Dictionary *dict = afc_dictionary_new();

	afc_dictionary_set(dict, "Fabio", afc_string_dup("Ciao, sono Fabio"));
	afc_dictionary_set(dict, "Pippo", afc_string_dup("afc_string_newa di Pippo"));

	printf("Fabio: %s\n", afc_dictionary_get(dict, "Fabio"));
	printf("Pippo: %s\n", afc_dictionary_get(dict, "Pippo"));
	printf("Pluto: %s\n", afc_dictionary_get(dict, "Pluto"));

	printf("Has \"Hello\": %d\n", afc_dictionary_has_key(dict, "Hello"));
	printf("Has \"Fabio\": %d\n", afc_dictionary_has_key(dict, "Fabio"));

	dump_all(dict);

	afc_dictionary_clear(dict);

	afc_dictionary_set(dict, "Gino", afc_string_dup("Ciao, sono Gino"));
	afc_dictionary_set(dict, "Pino", afc_string_dup("afc_string_newa di Pino"));

	dump_all(dict);

	afc_dictionary_set(dict, "Gino", NULL); // Try to remove a key by setting a NULL value

	dump_all(dict);

	afc_dictionary_clear(dict);

	afc_dictionary_delete(dict);
}
// }}}
#endif
