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
#ifndef AFC_NODEMASTER_H
#define AFC_NODEMASTER_H

/*  NodeMaster.h  $ 19/05/97 FR MT $  */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "base.h"
#include "string.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* AFC NodeMaster 'Magic' value: 'NODE' */
#define AFC_NODEMASTER_MAGIC ('N' << 24 | 'O' << 16 | 'D' << 8 | 'E')

/* AFC NodeMaster Base                  */
#define AFC_NODEMASTER_BASE 0x1000

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
		AFC_NODEMASTER_ADD_HEAD = AFC_NODEMASTER_BASE + 1,
		AFC_NODEMASTER_ADD_HERE,
		AFC_NODEMASTER_ADD_TAIL
	};

	struct afc_nodemaster
	{
		unsigned long magic; /* NodeMaster Magic value                         */

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

	typedef struct afc_nodemaster NodeMaster;

#define afc_nodemaster_new() _afc_nodemaster_new(__FILE__, __FUNCTION__, __LINE__)
#define afc_nodemaster_delete(nm)   \
	if (nm)                         \
	{                               \
		_afc_nodemaster_delete(nm); \
		nm = NULL;                  \
	}

	NodeMaster *_afc_nodemaster_new(const char *file, const char *func, const unsigned int line);

	int _afc_nodemaster_delete(NodeMaster *);
	void *afc_nodemaster_add(NodeMaster *, void *, unsigned long);
	short afc_nodemaster_is_empty(NodeMaster *);
	void *afc_nodemaster_first(NodeMaster *);
	struct Node *afc_nodemaster_get(NodeMaster *);
	struct List *afc_nodemaster_addr(NodeMaster *);
	short afc_nodemaster_push(NodeMaster *);
	void *afc_nodemaster_pop(NodeMaster *, short);
	void *afc_nodemaster_obj(NodeMaster *);
	void *afc_nodemaster_del(NodeMaster *);
	void afc_nodemaster_clear_stack(NodeMaster *);
	void *afc_nodemaster_last(NodeMaster *);
	void *afc_nodemaster_next(NodeMaster *);
	void *afc_nodemaster_prev(NodeMaster *);
	void *afc_nodemaster_insert(NodeMaster *, void *);
	int afc_nodemaster_clear(NodeMaster *);
#define afc_nodemaster_succ(nm) afc_nodemaster_next(nm)
#define afc_nodemaster_len(nm) (nm ? nm->num : 0)
#define afc_nodemaster_num_items(nm) (nm ? nm->num : 0)
#define afc_nodemaster_pos(nm) (nm ? nm->npos : 0)
#define afc_nodemaster_numerical_pos(nm) (nm ? nm->npos : 0)
#define afc_nodemaster_add_tail(nm, itm) afc_nodemaster_add(nm, itm, AFC_NODEMASTER_ADD_TAIL)
#define afc_nodemaster_add_head(nm, itm) afc_nodemaster_add(nm, itm, AFC_NODEMASTER_ADD_HEAD)
	// unsigned long  afc_nodemaster_num_items(NodeMaster * );
	// unsigned long  afc_nodemaster_pos(NodeMaster * );
	void *afc_nodemaster_item(NodeMaster *, unsigned long);
	void *afc_nodemaster_change(NodeMaster *, void *);
	void *afc_nodemaster_change_pos(NodeMaster *, struct Node *);
	void afc_nodemaster_change_numerical_pos(NodeMaster *, unsigned long);
	void *afc_nodemaster_sort(NodeMaster *, long (*comp)(void *, void *, void *), void *);
	short afc_nodemaster_is_last(NodeMaster *);
	short afc_nodemaster_is_first(NodeMaster *);
	struct Node **afc_nodemaster_create_array(NodeMaster *);
	void afc_nodemaster_free_array(NodeMaster *);

	void *afc_nodemaster_fast_sort(NodeMaster *nm, long (*comp)(void *, void *, void *), void *);
	void *afc_nodemaster_ultra_sort(NodeMaster *nm, int (*comp)(const void *, const void *));
	long afc_nodemaster_for_each(NodeMaster *nm, long (*funct)(NodeMaster *nm, void *, void *), void *);
	int afc_nodemaster_before_first(NodeMaster *nm);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
