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
 * test_dictionary.c - Comprehensive tests for the AFC Dictionary module.
 *
 * Tests cover creation, set/get, has_key, get_default, iteration
 * (first/next), del, del_item, clear, and overwrite behavior.
 *
 * NOTE: Dictionary uses string keys internally. Values stored are
 * void pointers. For testing we store string literals (which are
 * not owned by the dictionary unless a clear func is set).
 */

#include "test_utils.h"
#include "../src/dictionary.h"

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ----------------------------------------------------------------
	 * 1. Creation and initial state
	 * ---------------------------------------------------------------- */
	Dictionary *dict = afc_dictionary_new();
	print_res("dict_new != NULL", (void *)(long)1, (void *)(long)(dict != NULL), 0);
	print_res("len after new", (void *)(long)0, (void *)(long)afc_dictionary_len(dict), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 2. afc_dictionary_set() / afc_dictionary_get()
	 * ---------------------------------------------------------------- */
	int res;
	res = afc_dictionary_set(dict, "name", "Fabio");
	print_res("set 'name' OK", (void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	res = afc_dictionary_set(dict, "city", "Milan");
	print_res("set 'city' OK", (void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	res = afc_dictionary_set(dict, "lang", "C");
	print_res("set 'lang' OK", (void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	print_res("len after 3 sets", (void *)(long)3, (void *)(long)afc_dictionary_len(dict), 0);

	char *s;
	s = (char *)afc_dictionary_get(dict, "name");
	print_res("get 'name' == Fabio", "Fabio", s, 1);

	s = (char *)afc_dictionary_get(dict, "city");
	print_res("get 'city' == Milan", "Milan", s, 1);

	s = (char *)afc_dictionary_get(dict, "lang");
	print_res("get 'lang' == C", "C", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 3. afc_dictionary_get() - non-existing key
	 * ---------------------------------------------------------------- */
	s = (char *)afc_dictionary_get(dict, "missing");
	print_res("get missing == NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 4. afc_dictionary_has_key()
	 * ---------------------------------------------------------------- */
	BOOL has = afc_dictionary_has_key(dict, "name");
	print_res("has_key 'name' == TRUE", (void *)(long)TRUE, (void *)(long)has, 0);

	has = afc_dictionary_has_key(dict, "missing");
	print_res("has_key 'missing' == FALSE", (void *)(long)FALSE, (void *)(long)has, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 5. afc_dictionary_get_default()
	 * ---------------------------------------------------------------- */
	s = (char *)afc_dictionary_get_default(dict, "name", "default");
	print_res("get_default existing", "Fabio", s, 1);

	s = (char *)afc_dictionary_get_default(dict, "nonexist", "fallback");
	print_res("get_default missing", "fallback", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 6. afc_dictionary_set() overwrite existing key
	 * ---------------------------------------------------------------- */
	afc_dictionary_set(dict, "city", "Rome");
	s = (char *)afc_dictionary_get(dict, "city");
	print_res("overwrite city == Rome", "Rome", s, 1);

	/* Length should remain the same after overwrite */
	print_res("len unchanged after overwrite", (void *)(long)3, (void *)(long)afc_dictionary_len(dict), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 7. afc_dictionary_first() / afc_dictionary_next() iteration
	 *    The iteration order depends on hash values, so we just
	 *    verify that we get exactly 3 elements and they are valid.
	 * ---------------------------------------------------------------- */
	int count = 0;
	void *val = afc_dictionary_first(dict);
	while (val)
	{
		count++;
		/* Verify we can get the key for the current item */
		char *key = afc_dictionary_get_key(dict);
		print_res("iter key != NULL", (void *)(long)1, (void *)(long)(key != NULL), 0);
		val = afc_dictionary_next(dict);
	}
	print_res("iteration count == 3", (void *)(long)3, (void *)(long)count, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 8. afc_dictionary_del_item() - delete by key
	 * ---------------------------------------------------------------- */
	res = afc_dictionary_del_item(dict, "lang");
	print_res("del_item 'lang' OK", (void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);
	print_res("len after del_item", (void *)(long)2, (void *)(long)afc_dictionary_len(dict), 0);

	/* The deleted key should no longer be found */
	s = (char *)afc_dictionary_get(dict, "lang");
	print_res("get 'lang' after del == NULL", (void *)(long)1, (void *)(long)(s == NULL), 0);

	/* Other keys still accessible */
	s = (char *)afc_dictionary_get(dict, "name");
	print_res("'name' still found", "Fabio", s, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 9. afc_dictionary_del() - delete current element via iteration
	 * ---------------------------------------------------------------- */
	/* Position on first element and delete it */
	val = afc_dictionary_first(dict);
	print_res("first != NULL before del", (void *)(long)1, (void *)(long)(val != NULL), 0);

	val = afc_dictionary_del(dict);
	print_res("len after del", (void *)(long)1, (void *)(long)afc_dictionary_len(dict), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 10. afc_dictionary_clear()
	 * ---------------------------------------------------------------- */
	afc_dictionary_clear(dict);
	print_res("len after clear", (void *)(long)0, (void *)(long)afc_dictionary_len(dict), 0);

	/* first() on empty dict returns NULL */
	val = afc_dictionary_first(dict);
	print_res("first on empty == NULL", (void *)(long)1, (void *)(long)(val == NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 11. Edge case: add after clear, verify recovery
	 * ---------------------------------------------------------------- */
	afc_dictionary_set(dict, "after_clear", "works");
	s = (char *)afc_dictionary_get(dict, "after_clear");
	print_res("get after re-add", "works", s, 1);
	print_res("len after re-add", (void *)(long)1, (void *)(long)afc_dictionary_len(dict), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 12. Edge case: set with NULL value removes the key
	 * ---------------------------------------------------------------- */
	afc_dictionary_set(dict, "after_clear", NULL);
	s = (char *)afc_dictionary_get(dict, "after_clear");
	print_res("NULL set removes key", (void *)(long)1, (void *)(long)(s == NULL), 0);
	print_res("len after NULL set", (void *)(long)0, (void *)(long)afc_dictionary_len(dict), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 13. Edge case: del_item on non-existing key
	 * ---------------------------------------------------------------- */
	res = afc_dictionary_del_item(dict, "nonexist");
	print_res("del_item missing != NO_ERR", (void *)(long)1, (void *)(long)(res != AFC_ERR_NO_ERROR), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 14. Multiple keys with same-length names to stress hashing
	 * ---------------------------------------------------------------- */
	afc_dictionary_set(dict, "aa", "val_aa");
	afc_dictionary_set(dict, "bb", "val_bb");
	afc_dictionary_set(dict, "cc", "val_cc");
	afc_dictionary_set(dict, "dd", "val_dd");

	print_res("len after 4 adds", (void *)(long)4, (void *)(long)afc_dictionary_len(dict), 0);

	s = (char *)afc_dictionary_get(dict, "aa");
	print_res("get 'aa'", "val_aa", s, 1);
	s = (char *)afc_dictionary_get(dict, "dd");
	print_res("get 'dd'", "val_dd", s, 1);

	/* ----------------------------------------------------------------
	 * Cleanup
	 * ---------------------------------------------------------------- */
	print_summary();

	afc_dictionary_delete(dict);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
