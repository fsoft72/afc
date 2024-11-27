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

/*
@config
	TITLE:     StringList
	VERSION:   1.10
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
	AUTHOR:    Massimo Tantignone - tanti@intercon.it
@endnode
*/

#include "string_list.h"

static const char class_name[] = "StringList";

static int afc_string_list_internal_set_tag(StringList *sn, int tag, void *val);
static long afc_string_list_internal_sort_nocase_noinv(void *a, void *b, void *info);
static long afc_string_list_internal_sort_case_noinv(void *a, void *b, void *info);
static long afc_string_list_internal_sort_nocase_inv(void *a, void *b, void *info);
static long afc_string_list_internal_sort_case_inv(void *a, void *b, void *info);

// {{{ docs
/*
@node quote
	*The trick to flying is to throw yourself at the ground and miss.*

		The Hitchhiker's Guide to the Galaxy
@endnode

@node intro
StringList is a class based on List created to handle lists of strings.
It is able to do almost everything that /List/ is able to do on lists,
making very easy to store a great number of strings at once.

StringList internal rappresentation of strings are AFC Strings. Please, see
afc_string_new() for more info about AFC Strings.
@endnode

@node history
	- 1.10	- Added afc_string_list_before_first () function
@endnode
*/
// }}}
// {{{ afc_string_list_new ()
/*
@node afc_string_list_new

		 NAME: afc_string_list_new () - Initializes a new StringList object.

			 SYNOPSIS: StringList * afc_string_list_new ()

		DESCRIPTION: Use this command to inizialize a StringList object.

		INPUT: NONE

	RESULTS: an initialized StringList structure.

			 SEE ALSO: afc_string_list_delete()
@endnode
*/
StringList *_afc_string_list_new(const char *file, const char *func, const unsigned int line)
{
	TRY(StringList *)

	StringList *sn = (StringList *)_afc_malloc(sizeof(StringList), file, func, line);

	if (sn == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "string_list", NULL);

	sn->magic = AFC_STRING_LIST_MAGIC;

	if ((sn->nm = afc_list_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "list", NULL);

	RETURN(sn);

	EXCEPT
	afc_string_list_delete(sn);

	FINALLY

	ENDTRY
}
// }}}
// {{{ afc_string_list_delete ( sn )
/*
@node afc_string_list_delete

		 NAME: afc_string_list_delete (sn) - Disposes an existing StringList

			 SYNOPSIS: int	afc_string_list_delete ( StringList * sn )

		DESCRIPTION: Use this function to dispose all memory allocated by a StringList
		 class.

		INPUT: - sn - a pointer to an already allocated StringList structure.

	RESULTS: should be AFC_ERR_NO_ERROR.

		NOTES: - This function can handle NULL pointers.

			 SEE ALSO: afc_string_list_new()
@endnode
*/
int _afc_string_list_delete(StringList *sn)
{
	int res;

	if ((res = afc_string_list_clear(sn)) != AFC_ERR_NO_ERROR)
		return (res);

	afc_list_delete(sn->nm);
	afc_free(sn);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_string_list_add ( sn, str, mode )
/*
@node afc_string_list_add

		 NAME: afc_string_list_add (sn, str, mode) - Adds a new string in the StringList list

			 SYNOPSIS: char * afc_string_list_add ( StringList * sn, const char * str, unsigned long mode )

		DESCRIPTION: this function adds a new string to the StringList.

		INPUT: - sn		 - an handler to an already allocated StringList structure.
		 - str		- the string you wish to add.
		 - mode	 - Where the new string will be added to the list.
					Possible values are:

					+ AFC_STRING_LIST_ADD_TAIL - Add the string to the end of the list.

					+ AFC_STRING_LIST_ADD_HEAD - Add the string as the first item in the list.

					+ AFC_STRING_LIST_ADD_HERE - Add the string right next the previous item in the list.

	RESULTS: - a pointer to the real string if everything went fine.
		 - NULL if there was no memory to add the string.

		NOTES: - The string will not be added directly to the list, but a copy of it will. So it is completely
			 safe to reuse the same buffer to add more than one single line to your StringList. For the same
			 reason, also static char strings are wellcome.

			 SEE ALSO: - afc_string_list_insert()
		 - afc_string_list_del()
		 - afc_string_list_clear()
		 - afc_list_add()
@endnode
*/
char *afc_string_list_add(StringList *sn, const char *s, unsigned long mode)
{
	char *g;

	if ((s != NULL) && (strlen(s) == 0) && (sn->discard_zero_len))
		return (NULL);

	// printf ( "Add: %s - Len: %d\n", s, strlen ( s ) );

	if ((s != NULL) && (strlen(s)))
		g = afc_string_dup(s);
	else
		g = afc_string_new(1);

	if (g == NULL)
	{
		AFC_LOG_FAST(AFC_ERR_NO_MEMORY);
		return (NULL);
	}

	return ((char *)afc_list_add(sn->nm, g, mode));
}
// }}}
// {{{ afc_string_list_insert ( sn, str )
/*
@node afc_string_list_insert

		 NAME: afc_string_list_insert (sn, str ) - Inserts a string in the current list position

			 SYNOPSIS: char * afc_string_list_insert ( StringList * sn, const char * str )

		DESCRIPTION: this function adds a new string in the current	list position.

		INPUT: - sn		 - an handler to an already allocated StringList structure.
		 - str		- the string you wish to add.

	RESULTS: - a pointer to the real string if everything went fine.
		 - NULL if there was no memory to add the string.

	 SEE ALSO: - afc_string_list_add()
		 - afc_string_list_del()
		 - afc_string_list_clear()
@endnode
*/
/*
char * afc_string_list_insert (StringList * sn, const char * s)
{
	return ( (char*) afc_string_list_add (sn->nm, s, AFC_STRING_LIST_ADD_HERE) );
}
*/
// }}}
// {{{ afc_string_list_obj ( sn )
/*
@node afc_string_list_obj

		 NAME: afc_string_list_obj (sn) - Returns the current string in the StringList's list

			 SYNOPSIS: char * afc_string_list_obj ( StringList * sn )

		DESCRIPTION: this function returns the current string in the StringList's list.

		INPUT: - sn		 - an handler to an already allocated StringList structure.

	RESULTS: - a pointer to a string if everything went fine.
		 - NULL if the list is empty.

			 SEE ALSO: - afc_string_list_item()
		 - afc_list_obj()
@endnode
*/
/*
char * afc_string_list_obj (StringList * sn)
{
	return ((char *)afc_list_obj (sn->nm));
}
*/
// }}}
// {{{ afc_string_list_is_empty ( sn )
/*
@node afc_string_list_is_empty

		 NAME: afc_string_list_is_empty (sn) - Checks if the StringList's list is empty

			 SYNOPSIS: char * afc_string_list_is_empty ( StringList * sn )

		DESCRIPTION: this function checks if the current StringList's list is empty or contains some items.

		INPUT: - sn		 - an handler to an already allocated StringList structure.

	RESULTS: - TRUE if the StringList does not contain any items.
		 - FALSE if at least one item is present in the current StringList's list

			 SEE ALSO: - afc_list_is_empty()
@endnode
*/
/*
short afc_string_list_is_empty (StringList * sn)
{
	return ( afc_list_is_empty (sn->nm) );
}
*/
// }}}
// {{{ afc_string_list_first ( sn )
/*
@node afc_string_list_first

		 NAME: afc_string_list_first (sn) - Returns the first item in the StringList's list

			 SYNOPSIS: char * afc_string_list_first ( StringList * sn )

		DESCRIPTION: this function returns the first string in the StringList's list

		INPUT: - sn		 - an handler to an already allocated StringList structure.

	RESULTS: - the string requested.
		 - NULL if the StringList is empty.

	 SEE ALSO: - afc_string_list_last()
		 - afc_string_list_next()
		 - afc_string_list_prev()
		 - afc_string_list_item()
		 - afc_list_first()
@endnode
*/
// char * afc_string_list_first ( StringList * sn ) { return ( ( char * ) afc_list_first ( sn->nm ) ); }
// }}}
// {{{ afc_string_list_next ( sn )
/*
@node afc_string_list_next

		 NAME: afc_string_list_next (sn) - Returns the next item in the StringList's list

			 SYNOPSIS: char * afc_string_list_next ( StringList * sn )

		DESCRIPTION: this function returns the next string in the StringList's list

		INPUT: - sn		 - an handler to an already allocated StringList structure.

	RESULTS: - the string requested.
		 - NULL if the list pointed to the last item and there is no next item, or because the list is empty.

	 SEE ALSO: - afc_string_list_first()
		 - afc_string_list_last()
		 - afc_string_list_prev()
		 - afc_string_list_item()
		 - afc_list_next()
@endnode
*/
// char * afc_string_list_next ( StringList * sn ) { return ( ( char * ) afc_list_next ( sn->nm ) ); }
// }}}
// {{{ afc_string_list_prev ( sn )
/*
@node afc_string_list_prev

		 NAME: afc_string_list_prev (sn) - Returns the previous item in the StringList's list

			 SYNOPSIS: char * afc_string_list_prev ( StringList * sn )

		DESCRIPTION: this function returns the previous string in the StringList's list

		INPUT: - sn		 - an handler to an already allocated StringList structure.

	RESULTS: - the string requested.
		 - NULL if the list pointed to the first item and there is no previous item, or because the list is empty.

			 SEE ALSO: - afc_string_list_first()
		 - afc_string_list_last()
		 - afc_string_list_next()
		 - afc_string_list_item()
		 - afc_list_prev()
@endnode
*/
// char * afc_string_list_prev ( StringList * sn ) { return ( ( char * ) afc_list_prev ( sn->nm ) ); }
// }}}
// {{{ afc_string_list_get ( sn )
/*
@node afc_string_list_get

		 NAME: afc_string_list_get (sn) - Returns a pointer to the current list Node.

			 SYNOPSIS: struct Node * afc_string_list_get ( StringList * sn )

		DESCRIPTION: this function returns a pointer to the current Node in the list managed by StringList.

		INPUT: - sn		 - an handler to an already allocated StringList structure.

	RESULTS: - the node requested.
		 - NULL if the list is empty.

		NOTES: - This is a very low level function. You should only use it if you know what you are doing.

			 SEE ALSO: - afc_string_list_addr()
		 - afc_list_get()
@endnode
*/
/*
struct Node * afc_string_list_get (StringList * sn)
{
	return (afc_list_get (sn->nm));
}
*/
// }}}
// {{{ afc_string_list_addr ( sn )
/*
@node afc_string_list_addr

		 NAME: afc_string_list_addr (sn) - Returns a pointer to the List managed by StringList

			 SYNOPSIS: struct List * afc_string_list_addr ( StringList * sn )

		DESCRIPTION: this function returns a pointer to the List managed by StringList

		INPUT: - sn		 - an handler to an already allocated StringList structure.

	RESULTS: - the List requested.

		NOTES: - This is a very low level function. You should only use it if you know what you are doing.
		 - This function does *never* return NULL, because even if the List is empty, it is always avaible.

			 SEE ALSO: - afc_string_list_get()
		 - afc_list_addr()
@endnode
*/
/*
struct List * afc_string_list_addr (StringList * sn)
{
	return (afc_list_addr (sn->nm));
}
*/
// }}}
// {{{ afc_string_list_push ( sn )
/*
@node afc_string_list_push

		 NAME: afc_string_list_push (sn) - Pushes the current list position on the stack

			 SYNOPSIS: short afc_string_list_addr ( StringList * sn )

		DESCRIPTION: this function pushes the current item in the StringList list on the stack.

		INPUT: - sn		 - an handler to an already allocated StringList structure.

	RESULTS: - TRUE if the push succeded
		 - FALSE if the stack is full

			 SEE ALSO: - afc_string_list_pop()
		 - afc_string_list_clear_stack()
		 - afc_list_push()
@endnode
*/
/*
short afc_string_list_push (StringList * sn)
{
	return (afc_list_push (sn->nm));
}
*/
// }}}
// {{{ afc_string_list_pop ( sn, autopos )
/*
@node afc_string_list_pop

		 NAME: afc_string_list_pop (sn, autopos) - Returns the last item pushed on the stack

			 SYNOPSIS: char * afc_string_list_pop ( StringList * sn, short autopos )

		DESCRIPTION: this function retrieves the last string pushed on the stack.

		INPUT: - sn	- an handler to an already allocated StringList structure.
		 - autopos	 - This is a boolean flag:

						 + TRUE -	restores the list pointer to the previously pushed node in list.

						 + FALSE - just removes the pushed node from the stack.


	RESULTS: - the last item pushed on the stack
		 - NULL if you set autopos to TRUE.


			 SEE ALSO: - afc_string_list_push()
		 - afc_string_list_clear_stack()
		 - afc_list_pop()
@endnode
*/
/*
char * afc_string_list_pop (StringList * sn, short autopos)
{
	return ((char *)afc_list_pop (sn->nm, autopos));
}
*/
// }}}
// {{{ afc_string_list_del ( sn )
/*
@node afc_string_list_del

		 NAME: afc_string_list_del (sn) - Deletes the current string from the list

			 SYNOPSIS: char * afc_string_list_delete ( StringList * sn )

		DESCRIPTION: this function deletes the current string from the list.

		INPUT: - sn	- an handler to an already allocated StringList structure.

	RESULTS: the (new) current string the list is pointing to.

			 SEE ALSO: - afc_string_list_clear()
		 - afc_list_del()
@endnode
*/
char *afc_string_list_del(StringList *sn)
{
	char *s = NULL;

	if (!afc_string_list_is_empty(sn))
	{
		if ((s = afc_list_obj(sn->nm)))
		{
			afc_string_delete(s);
			s = (char *)afc_list_del(sn->nm);
		}
	}

	return (s);
}
// }}}
// {{{ afc_string_list_clear_stack ( sn )
/*
@node afc_string_list_clear_stack

		 NAME: afc_string_list_clear_stack (sn) - Clears the stack of pushed string

			 SYNOPSIS: void afc_string_list_clear_stack ( StringList * sn )

		DESCRIPTION: this function retrieves the last string pushed on the stack.

		INPUT: - sn	- an handler to an already allocated StringList structure.

	RESULTS: NONE

			 SEE ALSO: - afc_string_list_push()
		 - afc_string_list_pop()
		 - afc_list_clear_stack()
@endnode
*/
/*
void afc_string_list_clear_stack (StringList * sn)
{
	afc_list_clear_stack (sn->nm);
}
*/
// }}}
// {{{ afc_string_list_last ( sn )
/*
@node afc_string_list_last

		 NAME: afc_string_list_last (sn) - Returns the last item in the StringList's list

			 SYNOPSIS: char * afc_string_list_last ( StringList * sn )

		DESCRIPTION: this function returns the last string in the StringList's list

		INPUT: - sn		 - an handler to an already allocated StringList structure.

	RESULTS: - the string requested.
		 - NULL if the list is empty.

			 SEE ALSO: - afc_string_list_first()
		 - afc_string_list_prev()
		 - afc_string_list_next()
		 - afc_string_list_item()
		 - afc_list_last()
@endnode
*/
/*
char * afc_string_list_last (StringList * sn)
{
	 return ((char *)afc_list_last (sn->nm));
}
*/
// }}}
// {{{ afc_string_list_clear ( sn )
/*
@node afc_string_list_clear

		 NAME: afc_string_list_clear (sn) - Deletes all the strings in the StringList

			 SYNOPSIS: int afc_string_list_clear ( StringList * sn )

		DESCRIPTION: this function deletes all the strings in the StringList.

		INPUT: - sn	- an handler to an already allocated StringList structure.

	RESULTS: NONE

			 SEE ALSO: - afc_string_list_del()
		 - afc_list_clear()
@endnode
*/
int afc_string_list_clear(StringList *sn)
{
	char *s;

	if (sn == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));
	if (sn->magic != AFC_STRING_LIST_MAGIC)
		return (AFC_LOG_FAST(AFC_ERR_INVALID_POINTER));

	s = (char *)afc_list_first(sn->nm);
	while (s)
	{
		afc_string_delete(s);

		s = (char *)afc_list_next(sn->nm);
	}

	return (afc_list_clear(sn->nm));
}
// }}}
// {{{ afc_string_list_len ( sn )
/*
@node afc_string_list_len

		 NAME: afc_string_list_len ( sn ) - Returns the number of strings in the list

			 SYNOPSIS: unsigned long afc_string_list_len (StringList * sn)

		DESCRIPTION: this function returns the number of strings present in the StringList

		INPUT: - sn		 - an handler to an already allocated StringList structure.

	RESULTS: the number of strings in the list.

			 SEE ALSO: - afc_list_len()
@endnode
*/
/*
unsigned long afc_string_list_len (StringList * sn)
{
	return (afc_list_len (sn->nm));
}
*/
// }}}
// {{{ afc_string_list_item ( sn, num )
/*
@node afc_string_list_item

		 NAME: afc_string_list_item (sn, num) - Returns the desired item in the StringList's list

			 SYNOPSIS: char * afc_string_list_item ( StringList * sn, unsigned long num )

		DESCRIPTION: this function returns the desired string in the StringList's list

		INPUT: - sn		 - an handler to an already allocated StringList structure.
		 - num		- the ordinal number of the desired item in the list.

	RESULTS: - the string requested.
		 - NULL if the list is empty.

		NOTES: - if num is less than 0, the first element in the list will be returned.
		 - if num is bigger than the last valid ordinal value, the last element in the list will be returned.

			 SEE ALSO: - afc_string_list_first()
		 - afc_string_list_last()
		 - afc_string_list_prev()
		 - afc_string_list_next()
		 - afc_list_item()
@endnode
*/
/*
char * afc_string_list_item (StringList * sn, unsigned long n)
{
	return ((char *)afc_list_item (sn->nm, n));
}
*/
// }}}
// {{{ afc_string_list_change ( sn, str )
/*
@node afc_string_list_change

		 NAME: afc_string_list_change (sn, str) - Changes the string of a node in the list

			 SYNOPSIS: char * afc_string_list_change ( StringList * sn, char * str )

		DESCRIPTION: this function changes the string contained in the current node in the list.

		INPUT: - sn		 - an handler to an already allocated StringList structure.
		 - str		- the string to insert in the current node.

	RESULTS: - AFC_ERR_NO_ERROR if everything went fine, an error code otherwise.

		NOTES: - This is a low level function and should only be used if you know what you are doing.

			 SEE ALSO: - afc_list_change()
@endnode
*/
int afc_string_list_change(StringList *sn, char *s)
{
	char *g;
	unsigned int len;

	if (afc_list_is_empty(sn->nm))
		return (AFC_ERR_NO_ERROR);

	if ((g = afc_list_obj(sn->nm)))
	{
		afc_string_delete(g);

		len = strlen(s);

		if ((g = afc_string_dup(s)) == NULL)
			return (AFC_LOG_FAST(AFC_ERR_NO_MEMORY));

		if (afc_list_change(sn->nm, g) == NULL)
			return (AFC_LOG(AFC_LOG_ERROR, AFC_STRING_LIST_ERR_CHANGE, "Cannot change string", NULL));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_string_list_pos ( sn )
/*
@node afc_string_list_pos

		 NAME: afc_string_list_pos (sn) - Returns the ordinal position of the current string in the list

			 SYNOPSIS: unsigned long afc_string_list_pos ( StringList * sn )

		DESCRIPTION: this function returns the ordinal position of the current string in the list.

		INPUT: - sn		 - an handler to an already allocated StringList structure.

	RESULTS: - the numerical value of the ordinal position of the string in the list

			 SEE ALSO: - afc_list_pos()
@endnode
*/
/*
unsigned long afc_string_list_pos (StringList * sn)
{
	return ( afc_list_pos (sn->nm) );
}
*/
// }}}

#ifndef MINGW
// {{{ afc_string_list_search ( sn, str, from_here, nocase )
/*
@node afc_string_list_search

		 NAME: afc_string_list_search (sn, str, from_here, nocase) - Searches the list for the specified string

			 SYNOPSIS: int afc_string_list_search (StringList * sn, char * str, short from_here, short	nocase )

		DESCRIPTION: this function searches the whole list (or part of it) for the specified pattern string.
				   The pattern can be defined using the standard Unix file name pattern syntax, like "*" or "?".

		INPUT: - sn			   - an handler to an already allocated StringList structure.
		 - str       - afc_string_new to search the list for.
				   - from_here - Set it to TRUE if you want to start searching from the current position
								 and not from the beginning of the list.
		 - nocase	   - If set to TRUE, the sort will be case insensitive.

	RESULTS: - AFC_ERR_NO_ERROR if everything went fine.

			NOTES: - This function may not be portable (it relies upon afc_string_pattern_match() )

			 SEE ALSO: - afc_list_sort()
@endnode
*/
char *afc_string_list_search(StringList *sn, char *str, short from_here, short nocase)
{
	char *s;

	if (afc_list_is_empty(sn->nm))
		return (NULL);

	afc_list_push(sn->nm);

	if (from_here == FALSE)
		s = (char *)afc_list_first(sn->nm);
	else
		s = (char *)afc_list_obj(sn->nm);

	while (s)
	{
		if (afc_string_pattern_match(s, str, nocase) == 0)
		{
			afc_list_pop(sn->nm, FALSE);
			return (s);
		}
		s = (char *)afc_list_next(sn->nm);
	}

	afc_list_pop(sn->nm, TRUE);
	return (NULL);
}
// }}}
#endif

// {{{ afc_string_list_sort ( sn, nocase, inverted )
/*
@node afc_string_list_sort

		 NAME: afc_string_list_sort (sn, nocase, inverted) - Sorts the strings in the list

			 SYNOPSIS: int afc_string_list_sort (StringList * sn, short	nocase, short	inverted)

		DESCRIPTION: this function sorts the strings in the list. You can specify some sorting methods,
		 such like if you want the sort being case insensitive (nocase) or if you want the
		 order from Z-A instead of A-Z (inverted).

		INPUT: - sn			 - an handler to an already allocated StringList structure.
		 - nocase	 - If set to TRUE, the sort will be case insensitive.
		 - inverted - If set to TRUE, the sort will be inverted: from Z to A and not A-Z.
		 - fast		 - If set to TRUE, the fast sort routine in List will be used.

	RESULTS: - AFC_ERR_NO_ERROR if everything went fine.

			 SEE ALSO: - afc_list_sort()
@endnode
*/
int afc_string_list_sort(StringList *sn, short nocase, short inverted, short fast)
{

	// afc_list_free_array(nm);

	if (nocase)
	{
		if (inverted)
		{
			if (fast)
				afc_list_fast_sort(sn->nm, afc_string_list_internal_sort_nocase_inv, NULL);
			else
				afc_list_sort(sn->nm, afc_string_list_internal_sort_nocase_inv, NULL);
		}
		else
		{
			if (fast)
				afc_list_fast_sort(sn->nm, afc_string_list_internal_sort_nocase_noinv, NULL);
			else
				afc_list_sort(sn->nm, afc_string_list_internal_sort_nocase_noinv, NULL);
		}
	}
	else
	{
		if (inverted)
		{
			if (fast)
				afc_list_fast_sort(sn->nm, afc_string_list_internal_sort_case_inv, NULL);
			else
				afc_list_sort(sn->nm, afc_string_list_internal_sort_case_inv, NULL);
		}
		else
		{
			if (fast)
				afc_list_fast_sort(sn->nm, afc_string_list_internal_sort_case_noinv, NULL);
			else
				afc_list_sort(sn->nm, afc_string_list_internal_sort_case_noinv, NULL);
		}
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_string_list_close ( sn )
/*
@node afc_string_list_clone

		 NAME: afc_string_list_clone (sn) - Clones a StringList creating a new one

			 SYNOPSIS: StringList * afc_string_list_clone (StringList * sn)

		DESCRIPTION: this function copies all entries of a StringList inside a new one.
		 The new StringList will result sorted and the internal array rappresentation
				   is recreated in case that the original StringList has these attributes set.

		INPUT: - sn			 - an handler to an already allocated StringList structure.

	RESULTS: a pointer to a valid new StringList object, or NULL if an error occurred.

	  NOTES: - The new StringList object must be freed with afc_string_list_delete() like
					 any other StringList object.

		 - The new StringList will contain copies of all string stored, not just references,
					 so it is perfectly safe to afc_string_list_delete() the original StringList.

		 - In case the original StringList had the internal array rappresentation of the list,
					 it will  be reproduced in the new StringList.

			 SEE ALSO: - afc_list_sort()
@endnode
*/
StringList *afc_string_list_clone(StringList *sn)
{
	StringList *sn2 = afc_string_list_new();
	char *s;

	if (sn2 == NULL)
		return (NULL);

	s = afc_string_list_first(sn);
	while (s)
	{
		afc_string_list_add(sn2, s, AFC_STRING_LIST_ADD_TAIL);
		s = afc_string_list_next(sn);
	}

	// Set the "is_sorted" flag inside the underlying List
	// to the new StringList.
	sn2->nm->is_sorted = sn->nm->is_sorted;

	if (sn->nm->is_array_valid)
		afc_list_create_array(sn2->nm);

	return (sn2);
}
// }}}
// {{{ afc_string_list_split ( sn, string, delimiters )
/*
@node afc_string_list_split

		 NAME: afc_string_list_split ( sn, string, delimiters )		 - Splits a string

			 SYNOPSIS: int afc_string_list_split ( StringList * sn, const char * string, const char * delimiters )

		DESCRIPTION: This function splits the string. You can specify one or more delimiters in one single string
				   and the splitter will create the smallest part of them. Once the split is done, you can access
				   the items usign afc_string_list_item(), afc_string_list_first(), afc_string_list_next()
				   and so on, because the fields are stored inside an AFC StringList.

		INPUT: - sn      - A pointer to a valid StringList instance
				   - string     - The string to split in pieces
				   - delimiters - All delimiters that the splitter must take care of.

	RESULTS: - AFC_ERR_NO_ERROR on success.
				   - An error code in case of error

			NOTES: - Before the split is done, the function calls afc_string_list_clear()

	 SEE ALSO: - afc_string_list_first()
				   - afc_string_list_next()
				   - afc_string_list_item()

@endnode
*/
int afc_string_list_split(StringList *sn, const char *string, const char *delimiters)
{
	char *base_string, *separators;
	char *start_pos, *end_pos, *end_string;
	char *x, *y, escape_char;
	int t, num_delimiters;

	if (string == NULL)
		return (AFC_LOG(AFC_LOG_WARNING, AFC_STRING_LIST_ERR_NULL_STRING, "Null string is invalid", NULL));
	if (delimiters == NULL)
		return (AFC_LOG(AFC_LOG_WARNING, AFC_STRING_LIST_ERR_NULL_DELIMITERS, "Null delimiters string is invalid", NULL));

	afc_string_list_clear(sn);

	base_string = afc_string_dup(string);
	separators = afc_string_dup(delimiters);

	num_delimiters = afc_string_len(separators);

	end_string = base_string + afc_string_len(base_string);
	end_pos = base_string;

	escape_char = sn->escape_char;

	while (end_pos != end_string)
	{
		y = NULL;

		start_pos = end_pos;

		// Cycle all char delimiters defined
		for (t = 0; t < num_delimiters; t++)
		{
			x = index(start_pos, separators[t]);

			while (x != NULL)
			{
				if ((escape_char) && (x != base_string) && (x[-1] == escape_char))
				{
					x = index(x + 1, separators[t]);
				}
				else
				{
					if ((x < y) || (y == NULL))
						y = x;
					x = NULL;
				}
			}
		}

		if (y)
		{
			*y = 0;
			end_pos = y + 1;
		}
		else
			end_pos = end_string;

		afc_string_list_add(sn, start_pos, AFC_STRING_LIST_ADD_TAIL);
	}

	afc_string_delete(base_string);
	afc_string_delete(separators);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_string_list_set_tags ( sn, first_tag, ... )
/*
@node afc_string_list_set_tags

		 NAME: afc_string_list_set_tags ( sn, first_tag, ... )		 - Set StringList Tags

			 SYNOPSIS: int afc_string_list_set_tags ( StringList * sn, int first_tag, ... );

	DESCRIPTION: This function sets StringList behaviours using tags.

		INPUT: - sn      	- A pointer to a valid StringList instance
			   - first_tag 	- First tag to be set.
					  Supported tags are:

						+ AFC_STRING_LIST_TAG_DISCARD_ZERO_LEN - (TRUE/FALSE) If set to TRUE
							StringList will not add to its list any string which is zero length.
							By default, an empty string is added to the list in place of the zero length string.

						+ AFC_STRING_LIST_TAG_ESCAPE_CHAR	- (char) This is the char to be used as the escape
							character during the afc_string_list_split() operations. Please, remember to provide a
							single character (ie. in single quotes '' and not double quotes) as escape char definition.

			   - ... 		- All values and tags

	RESULTS: - AFC_ERR_NO_ERROR on success.
				   - An error code in case of error

			NOTES: - Remember to end the tag list with AFC_TAG_END

	 SEE ALSO: - afc_string_list_first()
				   - afc_string_list_next()
				   - afc_string_list_item()

@endnode
*/
int _afc_string_list_set_tags(StringList *sn, int first_tag, ...)
{
	va_list args;
	unsigned int tag;
	void *val;

	va_start(args, first_tag);

	tag = first_tag;

	while (tag != AFC_TAG_END)
	{
		val = va_arg(args, void *);

		afc_string_list_internal_set_tag(sn, tag, val);

		tag = va_arg(args, int);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_string_list_before_first ( sn ) ************
// int afc_string_list_before_first ( StringList * sn ) { return ( afc_list_before_first ( sn->nm ) ); }
// }}}

/* -------------------------------------------------------------------------------------------
	 INTERNAL FUNCTIONS
--------------------------------------------------------------------------------------------- */
// {{{ afc_string_list_internal_set_tag ( sn, tag, val )
static int afc_string_list_internal_set_tag(StringList *sn, int tag, void *val)
{
	switch (tag)
	{
	case AFC_STRING_LIST_TAG_DISCARD_ZERO_LEN:
		sn->discard_zero_len = (BOOL)(int)(long)val;
		break;

	case AFC_STRING_LIST_TAG_ESCAPE_CHAR:
		sn->escape_char = (char)(int)(long)val;
		break;

	default:
		break;
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_string_list_internal_sort_nocase_noinv ( a, b, info )
static long afc_string_list_internal_sort_nocase_noinv(void *a, void *b, void *info)
{
	return (-afc_string_comp((char *)a, (char *)b, ALL));
}
// }}}
// {{{ afc_string_list_internal_sort_case_noinv ( a, b, info )
static long afc_string_list_internal_sort_case_noinv(void *a, void *b, void *info)
{
	char *aa, *bb;
	signed long ret;

	aa = afc_string_new(strlen((char *)a));
	bb = afc_string_new(strlen((char *)b));

	afc_string_copy(aa, (char *)a, TRUE);
	afc_string_copy(bb, (char *)b, TRUE);
	afc_string_upper(aa);
	afc_string_upper(bb);

	ret = -afc_string_comp(aa, bb, TRUE);

	afc_string_delete(aa);
	afc_string_delete(bb);

	return (ret);
}
// }}}
// {{{ afc_string_list_internal_sort_nocase_inv ( a, b, info )
static long afc_string_list_internal_sort_nocase_inv(void *a, void *b, void *info)
{
	return (afc_string_comp((char *)a, (char *)b, ALL));
}
// }}}
// {{{ afc_string_list_internal_sort_case_inv ( a, b, info )
static long afc_string_list_internal_sort_case_inv(void *a, void *b, void *info)
{
	return (-afc_string_list_internal_sort_case_noinv(a, b, info));
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
void dosort(struct afc_string_list *n)
{
	afc_string_list_sort(n, TRUE, TRUE, FALSE);
}

void shwall(struct afc_string_list *n)
{
	printf("-----------------------\n");

	if (afc_string_list_first(n))
	{
		do
			printf("Item: %s - Pos:%ld\n", afc_string_list_obj(n), afc_string_list_numerical_pos(n));
		while (afc_string_list_next(n));
	}

	printf("-----------------------\n");
}

int main(void)
{
	struct afc_string_list *sn;
	// struct TagItem tags[] = { TAGSTR_MaxChars, 3, TAG_END };

	sn = afc_string_list_new();

	if (sn)
	{
		afc_string_list_first(sn);

		afc_string_list_add(sn, "Ciao Mamma", AFC_STRING_LIST_ADD_TAIL);
		afc_string_list_add(sn, "Zio Peppino", AFC_STRING_LIST_ADD_TAIL);
		afc_string_list_add(sn, "Paperino", AFC_STRING_LIST_ADD_TAIL);
		afc_string_list_add(sn, "Tom & Jerry", AFC_STRING_LIST_ADD_TAIL);
		afc_string_list_add(sn, "Pluto", AFC_STRING_LIST_ADD_TAIL);
		afc_string_list_add(sn, "Anna", AFC_STRING_LIST_ADD_TAIL);
		afc_string_list_add(sn, "Zorro", AFC_STRING_LIST_ADD_TAIL);
		afc_string_list_add(sn, "Vienna", AFC_STRING_LIST_ADD_TAIL);
		afc_string_list_add(sn, "PIPPO", AFC_STRING_LIST_ADD_TAIL);

		dosort(sn);
		shwall(sn);

		// afc_string_list_search ("Tom#?");
		// afc_string_list_search ("PLA");

		// printf ("Running!\n");

		// printf ("STRING:%s - Pos:%ld\n",afc_string_list_obj (),afc_string_list_pos ());

		afc_string_list_clear(sn);

		afc_string_list_split(sn, "ciao|mamma|bella|come|stai", "|");

		shwall(sn);

		// afc_string_list_setattrs (tags);
		afc_string_list_add(sn, "Ciao Mammina Bella", AFC_STRING_LIST_ADD_TAIL);
		printf("Str:%s - Chars (%ld)\n", afc_string_list_obj(sn), afc_string_len(afc_string_list_obj(sn)));
		afc_string_list_change(sn, "Ciao Mammina Be");
		printf("Str:%s - Chars (%ld)\n", afc_string_list_obj(sn), afc_string_len(afc_string_list_obj(sn)));
		afc_string_list_change(sn, "Pippo");
		printf("Str:%s - Chars (%ld)\n", afc_string_list_obj(sn), afc_string_len(afc_string_list_obj(sn)));

		afc_string_list_delete(sn);
	}

	return (0);
}
// }}}
#endif
