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
 * test_array.c - Comprehensive tests for the AFC Array module.
 *
 * Tests cover creation, initialization, insertion (HEAD/TAIL/HERE),
 * traversal (first/next/prev/last), indexed access, length, emptiness,
 * deletion, sorting with a custom comparator, and clearing.
 */

#include "test_utils.h"
#include "../src/array.h"

/* Size for explicit afc_array_init() test */
#define INIT_SIZE 10

/**
 * _compare_strings_asc - Custom comparator for ascending string sort.
 *
 * Used with afc_array_sort() to order string pointers alphabetically.
 * The comparator receives pointers-to-pointers (void **) because the
 * array stores void* elements.
 */
static int _compare_strings_asc(const void *a, const void *b)
{
	const char *s1 = *(const char **)a;
	const char *s2 = *(const char **)b;
	return strcmp(s1, s2);
}

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ----------------------------------------------------------------
	 * 1. Creation and initial state
	 * ---------------------------------------------------------------- */
	Array *am = afc_array_new();
	print_res("array_new != NULL", (void *)(long)1, (void *)(long)(am != NULL), 0);
	print_res("is_empty after new", (void *)(long)1, (void *)(long)afc_array_is_empty(am), 0);
	print_res("len after new", (void *)(long)0, (void *)(long)afc_array_len(am), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 2. afc_array_init() with a custom size
	 * ---------------------------------------------------------------- */
	int res = afc_array_init(am, INIT_SIZE);
	print_res("init returns NO_ERROR", (void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);
	print_res("is_empty after init", (void *)(long)1, (void *)(long)afc_array_is_empty(am), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 3. afc_array_add() at TAIL
	 * ---------------------------------------------------------------- */
	afc_array_add(am, "alpha", AFC_ARRAY_ADD_TAIL);
	afc_array_add(am, "beta", AFC_ARRAY_ADD_TAIL);
	afc_array_add(am, "gamma", AFC_ARRAY_ADD_TAIL);

	print_res("len after 3 adds", (void *)(long)3, (void *)(long)afc_array_len(am), 0);
	print_res("is_empty false", (void *)(long)0, (void *)(long)afc_array_is_empty(am), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 4. afc_array_first / next / prev / last traversal
	 * ---------------------------------------------------------------- */
	char *s = (char *)afc_array_first(am);
	print_res("first == alpha", "alpha", s, 1);

	s = (char *)afc_array_next(am);
	print_res("next == beta", "beta", s, 1);

	s = (char *)afc_array_next(am);
	print_res("next == gamma", "gamma", s, 1);

	/* next past end should return NULL */
	s = (char *)afc_array_next(am);
	print_res("next past end == NULL", (void *)(long)0, (void *)(long)(s != NULL), 0);

	s = (char *)afc_array_last(am);
	print_res("last == gamma", "gamma", s, 1);

	s = (char *)afc_array_prev(am);
	print_res("prev == beta", "beta", s, 1);

	s = (char *)afc_array_prev(am);
	print_res("prev == alpha", "alpha", s, 1);

	/* prev past beginning should return NULL */
	s = (char *)afc_array_prev(am);
	print_res("prev past start == NULL", (void *)(long)0, (void *)(long)(s != NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 5. afc_array_item() by index
	 * ---------------------------------------------------------------- */
	s = (char *)afc_array_item(am, 0);
	print_res("item(0) == alpha", "alpha", s, 1);

	s = (char *)afc_array_item(am, 1);
	print_res("item(1) == beta", "beta", s, 1);

	s = (char *)afc_array_item(am, 2);
	print_res("item(2) == gamma", "gamma", s, 1);

	/* Out-of-bounds should return NULL */
	s = (char *)afc_array_item(am, 99);
	print_res("item(99) == NULL", (void *)(long)0, (void *)(long)(s != NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 6. afc_array_add() at HEAD
	 * ---------------------------------------------------------------- */
	afc_array_add(am, "head_item", AFC_ARRAY_ADD_HEAD);
	print_res("len after add HEAD", (void *)(long)4, (void *)(long)afc_array_len(am), 0);

	s = (char *)afc_array_item(am, 0);
	print_res("item(0) after HEAD add", "head_item", s, 1);

	/* Previous items shifted */
	s = (char *)afc_array_item(am, 1);
	print_res("item(1) == alpha", "alpha", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 7. afc_array_add() at HERE (inserts after current position)
	 * ---------------------------------------------------------------- */
	/* Position on index 1 (alpha), then insert after it */
	afc_array_item(am, 1);
	afc_array_add(am, "inserted", AFC_ARRAY_ADD_HERE);
	print_res("len after add HERE", (void *)(long)5, (void *)(long)afc_array_len(am), 0);

	s = (char *)afc_array_item(am, 2);
	print_res("item(2) == inserted", "inserted", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 8. afc_array_del() - delete current element
	 * ---------------------------------------------------------------- */
	/* Position on the "inserted" element (index 2) and delete it */
	afc_array_item(am, 2);
	afc_array_del(am);
	print_res("len after del", (void *)(long)4, (void *)(long)afc_array_len(am), 0);

	/* After deletion the element at index 2 should be "beta" */
	s = (char *)afc_array_item(am, 2);
	print_res("item(2) after del == beta", "beta", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 9. afc_array_sort() with a custom comparator
	 * ---------------------------------------------------------------- */
	/* Clear and repopulate for a clean sort test */
	afc_array_clear(am);
	afc_array_add(am, "delta", AFC_ARRAY_ADD_TAIL);
	afc_array_add(am, "alpha", AFC_ARRAY_ADD_TAIL);
	afc_array_add(am, "charlie", AFC_ARRAY_ADD_TAIL);
	afc_array_add(am, "bravo", AFC_ARRAY_ADD_TAIL);

	afc_array_sort(am, _compare_strings_asc);

	s = (char *)afc_array_item(am, 0);
	print_res("sorted item(0) == alpha", "alpha", s, 1);
	s = (char *)afc_array_item(am, 1);
	print_res("sorted item(1) == bravo", "bravo", s, 1);
	s = (char *)afc_array_item(am, 2);
	print_res("sorted item(2) == charlie", "charlie", s, 1);
	s = (char *)afc_array_item(am, 3);
	print_res("sorted item(3) == delta", "delta", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 10. afc_array_clear()
	 * ---------------------------------------------------------------- */
	afc_array_clear(am);
	print_res("len after clear", (void *)(long)0, (void *)(long)afc_array_len(am), 0);
	print_res("is_empty after clear", (void *)(long)1, (void *)(long)afc_array_is_empty(am), 0);

	/* first() on empty array should return NULL */
	s = (char *)afc_array_first(am);
	print_res("first on empty == NULL", (void *)(long)0, (void *)(long)(s != NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 11. Edge case: delete last element leaves array empty
	 * ---------------------------------------------------------------- */
	afc_array_add(am, "only", AFC_ARRAY_ADD_TAIL);
	print_res("len after 1 add", (void *)(long)1, (void *)(long)afc_array_len(am), 0);

	afc_array_first(am);
	void *next = afc_array_del(am);
	print_res("del single -> NULL", (void *)(long)0, (void *)(long)(next != NULL), 0);
	print_res("is_empty after del last", (void *)(long)1, (void *)(long)afc_array_is_empty(am), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 12. Convenience macros: add_tail, add_head, num_items
	 * ---------------------------------------------------------------- */
	afc_array_add_tail(am, "tail1");
	afc_array_add_tail(am, "tail2");
	afc_array_add_head(am, "head1");
	print_res("num_items == 3", (void *)(long)3, (void *)(long)afc_array_num_items(am), 0);

	s = (char *)afc_array_item(am, 0);
	print_res("add_head -> item(0)", "head1", s, 1);

	/* ----------------------------------------------------------------
	 * Cleanup
	 * ---------------------------------------------------------------- */
	print_summary();

	afc_array_delete(am);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
