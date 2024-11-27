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
#ifndef AFC_HASH_MASTER_H
#define AFC_HASH_MASTER_H
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

/* AFC afc_hash_master Magic Number */
#define AFC_HASH_MASTER_MAGIC ('H' << 24 | 'A' << 16 | 'S' << 8 | 'H')

/* AFC afc_hash_master Base value for constants */
#define AFC_HASH_MASTER_BASE 0x7000

	struct hash_master_internal_data
	{
		unsigned long int hash_value;
		void *data;
	};

	typedef struct hash_master_internal_data HashData;

	struct afc_hash_master
	{
		unsigned long magic;

		ArrayMaster *am;

		int (*func_clear)(struct afc_hash_master *, void *);

		void *info; // Generic Info pointer
	};

	typedef struct afc_hash_master HashMaster;

#define afc_hash_master_delete(hm)   \
	if (hm)                          \
	{                                \
		_afc_hash_master_delete(hm); \
		hm = NULL;                   \
	}

	struct afc_hash_master *afc_hash_master_new(void);
	int _afc_hash_master_delete(struct afc_hash_master *);
	int afc_hash_master_clear(struct afc_hash_master *);
	int afc_hash_master_add(HashMaster *, unsigned long int, void *);
	void *afc_hash_master_find(HashMaster *, unsigned long int);
	void *afc_hash_master_del(HashMaster *);
	HashData *afc_hash_master_item(HashMaster *, int);
	void *afc_hash_master_first(HashMaster *hm);
	void *afc_hash_master_next(HashMaster *hm);
	void *afc_hash_master_last(HashMaster *hm);
	void *afc_hash_master_prev(HashMaster *hm);
	int afc_hash_master_for_each(HashMaster *hm, int (*func)(HashMaster *hm, int pos, void *v, void *info), void *info);

#define afc_hash_master_succ(hm) afc_hash_master_next(hm)
#define afc_hash_master_set_clear_func(hm, func) \
	if (hm)                                      \
	{                                            \
		hm->func_clear = func;                   \
	}
#define afc_hash_master_set_custom_sort(hm, func) \
	if ((hm) && (hm->am))                         \
	{                                             \
		hm->am->custom_sort = func;               \
	}
#define afc_hash_master_before_first(hm) (hm ? afc_array_before_first(hm->am) : AFC_ERR_NULL_POINTER)
#define afc_hash_master_is_empty(hm) (hm ? afc_array_is_empty(hm->am) : TRUE)
#define afc_hash_master_is_first(hm) (hm ? afc_array_is_first(hm->am) : FALSE)
#define afc_hash_master_is_last(hm) (hm ? afc_array_is_last(hm->am) : FALSE)
#define afc_hash_master_len(hm) (hm ? afc_array_len(hm->am) : -1)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
