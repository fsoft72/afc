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
/* Edge case tests for AFC String */
#include "../test_utils.h"

int main()
{
	AFC *afc = afc_new();
	char *s, *t;
	char *result;

	test_header();

	/* ===== NULL pointer handling ===== */

	/* afc_string_new(0) should work (creates 1-char string) */
	s = afc_string_new(0);
	print_res("afc_string_new(0)", (void *)1, (void *)afc_string_max(s), 0);
	afc_string_delete(s);

	/* afc_string_copy with NULL dest */
	result = afc_string_copy(NULL, "test", ALL);
	print_res("afc_string_copy(NULL,src)", NULL, result, 1);

	/* afc_string_copy with NULL source */
	s = afc_string_new(10);
	result = afc_string_copy(s, NULL, ALL);
	print_res("afc_string_copy(dest,NULL)", NULL, result, 1);

	/* afc_string_add with NULL dest */
	result = afc_string_add(NULL, "test", ALL);
	print_res("afc_string_add(NULL,src)", NULL, result, 1);

	/* afc_string_add with NULL source */
	result = afc_string_add(s, NULL, ALL);
	print_res("afc_string_add(dest,NULL)", NULL, result, 1);

	/* afc_string_len with NULL */
	print_res("afc_string_len(NULL)", (void *)0, (void *)afc_string_len(NULL), 0);

	/* afc_string_max with NULL */
	print_res("afc_string_max(NULL)", (void *)0, (void *)afc_string_max(NULL), 0);

	/* afc_string_clear with NULL */
	result = afc_string_clear(NULL);
	print_res("afc_string_clear(NULL)", NULL, result, 1);

	/* afc_string_upper with NULL */
	result = afc_string_upper(NULL);
	print_res("afc_string_upper(NULL)", NULL, result, 1);

	/* afc_string_lower with NULL */
	result = afc_string_lower(NULL);
	print_res("afc_string_lower(NULL)", NULL, result, 1);

	/* afc_string_trim with NULL */
	result = afc_string_trim(NULL);
	print_res("afc_string_trim(NULL)", NULL, result, 1);

	/* afc_string_dup with NULL */
	result = afc_string_dup(NULL);
	print_res("afc_string_dup(NULL)", NULL, result, 1);

	/* afc_string_instr with NULL str */
	result = afc_string_instr(NULL, "test", 0);
	print_res("afc_string_instr(NULL,match)", NULL, result, 1);

	/* afc_string_instr with NULL match */
	result = afc_string_instr("test", NULL, 0);
	print_res("afc_string_instr(str,NULL)", NULL, result, 1);

	/* afc_string_mid with NULL src */
	result = afc_string_mid(s, NULL, 0, 5);
	print_res("afc_string_mid(dest,NULL)", NULL, result, 1);

	/* afc_string_left with NULL src */
	result = afc_string_left(s, NULL, 5);
	print_res("afc_string_left(dest,NULL)", NULL, result, 1);

	/* afc_string_right with NULL src */
	result = afc_string_right(s, NULL, 5);
	print_res("afc_string_right(dest,NULL)", NULL, result, 1);

	/* afc_string_make with NULL dest */
	result = afc_string_make(NULL, "%s", "test");
	print_res("afc_string_make(NULL,fmt)", NULL, result, 1);

	/* afc_string_make with NULL fmt */
	afc_string_copy(s, "hello", ALL);
	result = afc_string_make(s, NULL);
	print_res("afc_string_make(dest,NULL)", NULL, result, 1);

	/* afc_string_comp with NULL s1 */
	print_res("afc_string_comp(NULL,s2)", (void *)1, (void *)(afc_string_comp(NULL, "test", ALL) != 0 ? 1 : 0), 0);

	/* afc_string_comp with NULL s2 */
	print_res("afc_string_comp(s1,NULL)", (void *)1, (void *)(afc_string_comp("test", NULL, ALL) != 0 ? 1 : 0), 0);

	/* afc_string_hash with NULL */
	print_res("afc_string_hash(NULL)", (void *)0, (void *)afc_string_hash(NULL, 0), 0);

	/* afc_string_dirname with NULL */
	result = afc_string_dirname(NULL);
	print_res("afc_string_dirname(NULL)", NULL, result, 1);

	/* afc_string_basename with NULL */
	result = afc_string_basename(NULL);
	print_res("afc_string_basename(NULL)", NULL, result, 1);

	/* afc_string_temp with NULL (uses default path) */
	result = afc_string_temp(NULL);
	print_res("afc_string_temp(NULL)!=NULL", (void *)1, (void *)(result != NULL ? 1 : 0), 0);
	if (result) afc_string_delete(result);

	/* afc_string_delete with NULL (should not crash) */
	{
		char *null_str = NULL;
		afc_string_delete(null_str);
		print_res("afc_string_delete(NULL)", (void *)1, (void *)(null_str == NULL ? 1 : 0), 0);
	}

	/* ===== Empty string handling ===== */

	/* Copy empty string */
	afc_string_copy(s, "", ALL);
	print_res("afc_string_copy(empty)", "", s, 1);
	print_res("afc_string_len(empty)", (void *)0, (void *)afc_string_len(s), 0);

	/* Add empty string */
	afc_string_copy(s, "test", ALL);
	afc_string_add(s, "", ALL);
	print_res("afc_string_add(empty)", "test", s, 1);

	/* Trim empty string */
	afc_string_clear(s);
	result = afc_string_trim(s);
	print_res("afc_string_trim(empty)", "", s, 1);

	/* ===== Boundary conditions ===== */

	/* String of exactly max length */
	s = afc_string_new(5);
	afc_string_copy(s, "12345", ALL);
	print_res("afc_string_copy(exact_max)", "12345", s, 1);
	print_res("afc_string_len(exact_max)", (void *)5, (void *)afc_string_len(s), 0);

	/* String longer than max (should truncate) */
	afc_string_copy(s, "1234567890", ALL);
	print_res("afc_string_copy(over_max)", "12345", s, 1);

	/* Add that would exceed max */
	afc_string_copy(s, "1234", ALL);
	afc_string_add(s, "567890", ALL);
	print_res("afc_string_add(over_max)", "12345", s, 1);

	/* ===== Radix validation ===== */

	/* radix < 2 should fail */
	print_res("afc_string_radix(0)", (void *)-1, (void *)(long)afc_string_radix(s, 10, 0), 0);
	print_res("afc_string_radix(1)", (void *)-1, (void *)(long)afc_string_radix(s, 10, 1), 0);

	/* radix > 64 should fail */
	print_res("afc_string_radix(65)", (void *)-1, (void *)(long)afc_string_radix(s, 10, 65), 0);

	/* radix with NULL dest should fail */
	print_res("afc_string_radix(NULL)", (void *)-1, (void *)(long)afc_string_radix(NULL, 10, 10), 0);

	/* Valid radix should work */
	print_res("afc_string_radix(10)", (void *)0, (void *)(long)afc_string_radix(s, 255, 16), 0);
	print_res("afc_string_radix(10) result", "ff", s, 1);

	/* ===== String comparison edge cases ===== */

	/* Compare equal strings */
	afc_string_copy(s, "test", ALL);
	t = afc_string_new(10);
	afc_string_copy(t, "test", ALL);
	print_res("afc_string_comp(equal)", (void *)0, (void *)afc_string_comp(s, t, ALL), 0);

	/* Compare with limit longer than string */
	print_res("afc_string_comp(limit>len)", (void *)0, (void *)afc_string_comp(s, t, 100), 0);

	/* Compare with limit 0 */
	print_res("afc_string_comp(limit=0)", (void *)0, (void *)afc_string_comp(s, t, 0), 0);

	/* ===== JS-like string functions ===== */

	/* char_at with negative index */
	afc_string_copy(s, "hello", ALL);
	print_res("afc_string_char_at(-1)", (void *)'o', (void *)(long)afc_string_char_at(s, -1), 0);
	print_res("afc_string_char_at(-5)", (void *)'h', (void *)(long)afc_string_char_at(s, -5), 0);
	print_res("afc_string_char_at(-6)", (void *)'\0', (void *)(long)afc_string_char_at(s, -6), 0);

	/* char_at with out of range index */
	print_res("afc_string_char_at(10)", (void *)'\0', (void *)(long)afc_string_char_at(s, 10), 0);
	print_res("afc_string_char_at(NULL)", (void *)'\0', (void *)(long)afc_string_char_at(NULL, 0), 0);

	/* starts_with / ends_with */
	print_res("starts_with('hello','hel')", (void *)1, (void *)afc_string_starts_with(s, "hel", 0), 0);
	print_res("starts_with('hello','ell')", (void *)0, (void *)afc_string_starts_with(s, "ell", 0), 0);
	print_res("starts_with('hello','ell',1)", (void *)1, (void *)afc_string_starts_with(s, "ell", 1), 0);
	print_res("ends_with('hello','llo')", (void *)1, (void *)afc_string_ends_with(s, "llo", ALL), 0);
	print_res("ends_with('hello','ell')", (void *)0, (void *)afc_string_ends_with(s, "ell", ALL), 0);

	/* starts_with / ends_with with NULL */
	print_res("starts_with(NULL,x)", (void *)0, (void *)afc_string_starts_with(NULL, "test", 0), 0);
	print_res("starts_with(x,NULL)", (void *)0, (void *)afc_string_starts_with(s, NULL, 0), 0);
	print_res("ends_with(NULL,x)", (void *)0, (void *)afc_string_ends_with(NULL, "test", ALL), 0);
	print_res("ends_with(x,NULL)", (void *)0, (void *)afc_string_ends_with(s, NULL, ALL), 0);

	/* repeat */
	result = afc_string_new(50);
	afc_string_repeat(result, "ab", 3);
	print_res("afc_string_repeat('ab',3)", "ababab", result, 1);

	/* repeat with count 0 */
	afc_string_repeat(result, "test", 0);
	print_res("afc_string_repeat('test',0)", "", result, 1);

	/* replace */
	afc_string_replace(result, "hello world", "world", "there");
	print_res("afc_string_replace", "hello there", result, 1);

	/* replace with no match */
	afc_string_replace(result, "hello world", "xyz", "there");
	print_res("afc_string_replace(no match)", "hello world", result, 1);

	/* replace_all */
	afc_string_replace_all(result, "aaa", "a", "bb");
	print_res("afc_string_replace_all", "bbbbbb", result, 1);

	/* slice */
	afc_string_slice(result, "hello", 1, 4);
	print_res("afc_string_slice(1,4)", "ell", result, 1);

	/* slice with negative indices */
	afc_string_slice(result, "hello", -3, -1);
	print_res("afc_string_slice(-3,-1)", "ll", result, 1);

	/* index_of */
	print_res("index_of('hello','ll')", (void *)2, (void *)afc_string_index_of("hello", "ll", 0), 0);
	print_res("index_of('hello','xyz')", (void *)-1, (void *)afc_string_index_of("hello", "xyz", 0), 0);

	/* last_index_of */
	print_res("last_index_of('hello','l')", (void *)3, (void *)afc_string_last_index_of("hello", "l", -1), 0);

	/* pad_start */
	afc_string_pad_start(result, "5", 3, "0");
	print_res("afc_string_pad_start", "005", result, 1);

	/* pad_end */
	afc_string_pad_end(result, "5", 3, "0");
	print_res("afc_string_pad_end", "500", result, 1);

	/* from_char_code */
	afc_string_delete(result);
	result = afc_string_from_char_code(65);
	print_res("afc_string_from_char_code(65)", "A", result, 1);

	/* trim_start / trim_end */
	afc_string_copy(result, "  hello  ", ALL);
	afc_string_trim_start(result);
	print_res("afc_string_trim_start", "hello  ", result, 1);

	afc_string_copy(result, "  hello  ", ALL);
	afc_string_trim_end(result);
	print_res("afc_string_trim_end", "  hello", result, 1);

	/* Cleanup */
	afc_string_delete(s);
	afc_string_delete(t);
	afc_string_delete(result);

	print_summary();

	afc_delete(afc);

	return (0);
}
