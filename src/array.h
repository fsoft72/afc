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
#ifndef AFC_ARRAY_H
#define AFC_ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "base.h"
#include "string.h"
#include "exceptions.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* AFC afc_array Magic Number */
#define AFC_ARRAY_MAGIC ('A' << 24 | 'R' << 16 | 'R' << 8 | 'A')

/* AFC afc_array Base value for constants */
#define AFC_ARRAY_BASE 0x8000

	/* Errors for afc_array */

#define AFC_ARRAY_DEFAULT_ITEMS 100

	enum
	{
		AFC_ARRAY_ADD_HERE = AFC_ARRAY_BASE + 1,
		AFC_ARRAY_ADD_TAIL,
		AFC_ARRAY_ADD_HEAD
	};

#define AFC_ARRAY_SORT_ELEMENT(kind, varname) (kind)(*(kind *)varname)

	struct afc_array
	{
		unsigned long magic;

		void **mem;

		unsigned long int max_items;
		unsigned long int current_pos;
		unsigned long int num_items;

		BOOL is_sorted;
		BOOL before_first;

		int (*func_clear)(void *);
		void (*custom_sort)(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
	};

	typedef struct afc_array Array;

#define afc_array_new(am) _afc_array_new(__FILE__, __FUNCTION__, __LINE__)
#define afc_array_delete(am)   \
	if (am)                    \
	{                          \
		_afc_array_delete(am); \
		am = NULL;             \
	}

	struct afc_array *_afc_array_new(const char *file, const char *func, const unsigned int line);
	int _afc_array_delete(struct afc_array *);
	int afc_array_clear(struct afc_array *);
	int afc_array_init(Array *, unsigned long);
	int afc_array_add(Array *, void *, int);
	void *afc_array_item(Array *, unsigned long);
	void *afc_array_first(Array *);
	void *afc_array_next(Array *);
	void *afc_array_prev(Array *);
	void *afc_array_last(Array *);
	void *afc_array_obj(Array *);
	short afc_array_is_first(Array *);
	short afc_array_is_last(Array *);
	short afc_array_is_empty(Array *);
	void *afc_array_del(Array *);
	void *afc_array_sort(Array *am, int (*comp)(const void *, const void *));
// unsigned long int afc_array_current_pos ( Array *  );
#define afc_array_pos(am) (am ? am->current_pos : 0)
#define afc_array_current_pos(am) (am ? am->current_pos : 0)
#define afc_array_succ(am) afc_array_next(am)
#define afc_array_num_items(am) afc_array_len(am)
#define afc_array_add_tail(am, itm) afc_array_add(am, itm, AFC_ARRAY_ADD_TAIL)
#define afc_array_add_head(am, itm) afc_array_add(am, itm, AFC_ARRAY_ADD_HEAD)
#define afc_array_insert(am, itm) afc_array_add(am, itm, AFC_ARRAY_ADD_HERE)
	unsigned long int afc_array_len(Array *);
	int afc_array_set_clear_func(Array *am, int (*func)(void *));
	int afc_array_for_each(Array *am, int (*func)(Array *am, int pos, void *v, void *info), void *info);
	int afc_array_set_custom_sort(Array *am, void (*func)(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)));
	int afc_array_before_first(Array *am);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
