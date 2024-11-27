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
#ifndef AFC_LIST_H
#define AFC_LIST_H

/*  List.h  $ 19/05/97 FR MT $  */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "base.h"
#include "string.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* AFC List 'Magic' value: 'NODE' */
#define AFC_LIST_MAGIC ('N' << 24 | 'O' << 16 | 'D' << 8 | 'E')

/* AFC List Base                  */
#define AFC_LIST_BASE 0x1000

	struct Node
	{
		struct Node *ln_Succ,
			*ln_Pred;

		char *ln_Name;
		//  unsigned char ln_Type;
		//  signed char   ln_Pri;
	};

	struct List
	{
		struct Node *lh_Head,
			*lh_Tail,
			*lh_TailPred;
	};

#define IsListEmpty(l) \
	((((struct List *)l)->lh_TailPred) == (struct Node *)(l))

	/* Insertion modes */

	enum
	{
		AFC_LIST_ADD_HEAD = AFC_LIST_BASE + 1,
		AFC_LIST_ADD_HERE,
		AFC_LIST_ADD_TAIL
	};

	struct afc_list
	{
		unsigned long magic; /* List Magic value                         */

		struct List *lst;	/* List Header                                    */
		struct Node *pos;	/* Actual Node                                    */
		unsigned long num;	/* Number of items linked to the list             */
		unsigned long npos; /* Ordinal position (0 = First Item, 1 Second...) */

		struct Node *spos[8];	 /* 8 Levels Stack                                 */
		unsigned char sposcount; /* Stack Counter                                  */
		unsigned long errorcode; /* Last error code                                */

		struct Node **array; /* Array of Node Master elements                  */

		BOOL is_sorted;		 /* it is TRUE if no modifications have been made since the last sort() */
		BOOL is_array_valid; /* It is TRUE if no modifications have been made since the last get_array() */
		BOOL before_first;

		int (*func_clear)(void *);
	};

	typedef struct afc_list List;

#define afc_list_new() _afc_list_new(__FILE__, __FUNCTION__, __LINE__)
#define afc_list_delete(nm)   \
	if (nm)                   \
	{                         \
		_afc_list_delete(nm); \
		nm = NULL;            \
	}

	List *_afc_list_new(const char *file, const char *func, const unsigned int line);

	int _afc_list_delete(List *);
	void *afc_list_add(List *, void *, unsigned long);
	short afc_list_is_empty(List *);
	void *afc_list_first(List *);
	struct Node *afc_list_get(List *);
	struct List *afc_list_addr(List *);
	short afc_list_push(List *);
	void *afc_list_pop(List *, short);
	void *afc_list_obj(List *);
	void *afc_list_del(List *);
	void afc_list_clear_stack(List *);
	void *afc_list_last(List *);
	void *afc_list_next(List *);
	void *afc_list_prev(List *);
	void *afc_list_insert(List *, void *);
	int afc_list_clear(List *);
#define afc_list_succ(nm) afc_list_next(nm)
#define afc_list_len(nm) (nm ? nm->num : 0)
#define afc_list_num_items(nm) (nm ? nm->num : 0)
#define afc_list_pos(nm) (nm ? nm->npos : 0)
#define afc_list_numerical_pos(nm) (nm ? nm->npos : 0)
#define afc_list_add_tail(nm, itm) afc_list_add(nm, itm, AFC_LIST_ADD_TAIL)
#define afc_list_add_head(nm, itm) afc_list_add(nm, itm, AFC_LIST_ADD_HEAD)
	// unsigned long  afc_list_num_items(List * );
	// unsigned long  afc_list_pos(List * );
	void *afc_list_item(List *, unsigned long);
	void *afc_list_change(List *, void *);
	void *afc_list_change_pos(List *, struct Node *);
	void afc_list_change_numerical_pos(List *, unsigned long);
	void *afc_list_sort(List *, long (*comp)(void *, void *, void *), void *);
	short afc_list_is_last(List *);
	short afc_list_is_first(List *);
	struct Node **afc_list_create_array(List *);
	void afc_list_free_array(List *);

	void *afc_list_fast_sort(List *nm, long (*comp)(void *, void *, void *), void *);
	void *afc_list_ultra_sort(List *nm, int (*comp)(const void *, const void *));
	long afc_list_for_each(List *nm, long (*funct)(List *nm, void *, void *), void *);
	int afc_list_before_first(List *nm);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
