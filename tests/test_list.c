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
 * test_list.c - Comprehensive tests for the AFC List (Node Master) module.
 *
 * Tests cover creation, insertion at HEAD/TAIL/HERE, traversal
 * (first/next/prev/last), obj(), is_empty(), del(), item() by position,
 * push()/pop() stack, and clear().
 */

#include "test_utils.h"
#include "../src/list.h"

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ----------------------------------------------------------------
	 * 1. Creation and initial state
	 * ---------------------------------------------------------------- */
	List *nm = afc_list_new();
	print_res("list_new != NULL", (void *)(long)1, (void *)(long)(nm != NULL), 0);
	print_res("is_empty after new", (void *)(long)1, (void *)(long)afc_list_is_empty(nm), 0);
	print_res("len after new", (void *)(long)0, (void *)(long)afc_list_len(nm), 0);

	/* obj() on empty list should return NULL */
	void *obj = afc_list_obj(nm);
	print_res("obj on empty == NULL", (void *)(long)1, (void *)(long)(obj == NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 2. afc_list_add() at TAIL
	 * ---------------------------------------------------------------- */
	afc_list_add(nm, "alpha", AFC_LIST_ADD_TAIL);
	afc_list_add(nm, "beta", AFC_LIST_ADD_TAIL);
	afc_list_add(nm, "gamma", AFC_LIST_ADD_TAIL);

	print_res("len after 3 TAIL adds", (void *)(long)3, (void *)(long)afc_list_len(nm), 0);
	print_res("is_empty false", (void *)(long)0, (void *)(long)afc_list_is_empty(nm), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 3. afc_list_first / next / prev / last traversal
	 * ---------------------------------------------------------------- */
	char *s = (char *)afc_list_first(nm);
	print_res("first == alpha", "alpha", s, 1);

	s = (char *)afc_list_next(nm);
	print_res("next == beta", "beta", s, 1);

	s = (char *)afc_list_next(nm);
	print_res("next == gamma", "gamma", s, 1);

	/* next past end should return NULL */
	s = (char *)afc_list_next(nm);
	print_res("next past end == NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);

	s = (char *)afc_list_last(nm);
	print_res("last == gamma", "gamma", s, 1);

	s = (char *)afc_list_prev(nm);
	print_res("prev == beta", "beta", s, 1);

	s = (char *)afc_list_prev(nm);
	print_res("prev == alpha", "alpha", s, 1);

	/* prev past beginning should return NULL */
	s = (char *)afc_list_prev(nm);
	print_res("prev past start == NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 4. afc_list_obj() returns current item
	 * ---------------------------------------------------------------- */
	afc_list_first(nm);
	s = (char *)afc_list_obj(nm);
	print_res("obj after first == alpha", "alpha", s, 1);

	afc_list_next(nm);
	s = (char *)afc_list_obj(nm);
	print_res("obj after next == beta", "beta", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 5. afc_list_item() by position (0-indexed)
	 * ---------------------------------------------------------------- */
	s = (char *)afc_list_item(nm, 0);
	print_res("item(0) == alpha", "alpha", s, 1);

	s = (char *)afc_list_item(nm, 1);
	print_res("item(1) == beta", "beta", s, 1);

	s = (char *)afc_list_item(nm, 2);
	print_res("item(2) == gamma", "gamma", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 6. afc_list_add() at HEAD
	 * ---------------------------------------------------------------- */
	afc_list_add(nm, "head_item", AFC_LIST_ADD_HEAD);
	print_res("len after HEAD add", (void *)(long)4, (void *)(long)afc_list_len(nm), 0);

	s = (char *)afc_list_first(nm);
	print_res("first after HEAD add", "head_item", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 7. afc_list_add() at HERE (inserts after current)
	 * ---------------------------------------------------------------- */
	/* Position on "alpha" (index 1) and insert after it */
	afc_list_item(nm, 1);
	afc_list_add(nm, "inserted", AFC_LIST_ADD_HERE);
	print_res("len after HERE add", (void *)(long)5, (void *)(long)afc_list_len(nm), 0);

	/* The inserted element should now be at index 2 */
	s = (char *)afc_list_item(nm, 2);
	print_res("item(2) == inserted", "inserted", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 8. afc_list_del() - delete the current item
	 * ---------------------------------------------------------------- */
	/* Delete the "inserted" element (currently at item 2) */
	afc_list_item(nm, 2);
	s = (char *)afc_list_del(nm);
	print_res("len after del", (void *)(long)4, (void *)(long)afc_list_len(nm), 0);

	/* After deletion, del returns the next item */
	print_res("del returns next item", "beta", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 9. afc_list_push() / afc_list_pop() stack
	 * ---------------------------------------------------------------- */
	/* Position on "beta" (item 2) */
	afc_list_item(nm, 2);
	s = (char *)afc_list_obj(nm);
	print_res("before push obj == beta", "beta", s, 1);

	/* Push current position */
	short push_ok = afc_list_push(nm);
	print_res("push returns TRUE", (void *)(long)1, (void *)(long)(push_ok == TRUE), 0);

	/* Move somewhere else */
	afc_list_first(nm);
	s = (char *)afc_list_obj(nm);
	print_res("after move obj == head_item", "head_item", s, 1);

	/* Pop restores position (autopos = TRUE) */
	s = (char *)afc_list_pop(nm, TRUE);
	print_res("pop restores beta", "beta", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 10. afc_list_clear()
	 * ---------------------------------------------------------------- */
	afc_list_clear(nm);
	print_res("len after clear", (void *)(long)0, (void *)(long)afc_list_len(nm), 0);
	print_res("is_empty after clear", (void *)(long)1, (void *)(long)afc_list_is_empty(nm), 0);

	/* first() on empty list returns NULL */
	s = (char *)afc_list_first(nm);
	print_res("first on empty == NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 11. Edge case: delete the only element
	 * ---------------------------------------------------------------- */
	afc_list_add(nm, "solo", AFC_LIST_ADD_TAIL);
	print_res("len after 1 add", (void *)(long)1, (void *)(long)afc_list_len(nm), 0);

	afc_list_first(nm);
	s = (char *)afc_list_del(nm);
	print_res("del single -> NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);
	print_res("is_empty after del last", (void *)(long)1, (void *)(long)afc_list_is_empty(nm), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 12. Convenience macros: add_tail, add_head, num_items
	 * ---------------------------------------------------------------- */
	afc_list_add_tail(nm, "t1");
	afc_list_add_tail(nm, "t2");
	afc_list_add_head(nm, "h1");
	print_res("num_items == 3", (void *)(long)3, (void *)(long)afc_list_num_items(nm), 0);

	s = (char *)afc_list_first(nm);
	print_res("add_head -> first == h1", "h1", s, 1);

	s = (char *)afc_list_last(nm);
	print_res("add_tail -> last == t2", "t2", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 13. Push on empty stack, pop with autopos=FALSE
	 * ---------------------------------------------------------------- */
	afc_list_clear(nm);
	afc_list_add_tail(nm, "one");
	afc_list_add_tail(nm, "two");

	afc_list_first(nm);
	afc_list_push(nm);
	afc_list_last(nm);

	/* Pop with autopos=FALSE discards the saved position */
	s = (char *)afc_list_pop(nm, FALSE);
	print_res("pop(FALSE) returns NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);

	/* Current position should still be where we were (last) */
	s = (char *)afc_list_obj(nm);
	print_res("pos unchanged after pop(F)", "two", s, 1);

	/* ----------------------------------------------------------------
	 * Cleanup
	 * ---------------------------------------------------------------- */
	print_summary();

	afc_list_delete(nm);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
