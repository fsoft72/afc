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
/*	List.cpp	$ 19/05/97 FR MT $	*/

// {{{ docs
/*
@config
	TITLE:     List
	VERSION:   4.20
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
	AUTHOR:    Massimo Tantignone - tanti@intercom.it
@endnode

@node quote
	*Those who forget the pasta are condemned to reheat it.*

		Anonymous
@endnode

@node history
	- 4.20	- Added afc_list_before_first() function.
@endnode

@node intro
List is a class that handles double linked lists.

Like all AFC classes, you can instance a new  List by calling afc_list_new (), and free it with afc_list_delete ().

To add elements to the array, use afc_list_add (); to delete all elements call
afc_list_clear(), and to delete just one of them there is afc_list_del() .
@endnode
*/
// }}}

#include "list.h"

static const char class_name[] = "List";

static void afc_list_internal_init_list(List *nm);
static void afc_list_internal_split(List *nm, unsigned long inf, unsigned long sup, signed long *mid, signed long (*comp)(void *, void *, void *), void *info);
static void afc_list_internal_fast_split(List *nm, unsigned long inf, unsigned long sup, signed long *mid, signed long (*comp)(void *, void *, void *), void *info);
static void afc_list_internal_quick_sort(List *nm, unsigned long inf, unsigned long sup, signed long (*comp)(void *, void *, void *), void *info);
static void afc_list_internal_fast_quick_sort(List *nm, unsigned long inf, unsigned long sup, signed long (*comp)(void *, void *, void *), void *info);
// static int afc_list_internal_ultra_comp ( List * nm, const void * s1, const void * s2 );

// {{{ list handling functions
static void AddTail(struct List *list, struct Node *node)
{
	node->ln_Succ = (struct Node *)&list->lh_Tail;
	node->ln_Pred = list->lh_TailPred;

	list->lh_TailPred->ln_Succ = node;
	list->lh_TailPred = node;
}

static void AddHead(struct List *list, struct Node *node)
{
	node->ln_Succ = list->lh_Head;
	node->ln_Pred = (struct Node *)&list->lh_Head;

	list->lh_Head->ln_Pred = node;
	list->lh_Head = node;
}

static void Insert(struct List *list, struct Node *node, struct Node *pred)
{
	if (pred)
	{
		if (pred->ln_Succ)
		{
			node->ln_Succ = pred->ln_Succ;
			node->ln_Pred = pred;

			pred->ln_Succ->ln_Pred = node;
			pred->ln_Succ = node;
		}
		else
		{
			node->ln_Succ = (struct Node *)&list->lh_Tail;
			node->ln_Pred = list->lh_TailPred;

			list->lh_TailPred->ln_Succ = node;
			list->lh_TailPred = node;
		}
	}
	else
	{
		node->ln_Succ = list->lh_Head;
		node->ln_Pred = (struct Node *)&list->lh_Head;
		list->lh_Head->ln_Pred = node;
		list->lh_Head = node;
	}
}

static void Remove(struct Node *node)
{
	node->ln_Pred->ln_Succ = node->ln_Succ;
	node->ln_Succ->ln_Pred = node->ln_Pred;
}
// }}}

// {{{ List * afc_list_new ()
/*
@node afc_list_new

		 NAME: afc_list_new () - Initializes a new List object.

			 SYNOPSIS: List * afc_list_new ()

		DESCRIPTION: Use this command to inizialize a List object.
		 This function does:
			 1. Alloc memory for the class structure
			 2. Alloc memory for the embedded list
			 3. Init the list

		INPUT: NONE

	RESULTS: an initialized List structure.

			 SEE ALSO: afc_list_delete()
@endnode
*/
List *_afc_list_new(const char *file, const char *func, const unsigned int line)
{
	TRY(List *)

	List *nm = (List *)_afc_malloc(sizeof(List), file, func, line);

	if (nm == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "nm", NULL);
	nm->magic = AFC_LIST_MAGIC;

	if ((nm->lst = afc_malloc(sizeof(struct List))) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "lst", NULL);

	afc_list_internal_init_list(nm);
	nm->is_sorted = TRUE;
	RETURN(nm);

	EXCEPT
	afc_list_delete(nm);

	FINALLY

	ENDTRY
}
// }}}
// {{{ int afc_list_delete(List * nm)
/*
@node afc_list_delete

		 NAME: afc_list_delete(nm) - Dispose a List object

			 SYNOPSIS: int afc_list_delete(List * nm)

		DESCRIPTION: This function frees an already alloc'd List.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: the return code (should be AFC_ERR_NO_ERROR )

		NOTES: - this method calls: afc_list_clear()

			 SEE ALSO: - afc_list_new()
@endnode
*/
int _afc_list_delete(List *nm)
{
	int res;

	if (nm == NULL)
		return AFC_LOG_FAST(AFC_ERR_NULL_POINTER);

	if (nm->magic != AFC_LIST_MAGIC)
		return AFC_LOG_FAST(AFC_ERR_INVALID_POINTER);

	if ((res = afc_list_clear(nm)) != AFC_ERR_NO_ERROR)
		return (res);

	afc_free(nm->lst);
	afc_free(nm);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ void * afc_list_add(List * nm, void * s, unsigned long mode)
/*
@node afc_list_add

		 NAME: afc_list_add(nm, object, mode) - Add an item to the list

			 SYNOPSIS: void * afc_list_add(List * nm, void * object, unsigned long mode)

		DESCRIPTION: Use this command to add an object to the list.

		INPUT: - object	 - This is the object to add.
		 - mode		 - This flag is very useful to
						choose _where_ a new node will be added.
						Usually, you should add the item after the last one, but you can
						add it as the first line or in the middle
						of the list (same as afc_list_insert() command).

						Possible values are:

						+ AFC_LIST_ADD_HEAD - Use this one to add the node
								as the first in list.

						+ AFC_LIST_ADD_HERE - Use this one to add the node
								AFTER the current one.
								(Same as afc_list_insert() method)

						+ AFC_LIST_ADD_TAIL - Use this one to add
								 the node as the last in list.


	RESULTS: the value of the node just created or NULL in case of errors


			 SEE ALSO: - afc_list_insert()
		 - afc_list_del()
		 - afc_list_item()
@endnode
*/
void *afc_list_add(List *nm, void *s, unsigned long mode)
{
	struct Node *nn;

	nn = (struct Node *)afc_malloc(sizeof(struct Node));
	if (nn == NULL)
		return (NULL);
	nn->ln_Succ = nn->ln_Pred = NULL;
	// nn->ln_Type = 0;
	// nn->ln_Pri	= 0;
	nn->ln_Name = (char *)s;

	nm->num++;

	if ((mode == AFC_LIST_ADD_HERE) && (!nm->pos || IsListEmpty(nm->lst)))
		mode = AFC_LIST_ADD_TAIL;

	switch (mode)
	{
	case AFC_LIST_ADD_TAIL:
		AddTail(nm->lst, nn);
		nm->pos = nm->lst->lh_TailPred;
		nm->npos = nm->num - 1;
		break;

	case AFC_LIST_ADD_HERE:
		Insert(nm->lst, nn, nm->pos);
		nm->pos = nm->pos->ln_Succ;
		break;

	case AFC_LIST_ADD_HEAD:
		AddHead(nm->lst, nn);
		nm->pos = nm->lst->lh_Head;
		nm->npos = 0;
		break;
	} /* End switch() */

	nm->is_sorted = FALSE;
	nm->is_array_valid = FALSE;

	return (nm->pos->ln_Name);
}
// }}}
// {{{ short afc_list_is_empty(List * nm)
/*
@node afc_list_is_empty

		 NAME: afc_list_is_empty(nm) - Checks for empty list

			 SYNOPSIS: short afc_list_is_empty(List * nm)

		DESCRIPTION: Use this command to check whether the list is empty or not.

		INPUT: - nm	 - Pointer to a vaalid List class.

	RESULTS: - TRUE			 - List is empty
		 - FALSE			- At least one item is present.

			 SEE ALSO: - afc_list_is_first()
		 - afc_list_is_last()

@endnode
*/
short afc_list_is_empty(List *nm)
{
	return (IsListEmpty(nm->lst));
}
// }}}
// {{{ void * afc_list_first(List * nm)
/*
@node afc_list_first

		 NAME: afc_list_first(nm) - Move to the first element

			 SYNOPSIS: void * afc_list_first(List * nm)

		DESCRIPTION: Use this command to jump to the first object in the list.

		INPUT: - nm			 - Pointer to a valid List class

	RESULTS: the first object in this list, NULL if the list is empty.

			 SEE ALSO: - afc_list_last()
@endnode
*/
void *afc_list_first(List *nm)
{
	nm->before_first = FALSE;

	return (IsListEmpty(nm->lst) ? NULL : (nm->npos = 0, (nm->pos = nm->lst->lh_Head))->ln_Name);
}
// }}}
// {{{ struct Node * afc_list_get(List * nm)
/*
@node afc_list_get

		 NAME: afc_list_get(nm) - Get the current element

			 SYNOPSIS: struct Node *afc_list_get(List * nm)

		DESCRIPTION: Use this command to get a pointer to the current node.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: the current list node, or NULL if the list is empty.

			 SEE ALSO: - afc_list_addr()
@endnode
*/
struct Node *afc_list_get(List *nm)
{
	return (nm->pos);
}
// }}}
// {{{ struct List * afc_list_addr(List * nm)
/*
@node afc_list_addr

		 NAME: afc_list_addr(nm) - Returns the list address

			 SYNOPSIS: struct List * afc_list_addr(List * nm)

		DESCRIPTION: Use this command to get the addr of the Exec List.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: the List List address.

			 SEE ALSO: - afc_list_get()
@endnode
*/
struct List *afc_list_addr(List *nm)
{
	return (nm->lst);
}
// }}}
// {{{ short afc_list_push(List * nm)
/*
@node afc_list_push

		 NAME: afc_list_push(nm) - Pushes an item on the stack

			 SYNOPSIS: short afc_list_push(List * nm)

		DESCRIPTION: Use this command to memorize the current node position.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: - TRUE	- pushing successful.

		 - FALSE - pushing failed (out of stack space, or no items)

			 SEE ALSO: - afc_list_pop()
		 - afc_list_clear_stack()
@endnode
*/
short afc_list_push(List *nm)
{
	short result = FALSE;

	if (nm->pos && (nm->sposcount < 8))
	{
		result = TRUE;
		nm->spos[nm->sposcount++] = nm->pos;
	}

	return (result);
}
// }}}
// {{{ void * afc_list_pop(List * nm, short autopos)
/*
@node afc_list_pop

		 NAME: afc_list_pop(nm, autopos) - Pops an item from the stack

			 SYNOPSIS: void * afc_list_pop(List * nm, short autopos)

		DESCRIPTION: Use this command to restore current node to the one
		 previously pushed.

		INPUT: - nm	- Pointer to a valid List class.
		 - autopos	 - This is a boolean flag:

						 + TRUE -	restores the list pointer to the previously pushed node in list.

						 + FALSE - just removes the pushed node from the stack.

	RESULTS: - the right node	- if pop succeded and autopos=TRUE.

		 - FALSE		 - not popped or autopos=FALSE.

		NOTES: - If no node was pushed the current node won't change.

			 SEE ALSO: - afc_list_push()
		 - afc_list_clear_stack()
@endnode
*/
void *afc_list_pop(List *nm, short autopos)
{
	if (nm->sposcount)
	{
		if (autopos)
		{
			return ((nm->pos = nm->spos[--nm->sposcount])->ln_Name);
		}
		else
		{
			nm->sposcount--;
			return (NULL);
		}
	}
	else
	{
		return (NULL);
	}
}
// }}}
// {{{ void * afc_list_obj(List * nm)
/*
@node afc_list_obj

		 NAME: afc_list_obj(nm) - Gets the current item

			 SYNOPSIS: void * afc_list_obj(List * nm)

		DESCRIPTION: Use this command to get the current node's contents.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: data contained in the current node. NULL if the list is empty.

			 SEE ALSO: - afc_list_add()
@endnode
*/
void *afc_list_obj(List *nm)
{
	return ((IsListEmpty(nm->lst) || !nm->pos) ? NULL : (nm->pos->ln_Name));
}
// }}}
// {{{ void * afc_list_del(List * nm)
/*
@node afc_list_del

		 NAME: afc_list_del(nm) - Deletes the current item from list

			 SYNOPSIS: void * afc_list_del(List * nm)

		DESCRIPTION: Use this command to delete the current node.
		 After deletion the CURRENT NODE will be the next one.
		 If the node you deleted was the last one, then the next
		 will be the previous one.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: Data contained in the next node. Starting from V3.00 afc_list_del() method
		 returns the pointer to the next actual obj data, or NULL
		 if it was the last node avaible in list (ie. list empty)

			 SEE ALSO: - afc_list_clear()
				   - afc_list_add()

@endnode
*/
void *afc_list_del(List *nm)
{
	struct Node *n = NULL;
	unsigned char t, i;

	if (IsListEmpty(nm->lst))
		return (NULL);

	if (nm->pos)
	{
		if (nm->func_clear)
			nm->func_clear(nm->pos->ln_Name);

		for (t = 0; t < (nm->sposcount - 1); t++)
		{
			if (nm->spos[t] == nm->pos)
			{
				for (i = (unsigned char)(t + 1); t < nm->sposcount; i++)
					nm->spos[t - 1] = nm->spos[t];

				nm->spos[--nm->sposcount] = NULL;
			}
		}

		if (nm->pos != nm->lst->lh_TailPred)
			n = nm->pos->ln_Succ;
		else
		{
			n = nm->pos->ln_Pred;
			nm->npos = nm->num - 2;
		}

		Remove(nm->pos);

		afc_free(nm->pos);
		nm->pos = n;
		nm->num--;
	}

	nm->is_sorted = FALSE;
	nm->is_array_valid = FALSE;

	if (IsListEmpty(nm->lst))
	{
		afc_list_internal_init_list(nm);
		afc_list_free_array(nm);
		return (NULL);
	}

	return (nm->pos ? nm->pos->ln_Name : NULL);
}
// }}}
// {{{ void afc_list_clear_stack(List * nm)
/*
@node afc_list_clear_stack

		 NAME: afc_list_clear_stack(nm) - Clears the stack

			 SYNOPSIS: void afc_list_clear_stack(List * nm)

		DESCRIPTION: Use this method to clear all pushed nodes in stack.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: push stack will be cleared.

			 SEE ALSO: - afc_list_push()
		 - afc_list_pop()
@endnode
*/
void afc_list_clear_stack(List *nm)
{
	nm->sposcount = 0;
}
// }}}
// {{{ void * afc_list_last(List * nm)
/*
@node afc_list_last

		 NAME: afc_list_last(nm) - Moves to the last item in list

			 SYNOPSIS: void * afc_list_last(List * nm)

		DESCRIPTION: Use this command to get the last node in list.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: The contents of the last node in list.	NULL if list is empty.

			 SEE ALSO: - afc_list_first()
		 - afc_list_item()
@endnode
*/
void *afc_list_last(List *nm)
{
	return (IsListEmpty(nm->lst) ? NULL : (nm->npos = nm->num - 1, (nm->pos = nm->lst->lh_TailPred)->ln_Name));
}
// }}}
// {{{ void * afc_list_next(List * nm)
/*
@node afc_list_next

		 NAME: afc_list_next(nm) - Moves to the next item in list

			 SYNOPSIS: void * afc_list_next(List * nm)

		DESCRIPTION: Use this command to position current node to the next one in list.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: Data stored in the next node, or NULL if it was the last node.

			 SEE ALSO: - afc_list_prev()
		 - afc_list_is_last()
@endnode
*/
void *afc_list_next(List *nm)
{
	if (nm->before_first)
		return (afc_list_first(nm));
	if (IsListEmpty(nm->lst))
		return (NULL);

	return ((nm->pos == nm->lst->lh_TailPred) ? NULL : (nm->npos++, (nm->pos = nm->pos->ln_Succ)->ln_Name));
}
// }}}
// {{{ void * afc_list_prev(List * nm)
/*
@node afc_list_prev

		 NAME: afc_list_prev(nm) - Moves to the previous item in list

			 SYNOPSIS: void * afc_list_prev(List * nm)

		DESCRIPTION: Use this command to go to the previous node in the list.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: node data: positioning successful.	NULL if no previous items.

			 SEE ALSO: - afc_list_next()
		 - afc_list_first()
		 - afc_list_last()
@endnode
*/
void *afc_list_prev(List *nm)
{
	if (IsListEmpty(nm->lst))
		return (NULL);

	return ((nm->pos == nm->lst->lh_Head) ? NULL : (nm->npos--, (nm->pos = nm->pos->ln_Pred)->ln_Name));
}
// }}}
// {{{ void * afc_list_insert(List * nm, void * s)
/*
@node afc_list_insert

		 NAME: afc_list_insert(nm, data) - Inserts a new item in the current list position

			 SYNOPSIS: void * afc_list_insert(List * nm, void * data)

		DESCRIPTION: Use this command to add an object AFTER the current node.

		INPUT: - nm	- Pointer to a valid List class.
		 - data			- Data you want to add.

	RESULTS: the data you have just added or NULL if an error occurred.

			 SEE ALSO: - afc_list_add()
		 - afc_list_del()
@endnode
*/
void *afc_list_insert(List *nm, void *s)
{
	return (afc_list_add(nm, s, AFC_LIST_ADD_HERE));
}
// }}}
// {{{ int afc_list_clear(List * nm)
/*
@node afc_list_clear

		 NAME: afc_list_clear(nm) - Clears all List list elements

			 SYNOPSIS: int afc_list_clear(List * nm)

		DESCRIPTION: Use this command to clear all items in the list.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: The list will be completely empty.
		 Eventually, the array memory is freed as well.

			 SEE ALSO: - afc_list_del()
		 - afc_list_free_array()

@endnode
*/
int afc_list_clear(List *nm)
{
	struct Node *n, *w;

	if (nm == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));
	if (nm->magic != AFC_LIST_MAGIC)
		return (AFC_LOG_FAST(AFC_ERR_INVALID_POINTER));

	nm->is_sorted = FALSE;

	if (IsListEmpty(nm->lst))
		return (AFC_ERR_NO_ERROR);

	w = nm->lst->lh_Head;

	while ((n = w->ln_Succ))
	{
		if (nm->func_clear)
			nm->func_clear(n->ln_Name);
		Remove(w);
		afc_free(w);
		w = n;
	}

	afc_list_internal_init_list(nm);
	afc_list_free_array(nm);

	return (AFC_ERR_NO_ERROR);
}
// }}}
//  {{{ unsigned long afc_list_len(List * nm)
/*
@node afc_list_len

		 NAME: afc_list_len(nm) - Returns the number of items in list

			 SYNOPSIS: unsigned long afc_list_len(List * nm)

		DESCRIPTION: Use this command to know how many items are added to the list.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: items	- Number of items in this list.

			 SEE ALSO: - afc_list_add()
		 - afc_list_del()
@endnode
*/
// unsigned long afc_list_len(List * nm) { return (nm->num); }
// }}}
// {{{ void * afc_list_item(List * nm, unsigned long n)
/*
@node afc_list_item

		 NAME: afc_list_item(nm, numitem) - Moves to the desired item

			 SYNOPSIS: void * afc_list_item(List * nm, unsigned long numitem)

		DESCRIPTION: Use this command to position current node to the ordinal numitem node.

		INPUT: - nm		- Pointer to a valid List class.
		 - numitem		 - signed long. Ordinal value of node position.

	RESULTS: Node data - Position correct. NULL if the list is empty.

		NOTES: From v4.00 it can take advantages of the array rappresentation of the
		 list to speed up item positioning. Please, see afc_list_create_array()
		 for more info.

			 SEE ALSO: - afc_list_len()
		 - afc_list_create_array()
@endnode
*/
void *afc_list_item(List *nm, unsigned long n)
{
	unsigned long t, s;
	struct Node *node;

	if (IsListEmpty(nm->lst))
		return (NULL);

	if (nm->is_array_valid)
	{
		// if (n<0) n=0;
		if (n >= nm->num)
			n = nm->num - 1;
		node = nm->array[n];
		nm->pos = node;
		nm->npos = n;
		return (node->ln_Name);
	}

	if (n != nm->npos)
	{

		if (n > nm->num)
			return (afc_list_last(nm));

		if (n == 0L)
		{
			afc_list_first(nm);
		}
		else if (n > nm->num)
		{
			afc_list_last(nm);
		}
		else
		{
			s = nm->npos;

			if (n > nm->npos)
			{
				for (t = s; t < n; t++)
					afc_list_next(nm);
			}
			else
			{
				for (t = s - 1; t >= n; t--)
					afc_list_prev(nm);
			}
		}
	}

	return (nm->pos->ln_Name);
}
// }}}
// {{{ void * afc_list_change(List * nm, void * s)
/*
@node afc_list_change

		 NAME: afc_list_change(nm, data) - Changes the data in the current item

			 SYNOPSIS: void * afc_list_change(List * nm, void * data)

		DESCRIPTION: Use this command to change the data contained in the current node.

		INPUT: - nm	- Pointer to a valid List class.
		 - data			- New data to be stored in the current node.

	RESULTS: The just inserted data or NULL if an error occurred.

			 SEE ALSO:
@endnode
*/
void *afc_list_change(List *nm, void *s)
{
	nm->is_sorted = FALSE;

	return (nm->pos ? (nm->pos->ln_Name = (char *)s) : NULL);
}
// }}}
// {{{ void * afc_list_change_pos(List * nm, struct Node *node)
/*
@node afc_list_change_pos

		 NAME: afc_list_changepos(nm, node) - Changes the current node position to another

			 SYNOPSIS: void * afc_list_change_pos(List * nm, struct Node *node)

		DESCRIPTION: Use this command to change current node position to another.

		INPUT: - nm	- Pointer to a valid List class.
		 - node			- new list node to change position to.

		NOTES: You *MUST* know exactly what you are doing. Passing a wrong
		 node as parameter could get to instability.
		 This command is designed only for "professional" user who
		 intend build new object inheriting this one.

	RESULTS: the new node data or NULL if an error occurred.

			 SEE ALSO: - afc_list_change_numerical_pos()

@endnode
*/
void *afc_list_change_pos(List *nm, struct Node *node)
{
	return (nm->pos ? ((nm->pos = node) ? nm->pos->ln_Name : NULL) : NULL);
}
// }}}
// {{{ unsigned long afc_list_pos(List * nm)
/*
@node afc_list_pos

		 NAME: afc_list_pos() - Returns the current ordinal item position

			 SYNOPSIS: unsigned long afc_list_pos(List * nm)

		DESCRIPTION: Use this command to know the ordinal object position inside
		 the list.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: The current ordinal node position is returned.

			 SEE ALSO: - afc_list_change_numerical_pos()

@endnode
*/
/*
unsigned long afc_list_pos(List * nm)
{
	 return (nm->npos);
}
*/
// }}}
// {{{ void afc_list_change_numerical_pos(List * nm, unsigned long newnum)
/*
@node afc_list_change_numerical_pos

		 NAME: afc_list_change_numerical_pos(nm, newnum) - Changes the current ordinal item position

			 SYNOPSIS: void afc_list_change_numerical_pos(List * nm, unsigned long newnum)

		DESCRIPTION: Use this command to change current node ordinal position
		 number.

		INPUT: - nm	- Pointer to a valid List class.
		 - newnum		- new ordinal number to assign to the current node.

		 NOTE: You *MUST* know exactly what you are doing. Passing a wrong
		 value as parameter could get to instability.
		 This command is designed only for "professional" user who
		 intend build new object inheriting this one.

	RESULTS: The current node ordinal number will be changed.

			 SEE ALSO: - afc_list_change_pos()
@endnode
*/
void afc_list_change_numerical_pos(List *nm, unsigned long newnum)
{
	nm->npos = newnum;
}
// }}}
// {{{ void * afc_list_sort(List * nm, signed long (*comp)(void *, void *, void *), void * info)
/*
@node afc_list_sort

		 NAME: afc_list_sort(nm, sortingroutine, info) - Sorts the item in the List

			 SYNOPSIS: void * afc_list_sort(List * nm, signed long (*sortingroutine)(void *, void *, void *), void * info)

		DESCRIPTION: Use this method to sort the list.

		INPUT: - nm				- Pointer to a valid List class.
		 - sortingroutine	- You MUST provide a comparison routine,
						 which will be used to sort your list.
						 The comp routine should accept three
						 params: item1, item2 and info.
						 Item1 and item2 are the two items you
						 should compare; while info is an
						 optional param containing to whatever
						 you want.
						 Your comp routine MUST return a value:

						 +	 >1 - Item1 > Item2
						 +	 =0 - Item1 = Item2
						 +	 <0 - Item1 < Item2

					 Then items will be sorted accordingly
					 by the sort method.

		 - info			- This is an optional param
						 that will be passed to your comp routine.
						 It can contain everything you like.

	RESULTS: - The new first object in the list - The list has been sorted correctly

		 - NULL		 - sort failed (maybe list empty)

		NOTES: - After a sort:
			 + Stack will be cleared.
			 + Current item will be the first one.

			 SEE ALSO: - afc_list_create_array()
@endnode
*/
void *afc_list_sort(List *nm, signed long (*comp)(void *, void *, void *), void *info)
{
	if (IsListEmpty(nm->lst))
		return (NULL);
	if (nm->is_sorted)
		return (afc_list_first(nm));

	afc_list_internal_quick_sort(nm, 0, (nm->num - 1), comp, info);
	afc_list_clear_stack(nm);

	nm->is_sorted = TRUE;

	return (afc_list_first(nm));
}
// }}}
// {{{ short afc_list_is_last(List * nm)
/*
@node afc_list_is_last

		 NAME: afc_list_is_last(nm) - Checks if the current item is actually the last one

			 SYNOPSIS: short islast(void)

		DESCRIPTION: This method checks if the current item is the last one.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: - TRUE		- The item is the last.
		 - FALSE	 - The item is not the last.

			 SEE ALSO: - afc_list_is_first()
@endnode
*/
short afc_list_is_last(List *nm)
{
	return ((short)(nm->pos == nm->lst->lh_TailPred));
}
//  }}}
// {{{ short afc_list_is_first(List * nm)
/*
@node afc_list_is_first

		 NAME: afc_list_is_first(nm) - Checks if the current item is actually the first one

			 SYNOPSIS: short afc_list_is_first(List * nm)

		DESCRIPTION: This method checks if the current item is the first one.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: - TRUE		- The item is the first.
		 - FALSE	 - The item is not the first.

			 SEE ALSO: - afc_list_is_last()
@endnode
*/
short afc_list_is_first(List *nm)
{
	return ((short)(nm->pos == nm->lst->lh_Head));
}
// }}}
// {{{ struct Node * * afc_list_create_array(List * nm)
/*
@node afc_list_create_array

		 NAME: afc_list_create_array(nm) - Creates an array rappresentation of the list

			 SYNOPSIS: struct Node * * afc_list_create_array(List * nm)

		DESCRIPTION: This method snapshots the current List contents
		 inside an ARRAY of pointers.
		 This method is usefull if you want to access a large amount
		 of	items data very quicly.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: The generated array.	NULL means something went wrong (maybe you are trying to
		 create an array from an empty List).

		NOTES: - if the Array inside the List has been already
			 generated, then it will be freed and generated again.

		 - for performance reasons, the array will *not* be kept aligned
			 with the list data automatically. So, if you add/del
			 some elements, you'll have to regenerate the array by yourself.

			 SEE ALSO: - afc_list_free_array()
		 - afc_list_clear()
@endnode
*/
struct Node **afc_list_create_array(List *nm)
{
	struct Node *n;
	struct Node **p;

	// unsigned long t=0;

	if (afc_list_is_empty(nm))
		return (NULL);

	if (nm->array)
		afc_free(nm->array);

	nm->array = (struct Node **)afc_malloc((sizeof(struct Node *) * (nm->num + 1)));

	p = nm->array;

	n = nm->lst->lh_Head;

	while (n)
	{
		*(p++) = n;
		// nm->array[t++]	=	n;
		n = n->ln_Succ;
	}

	nm->is_array_valid = TRUE;

	return (nm->array);
}
// }}}
// {{{ void afc_list_free_array(List * nm)
/*
@node afc_list_free_array

		 NAME: afc_list_free_array(nm) - Frees the array rappresentation of the List

			 SYNOPSIS: void afc_list_free_array(List * nm)

		DESCRIPTION: This method will free the memory used by the static array.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: NONE

			 SEE ALSO: - afc_list_create_array()
@endnode
*/
void afc_list_free_array(List *nm)
{
	if (nm->array)
		afc_free(nm->array);

	nm->is_array_valid = FALSE;
	nm->array = NULL;
}
// }}}
// {{{ List * afc_list_clone(List * nm)
/*
@node afc_list_clone

		 NAME: afc_list_clone(nm) - Clones a List list

			 SYNOPSIS: List * afc_list_clone(List * nm)

		DESCRIPTION: This method clones the current List and its contents
		 and returns a new (cloned) List object.

		INPUT: - nm	- Pointer to a valid List class.

	RESULTS: a pointer to a valid List class. May be NULL on errors.

		 NOTE: - The cloned class resulting from this method should be
			 considered a List class in all of it parts.
			 So, you are supposed to delete the class by
			 yourself, when you have finished with it.

		 - The Resource Tracker used by the "original" class is
			 provided to the cloned one during creation.

			 SEE ALSO:
@endnode
*/
List *afc_list_clone(List *nm)
{
	List *nm_new = afc_list_new();
	void *v;

	afc_list_push(nm);
	v = afc_list_first(nm);
	while (v)
	{
		afc_list_add(nm_new, v, AFC_LIST_ADD_TAIL);
		v = afc_list_next(nm);
	}
	afc_list_pop(nm, TRUE);

	return (nm_new);
}
// }}}
// {{{ void * afc_list_fast_sort(List * nm, signed long (*comp)(void *, void *, void *), void * info)
void *afc_list_fast_sort(List *nm, signed long (*comp)(void *, void *, void *), void *info)
{
	if (IsListEmpty(nm->lst))
		return (NULL);
	if (nm->is_sorted)
		return (afc_list_first(nm));

	afc_list_create_array(nm);

	afc_list_internal_fast_quick_sort(nm, 0, (nm->num - 1), comp, info);
	afc_list_clear_stack(nm);

	// afc_list_free_array(nm);

	nm->is_sorted = TRUE;

	return (afc_list_first(nm));
}
// }}}
// {{{ void * afc_list_ultra_sort(List * nm, int (*comp)( const void *, const void * ) )
void *afc_list_ultra_sort(List *nm, int (*comp)(const void *, const void *))
{
	char **mem = (char **)afc_malloc((sizeof(char **)) * (nm->num + 5));
	int c = 0;
	struct Node *n;

	if (IsListEmpty(nm->lst))
		return (NULL);
	if (nm->is_sorted)
		return (afc_list_first(nm));
	if (mem == NULL)
		return (NULL);

	c = 0;
	n = nm->lst->lh_Head;
	while (n)
	{
		mem[c++] = (char *)n->ln_Name;
		n = n->ln_Succ;
	}

	qsort(mem, nm->num, sizeof(char **), comp);

	c = 0;
	n = nm->lst->lh_Head;
	while (n)
	{
		n->ln_Name = mem[c++];
		n = n->ln_Succ;
	}

	afc_free(mem);

	if (nm->is_array_valid == FALSE)
		afc_list_create_array(nm);

	nm->is_sorted = TRUE;
	return (afc_list_first(nm));
}
// }}}
// {{{ long afc_list_for_each(List * nm, long (*funct) (void *, void *), void * info )
long afc_list_for_each(List *nm, long (*funct)(List *nm, void *, void *), void *info)
{
	void *data;
	long result;

	if (IsListEmpty(nm->lst))
		return (AFC_ERR_NO_ERROR);

	data = afc_list_first(nm);

	while (data)
	{
		if ((result = funct(nm, data, info)) != AFC_ERR_NO_ERROR)
			return (result);

		data = afc_list_next(nm);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ int afc_list_set_clear_func ( List * am, int ( * func ) ( void * )  )
/*
@node afc_list_set_clear_func

	   NAME: afc_list_set_clear_func(am, func) - Sets the clear func

   SYNOPSIS: int afc_list_set_clear_func (List * am, int ( *func ) ( void * ) )

	  SINCE: 1.10

DESCRIPTION: Use this command to set a clear function. The function will be called each time an
		 item is being deleted from the list with afc_list_del() or afc_list_clear().
		 To remove this function, pass a NULL value as function pointer.

	  INPUT: - am	- Pointer to a valid List class.
		 - func	- Function to be called in clearing operations.

	RESULTS: AFC_ERR_NO_ERROR

   SEE ALSO: - afc_list_del()
		 - afc_list_clear()
@endnode
*/
int afc_list_set_clear_func(List *nm, int (*func)(void *))
{
	nm->func_clear = func;

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_list_before_first ( nm ) ***********
int afc_list_before_first(List *nm)
{
	nm->before_first = TRUE;

	return (AFC_ERR_NO_ERROR);
}
// }}}

/* ===========================================================================
	INTERNAL FUNCTIONS
=========================================================================== */
// {{{ afc_list_internal_init_list ( nm )
static void afc_list_internal_init_list(List *nm)
{
	nm->lst->lh_Head = (struct Node *)&(nm->lst->lh_Tail);
	nm->lst->lh_Tail = (struct Node *)NULL;
	nm->lst->lh_TailPred = (struct Node *)nm->lst;

	nm->pos = nm->lst->lh_Head;
	nm->num = 0;

	afc_list_clear_stack(nm);

	nm->sposcount = 0;
	nm->npos = -1; /* No Items!! */
}
// }}}
// {{{ afc_list_internal_split ( nm, inf, sup, mid, comp )
static void afc_list_internal_split(List *nm, unsigned long inf, unsigned long sup, signed long *mid, signed long (*comp)(void *, void *, void *), void *info)
{
	void *l, *m;
	short quit = FALSE;

	m = afc_list_item(nm, inf);

	while (!quit)
	{
		l = afc_list_item(nm, sup);

		while ((inf < sup) && ((*comp)(l, m, info) >= 0))
		{
			l = afc_list_prev(nm);
			sup--;
		}

		if (inf < sup)
		{
			afc_list_item(nm, inf);
			nm->pos->ln_Name = (char *)l;
			inf++;
			l = afc_list_next(nm);

			while ((inf < sup) && ((*comp)(l, m, info) <= 0))
			{
				l = afc_list_next(nm);
				inf++;
			}

			if (inf < sup)
			{
				afc_list_item(nm, sup);
				nm->pos->ln_Name = (char *)l;
				sup--;
			}
			else
			{
				quit = TRUE;
			}
		}
		else
		{
			quit = TRUE;
		}

	} /* End while */

	afc_list_item(nm, sup);
	nm->pos->ln_Name = (char *)m;
	*mid = sup;
}
// }}}

static void afc_list_internal_fast_split(List *nm, unsigned long inf, unsigned long sup, signed long *mid, signed long (*comp)(void *, void *, void *), void *info)
{
	void *l, *m;
	short quit = FALSE;
	struct Node *node1, *node2;

	// m = afc_list_internal_fast_item(nm, inf);
	m = (nm->array[inf])->ln_Name;

	while (!quit)
	{
		//			l = afc_list_internal_fast_item(nm, sup);
		l = (node1 = nm->array[sup])->ln_Name;

		// OPT2:		 while ((inf < sup) && ((*comp)(l,m,info) >= 0))
		while ((inf < sup) && ((*comp)(node1->ln_Name, m, info) >= 0))
			// OPT3:			{
			//	 l = afc_list_internal_fast_item(nm, --sup);
			// OPT2:	l = ( node1=nm->array[--sup])->ln_Name;
			node1 = nm->array[--sup];
		// OPT3:			}

		// OPT2: ONLY
		l = node1->ln_Name;

		if (inf < sup)
		{
			// afc_list_internal_fast_item(nm, inf);
			// OPT3:	 node2 = nm->array[inf];
			// nm->pos->ln_Name = (char *)l;
			// OPT3:	 node2->ln_Name = l;

			// OPT3: ONLY
			nm->array[inf]->ln_Name = l;

			// l = afc_list_internal_fast_item(nm, ++inf);
			l = (node2 = nm->array[++inf])->ln_Name;

			// OPT2:	 while ((inf < sup) && ((*comp)(l,m,info) <= 0))
			while ((inf < sup) && ((*comp)(node2->ln_Name, m, info) <= 0))
				// OPT3:	 {
				// l = afc_list_internal_fast_item(nm, ++inf);
				// OPT2:		 l = ( node2=nm->array[++inf] )->ln_Name;
				node2 = nm->array[++inf];
			// OPT3:	 }

			// OPT2: ONLY
			l = node2->ln_Name;

			if (inf < sup)
			{
				// afc_list_internal_fast_item(nm, sup--);
				node1 = nm->array[sup--];

				// nm->pos->ln_Name = (char *)l;
				node1->ln_Name = l;
			}
			else
				quit = TRUE;
		}
		else
			quit = TRUE;

	} /* End while */

	node1 = nm->array[sup]; // afc_list_internal_fast_item(nm, sup);
	// nm->pos->ln_Name = (char *)m;
	node1->ln_Name = m;

	*mid = sup;
}

static void afc_list_internal_quick_sort(List *nm, unsigned long inf, unsigned long sup, signed long (*comp)(void *, void *, void *), void *info)
{
	signed long mid = 0;

	if (inf < sup)
	{
		afc_list_internal_split(nm, inf, sup, &mid, comp, info);
		if ((signed long)inf < (mid - 1))
			afc_list_internal_quick_sort(nm, inf, mid - 1, comp, info);
		if ((mid + 1) < (signed long)sup)
			afc_list_internal_quick_sort(nm, mid + 1, sup, comp, info);
	}
}

static void afc_list_internal_fast_quick_sort(List *nm, unsigned long inf, unsigned long sup, signed long (*comp)(void *, void *, void *), void *info)
{
	signed long mid = 0;

	if (inf < sup)
	{
		afc_list_internal_fast_split(nm, inf, sup, &mid, comp, info);
		if ((signed long)inf < (mid - 1))
			afc_list_internal_fast_quick_sort(nm, inf, mid - 1, comp, info);
		if ((mid + 1) < (signed long)sup)
			afc_list_internal_fast_quick_sort(nm, mid + 1, sup, comp, info);
	}
}

/*
static int afc_list_internal_ultra_comp ( List * nm, const void * s1, const void * s2 )
{
	char * n1 = ( char * ) ( * ( char * *) s1 );
	char * n2 = ( char * ) ( * ( char * *) s2 );

	return ( (strcmp ( n1, n2 ) ) ) ;
}
*/

#ifdef TEST_NODEMASTER
signed long comp(void *s1, void *s2, void *info)
{
	return (strcmp((char *)s1, (char *)s2));
}

int comp_ultra(const void *s1, const void *s2)
{
	char *n1 = (char *)(*(char **)s1);
	char *n2 = (char *)(*(char **)s2);

	return ((strcmp(n1, n2)));
}

void additem(List *nm, int t)
{
	char *s = (char *)afc_malloc(10);

	sprintf(s, "%10.10d", t);
	afc_list_add(nm, s, AFC_LIST_ADD_TAIL);
}

int main()
{
	List *nm, *nm_new;
	char *s;
	int t;
	struct Node *n;

	nm = afc_list_new();

	printf("Adding...\n");

	for (t = 3000000; t > 0; t--)
		additem(nm, t);

	// afc_list_create_array(nm);

	printf("Sorting...\n");
	afc_list_ultra_sort(nm, comp_ultra);

	printf("Done :-)\n");

	// afc_list_free_array ( nm );
	/*
		s = afc_list_first(nm);
		while(s)
		{
			//printf("Str: %s\n", s);
			//s = afc_list_next(nm);
		}

		afc_list_delete(nm);
	*/

	exit(0);
}
#endif
