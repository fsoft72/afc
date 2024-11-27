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
#ifndef AFC_STRING_LIST_H
#define AFC_STRING_LIST_H

/*  StringList.h  $ 19/05/97 FR MT $  */

/*

		$VER: StringList 1.01

		1.01 - ADD: clone() method

		1.02 - ADD: escape_char support in split()

*/

#include <stdio.h>
#include "string.h"

#include "base.h"
#include "list.h"

#ifdef MINGW
#define index strchr
#endif

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	/* Version and revision */

#define STRING_LIST_VERSION 1
#define STRING_LIST_REVISION 2

#define AFC_STRING_LIST_ADD_HEAD AFC_LIST_ADD_HEAD
#define AFC_STRING_LIST_ADD_TAIL AFC_LIST_ADD_TAIL
#define AFC_STRING_LIST_ADD_HERE AFC_LIST_ADD_HERE

	/* Error codes */

#define AFC_STRING_LIST_BASE 0x2000

	enum
	{
		AFC_STRING_LIST_ERR_CHANGE = AFC_STRING_LIST_BASE + 1,
		AFC_STRING_LIST_ERR_NULL_STRING,
		AFC_STRING_LIST_ERR_NULL_DELIMITERS
	};

	enum
	{
		AFC_STRING_LIST_TAG_DISCARD_ZERO_LEN = AFC_STRING_LIST_BASE + 1,
		AFC_STRING_LIST_TAG_ESCAPE_CHAR
	};

/* AFC StringList Magic value: STRN */
#define AFC_STRING_LIST_MAGIC ('S' << 24 | 'T' << 16 | 'R' << 8 | 'N')

	struct afc_string_list
	{
		unsigned long magic; /* AFC Magic Number */

		List *nm; /* Pointer to our super-class */

		// INTERNAL STUFF

		short discard_zero_len; // Flag T/F. If T StringList will not accept (using _add()) zero lenght strings
		char escape_char;		// Escape character (used for the _split() method)
	};

	typedef struct afc_string_list StringList;

#define afc_string_list_new() _afc_string_list_new(__FILE__, __FUNCTION__, __LINE__)
#define afc_string_list_delete(sn)   \
	if (sn)                          \
	{                                \
		_afc_string_list_delete(sn); \
		sn = NULL;                   \
	}

	StringList *_afc_string_list_new(const char *file, const char *func, const unsigned int line);
	int _afc_string_list_delete(StringList *);
	char *afc_string_list_add(StringList *, const char *, unsigned long);
#define afc_string_list_insert(sn, txt) afc_string_list_add(sn, txt, AFC_STRING_LIST_ADD_HERE)
#define afc_string_list_obj(sn) (char *)(sn ? afc_list_obj(sn->nm) : NULL)
#define afc_string_list_is_empty(sn) (BOOL) afc_list_is_empty(sn->nm)
#define afc_string_list_first(sn) (char *)afc_list_first(sn->nm)
#define afc_string_list_next(sn) (char *)afc_list_next(sn->nm)
#define afc_string_list_succ(sn) (char *)afc_list_next(sn->nm)
#define afc_string_list_prev(sn) (char *)afc_list_prev(sn->nm)
#define afc_string_list_last(sn) (char *)afc_list_last(sn->nm)
#define afc_string_list_item(sn, n) (char *)afc_list_item(sn->nm, n)
#define afc_string_list_get(sn) (struct Node *)(sn ? afc_list_get(sn->nm) : NULL)
#define afc_string_list_addr(sn) (struct List *)(sn ? afc_list_addr(sn->nm) : NULL)
#define afc_string_list_push(sn) (BOOL)(sn ? afc_list_push(sn->nm) : FALSE)
#define afc_string_list_pop(sn, apos) (char *)(sn ? afc_list_pop(sn->nm, apos) : NULL)
#define afc_string_list_clear_stack(sn) afc_list_clear_stack(sn->nm)
#define afc_string_list_len(sn) (sn ? afc_list_len(sn->nm) : 0)
#define afc_string_list_num_items(sn) (sn ? afc_list_len(sn->nm) : 0)
#define afc_string_list_pos(sn) (sn ? afc_list_pos(sn->nm) : 0)
#define afc_string_list_before_first(sn) (sn ? afc_list_before_first(sn->nm) : AFC_ERR_NULL_POINTER)

#define afc_string_list_add_tail(sn, s) afc_string_list_add(sn, s, AFC_STRING_LIST_ADD_TAIL)
#define afc_string_list_add_head(sn, s) afc_string_list_add(sn, s, AFC_STRING_LIST_ADD_HEAD)
	char *afc_string_list_del(StringList *);
	int afc_string_list_clear(StringList *);
	int afc_string_list_change(StringList *, char *);
	int afc_string_list_sort(StringList *, short, short, short);
#ifndef MINGW
	char *afc_string_list_search(StringList *, char *, short, short);
#endif
	StringList *afc_string_list_clone(StringList *);
	int afc_string_list_split(StringList *sn, const char *string, const char *delimiters);
#define afc_string_list_set_tags(sn, first, ...) _afc_string_list_set_tags(sn, first, ##__VA_ARGS__, AFC_TAG_END)
	int _afc_string_list_set_tags(StringList *sn, int first_tag, ...);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
