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
#ifndef AFC_HASH_H
#define AFC_HASH_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "base.h"
#include "array.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* AFC afc_hash Magic Number */
#define AFC_HASH_MAGIC ('H' << 24 | 'A' << 16 | 'S' << 8 | 'H')

/* AFC afc_hash Base value for constants */
#define AFC_HASH_BASE 0x7000

	struct hash_internal_data
	{
		unsigned long int hash_value;
		void *data;
	};

	typedef struct hash_internal_data HashData;

	struct afc_hash
	{
		unsigned long magic;

		Array *am;

		int (*func_clear)(struct afc_hash *, void *);

		void *info; // Generic Info pointer
	};

	typedef struct afc_hash Hash;

#define afc_hash_delete(hm)   \
	if (hm)                   \
	{                         \
		_afc_hash_delete(hm); \
		hm = NULL;            \
	}

	struct afc_hash *afc_hash_new(void);
	int _afc_hash_delete(struct afc_hash *);
	int afc_hash_clear(struct afc_hash *);
	int afc_hash_add(Hash *, unsigned long int, void *);
	void *afc_hash_find(Hash *, unsigned long int);
	void *afc_hash_del(Hash *);
	HashData *afc_hash_item(Hash *, int);
	void *afc_hash_first(Hash *hm);
	void *afc_hash_next(Hash *hm);
	void *afc_hash_last(Hash *hm);
	void *afc_hash_prev(Hash *hm);
	int afc_hash_for_each(Hash *hm, int (*func)(Hash *hm, int pos, void *v, void *info), void *info);

#define afc_hash_succ(hm) afc_hash_next(hm)
#define afc_hash_set_clear_func(hm, func) \
	if (hm)                               \
	{                                     \
		hm->func_clear = func;            \
	}
#define afc_hash_set_custom_sort(hm, func) \
	if ((hm) && (hm->am))                  \
	{                                      \
		hm->am->custom_sort = func;        \
	}
#define afc_hash_before_first(hm) (hm ? afc_array_before_first(hm->am) : AFC_ERR_NULL_POINTER)
#define afc_hash_is_empty(hm) (hm ? afc_array_is_empty(hm->am) : TRUE)
#define afc_hash_is_first(hm) (hm ? afc_array_is_first(hm->am) : FALSE)
#define afc_hash_is_last(hm) (hm ? afc_array_is_last(hm->am) : FALSE)
#define afc_hash_len(hm) (hm ? afc_array_len(hm->am) : -1)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
