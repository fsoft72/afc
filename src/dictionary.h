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
#ifndef AFC_DICTIONARY_H
#define AFC_DICTIONARY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "base.h"
#include "string.h"
#include "array.h"
#include "hash_master.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* AFC afc_dictionary Magic Number */
#define AFC_DICTIONARY_MAGIC ('D' << 24 | 'I' << 16 | 'C' << 8 | 'T')

/* AFC afc_dictionary Base value for constants */
#define AFC_DICTIONARY_BASE 0xa000

	/* Errors for afc_dictionary */
	enum
	{
		AFC_DICTIONARY_ERR_HASHING = AFC_DICTIONARY_BASE + 1, /* Ths HashMaster class reported an error  */
		AFC_DICTIONARY_ERR_NOT_FOUND
	}; /* Requesteq key cannot be found           */

	struct afc_dictionary_internal_data
	{
		char *key;
		void *value;
	};

	struct afc_dictionary
	{
		unsigned long magic;

		HashMaster *hash;

		struct afc_dictionary_internal_data *curr_data;

		int (*func_clear)(void *);

		BOOL skip_find;
	};

	typedef struct afc_dictionary Dictionary;

	typedef struct afc_dictionary_internal_data DictionaryData;

#define afc_dictionary_delete(dict)   \
	if (dict)                         \
	{                                 \
		_afc_dictionary_delete(dict); \
		dict = NULL;                  \
	}

	struct afc_dictionary *afc_dictionary_new(void);
	int _afc_dictionary_delete(struct afc_dictionary *);
	int afc_dictionary_clear(struct afc_dictionary *);

	int afc_dictionary_set(Dictionary *, const char *, void *);
	void *afc_dictionary_get(Dictionary *, const char *);
	void *afc_dictionary_get_default(Dictionary *, const char *, void *def_val);
	void *afc_dictionary_first(Dictionary *);
#define afc_dictionary_succ(d) afc_dictionary_next(d)
	void *afc_dictionary_next(Dictionary *);
	void *afc_dictionary_prev(Dictionary *);
	BOOL afc_dictionary_has_key(Dictionary *, const char *);
	void *afc_dictionary_del(Dictionary *);
	int afc_dictionary_del_item(Dictionary *dict, const char *key);
	char *afc_dictionary_find_key(Dictionary *dict, void *data);
#define afc_dictionary_num_items(d) (d ? afc_array_len(d->hash->am) : 0)
#define afc_dictionary_len(d) (d ? afc_array_len(d->hash->am) : 0)
	int afc_dictionary_for_each(Dictionary *dict, int (*func)(Dictionary *am, int pos, void *v, void *info), void *info);
#define afc_dictionary_set_custom_sort(d, func) d->hash->am->custom_sort = func
#define afc_dictionary_before_first(d) (d ? afc_array_before_first(d->hash->am) : AFC_ERR_NULL_POINTER)
#define afc_dictionary_obj(d) (d ? d->curr_data->value : AFC_ERR_NULL_POINTER)
#define afc_dictionary_get_key(d) (char *)(d ? d->curr_data->key : NULL)
#define afc_dictionary_set_clear_func(d, func) \
	if (d)                                     \
	{                                          \
		d->func_clear = func;                  \
	}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
