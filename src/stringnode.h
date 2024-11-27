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
#ifndef AFC_STRINGNODE_H
#define AFC_STRINGNODE_H

/*  StringNode.h  $ 19/05/97 FR MT $  */

/*

		$VER: StringNode 1.01

		1.01 - ADD: clone() method

		1.02 - ADD: escape_char support in split()

*/

#include <stdio.h>
#include "string.h"

#include "base.h"
#include "nodemaster.h"

#ifdef MINGW
#define index strchr
#endif

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	/* Version and revision */

#define STRINGNODE_VERSION 1
#define STRINGNODE_REVISION 2

#define AFC_STRINGNODE_ADD_HEAD AFC_NODEMASTER_ADD_HEAD
#define AFC_STRINGNODE_ADD_TAIL AFC_NODEMASTER_ADD_TAIL
#define AFC_STRINGNODE_ADD_HERE AFC_NODEMASTER_ADD_HERE

	/* Error codes */

#define AFC_STRINGNODE_BASE 0x2000

	enum
	{
		AFC_STRINGNODE_ERR_CHANGE = AFC_STRINGNODE_BASE + 1,
		AFC_STRINGNODE_ERR_NULL_STRING,
		AFC_STRINGNODE_ERR_NULL_DELIMITERS
	};

	enum
	{
		AFC_STRINGNODE_TAG_DISCARD_ZERO_LEN = AFC_STRINGNODE_BASE + 1,
		AFC_STRINGNODE_TAG_ESCAPE_CHAR
	};

/* AFC StringNode Magic value: STRN */
#define AFC_STRINGNODE_MAGIC ('S' << 24 | 'T' << 16 | 'R' << 8 | 'N')

	struct afc_stringnode
	{
		unsigned long magic; /* AFC Magic Number */

		NodeMaster *nm; /* Pointer to our super-class */

		// INTERNAL STUFF

		short discard_zero_len; // Flag T/F. If T StringNode will not accept (using _add()) zero lenght strings
		char escape_char;		// Escape character (used for the _split() method)
	};

	typedef struct afc_stringnode StringNode;

#define afc_stringnode_new() _afc_stringnode_new(__FILE__, __FUNCTION__, __LINE__)
#define afc_stringnode_delete(sn)   \
	if (sn)                         \
	{                               \
		_afc_stringnode_delete(sn); \
		sn = NULL;                  \
	}

	StringNode *_afc_stringnode_new(const char *file, const char *func, const unsigned int line);
	int _afc_stringnode_delete(StringNode *);
	char *afc_stringnode_add(StringNode *, const char *, unsigned long);
#define afc_stringnode_insert(sn, txt) afc_stringnode_add(sn, txt, AFC_STRINGNODE_ADD_HERE)
#define afc_stringnode_obj(sn) (char *)(sn ? afc_nodemaster_obj(sn->nm) : NULL)
#define afc_stringnode_is_empty(sn) (BOOL) afc_nodemaster_is_empty(sn->nm)
#define afc_stringnode_first(sn) (char *)afc_nodemaster_first(sn->nm)
#define afc_stringnode_next(sn) (char *)afc_nodemaster_next(sn->nm)
#define afc_stringnode_succ(sn) (char *)afc_nodemaster_next(sn->nm)
#define afc_stringnode_prev(sn) (char *)afc_nodemaster_prev(sn->nm)
#define afc_stringnode_last(sn) (char *)afc_nodemaster_last(sn->nm)
#define afc_stringnode_item(sn, n) (char *)afc_nodemaster_item(sn->nm, n)
#define afc_stringnode_get(sn) (struct Node *)(sn ? afc_nodemaster_get(sn->nm) : NULL)
#define afc_stringnode_addr(sn) (struct List *)(sn ? afc_nodemaster_addr(sn->nm) : NULL)
#define afc_stringnode_push(sn) (BOOL)(sn ? afc_nodemaster_push(sn->nm) : FALSE)
#define afc_stringnode_pop(sn, apos) (char *)(sn ? afc_nodemaster_pop(sn->nm, apos) : NULL)
#define afc_stringnode_clear_stack(sn) afc_nodemaster_clear_stack(sn->nm)
#define afc_stringnode_len(sn) (sn ? afc_nodemaster_len(sn->nm) : 0)
#define afc_stringnode_num_items(sn) (sn ? afc_nodemaster_len(sn->nm) : 0)
#define afc_stringnode_pos(sn) (sn ? afc_nodemaster_pos(sn->nm) : 0)
#define afc_stringnode_before_first(sn) (sn ? afc_nodemaster_before_first(sn->nm) : AFC_ERR_NULL_POINTER)

#define afc_stringnode_add_tail(sn, s) afc_stringnode_add(sn, s, AFC_STRINGNODE_ADD_TAIL)
#define afc_stringnode_add_head(sn, s) afc_stringnode_add(sn, s, AFC_STRINGNODE_ADD_HEAD)
	char *afc_stringnode_del(StringNode *);
	int afc_stringnode_clear(StringNode *);
	int afc_stringnode_change(StringNode *, char *);
	int afc_stringnode_sort(StringNode *, short, short, short);
#ifndef MINGW
	char *afc_stringnode_search(StringNode *, char *, short, short);
#endif
	StringNode *afc_stringnode_clone(StringNode *);
	int afc_stringnode_split(StringNode *sn, const char *string, const char *delimiters);
#define afc_stringnode_set_tags(sn, first, ...) _afc_stringnode_set_tags(sn, first, ##__VA_ARGS__, AFC_TAG_END)
	int _afc_stringnode_set_tags(StringNode *sn, int first_tag, ...);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
