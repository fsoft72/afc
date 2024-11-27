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
/*	StringNode.cpp	$ 19/05/97 FR MT AG $	*/

/*
@config
	TITLE:     StringNode
	VERSION:   1.10
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
	AUTHOR:    Massimo Tantignone - tanti@intercon.it
@endnode
*/

#include "stringnode.h"

static const char class_name[] = "StringNode";

static int afc_stringnode_internal_set_tag(StringNode *sn, int tag, void *val);
static long afc_stringnode_internal_sort_nocase_noinv(void *a, void *b, void *info);
static long afc_stringnode_internal_sort_case_noinv(void *a, void *b, void *info);
static long afc_stringnode_internal_sort_nocase_inv(void *a, void *b, void *info);
static long afc_stringnode_internal_sort_case_inv(void *a, void *b, void *info);

// {{{ docs
/*
@node quote
	*The trick to flying is to throw yourself at the ground and miss.*

		The Hitchhiker's Guide to the Galaxy
@endnode

@node intro
StringNode is a class based on NodeMaster created to handle lists of strings.
It is able to do almost everything that /NodeMaster/ is able to do on lists,
making very easy to store a great number of strings at once.

StringNode internal rappresentation of strings are AFC Strings. Please, see
afc_string_new() for more info about AFC Strings.
@endnode

@node history
	- 1.10	- Added afc_stringnode_before_first () function
@endnode
*/
// }}}
// {{{ afc_stringnode_new ()
/*
@node afc_stringnode_new

		 NAME: afc_stringnode_new () - Initializes a new StringNode object.

			 SYNOPSIS: StringNode * afc_stringnode_new ()

		DESCRIPTION: Use this command to inizialize a StringNode object.

		INPUT: NONE

	RESULTS: an initialized StringNode structure.

			 SEE ALSO: afc_stringnode_delete()
@endnode
*/
StringNode *_afc_stringnode_new(const char *file, const char *func, const unsigned int line)
{
	TRY(StringNode *)

	StringNode *sn = (StringNode *)_afc_malloc(sizeof(StringNode), file, func, line);

	if (sn == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "stringnode", NULL);

	sn->magic = AFC_STRINGNODE_MAGIC;

	if ((sn->nm = afc_nodemaster_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "nodemaster", NULL);

	RETURN(sn);

	EXCEPT
	afc_stringnode_delete(sn);

	FINALLY

	ENDTRY
}
// }}}
// {{{ afc_stringnode_delete ( sn )
/*
@node afc_stringnode_delete

		 NAME: afc_stringnode_delete (sn) - Disposes an existing StringNode

			 SYNOPSIS: int	afc_stringnode_delete ( StringNode * sn )

		DESCRIPTION: Use this function to dispose all memory allocated by a StringNode
		 class.

		INPUT: - sn - a pointer to an already allocated StringNode structure.

	RESULTS: should be AFC_ERR_NO_ERROR.

		NOTES: - This function can handle NULL pointers.

			 SEE ALSO: afc_stringnode_new()
@endnode
*/
int _afc_stringnode_delete(StringNode *sn)
{
	int res;

	if ((res = afc_stringnode_clear(sn)) != AFC_ERR_NO_ERROR)
		return (res);

	afc_nodemaster_delete(sn->nm);
	afc_free(sn);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_stringnode_add ( sn, str, mode )
/*
@node afc_stringnode_add

		 NAME: afc_stringnode_add (sn, str, mode) - Adds a new string in the StringNode list

			 SYNOPSIS: char * afc_stringnode_add ( StringNode * sn, const char * str, unsigned long mode )

		DESCRIPTION: this function adds a new string to the StringNode.

		INPUT: - sn		 - an handler to an already allocated StringNode structure.
		 - str		- the string you wish to add.
		 - mode	 - Where the new string will be added to the list.
					Possible values are:

					+ AFC_STRINGNODE_ADD_TAIL - Add the string to the end of the list.

					+ AFC_STRINGNODE_ADD_HEAD - Add the string as the first item in the list.

					+ AFC_STRINGNODE_ADD_HERE - Add the string right next the previous item in the list.

	RESULTS: - a pointer to the real string if everything went fine.
		 - NULL if there was no memory to add the string.

		NOTES: - The string will not be added directly to the list, but a copy of it will. So it is completely
			 safe to reuse the same buffer to add more than one single line to your StringNode. For the same
			 reason, also static char strings are wellcome.

			 SEE ALSO: - afc_stringnode_insert()
		 - afc_stringnode_del()
		 - afc_stringnode_clear()
		 - afc_nodemaster_add()
@endnode
*/
char *afc_stringnode_add(StringNode *sn, const char *s, unsigned long mode)
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

	return ((char *)afc_nodemaster_add(sn->nm, g, mode));
}
// }}}
// {{{ afc_stringnode_insert ( sn, str )
/*
@node afc_stringnode_insert

		 NAME: afc_stringnode_insert (sn, str ) - Inserts a string in the current list position

			 SYNOPSIS: char * afc_stringnode_insert ( StringNode * sn, const char * str )

		DESCRIPTION: this function adds a new string in the current	list position.

		INPUT: - sn		 - an handler to an already allocated StringNode structure.
		 - str		- the string you wish to add.

	RESULTS: - a pointer to the real string if everything went fine.
		 - NULL if there was no memory to add the string.

	 SEE ALSO: - afc_stringnode_add()
		 - afc_stringnode_del()
		 - afc_stringnode_clear()
@endnode
*/
/*
char * afc_stringnode_insert (StringNode * sn, const char * s)
{
	return ( (char*) afc_stringnode_add (sn->nm, s, AFC_STRINGNODE_ADD_HERE) );
}
*/
// }}}
// {{{ afc_stringnode_obj ( sn )
/*
@node afc_stringnode_obj

		 NAME: afc_stringnode_obj (sn) - Returns the current string in the StringNode's list

			 SYNOPSIS: char * afc_stringnode_obj ( StringNode * sn )

		DESCRIPTION: this function returns the current string in the StringNode's list.

		INPUT: - sn		 - an handler to an already allocated StringNode structure.

	RESULTS: - a pointer to a string if everything went fine.
		 - NULL if the list is empty.

			 SEE ALSO: - afc_stringnode_item()
		 - afc_nodemaster_obj()
@endnode
*/
/*
char * afc_stringnode_obj (StringNode * sn)
{
	return ((char *)afc_nodemaster_obj (sn->nm));
}
*/
// }}}
// {{{ afc_stringnode_is_empty ( sn )
/*
@node afc_stringnode_is_empty

		 NAME: afc_stringnode_is_empty (sn) - Checks if the StringNode's list is empty

			 SYNOPSIS: char * afc_stringnode_is_empty ( StringNode * sn )

		DESCRIPTION: this function checks if the current StringNode's list is empty or contains some items.

		INPUT: - sn		 - an handler to an already allocated StringNode structure.

	RESULTS: - TRUE if the StringNode does not contain any items.
		 - FALSE if at least one item is present in the current StringNode's list

			 SEE ALSO: - afc_nodemaster_is_empty()
@endnode
*/
/*
short afc_stringnode_is_empty (StringNode * sn)
{
	return ( afc_nodemaster_is_empty (sn->nm) );
}
*/
// }}}
// {{{ afc_stringnode_first ( sn )
/*
@node afc_stringnode_first

		 NAME: afc_stringnode_first (sn) - Returns the first item in the StringNode's list

			 SYNOPSIS: char * afc_stringnode_first ( StringNode * sn )

		DESCRIPTION: this function returns the first string in the StringNode's list

		INPUT: - sn		 - an handler to an already allocated StringNode structure.

	RESULTS: - the string requested.
		 - NULL if the StringNode is empty.

	 SEE ALSO: - afc_stringnode_last()
		 - afc_stringnode_next()
		 - afc_stringnode_prev()
		 - afc_stringnode_item()
		 - afc_nodemaster_first()
@endnode
*/
// char * afc_stringnode_first ( StringNode * sn ) { return ( ( char * ) afc_nodemaster_first ( sn->nm ) ); }
// }}}
// {{{ afc_stringnode_next ( sn )
/*
@node afc_stringnode_next

		 NAME: afc_stringnode_next (sn) - Returns the next item in the StringNode's list

			 SYNOPSIS: char * afc_stringnode_next ( StringNode * sn )

		DESCRIPTION: this function returns the next string in the StringNode's list

		INPUT: - sn		 - an handler to an already allocated StringNode structure.

	RESULTS: - the string requested.
		 - NULL if the list pointed to the last item and there is no next item, or because the list is empty.

	 SEE ALSO: - afc_stringnode_first()
		 - afc_stringnode_last()
		 - afc_stringnode_prev()
		 - afc_stringnode_item()
		 - afc_nodemaster_next()
@endnode
*/
// char * afc_stringnode_next ( StringNode * sn ) { return ( ( char * ) afc_nodemaster_next ( sn->nm ) ); }
// }}}
// {{{ afc_stringnode_prev ( sn )
/*
@node afc_stringnode_prev

		 NAME: afc_stringnode_prev (sn) - Returns the previous item in the StringNode's list

			 SYNOPSIS: char * afc_stringnode_prev ( StringNode * sn )

		DESCRIPTION: this function returns the previous string in the StringNode's list

		INPUT: - sn		 - an handler to an already allocated StringNode structure.

	RESULTS: - the string requested.
		 - NULL if the list pointed to the first item and there is no previous item, or because the list is empty.

			 SEE ALSO: - afc_stringnode_first()
		 - afc_stringnode_last()
		 - afc_stringnode_next()
		 - afc_stringnode_item()
		 - afc_nodemaster_prev()
@endnode
*/
// char * afc_stringnode_prev ( StringNode * sn ) { return ( ( char * ) afc_nodemaster_prev ( sn->nm ) ); }
// }}}
// {{{ afc_stringnode_get ( sn )
/*
@node afc_stringnode_get

		 NAME: afc_stringnode_get (sn) - Returns a pointer to the current list Node.

			 SYNOPSIS: struct Node * afc_stringnode_get ( StringNode * sn )

		DESCRIPTION: this function returns a pointer to the current Node in the list managed by StringNode.

		INPUT: - sn		 - an handler to an already allocated StringNode structure.

	RESULTS: - the node requested.
		 - NULL if the list is empty.

		NOTES: - This is a very low level function. You should only use it if you know what you are doing.

			 SEE ALSO: - afc_stringnode_addr()
		 - afc_nodemaster_get()
@endnode
*/
/*
struct Node * afc_stringnode_get (StringNode * sn)
{
	return (afc_nodemaster_get (sn->nm));
}
*/
// }}}
// {{{ afc_stringnode_addr ( sn )
/*
@node afc_stringnode_addr

		 NAME: afc_stringnode_addr (sn) - Returns a pointer to the List managed by StringNode

			 SYNOPSIS: struct List * afc_stringnode_addr ( StringNode * sn )

		DESCRIPTION: this function returns a pointer to the List managed by StringNode

		INPUT: - sn		 - an handler to an already allocated StringNode structure.

	RESULTS: - the List requested.

		NOTES: - This is a very low level function. You should only use it if you know what you are doing.
		 - This function does *never* return NULL, because even if the List is empty, it is always avaible.

			 SEE ALSO: - afc_stringnode_get()
		 - afc_nodemaster_addr()
@endnode
*/
/*
struct List * afc_stringnode_addr (StringNode * sn)
{
	return (afc_nodemaster_addr (sn->nm));
}
*/
// }}}
// {{{ afc_stringnode_push ( sn )
/*
@node afc_stringnode_push

		 NAME: afc_stringnode_push (sn) - Pushes the current list position on the stack

			 SYNOPSIS: short afc_stringnode_addr ( StringNode * sn )

		DESCRIPTION: this function pushes the current item in the StringNode list on the stack.

		INPUT: - sn		 - an handler to an already allocated StringNode structure.

	RESULTS: - TRUE if the push succeded
		 - FALSE if the stack is full

			 SEE ALSO: - afc_stringnode_pop()
		 - afc_stringnode_clear_stack()
		 - afc_nodemaster_push()
@endnode
*/
/*
short afc_stringnode_push (StringNode * sn)
{
	return (afc_nodemaster_push (sn->nm));
}
*/
// }}}
// {{{ afc_stringnode_pop ( sn, autopos )
/*
@node afc_stringnode_pop

		 NAME: afc_stringnode_pop (sn, autopos) - Returns the last item pushed on the stack

			 SYNOPSIS: char * afc_stringnode_pop ( StringNode * sn, short autopos )

		DESCRIPTION: this function retrieves the last string pushed on the stack.

		INPUT: - sn	- an handler to an already allocated StringNode structure.
		 - autopos	 - This is a boolean flag:

						 + TRUE -	restores the list pointer to the previously pushed node in list.

						 + FALSE - just removes the pushed node from the stack.


	RESULTS: - the last item pushed on the stack
		 - NULL if you set autopos to TRUE.


			 SEE ALSO: - afc_stringnode_push()
		 - afc_stringnode_clear_stack()
		 - afc_nodemaster_pop()
@endnode
*/
/*
char * afc_stringnode_pop (StringNode * sn, short autopos)
{
	return ((char *)afc_nodemaster_pop (sn->nm, autopos));
}
*/
// }}}
// {{{ afc_stringnode_del ( sn )
/*
@node afc_stringnode_del

		 NAME: afc_stringnode_del (sn) - Deletes the current string from the list

			 SYNOPSIS: char * afc_stringnode_delete ( StringNode * sn )

		DESCRIPTION: this function deletes the current string from the list.

		INPUT: - sn	- an handler to an already allocated StringNode structure.

	RESULTS: the (new) current string the list is pointing to.

			 SEE ALSO: - afc_stringnode_clear()
		 - afc_nodemaster_del()
@endnode
*/
char *afc_stringnode_del(StringNode *sn)
{
	char *s = NULL;

	if (!afc_stringnode_is_empty(sn))
	{
		if ((s = afc_nodemaster_obj(sn->nm)))
		{
			afc_string_delete(s);
			s = (char *)afc_nodemaster_del(sn->nm);
		}
	}

	return (s);
}
// }}}
// {{{ afc_stringnode_clear_stack ( sn )
/*
@node afc_stringnode_clear_stack

		 NAME: afc_stringnode_clear_stack (sn) - Clears the stack of pushed string

			 SYNOPSIS: void afc_stringnode_clear_stack ( StringNode * sn )

		DESCRIPTION: this function retrieves the last string pushed on the stack.

		INPUT: - sn	- an handler to an already allocated StringNode structure.

	RESULTS: NONE

			 SEE ALSO: - afc_stringnode_push()
		 - afc_stringnode_pop()
		 - afc_nodemaster_clear_stack()
@endnode
*/
/*
void afc_stringnode_clear_stack (StringNode * sn)
{
	afc_nodemaster_clear_stack (sn->nm);
}
*/
// }}}
// {{{ afc_stringnode_last ( sn )
/*
@node afc_stringnode_last

		 NAME: afc_stringnode_last (sn) - Returns the last item in the StringNode's list

			 SYNOPSIS: char * afc_stringnode_last ( StringNode * sn )

		DESCRIPTION: this function returns the last string in the StringNode's list

		INPUT: - sn		 - an handler to an already allocated StringNode structure.

	RESULTS: - the string requested.
		 - NULL if the list is empty.

			 SEE ALSO: - afc_stringnode_first()
		 - afc_stringnode_prev()
		 - afc_stringnode_next()
		 - afc_stringnode_item()
		 - afc_nodemaster_last()
@endnode
*/
/*
char * afc_stringnode_last (StringNode * sn)
{
	 return ((char *)afc_nodemaster_last (sn->nm));
}
*/
// }}}
// {{{ afc_stringnode_clear ( sn )
/*
@node afc_stringnode_clear

		 NAME: afc_stringnode_clear (sn) - Deletes all the strings in the StringNode

			 SYNOPSIS: int afc_stringnode_clear ( StringNode * sn )

		DESCRIPTION: this function deletes all the strings in the StringNode.

		INPUT: - sn	- an handler to an already allocated StringNode structure.

	RESULTS: NONE

			 SEE ALSO: - afc_stringnode_del()
		 - afc_nodemaster_clear()
@endnode
*/
int afc_stringnode_clear(StringNode *sn)
{
	char *s;

	if (sn == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));
	if (sn->magic != AFC_STRINGNODE_MAGIC)
		return (AFC_LOG_FAST(AFC_ERR_INVALID_POINTER));

	s = (char *)afc_nodemaster_first(sn->nm);
	while (s)
	{
		afc_string_delete(s);

		s = (char *)afc_nodemaster_next(sn->nm);
	}

	return (afc_nodemaster_clear(sn->nm));
}
// }}}
// {{{ afc_stringnode_len ( sn )
/*
@node afc_stringnode_len

		 NAME: afc_stringnode_len ( sn ) - Returns the number of strings in the list

			 SYNOPSIS: unsigned long afc_stringnode_len (StringNode * sn)

		DESCRIPTION: this function returns the number of strings present in the StringNode

		INPUT: - sn		 - an handler to an already allocated StringNode structure.

	RESULTS: the number of strings in the list.

			 SEE ALSO: - afc_nodemaster_len()
@endnode
*/
/*
unsigned long afc_stringnode_len (StringNode * sn)
{
	return (afc_nodemaster_len (sn->nm));
}
*/
// }}}
// {{{ afc_stringnode_item ( sn, num )
/*
@node afc_stringnode_item

		 NAME: afc_stringnode_item (sn, num) - Returns the desired item in the StringNode's list

			 SYNOPSIS: char * afc_stringnode_item ( StringNode * sn, unsigned long num )

		DESCRIPTION: this function returns the desired string in the StringNode's list

		INPUT: - sn		 - an handler to an already allocated StringNode structure.
		 - num		- the ordinal number of the desired item in the list.

	RESULTS: - the string requested.
		 - NULL if the list is empty.

		NOTES: - if num is less than 0, the first element in the list will be returned.
		 - if num is bigger than the last valid ordinal value, the last element in the list will be returned.

			 SEE ALSO: - afc_stringnode_first()
		 - afc_stringnode_last()
		 - afc_stringnode_prev()
		 - afc_stringnode_next()
		 - afc_nodemaster_item()
@endnode
*/
/*
char * afc_stringnode_item (StringNode * sn, unsigned long n)
{
	return ((char *)afc_nodemaster_item (sn->nm, n));
}
*/
// }}}
// {{{ afc_stringnode_change ( sn, str )
/*
@node afc_stringnode_change

		 NAME: afc_stringnode_change (sn, str) - Changes the string of a node in the list

			 SYNOPSIS: char * afc_stringnode_change ( StringNode * sn, char * str )

		DESCRIPTION: this function changes the string contained in the current node in the list.

		INPUT: - sn		 - an handler to an already allocated StringNode structure.
		 - str		- the string to insert in the current node.

	RESULTS: - AFC_ERR_NO_ERROR if everything went fine, an error code otherwise.

		NOTES: - This is a low level function and should only be used if you know what you are doing.

			 SEE ALSO: - afc_nodemaster_change()
@endnode
*/
int afc_stringnode_change(StringNode *sn, char *s)
{
	char *g;
	unsigned int len;

	if (afc_nodemaster_is_empty(sn->nm))
		return (AFC_ERR_NO_ERROR);

	if ((g = afc_nodemaster_obj(sn->nm)))
	{
		afc_string_delete(g);

		len = strlen(s);

		if ((g = afc_string_dup(s)) == NULL)
			return (AFC_LOG_FAST(AFC_ERR_NO_MEMORY));

		if (afc_nodemaster_change(sn->nm, g) == NULL)
			return (AFC_LOG(AFC_LOG_ERROR, AFC_STRINGNODE_ERR_CHANGE, "Cannot change string", NULL));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_stringnode_pos ( sn )
/*
@node afc_stringnode_pos

		 NAME: afc_stringnode_pos (sn) - Returns the ordinal position of the current string in the list

			 SYNOPSIS: unsigned long afc_stringnode_pos ( StringNode * sn )

		DESCRIPTION: this function returns the ordinal position of the current string in the list.

		INPUT: - sn		 - an handler to an already allocated StringNode structure.

	RESULTS: - the numerical value of the ordinal position of the string in the list

			 SEE ALSO: - afc_nodemaster_pos()
@endnode
*/
/*
unsigned long afc_stringnode_pos (StringNode * sn)
{
	return ( afc_nodemaster_pos (sn->nm) );
}
*/
// }}}

#ifndef MINGW
// {{{ afc_stringnode_search ( sn, str, from_here, nocase )
/*
@node afc_stringnode_search

		 NAME: afc_stringnode_search (sn, str, from_here, nocase) - Searches the list for the specified string

			 SYNOPSIS: int afc_stringnode_search (StringNode * sn, char * str, short from_here, short	nocase )

		DESCRIPTION: this function searches the whole list (or part of it) for the specified pattern string.
				   The pattern can be defined using the standard Unix file name pattern syntax, like "*" or "?".

		INPUT: - sn			   - an handler to an already allocated StringNode structure.
		 - str       - afc_string_new to search the list for.
				   - from_here - Set it to TRUE if you want to start searching from the current position
								 and not from the beginning of the list.
		 - nocase	   - If set to TRUE, the sort will be case insensitive.

	RESULTS: - AFC_ERR_NO_ERROR if everything went fine.

			NOTES: - This function may not be portable (it relies upon afc_string_pattern_match() )

			 SEE ALSO: - afc_nodemaster_sort()
@endnode
*/
char *afc_stringnode_search(StringNode *sn, char *str, short from_here, short nocase)
{
	char *s;

	if (afc_nodemaster_is_empty(sn->nm))
		return (NULL);

	afc_nodemaster_push(sn->nm);

	if (from_here == FALSE)
		s = (char *)afc_nodemaster_first(sn->nm);
	else
		s = (char *)afc_nodemaster_obj(sn->nm);

	while (s)
	{
		if (afc_string_pattern_match(s, str, nocase) == 0)
		{
			afc_nodemaster_pop(sn->nm, FALSE);
			return (s);
		}
		s = (char *)afc_nodemaster_next(sn->nm);
	}

	afc_nodemaster_pop(sn->nm, TRUE);
	return (NULL);
}
// }}}
#endif

// {{{ afc_stringnode_sort ( sn, nocase, inverted )
/*
@node afc_stringnode_sort

		 NAME: afc_stringnode_sort (sn, nocase, inverted) - Sorts the strings in the list

			 SYNOPSIS: int afc_stringnode_sort (StringNode * sn, short	nocase, short	inverted)

		DESCRIPTION: this function sorts the strings in the list. You can specify some sorting methods,
		 such like if you want the sort being case insensitive (nocase) or if you want the
		 order from Z-A instead of A-Z (inverted).

		INPUT: - sn			 - an handler to an already allocated StringNode structure.
		 - nocase	 - If set to TRUE, the sort will be case insensitive.
		 - inverted - If set to TRUE, the sort will be inverted: from Z to A and not A-Z.
		 - fast		 - If set to TRUE, the fast sort routine in NodeMaster will be used.

	RESULTS: - AFC_ERR_NO_ERROR if everything went fine.

			 SEE ALSO: - afc_nodemaster_sort()
@endnode
*/
int afc_stringnode_sort(StringNode *sn, short nocase, short inverted, short fast)
{

	// afc_nodemaster_free_array(nm);

	if (nocase)
	{
		if (inverted)
		{
			if (fast)
				afc_nodemaster_fast_sort(sn->nm, afc_stringnode_internal_sort_nocase_inv, NULL);
			else
				afc_nodemaster_sort(sn->nm, afc_stringnode_internal_sort_nocase_inv, NULL);
		}
		else
		{
			if (fast)
				afc_nodemaster_fast_sort(sn->nm, afc_stringnode_internal_sort_nocase_noinv, NULL);
			else
				afc_nodemaster_sort(sn->nm, afc_stringnode_internal_sort_nocase_noinv, NULL);
		}
	}
	else
	{
		if (inverted)
		{
			if (fast)
				afc_nodemaster_fast_sort(sn->nm, afc_stringnode_internal_sort_case_inv, NULL);
			else
				afc_nodemaster_sort(sn->nm, afc_stringnode_internal_sort_case_inv, NULL);
		}
		else
		{
			if (fast)
				afc_nodemaster_fast_sort(sn->nm, afc_stringnode_internal_sort_case_noinv, NULL);
			else
				afc_nodemaster_sort(sn->nm, afc_stringnode_internal_sort_case_noinv, NULL);
		}
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_stringnode_close ( sn )
/*
@node afc_stringnode_clone

		 NAME: afc_stringnode_clone (sn) - Clones a StringNode creating a new one

			 SYNOPSIS: StringNode * afc_stringnode_clone (StringNode * sn)

		DESCRIPTION: this function copies all entries of a StringNode inside a new one.
		 The new StringNode will result sorted and the internal array rappresentation
				   is recreated in case that the original StringNode has these attributes set.

		INPUT: - sn			 - an handler to an already allocated StringNode structure.

	RESULTS: a pointer to a valid new StringNode object, or NULL if an error occurred.

	  NOTES: - The new StringNode object must be freed with afc_stringnode_delete() like
					 any other StringNode object.

		 - The new StringNode will contain copies of all string stored, not just references,
					 so it is perfectly safe to afc_stringnode_delete() the original StringNode.

		 - In case the original StringNode had the internal array rappresentation of the list,
					 it will  be reproduced in the new StringNode.

			 SEE ALSO: - afc_nodemaster_sort()
@endnode
*/
StringNode *afc_stringnode_clone(StringNode *sn)
{
	StringNode *sn2 = afc_stringnode_new();
	char *s;

	if (sn2 == NULL)
		return (NULL);

	s = afc_stringnode_first(sn);
	while (s)
	{
		afc_stringnode_add(sn2, s, AFC_STRINGNODE_ADD_TAIL);
		s = afc_stringnode_next(sn);
	}

	// Set the "is_sorted" flag inside the underlying NodeMaster
	// to the new StringNode.
	sn2->nm->is_sorted = sn->nm->is_sorted;

	if (sn->nm->is_array_valid)
		afc_nodemaster_create_array(sn2->nm);

	return (sn2);
}
// }}}
// {{{ afc_stringnode_split ( sn, string, delimiters )
/*
@node afc_stringnode_split

		 NAME: afc_stringnode_split ( sn, string, delimiters )		 - Splits a string

			 SYNOPSIS: int afc_stringnode_split ( StringNode * sn, const char * string, const char * delimiters )

		DESCRIPTION: This function splits the string. You can specify one or more delimiters in one single string
				   and the splitter will create the smallest part of them. Once the split is done, you can access
				   the items usign afc_stringnode_item(), afc_stringnode_first(), afc_stringnode_next()
				   and so on, because the fields are stored inside an AFC StringNode.

		INPUT: - sn      - A pointer to a valid StringNode instance
				   - string     - The string to split in pieces
				   - delimiters - All delimiters that the splitter must take care of.

	RESULTS: - AFC_ERR_NO_ERROR on success.
				   - An error code in case of error

			NOTES: - Before the split is done, the function calls afc_stringnode_clear()

	 SEE ALSO: - afc_stringnode_first()
				   - afc_stringnode_next()
				   - afc_stringnode_item()

@endnode
*/
int afc_stringnode_split(StringNode *sn, const char *string, const char *delimiters)
{
	char *base_string, *separators;
	char *start_pos, *end_pos, *end_string;
	char *x, *y, escape_char;
	int t, num_delimiters;

	if (string == NULL)
		return (AFC_LOG(AFC_LOG_WARNING, AFC_STRINGNODE_ERR_NULL_STRING, "Null string is invalid", NULL));
	if (delimiters == NULL)
		return (AFC_LOG(AFC_LOG_WARNING, AFC_STRINGNODE_ERR_NULL_DELIMITERS, "Null delimiters string is invalid", NULL));

	afc_stringnode_clear(sn);

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

		afc_stringnode_add(sn, start_pos, AFC_STRINGNODE_ADD_TAIL);
	}

	afc_string_delete(base_string);
	afc_string_delete(separators);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_stringnode_set_tags ( sn, first_tag, ... )
/*
@node afc_stringnode_set_tags

		 NAME: afc_stringnode_set_tags ( sn, first_tag, ... )		 - Set StringNode Tags

			 SYNOPSIS: int afc_stringnode_set_tags ( StringNode * sn, int first_tag, ... );

	DESCRIPTION: This function sets StringNode behaviours using tags.

		INPUT: - sn      	- A pointer to a valid StringNode instance
			   - first_tag 	- First tag to be set.
					  Supported tags are:

						+ AFC_STRINGNODE_TAG_DISCARD_ZERO_LEN - (TRUE/FALSE) If set to TRUE
							StringNode will not add to its list any string which is zero length.
							By default, an empty string is added to the list in place of the zero length string.

						+ AFC_STRINGNODE_TAG_ESCAPE_CHAR	- (char) This is the char to be used as the escape
							character during the afc_stringnode_split() operations. Please, remember to provide a
							single character (ie. in single quotes '' and not double quotes) as escape char definition.

			   - ... 		- All values and tags

	RESULTS: - AFC_ERR_NO_ERROR on success.
				   - An error code in case of error

			NOTES: - Remember to end the tag list with AFC_TAG_END

	 SEE ALSO: - afc_stringnode_first()
				   - afc_stringnode_next()
				   - afc_stringnode_item()

@endnode
*/
int _afc_stringnode_set_tags(StringNode *sn, int first_tag, ...)
{
	va_list args;
	unsigned int tag;
	void *val;

	va_start(args, first_tag);

	tag = first_tag;

	while (tag != AFC_TAG_END)
	{
		val = va_arg(args, void *);

		afc_stringnode_internal_set_tag(sn, tag, val);

		tag = va_arg(args, int);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_stringnode_before_first ( sn ) ************
// int afc_stringnode_before_first ( StringNode * sn ) { return ( afc_nodemaster_before_first ( sn->nm ) ); }
// }}}

/* -------------------------------------------------------------------------------------------
	 INTERNAL FUNCTIONS
--------------------------------------------------------------------------------------------- */
// {{{ afc_stringnode_internal_set_tag ( sn, tag, val )
static int afc_stringnode_internal_set_tag(StringNode *sn, int tag, void *val)
{
	switch (tag)
	{
	case AFC_STRINGNODE_TAG_DISCARD_ZERO_LEN:
		sn->discard_zero_len = (BOOL)(int)(long)val;
		break;

	case AFC_STRINGNODE_TAG_ESCAPE_CHAR:
		sn->escape_char = (char)(int)(long)val;
		break;

	default:
		break;
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_stringnode_internal_sort_nocase_noinv ( a, b, info )
static long afc_stringnode_internal_sort_nocase_noinv(void *a, void *b, void *info)
{
	return (-afc_string_comp((char *)a, (char *)b, ALL));
}
// }}}
// {{{ afc_stringnode_internal_sort_case_noinv ( a, b, info )
static long afc_stringnode_internal_sort_case_noinv(void *a, void *b, void *info)
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
// {{{ afc_stringnode_internal_sort_nocase_inv ( a, b, info )
static long afc_stringnode_internal_sort_nocase_inv(void *a, void *b, void *info)
{
	return (afc_string_comp((char *)a, (char *)b, ALL));
}
// }}}
// {{{ afc_stringnode_internal_sort_case_inv ( a, b, info )
static long afc_stringnode_internal_sort_case_inv(void *a, void *b, void *info)
{
	return (-afc_stringnode_internal_sort_case_noinv(a, b, info));
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
void dosort(struct afc_stringnode *n)
{
	afc_stringnode_sort(n, TRUE, TRUE, FALSE);
}

void shwall(struct afc_stringnode *n)
{
	printf("-----------------------\n");

	if (afc_stringnode_first(n))
	{
		do
			printf("Item: %s - Pos:%ld\n", afc_stringnode_obj(n), afc_stringnode_numerical_pos(n));
		while (afc_stringnode_next(n));
	}

	printf("-----------------------\n");
}

int main(void)
{
	struct afc_stringnode *sn;
	// struct TagItem tags[] = { TAGSTR_MaxChars, 3, TAG_END };

	sn = afc_stringnode_new();

	if (sn)
	{
		afc_stringnode_first(sn);

		afc_stringnode_add(sn, "Ciao Mamma", AFC_STRINGNODE_ADD_TAIL);
		afc_stringnode_add(sn, "Zio Peppino", AFC_STRINGNODE_ADD_TAIL);
		afc_stringnode_add(sn, "Paperino", AFC_STRINGNODE_ADD_TAIL);
		afc_stringnode_add(sn, "Tom & Jerry", AFC_STRINGNODE_ADD_TAIL);
		afc_stringnode_add(sn, "Pluto", AFC_STRINGNODE_ADD_TAIL);
		afc_stringnode_add(sn, "Anna", AFC_STRINGNODE_ADD_TAIL);
		afc_stringnode_add(sn, "Zorro", AFC_STRINGNODE_ADD_TAIL);
		afc_stringnode_add(sn, "Vienna", AFC_STRINGNODE_ADD_TAIL);
		afc_stringnode_add(sn, "PIPPO", AFC_STRINGNODE_ADD_TAIL);

		dosort(sn);
		shwall(sn);

		// afc_stringnode_search ("Tom#?");
		// afc_stringnode_search ("PLA");

		// printf ("Running!\n");

		// printf ("STRING:%s - Pos:%ld\n",afc_stringnode_obj (),afc_stringnode_pos ());

		afc_stringnode_clear(sn);

		afc_stringnode_split(sn, "ciao|mamma|bella|come|stai", "|");

		shwall(sn);

		// afc_stringnode_setattrs (tags);
		afc_stringnode_add(sn, "Ciao Mammina Bella", AFC_STRINGNODE_ADD_TAIL);
		printf("Str:%s - Chars (%ld)\n", afc_stringnode_obj(sn), afc_string_len(afc_stringnode_obj(sn)));
		afc_stringnode_change(sn, "Ciao Mammina Be");
		printf("Str:%s - Chars (%ld)\n", afc_stringnode_obj(sn), afc_string_len(afc_stringnode_obj(sn)));
		afc_stringnode_change(sn, "Pippo");
		printf("Str:%s - Chars (%ld)\n", afc_stringnode_obj(sn), afc_string_len(afc_stringnode_obj(sn)));

		afc_stringnode_delete(sn);
	}

	return (0);
}
// }}}
#endif
