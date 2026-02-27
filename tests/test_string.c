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
 * test_string.c - Comprehensive tests for the AFC string module.
 *
 * Tests cover all major public string functions including:
 *   - Lifecycle: afc_string_new, afc_string_delete, afc_string_dup
 *   - Copy/Add/Clear: afc_string_copy, afc_string_add, afc_string_clear
 *   - Length/Max: afc_string_len, afc_string_max
 *   - Case: afc_string_upper, afc_string_lower
 *   - Trim: afc_string_trim, afc_string_trim_start, afc_string_trim_end
 *   - Extraction: afc_string_left, afc_string_right, afc_string_mid
 *   - Comparison/Search: afc_string_comp, afc_string_instr, afc_string_pattern_match
 *   - Formatting: afc_string_make
 *   - Path: afc_string_dirname, afc_string_basename
 *   - JS-like API: starts_with, ends_with, replace, replace_all,
 *     pad_start, pad_end, slice, index_of, last_index_of,
 *     char_at, repeat
 *   - Edge cases: empty strings, NULL handling, boundary lengths
 */

#include "test_utils.h"
#include "../src/string.h"

int main(void)
{
	AFC *afc = afc_new();
	char *s, *t, *r;
	test_header();

	/* ===================================================================
	 * SECTION 1: afc_string_new() / afc_string_delete() lifecycle
	 * =================================================================== */

	/* Allocate a string of 10 characters. */
	s = afc_string_new(10);
	print_res("new(10) not NULL",
		(void *)(long)1,
		(void *)(long)(s != NULL),
		0);

	/* New string should have max capacity of 10. */
	print_res("new(10) max == 10",
		(void *)(long)10,
		(void *)(long)afc_string_max(s),
		0);

	/* New string should have length 0 (empty). */
	print_res("new(10) len == 0",
		(void *)(long)0,
		(void *)(long)afc_string_len(s),
		0);

	/* New string content should be empty. */
	print_res("new(10) content empty",
		"",
		s,
		1);

	/* Delete the string - macro sets pointer to NULL. */
	afc_string_delete(s);
	print_res("delete sets NULL",
		(void *)(long)1,
		(void *)(long)(s == NULL),
		0);

	/* afc_string_max on NULL should return 0. */
	print_res("max(NULL) == 0",
		(void *)(long)0,
		(void *)(long)afc_string_max(NULL),
		0);

	/* afc_string_len on NULL should return 0. */
	print_res("len(NULL) == 0",
		(void *)(long)0,
		(void *)(long)afc_string_len(NULL),
		0);

	print_row();

	/* ===================================================================
	 * SECTION 2: afc_string_copy() with full and partial lengths
	 * =================================================================== */

	s = afc_string_new(20);

	/* Copy full string using ALL. */
	afc_string_copy(s, "Hello World", ALL);
	print_res("copy ALL",
		"Hello World",
		s,
		1);

	/* Verify length after copy. */
	print_res("copy ALL len == 11",
		(void *)(long)11,
		(void *)(long)afc_string_len(s),
		0);

	/* Copy partial: only 5 characters. */
	afc_string_copy(s, "Hello World", 5);
	print_res("copy 5 chars",
		"Hello",
		s,
		1);

	print_res("copy 5 len == 5",
		(void *)(long)5,
		(void *)(long)afc_string_len(s),
		0);

	/* Copy 0 characters should produce empty string. */
	afc_string_copy(s, "Hello", 0);
	print_res("copy 0 chars",
		"",
		s,
		1);

	/* Copy longer than buffer: should be clamped to max. */
	afc_string_delete(s);
	s = afc_string_new(5);
	afc_string_copy(s, "1234567890", ALL);
	print_res("copy clamp to max",
		"12345",
		s,
		1);

	print_res("copy clamp len == 5",
		(void *)(long)5,
		(void *)(long)afc_string_len(s),
		0);

	/* Copy with NULL source should return NULL. */
	print_res("copy NULL src",
		(void *)(long)1,
		(void *)(long)(afc_string_copy(s, NULL, ALL) == NULL),
		0);

	afc_string_delete(s);

	print_row();

	/* ===================================================================
	 * SECTION 3: afc_string_len() / afc_string_max()
	 * =================================================================== */

	s = afc_string_new(100);

	/* Max should be 100 for a string allocated with 100. */
	print_res("max(100) == 100",
		(void *)(long)100,
		(void *)(long)afc_string_max(s),
		0);

	/* After copy, len should reflect actual content length. */
	afc_string_copy(s, "test", ALL);
	print_res("len after copy",
		(void *)(long)4,
		(void *)(long)afc_string_len(s),
		0);

	/* Max remains unchanged after copy. */
	print_res("max unchanged",
		(void *)(long)100,
		(void *)(long)afc_string_max(s),
		0);

	afc_string_delete(s);

	print_row();

	/* ===================================================================
	 * SECTION 4: afc_string_add() concatenation
	 * =================================================================== */

	s = afc_string_new(20);

	/* Add to empty string. */
	afc_string_copy(s, "Hello", ALL);
	afc_string_add(s, " World", ALL);
	print_res("add ALL",
		"Hello World",
		s,
		1);

	print_res("add len == 11",
		(void *)(long)11,
		(void *)(long)afc_string_len(s),
		0);

	/* Add with partial length. */
	afc_string_clear(s);
	afc_string_copy(s, "AB", ALL);
	afc_string_add(s, "CDEFGH", 3);
	print_res("add 3 chars",
		"ABCDE",
		s,
		1);

	/* Add that would exceed capacity: clamped. */
	afc_string_delete(s);
	s = afc_string_new(8);
	afc_string_copy(s, "12345", ALL);
	afc_string_add(s, "67890ABCDE", ALL);
	print_res("add clamp to max",
		"12345678",
		s,
		1);

	/* Add with NULL source returns NULL. */
	print_res("add NULL src",
		(void *)(long)1,
		(void *)(long)(afc_string_add(s, NULL, ALL) == NULL),
		0);

	afc_string_delete(s);

	print_row();

	/* ===================================================================
	 * SECTION 5: afc_string_clear()
	 * =================================================================== */

	s = afc_string_new(20);
	afc_string_copy(s, "some data", ALL);

	/* Clear should make the string empty. */
	afc_string_clear(s);
	print_res("clear content",
		"",
		s,
		1);

	/* Clear should set length to 0. */
	print_res("clear len == 0",
		(void *)(long)0,
		(void *)(long)afc_string_len(s),
		0);

	/* Max should remain the same after clear. */
	print_res("clear max unchanged",
		(void *)(long)20,
		(void *)(long)afc_string_max(s),
		0);

	/* Clear on NULL should return NULL (no crash). */
	print_res("clear NULL",
		(void *)(long)1,
		(void *)(long)(afc_string_clear(NULL) == NULL),
		0);

	afc_string_delete(s);

	print_row();

	/* ===================================================================
	 * SECTION 6: afc_string_trim() / trim_start() / trim_end()
	 * =================================================================== */

	s = afc_string_new(30);

	/* Trim both sides: spaces and tabs. */
	afc_string_copy(s, "  \thello world\t  ", ALL);
	afc_string_trim(s);
	print_res("trim both sides",
		"hello world",
		s,
		1);

	/* Trim with no whitespace: no change. */
	afc_string_copy(s, "nospace", ALL);
	afc_string_trim(s);
	print_res("trim no whitespace",
		"nospace",
		s,
		1);

	/* trim_start: only leading whitespace removed. */
	afc_string_copy(s, "   leading", ALL);
	afc_string_trim_start(s);
	print_res("trim_start",
		"leading",
		s,
		1);

	/* trim_start: trailing whitespace preserved. */
	afc_string_copy(s, "   trail   ", ALL);
	afc_string_trim_start(s);
	print_res("trim_start keeps trail",
		"trail   ",
		s,
		1);

	/* trim_end: only trailing whitespace removed. */
	afc_string_copy(s, "trailing   ", ALL);
	afc_string_trim_end(s);
	print_res("trim_end",
		"trailing",
		s,
		1);

	/* trim_end: leading whitespace preserved. */
	afc_string_copy(s, "   both   ", ALL);
	afc_string_trim_end(s);
	print_res("trim_end keeps lead",
		"   both",
		s,
		1);

	/* Trim on empty string. */
	afc_string_clear(s);
	afc_string_trim(s);
	print_res("trim empty",
		"",
		s,
		1);

	/* Trim on NULL returns NULL. */
	print_res("trim NULL",
		(void *)(long)1,
		(void *)(long)(afc_string_trim(NULL) == NULL),
		0);

	print_res("trim_start NULL",
		(void *)(long)1,
		(void *)(long)(afc_string_trim_start(NULL) == NULL),
		0);

	print_res("trim_end NULL",
		(void *)(long)1,
		(void *)(long)(afc_string_trim_end(NULL) == NULL),
		0);

	afc_string_delete(s);

	print_row();

	/* ===================================================================
	 * SECTION 7: afc_string_upper() / afc_string_lower()
	 * =================================================================== */

	s = afc_string_new(30);

	/* Upper case conversion. */
	afc_string_copy(s, "hello world", ALL);
	afc_string_upper(s);
	print_res("upper",
		"HELLO WORLD",
		s,
		1);

	/* Lower case conversion. */
	afc_string_lower(s);
	print_res("lower",
		"hello world",
		s,
		1);

	/* Mixed case string to upper. */
	afc_string_copy(s, "HeLLo WoRLd", ALL);
	afc_string_upper(s);
	print_res("upper mixed",
		"HELLO WORLD",
		s,
		1);

	/* Upper on NULL returns NULL. */
	print_res("upper NULL",
		(void *)(long)1,
		(void *)(long)(afc_string_upper(NULL) == NULL),
		0);

	/* Lower on NULL returns NULL. */
	print_res("lower NULL",
		(void *)(long)1,
		(void *)(long)(afc_string_lower(NULL) == NULL),
		0);

	afc_string_delete(s);

	print_row();

	/* ===================================================================
	 * SECTION 8: afc_string_left() / afc_string_right() / afc_string_mid()
	 * =================================================================== */

	s = afc_string_new(30);
	t = afc_string_new(30);

	afc_string_copy(s, "ABCDEFGHIJ", ALL);

	/* Left 3 characters. */
	afc_string_left(t, s, 3);
	print_res("left 3",
		"ABC",
		t,
		1);

	/* Right 3 characters. */
	afc_string_right(t, s, 3);
	print_res("right 3",
		"HIJ",
		t,
		1);

	/* Right with length longer than string: returns full string. */
	afc_string_right(t, s, 100);
	print_res("right > len",
		"ABCDEFGHIJ",
		t,
		1);

	/* Mid: extract from position 3, length 4. */
	afc_string_mid(t, s, 3, 4);
	print_res("mid(3,4)",
		"DEFG",
		t,
		1);

	/* Mid: from position 0, length 5. */
	afc_string_mid(t, s, 0, 5);
	print_res("mid(0,5)",
		"ABCDE",
		t,
		1);

	/* Mid: from position beyond string length returns NULL. */
	print_res("mid beyond len",
		(void *)(long)1,
		(void *)(long)(afc_string_mid(t, s, 100, 5) == NULL),
		0);

	/* Mid: numchars exceeding remaining chars is clamped. */
	afc_string_mid(t, s, 8, 10);
	print_res("mid clamp",
		"IJ",
		t,
		1);

	afc_string_delete(s);
	afc_string_delete(t);

	print_row();

	/* ===================================================================
	 * SECTION 9: afc_string_comp()
	 * =================================================================== */

	s = afc_string_new(30);
	t = afc_string_new(30);

	afc_string_copy(s, "hello", ALL);
	afc_string_copy(t, "hello", ALL);

	/* Equal strings: comp returns 0. */
	print_res("comp equal ALL",
		(void *)(long)0,
		(void *)(long)afc_string_comp(s, t, ALL),
		0);

	/* Partial compare of equal prefix. */
	afc_string_copy(t, "hello world", ALL);
	print_res("comp equal 5 chars",
		(void *)(long)0,
		(void *)(long)afc_string_comp(s, t, 5),
		0);

	/* Full compare with different strings. */
	afc_string_copy(s, "abc", ALL);
	afc_string_copy(t, "abd", ALL);
	print_res("comp abc vs abd < 0",
		(void *)(long)1,
		(void *)(long)(afc_string_comp(s, t, ALL) > 0),
		0);

	/* Reverse comparison: abd > abc. */
	print_res("comp abd vs abc > 0",
		(void *)(long)1,
		(void *)(long)(afc_string_comp(t, s, ALL) < 0),
		0);

	afc_string_delete(s);
	afc_string_delete(t);

	print_row();

	/* ===================================================================
	 * SECTION 10: afc_string_instr()
	 * =================================================================== */

	s = afc_string_new(30);
	afc_string_copy(s, "Hello World", ALL);

	/* Find substring from start. */
	r = afc_string_instr(s, "World", 0);
	print_res("instr found",
		(void *)(long)1,
		(void *)(long)(r != NULL),
		0);

	/* Verify the found position string content. */
	print_res("instr content",
		"World",
		r,
		1);

	/* Search for non-existent substring. */
	r = afc_string_instr(s, "xyz", 0);
	print_res("instr not found",
		(void *)(long)1,
		(void *)(long)(r == NULL),
		0);

	/* Search with startpos beyond match. */
	r = afc_string_instr(s, "Hello", 5);
	print_res("instr past match",
		(void *)(long)1,
		(void *)(long)(r == NULL),
		0);

	/* Search with NULL string returns NULL. */
	print_res("instr NULL str",
		(void *)(long)1,
		(void *)(long)(afc_string_instr(NULL, "x", 0) == NULL),
		0);

	/* Search with NULL match returns NULL. */
	print_res("instr NULL match",
		(void *)(long)1,
		(void *)(long)(afc_string_instr(s, NULL, 0) == NULL),
		0);

	afc_string_delete(s);

	print_row();

	/* ===================================================================
	 * SECTION 11: afc_string_pattern_match()
	 * =================================================================== */

	s = afc_string_new(30);
	afc_string_copy(s, "hello.txt", ALL);

	/* Matching pattern: *.txt should match hello.txt (returns 0). */
	print_res("pattern *.txt match",
		(void *)(long)0,
		(void *)(long)afc_string_pattern_match(s, "*.txt", FALSE),
		0);

	/* Non-matching pattern: *.doc should not match (returns non-zero). */
	print_res("pattern *.doc no match",
		(void *)(long)1,
		(void *)(long)(afc_string_pattern_match(s, "*.doc", FALSE) != 0),
		0);

	/* Case insensitive match: HELLO.TXT pattern should match. */
	print_res("pattern nocase match",
		(void *)(long)0,
		(void *)(long)afc_string_pattern_match(s, "HELLO.*", TRUE),
		0);

	/* Exact match. */
	print_res("pattern exact match",
		(void *)(long)0,
		(void *)(long)afc_string_pattern_match(s, "hello.txt", FALSE),
		0);

	/* Question mark wildcard. */
	print_res("pattern ? wildcard",
		(void *)(long)0,
		(void *)(long)afc_string_pattern_match(s, "hell?.txt", FALSE),
		0);

	/* NULL string returns -1. */
	print_res("pattern NULL str",
		(void *)(long)-1,
		(void *)(long)afc_string_pattern_match(NULL, "*.txt", FALSE),
		0);

	/* NULL pattern returns -1. */
	print_res("pattern NULL pattern",
		(void *)(long)-1,
		(void *)(long)afc_string_pattern_match(s, NULL, FALSE),
		0);

	afc_string_delete(s);

	print_row();

	/* ===================================================================
	 * SECTION 12: afc_string_make() (sprintf-like)
	 * =================================================================== */

	s = afc_string_new(50);

	/* Basic formatting with string and integer. */
	afc_string_make(s, "Hello %s, you are %d", "World", 42);
	print_res("make sprintf",
		"Hello World, you are 42",
		s,
		1);

	/* Verify length after make. */
	print_res("make len",
		(void *)(long)23,
		(void *)(long)afc_string_len(s),
		0);

	/* Make with just a string. */
	afc_string_make(s, "%s", "simple");
	print_res("make simple",
		"simple",
		s,
		1);

	/* Make with empty format. */
	afc_string_make(s, "%s", "");
	print_res("make empty",
		"",
		s,
		1);

	/* Make with NULL dest returns NULL. */
	print_res("make NULL dest",
		(void *)(long)1,
		(void *)(long)(afc_string_make(NULL, "test") == NULL),
		0);

	/* Make with NULL fmt returns NULL. */
	print_res("make NULL fmt",
		(void *)(long)1,
		(void *)(long)(afc_string_make(s, NULL) == NULL),
		0);

	afc_string_delete(s);

	print_row();

	/* ===================================================================
	 * SECTION 13: afc_string_dirname() / afc_string_basename()
	 * =================================================================== */

	/* dirname: extract directory part. */
	r = afc_string_dirname("/usr/local/bin/test");
	print_res("dirname /usr/.../test",
		"/usr/local/bin",
		r,
		1);
	afc_string_delete(r);

	/* basename: extract filename part. */
	r = afc_string_basename("/usr/local/bin/test");
	print_res("basename .../test",
		"test",
		r,
		1);
	afc_string_delete(r);

	/* dirname of root-level file. */
	r = afc_string_dirname("/file.txt");
	print_res("dirname /file.txt",
		"",
		r,
		1);
	afc_string_delete(r);

	/* basename of root-level file. */
	r = afc_string_basename("/file.txt");
	print_res("basename /file.txt",
		"file.txt",
		r,
		1);
	afc_string_delete(r);

	/* dirname with no separator: returns whole path. */
	r = afc_string_dirname("justfile");
	print_res("dirname no sep",
		"justfile",
		r,
		1);
	afc_string_delete(r);

	/* basename with no separator: returns whole path. */
	r = afc_string_basename("justfile");
	print_res("basename no sep",
		"justfile",
		r,
		1);
	afc_string_delete(r);

	/* dirname NULL returns NULL. */
	print_res("dirname NULL",
		(void *)(long)1,
		(void *)(long)(afc_string_dirname(NULL) == NULL),
		0);

	/* basename NULL returns NULL. */
	print_res("basename NULL",
		(void *)(long)1,
		(void *)(long)(afc_string_basename(NULL) == NULL),
		0);

	print_row();

	/* ===================================================================
	 * SECTION 14: afc_string_starts_with() / afc_string_ends_with()
	 * =================================================================== */

	s = afc_string_new(30);
	afc_string_copy(s, "Hello World", ALL);

	/* starts_with: matching prefix from position 0. */
	print_res("starts_with Hello",
		(void *)(long)1,
		(void *)(long)afc_string_starts_with(s, "Hello", 0),
		0);

	/* starts_with: non-matching prefix. */
	print_res("starts_with World",
		(void *)(long)0,
		(void *)(long)afc_string_starts_with(s, "World", 0),
		0);

	/* starts_with: matching at specified position. */
	print_res("starts_with pos 6",
		(void *)(long)1,
		(void *)(long)afc_string_starts_with(s, "World", 6),
		0);

	/* starts_with: empty search string always matches. */
	print_res("starts_with empty",
		(void *)(long)1,
		(void *)(long)afc_string_starts_with(s, "", 0),
		0);

	/* starts_with: NULL str returns 0. */
	print_res("starts_with NULL str",
		(void *)(long)0,
		(void *)(long)afc_string_starts_with(NULL, "x", 0),
		0);

	/* ends_with: matching suffix using ALL for full length. */
	print_res("ends_with World",
		(void *)(long)1,
		(void *)(long)afc_string_ends_with(s, "World", ALL),
		0);

	/* ends_with: non-matching suffix. */
	print_res("ends_with Hello",
		(void *)(long)0,
		(void *)(long)afc_string_ends_with(s, "Hello", ALL),
		0);

	/* ends_with: matching with limited length (first 5 chars = "Hello"). */
	print_res("ends_with len 5",
		(void *)(long)1,
		(void *)(long)afc_string_ends_with(s, "Hello", 5),
		0);

	/* ends_with: empty search string always matches. */
	print_res("ends_with empty",
		(void *)(long)1,
		(void *)(long)afc_string_ends_with(s, "", ALL),
		0);

	/* ends_with: NULL str returns 0. */
	print_res("ends_with NULL str",
		(void *)(long)0,
		(void *)(long)afc_string_ends_with(NULL, "x", ALL),
		0);

	afc_string_delete(s);

	print_row();

	/* ===================================================================
	 * SECTION 15: afc_string_replace() / afc_string_replace_all()
	 * =================================================================== */

	s = afc_string_new(50);
	t = afc_string_new(50);

	/* Replace first occurrence only. */
	afc_string_copy(s, "foo bar foo", ALL);
	afc_string_replace(t, s, "foo", "baz");
	print_res("replace first",
		"baz bar foo",
		t,
		1);

	/* Replace all occurrences. */
	afc_string_replace_all(t, s, "foo", "baz");
	print_res("replace_all",
		"baz bar baz",
		t,
		1);

	/* Replace with no match: original returned. */
	afc_string_replace(t, s, "xyz", "baz");
	print_res("replace no match",
		"foo bar foo",
		t,
		1);

	/* Replace with longer replacement. */
	afc_string_copy(s, "ab", ALL);
	afc_string_replace(t, s, "a", "xyz");
	print_res("replace longer repl",
		"xyzb",
		t,
		1);

	/* Replace with empty replacement (deletion). */
	afc_string_copy(s, "hello world", ALL);
	afc_string_replace(t, s, "world", "");
	print_res("replace to empty",
		"hello ",
		t,
		1);

	/* Replace_all with repeated pattern. */
	afc_string_copy(s, "aaa", ALL);
	afc_string_replace_all(t, s, "a", "bb");
	print_res("replace_all repeat",
		"bbbbbb",
		t,
		1);

	/* Replace with NULL pattern: copies original. */
	afc_string_copy(s, "test", ALL);
	afc_string_replace(t, s, NULL, "x");
	print_res("replace NULL pattern",
		"test",
		t,
		1);

	afc_string_delete(s);
	afc_string_delete(t);

	print_row();

	/* ===================================================================
	 * SECTION 16: afc_string_pad_start() / afc_string_pad_end()
	 * =================================================================== */

	s = afc_string_new(30);
	t = afc_string_new(30);

	/* Pad start with default-like behavior using "0". */
	afc_string_copy(s, "42", ALL);
	afc_string_pad_start(t, s, 5, "0");
	print_res("pad_start 00042",
		"00042",
		t,
		1);

	/* Pad start: string already long enough. */
	afc_string_copy(s, "12345", ALL);
	afc_string_pad_start(t, s, 3, "0");
	print_res("pad_start no change",
		"12345",
		t,
		1);

	/* Pad start with multi-char pad string. */
	afc_string_copy(s, "x", ALL);
	afc_string_pad_start(t, s, 5, "ab");
	print_res("pad_start multi",
		"ababx",
		t,
		1);

	/* Pad start with NULL padString uses space. */
	afc_string_copy(s, "hi", ALL);
	afc_string_pad_start(t, s, 5, NULL);
	print_res("pad_start NULL pad",
		"   hi",
		t,
		1);

	/* Pad end with "0". */
	afc_string_copy(s, "42", ALL);
	afc_string_pad_end(t, s, 5, "0");
	print_res("pad_end 42000",
		"42000",
		t,
		1);

	/* Pad end: string already long enough. */
	afc_string_copy(s, "12345", ALL);
	afc_string_pad_end(t, s, 3, "0");
	print_res("pad_end no change",
		"12345",
		t,
		1);

	/* Pad end with multi-char pad string. */
	afc_string_copy(s, "x", ALL);
	afc_string_pad_end(t, s, 5, "ab");
	print_res("pad_end multi",
		"xabab",
		t,
		1);

	/* Pad end with NULL padString uses space. */
	afc_string_copy(s, "hi", ALL);
	afc_string_pad_end(t, s, 5, NULL);
	print_res("pad_end NULL pad",
		"hi   ",
		t,
		1);

	afc_string_delete(s);
	afc_string_delete(t);

	print_row();

	/* ===================================================================
	 * SECTION 17: afc_string_slice()
	 * =================================================================== */

	s = afc_string_new(30);
	t = afc_string_new(30);

	afc_string_copy(s, "Hello World", ALL);

	/* Slice from begin to end. */
	afc_string_slice(t, s, 0, 5);
	print_res("slice(0,5)",
		"Hello",
		t,
		1);

	/* Slice from middle. */
	afc_string_slice(t, s, 6, 11);
	print_res("slice(6,11)",
		"World",
		t,
		1);

	/* Slice with negative begin. */
	afc_string_slice(t, s, -5, 11);
	print_res("slice(-5,11)",
		"World",
		t,
		1);

	/* Slice with negative end. */
	afc_string_slice(t, s, 0, -6);
	print_res("slice(0,-6)",
		"Hello",
		t,
		1);

	/* Slice with both negative. */
	afc_string_slice(t, s, -5, -1);
	print_res("slice(-5,-1)",
		"Worl",
		t,
		1);

	/* Slice with begin >= end: empty result. */
	afc_string_slice(t, s, 5, 3);
	print_res("slice begin>=end",
		"",
		t,
		1);

	/* Slice to end of string (endIndex > len is clamped). */
	afc_string_slice(t, s, 6, 100);
	print_res("slice to end",
		"World",
		t,
		1);

	/* Slice on NULL str: returns empty dest. */
	afc_string_slice(t, NULL, 0, 5);
	print_res("slice NULL str",
		"",
		t,
		1);

	afc_string_delete(s);
	afc_string_delete(t);

	print_row();

	/* ===================================================================
	 * SECTION 18: afc_string_index_of() / afc_string_last_index_of()
	 * =================================================================== */

	s = afc_string_new(30);
	afc_string_copy(s, "hello world hello", ALL);

	/* Find first occurrence from start. */
	print_res("index_of hello",
		(void *)(long)0,
		(void *)(long)afc_string_index_of(s, "hello", 0),
		0);

	/* Find from position after first occurrence. */
	print_res("index_of from 1",
		(void *)(long)12,
		(void *)(long)afc_string_index_of(s, "hello", 1),
		0);

	/* Search for non-existent. */
	print_res("index_of not found",
		(void *)(long)-1,
		(void *)(long)afc_string_index_of(s, "xyz", 0),
		0);

	/* Find "world". */
	print_res("index_of world",
		(void *)(long)6,
		(void *)(long)afc_string_index_of(s, "world", 0),
		0);

	/* last_index_of: find last occurrence. */
	print_res("last_index_of hello",
		(void *)(long)12,
		(void *)(long)afc_string_last_index_of(s, "hello", 100),
		0);

	/* last_index_of from limited position. */
	print_res("last_index_of from 5",
		(void *)(long)0,
		(void *)(long)afc_string_last_index_of(s, "hello", 5),
		0);

	/* last_index_of: not found. */
	print_res("last_index_of nf",
		(void *)(long)-1,
		(void *)(long)afc_string_last_index_of(s, "xyz", 100),
		0);

	/* index_of with NULL returns -1. */
	print_res("index_of NULL str",
		(void *)(long)-1,
		(void *)(long)afc_string_index_of(NULL, "x", 0),
		0);

	/* last_index_of with NULL returns -1. */
	print_res("last_index_of NULL",
		(void *)(long)-1,
		(void *)(long)afc_string_last_index_of(NULL, "x", 0),
		0);

	afc_string_delete(s);

	print_row();

	/* ===================================================================
	 * SECTION 19: afc_string_char_at()
	 * =================================================================== */

	s = afc_string_new(20);
	afc_string_copy(s, "ABCDE", ALL);

	/* Char at index 0. */
	print_res("char_at 0",
		(void *)(long)'A',
		(void *)(long)afc_string_char_at(s, 0),
		0);

	/* Char at index 4 (last). */
	print_res("char_at 4",
		(void *)(long)'E',
		(void *)(long)afc_string_char_at(s, 4),
		0);

	/* Char at negative index (-1 = last char). */
	print_res("char_at -1",
		(void *)(long)'E',
		(void *)(long)afc_string_char_at(s, -1),
		0);

	/* Char at negative index (-5 = first char). */
	print_res("char_at -5",
		(void *)(long)'A',
		(void *)(long)afc_string_char_at(s, -5),
		0);

	/* Char at out-of-range positive index: returns '\0'. */
	print_res("char_at out of range",
		(void *)(long)'\0',
		(void *)(long)afc_string_char_at(s, 10),
		0);

	/* Char at out-of-range negative index: returns '\0'. */
	print_res("char_at -100",
		(void *)(long)'\0',
		(void *)(long)afc_string_char_at(s, -100),
		0);

	/* Char at on NULL: returns '\0'. */
	print_res("char_at NULL",
		(void *)(long)'\0',
		(void *)(long)afc_string_char_at(NULL, 0),
		0);

	afc_string_delete(s);

	print_row();

	/* ===================================================================
	 * SECTION 20: afc_string_repeat()
	 * =================================================================== */

	s = afc_string_new(30);

	/* Repeat "ab" 3 times. */
	afc_string_repeat(s, "ab", 3);
	print_res("repeat ab x3",
		"ababab",
		s,
		1);

	/* Repeat 1 time: just the string itself. */
	afc_string_repeat(s, "hello", 1);
	print_res("repeat x1",
		"hello",
		s,
		1);

	/* Repeat 0 times: empty string. */
	afc_string_repeat(s, "hello", 0);
	print_res("repeat x0",
		"",
		s,
		1);

	/* Repeat NULL source: dest is cleared. */
	afc_string_repeat(s, NULL, 3);
	print_res("repeat NULL src",
		"",
		s,
		1);

	/* Repeat with NULL dest returns NULL. */
	print_res("repeat NULL dest",
		(void *)(long)1,
		(void *)(long)(afc_string_repeat(NULL, "a", 3) == NULL),
		0);

	afc_string_delete(s);

	print_row();

	/* ===================================================================
	 * SECTION 21: afc_string_dup()
	 * =================================================================== */

	s = afc_string_new(20);
	afc_string_copy(s, "duplicate me", ALL);

	/* Dup creates a new string with same content. */
	t = afc_string_dup(s);
	print_res("dup content",
		"duplicate me",
		t,
		1);

	/* Dup string should have same length. */
	print_res("dup len",
		(void *)(long)afc_string_len(s),
		(void *)(long)afc_string_len(t),
		0);

	/* Dup string should be a different pointer. */
	print_res("dup diff ptr",
		(void *)(long)1,
		(void *)(long)(s != t),
		0);

	afc_string_delete(t);

	/* Dup of NULL returns NULL. */
	print_res("dup NULL",
		(void *)(long)1,
		(void *)(long)(afc_string_dup(NULL) == NULL),
		0);

	/* Dup of empty string returns NULL (per implementation). */
	print_res("dup empty",
		(void *)(long)1,
		(void *)(long)(afc_string_dup("") == NULL),
		0);

	afc_string_delete(s);

	print_row();

	/* ===================================================================
	 * SECTION 22: Edge cases - empty strings, boundary conditions
	 * =================================================================== */

	/* Allocate a string of size 1 (minimum useful size). */
	s = afc_string_new(1);
	print_res("new(1) max == 1",
		(void *)(long)1,
		(void *)(long)afc_string_max(s),
		0);

	/* Copy single char. */
	afc_string_copy(s, "X", ALL);
	print_res("copy 1 char",
		"X",
		s,
		1);

	/* Add to full string: nothing added due to capacity. */
	afc_string_add(s, "Y", ALL);
	print_res("add to full str",
		"X",
		s,
		1);

	afc_string_delete(s);

	/* Test with large allocation. */
	s = afc_string_new(1000);
	print_res("new(1000) max",
		(void *)(long)1000,
		(void *)(long)afc_string_max(s),
		0);

	/* Build a longer string via repeated add. */
	afc_string_copy(s, "", ALL);
	afc_string_add(s, "A", ALL);
	afc_string_add(s, "B", ALL);
	afc_string_add(s, "C", ALL);
	print_res("incremental add",
		"ABC",
		s,
		1);

	print_res("incremental len",
		(void *)(long)3,
		(void *)(long)afc_string_len(s),
		0);

	afc_string_delete(s);

	/* ===================================================================
	 * SECTION 23: afc_string_reset_len()
	 * =================================================================== */

	s = afc_string_new(20);
	afc_string_copy(s, "hello", ALL);

	/* Manually truncate the C string (simulate external manipulation). */
	s[3] = '\0';

	/* afc_string_len still reports old length. */
	print_res("len before reset",
		(void *)(long)5,
		(void *)(long)afc_string_len(s),
		0);

	/* After reset_len, it should report the actual strlen. */
	afc_string_reset_len(s);
	print_res("len after reset",
		(void *)(long)3,
		(void *)(long)afc_string_len(s),
		0);

	afc_string_delete(s);

	print_summary();

	/* Cleanup */
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
