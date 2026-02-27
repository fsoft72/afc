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

/*
 * test_hash.c - Comprehensive tests for the AFC Hash module.
 *
 * Tests cover creation, adding items with numeric hash keys,
 * finding existing and non-existing keys, traversal (first/next),
 * deletion, length, emptiness, and clearing.
 *
 * NOTE: Hash keys are unsigned long integers, not strings.
 * The hash table is always kept sorted by hash_value internally.
 */

#include "test_utils.h"
#include "../src/hash.h"

/** Returns the hash length as a long, avoiding sign-compare warning in afc_hash_len macro. */
static long _hash_len(Hash *hm)
{
	if (!hm) return -1;
	return (long)afc_array_len(hm->am);
}

/* Hash key constants for test clarity */
#define HASH_KEY_ALPHA   100
#define HASH_KEY_BETA    200
#define HASH_KEY_GAMMA   300
#define HASH_KEY_DELTA   400
#define HASH_KEY_MISSING 999

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ----------------------------------------------------------------
	 * 1. Creation and initial state
	 * ---------------------------------------------------------------- */
	Hash *hm = afc_hash_new();
	print_res("hash_new != NULL", (void *)(long)1, (void *)(long)(hm != NULL), 0);
	print_res("is_empty after new", (void *)(long)1, (void *)(long)afc_hash_is_empty(hm), 0);
	print_res("len after new", (void *)(long)0, (void *)(long)_hash_len(hm), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 2. afc_hash_add() multiple items
	 * ---------------------------------------------------------------- */
	int res;
	res = afc_hash_add(hm, HASH_KEY_ALPHA, "alpha");
	print_res("add alpha OK", (void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	res = afc_hash_add(hm, HASH_KEY_GAMMA, "gamma");
	print_res("add gamma OK", (void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	res = afc_hash_add(hm, HASH_KEY_BETA, "beta");
	print_res("add beta OK", (void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	print_res("len after 3 adds", (void *)(long)3, (void *)(long)_hash_len(hm), 0);
	print_res("is_empty false", (void *)(long)0, (void *)(long)afc_hash_is_empty(hm), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 3. afc_hash_find() - existing keys
	 * ---------------------------------------------------------------- */
	char *s;
	s = (char *)afc_hash_find(hm, HASH_KEY_ALPHA);
	print_res("find alpha", "alpha", s, 1);

	s = (char *)afc_hash_find(hm, HASH_KEY_BETA);
	print_res("find beta", "beta", s, 1);

	s = (char *)afc_hash_find(hm, HASH_KEY_GAMMA);
	print_res("find gamma", "gamma", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 4. afc_hash_find() - non-existing key
	 * ---------------------------------------------------------------- */
	s = (char *)afc_hash_find(hm, HASH_KEY_MISSING);
	print_res("find missing == NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 5. afc_hash_first() / afc_hash_next() traversal
	 *    Elements are sorted by hash_value ascending, so:
	 *    100 (alpha) -> 200 (beta) -> 300 (gamma)
	 * ---------------------------------------------------------------- */
	s = (char *)afc_hash_first(hm);
	print_res("first == alpha (key 100)", "alpha", s, 1);

	s = (char *)afc_hash_next(hm);
	print_res("next == beta (key 200)", "beta", s, 1);

	s = (char *)afc_hash_next(hm);
	print_res("next == gamma (key 300)", "gamma", s, 1);

	/* next past end should return NULL */
	s = (char *)afc_hash_next(hm);
	print_res("next past end == NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 6. afc_hash_last() / afc_hash_prev() traversal
	 * ---------------------------------------------------------------- */
	s = (char *)afc_hash_last(hm);
	print_res("last == gamma", "gamma", s, 1);

	s = (char *)afc_hash_prev(hm);
	print_res("prev == beta", "beta", s, 1);

	s = (char *)afc_hash_prev(hm);
	print_res("prev == alpha", "alpha", s, 1);

	/* prev past start should return NULL */
	s = (char *)afc_hash_prev(hm);
	print_res("prev past start == NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 7. afc_hash_del() - delete the current element
	 *    Position on "beta" (the middle element) and delete it
	 * ---------------------------------------------------------------- */
	afc_hash_first(hm);
	afc_hash_next(hm); /* now pointing at beta (key 200) */

	void *next = afc_hash_del(hm);
	print_res("len after del", (void *)(long)2, (void *)(long)_hash_len(hm), 0);

	/* After deleting beta, the next item should be gamma */
	print_res("del returns gamma", "gamma", (char *)next, 1);

	/* beta should no longer be findable */
	s = (char *)afc_hash_find(hm, HASH_KEY_BETA);
	print_res("find beta after del == NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);

	/* alpha and gamma should still be findable */
	s = (char *)afc_hash_find(hm, HASH_KEY_ALPHA);
	print_res("alpha still findable", "alpha", s, 1);

	s = (char *)afc_hash_find(hm, HASH_KEY_GAMMA);
	print_res("gamma still findable", "gamma", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 8. afc_hash_clear()
	 * ---------------------------------------------------------------- */
	afc_hash_clear(hm);
	print_res("len after clear", (void *)(long)0, (void *)(long)_hash_len(hm), 0);
	print_res("is_empty after clear", (void *)(long)1, (void *)(long)afc_hash_is_empty(hm), 0);

	/* first() on empty hash should return NULL */
	s = (char *)afc_hash_first(hm);
	print_res("first on empty == NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 9. Edge case: add after clear, verify recovery
	 * ---------------------------------------------------------------- */
	afc_hash_add(hm, HASH_KEY_DELTA, "delta");
	print_res("len after re-add", (void *)(long)1, (void *)(long)_hash_len(hm), 0);

	s = (char *)afc_hash_find(hm, HASH_KEY_DELTA);
	print_res("find delta after re-add", "delta", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 10. Edge case: delete the only element
	 * ---------------------------------------------------------------- */
	afc_hash_first(hm);
	next = afc_hash_del(hm);
	print_res("del single -> NULL", (void *)(long)1, (void *)(long)(next == NULL), 0);
	print_res("is_empty after del all", (void *)(long)1, (void *)(long)afc_hash_is_empty(hm), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 11. Multiple items with sequential keys to stress binary search
	 * ---------------------------------------------------------------- */
	unsigned long int i;
	for (i = 1; i <= 10; i++)
	{
		/* Store the key value as the data pointer for simplicity */
		afc_hash_add(hm, i, (void *)(long)i);
	}
	print_res("len after 10 adds", (void *)(long)10, (void *)(long)_hash_len(hm), 0);

	/* Verify every element is findable */
	int all_found = 1;
	for (i = 1; i <= 10; i++)
	{
		void *v = afc_hash_find(hm, i);
		if ((long)v != (long)i)
		{
			all_found = 0;
			break;
		}
	}
	print_res("all 10 keys found", (void *)(long)1, (void *)(long)all_found, 0);

	/* ----------------------------------------------------------------
	 * Cleanup
	 * ---------------------------------------------------------------- */
	print_summary();

	afc_hash_delete(hm);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
