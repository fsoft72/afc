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
 * test_string_list.c - Comprehensive tests for the AFC StringList module.
 *
 * Tests cover creation, adding strings, split with various delimiters,
 * sorting (case/nocase, normal/inverted), deletion, clearing,
 * and iteration via the wrapper macros.
 *
 * StringList wraps List, storing duplicated AFC strings internally.
 * Iteration uses afc_string_list_first/next/obj macros.
 */

#include "test_utils.h"
#include "../src/string_list.h"

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ----------------------------------------------------------------
	 * 1. Creation and initial state
	 * ---------------------------------------------------------------- */
	StringList *sn = afc_string_list_new();
	print_res("string_list_new != NULL", (void *)(long)1, (void *)(long)(sn != NULL), 0);
	print_res("is_empty after new", (void *)(long)1, (void *)(long)afc_string_list_is_empty(sn), 0);
	print_res("len after new", (void *)(long)0, (void *)(long)afc_string_list_len(sn), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 2. afc_string_list_add() at TAIL
	 * ---------------------------------------------------------------- */
	afc_string_list_add(sn, "cherry", AFC_STRING_LIST_ADD_TAIL);
	afc_string_list_add(sn, "apple", AFC_STRING_LIST_ADD_TAIL);
	afc_string_list_add(sn, "banana", AFC_STRING_LIST_ADD_TAIL);

	print_res("len after 3 adds", (void *)(long)3, (void *)(long)afc_string_list_len(sn), 0);
	print_res("is_empty false", (void *)(long)0, (void *)(long)afc_string_list_is_empty(sn), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 3. Traversal via first/next/obj macros
	 * ---------------------------------------------------------------- */
	char *s;
	s = afc_string_list_first(sn);
	print_res("first == cherry", "cherry", s, 1);

	s = afc_string_list_next(sn);
	print_res("next == apple", "apple", s, 1);

	s = afc_string_list_next(sn);
	print_res("next == banana", "banana", s, 1);

	/* next past end should return NULL */
	s = afc_string_list_next(sn);
	print_res("next past end == NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);

	/* obj() should return current item (banana, the last visited) */
	s = afc_string_list_obj(sn);
	print_res("obj == banana", "banana", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 4. last / prev traversal
	 * ---------------------------------------------------------------- */
	s = afc_string_list_last(sn);
	print_res("last == banana", "banana", s, 1);

	s = afc_string_list_prev(sn);
	print_res("prev == apple", "apple", s, 1);

	s = afc_string_list_prev(sn);
	print_res("prev == cherry", "cherry", s, 1);

	s = afc_string_list_prev(sn);
	print_res("prev past start == NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 5. afc_string_list_add() at HEAD
	 * ---------------------------------------------------------------- */
	afc_string_list_add(sn, "date", AFC_STRING_LIST_ADD_HEAD);
	print_res("len after HEAD add", (void *)(long)4, (void *)(long)afc_string_list_len(sn), 0);

	s = afc_string_list_first(sn);
	print_res("first after HEAD add", "date", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 6. afc_string_list_item() by index
	 * ---------------------------------------------------------------- */
	s = afc_string_list_item(sn, 0);
	print_res("item(0) == date", "date", s, 1);

	s = afc_string_list_item(sn, 1);
	print_res("item(1) == cherry", "cherry", s, 1);

	s = afc_string_list_item(sn, 2);
	print_res("item(2) == apple", "apple", s, 1);

	s = afc_string_list_item(sn, 3);
	print_res("item(3) == banana", "banana", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 7. afc_string_list_sort() - case-sensitive, not inverted
	 *    sort(sn, nocase=FALSE, inverted=FALSE, fast=FALSE)
	 * ---------------------------------------------------------------- */
	afc_string_list_sort(sn, FALSE, FALSE, FALSE);

	s = afc_string_list_first(sn);
	print_res("sorted first == apple", "apple", s, 1);

	s = afc_string_list_next(sn);
	print_res("sorted next == banana", "banana", s, 1);

	s = afc_string_list_next(sn);
	print_res("sorted next == cherry", "cherry", s, 1);

	s = afc_string_list_next(sn);
	print_res("sorted next == date", "date", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 8. afc_string_list_del() - delete current element
	 *    Delete "date" (the current/last after sort traversal)
	 * ---------------------------------------------------------------- */
	/* Position on "banana" (item 1) and delete it */
	afc_string_list_item(sn, 1);
	s = afc_string_list_del(sn);
	print_res("len after del", (void *)(long)3, (void *)(long)afc_string_list_len(sn), 0);

	/* del returns the next string after the deleted one */
	print_res("del returns next", "cherry", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 9. afc_string_list_clear()
	 * ---------------------------------------------------------------- */
	afc_string_list_clear(sn);
	print_res("len after clear", (void *)(long)0, (void *)(long)afc_string_list_len(sn), 0);
	print_res("is_empty after clear", (void *)(long)1, (void *)(long)afc_string_list_is_empty(sn), 0);

	s = afc_string_list_first(sn);
	print_res("first on empty == NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 10. afc_string_list_split() with pipe delimiter
	 * ---------------------------------------------------------------- */
	int split_res = afc_string_list_split(sn, "one|two|three|four", "|");
	print_res("split pipe OK", (void *)(long)AFC_ERR_NO_ERROR, (void *)(long)split_res, 0);
	print_res("split pipe count", (void *)(long)4, (void *)(long)afc_string_list_len(sn), 0);

	s = afc_string_list_first(sn);
	print_res("split[0] == one", "one", s, 1);

	s = afc_string_list_next(sn);
	print_res("split[1] == two", "two", s, 1);

	s = afc_string_list_next(sn);
	print_res("split[2] == three", "three", s, 1);

	s = afc_string_list_next(sn);
	print_res("split[3] == four", "four", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 11. afc_string_list_split() with comma delimiter
	 * ---------------------------------------------------------------- */
	split_res = afc_string_list_split(sn, "alpha,beta,gamma", ",");
	print_res("split comma OK", (void *)(long)AFC_ERR_NO_ERROR, (void *)(long)split_res, 0);
	print_res("split comma count", (void *)(long)3, (void *)(long)afc_string_list_len(sn), 0);

	s = afc_string_list_first(sn);
	print_res("comma split[0] == alpha", "alpha", s, 1);

	s = afc_string_list_item(sn, 2);
	print_res("comma split[2] == gamma", "gamma", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 12. afc_string_list_split() with multiple delimiter characters
	 *     (both comma and semicolon act as separators)
	 * ---------------------------------------------------------------- */
	split_res = afc_string_list_split(sn, "a,b;c,d", ",;");
	print_res("split multi-delim OK", (void *)(long)AFC_ERR_NO_ERROR, (void *)(long)split_res, 0);
	print_res("multi-delim count", (void *)(long)4, (void *)(long)afc_string_list_len(sn), 0);

	s = afc_string_list_first(sn);
	print_res("multi[0] == a", "a", s, 1);

	s = afc_string_list_item(sn, 1);
	print_res("multi[1] == b", "b", s, 1);

	s = afc_string_list_item(sn, 2);
	print_res("multi[2] == c", "c", s, 1);

	s = afc_string_list_item(sn, 3);
	print_res("multi[3] == d", "d", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 13. afc_string_list_split() with empty fields
	 *     Consecutive delimiters produce empty strings by default
	 * ---------------------------------------------------------------- */
	split_res = afc_string_list_split(sn, "a||b", "|");
	print_res("split empty field OK", (void *)(long)AFC_ERR_NO_ERROR, (void *)(long)split_res, 0);
	print_res("empty field count", (void *)(long)3, (void *)(long)afc_string_list_len(sn), 0);

	s = afc_string_list_first(sn);
	print_res("ef[0] == a", "a", s, 1);

	s = afc_string_list_next(sn);
	print_res("ef[1] == empty str", "", s, 1);

	s = afc_string_list_next(sn);
	print_res("ef[2] == b", "b", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 14. Sort after split
	 * ---------------------------------------------------------------- */
	afc_string_list_split(sn, "cherry,apple,banana,date", ",");
	afc_string_list_sort(sn, FALSE, FALSE, FALSE);

	s = afc_string_list_first(sn);
	print_res("sort after split [0]", "apple", s, 1);

	s = afc_string_list_next(sn);
	print_res("sort after split [1]", "banana", s, 1);

	s = afc_string_list_next(sn);
	print_res("sort after split [2]", "cherry", s, 1);

	s = afc_string_list_next(sn);
	print_res("sort after split [3]", "date", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 15. Convenience macros: add_tail, add_head
	 * ---------------------------------------------------------------- */
	afc_string_list_clear(sn);
	afc_string_list_add_tail(sn, "tail1");
	afc_string_list_add_tail(sn, "tail2");
	afc_string_list_add_head(sn, "head1");

	print_res("num_items == 3", (void *)(long)3, (void *)(long)afc_string_list_num_items(sn), 0);

	s = afc_string_list_first(sn);
	print_res("add_head -> first", "head1", s, 1);

	s = afc_string_list_last(sn);
	print_res("add_tail -> last", "tail2", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 16. Edge case: delete the only element
	 * ---------------------------------------------------------------- */
	afc_string_list_clear(sn);
	afc_string_list_add_tail(sn, "solo");
	afc_string_list_first(sn);
	s = afc_string_list_del(sn);
	print_res("del single -> NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);
	print_res("is_empty after del all", (void *)(long)1, (void *)(long)afc_string_list_is_empty(sn), 0);

	/* ----------------------------------------------------------------
	 * Cleanup
	 * ---------------------------------------------------------------- */
	print_summary();

	afc_string_list_delete(sn);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
