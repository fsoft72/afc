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

/**
 * test_regexp.c - Comprehensive tests for the RegExp class.
 *
 * Tests cover:
 *   - Object creation and deletion
 *   - Compile valid and invalid patterns
 *   - Match and no-match scenarios
 *   - Capture groups via get_sub_string()
 *   - Replace with substitution (single and global)
 *   - Case-insensitive matching via set_options()
 *   - Clear and re-use
 *   - Matching with start position
 */

#include "test_utils.h"
#include "../src/regexp.h"
#include "../src/string.h"

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ---- Test 1: Object creation ---- */
	RegExp *re = afc_regexp_new();
	print_res("regexp_new() not NULL",
		(void *)(long)1,
		(void *)(long)(re != NULL),
		0);

	print_row();

	/* ---- Test 2: Compile a valid pattern ---- */
	{
		int res = afc_regexp_compile(re, "hello");
		print_res("compile 'hello' -> OK",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
	}

	/* ---- Test 3: Match against a string containing the pattern ---- */
	{
		int res = afc_regexp_match(re, "say hello world", 0);
		print_res("match 'say hello world'",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
	}

	/* ---- Test 4: No match against a string without the pattern ---- */
	{
		int res = afc_regexp_match(re, "goodbye world", 0);
		print_res("no match 'goodbye world'",
			(void *)(long)AFC_REGEXP_ERR_NO_MATCH,
			(void *)(long)res,
			0);
	}

	print_row();

	/* ---- Test 5: Compile pattern with capture groups ---- */
	{
		int res = afc_regexp_compile(re, "(\\w+)@(\\w+)\\.(\\w+)");
		print_res("compile email pattern",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
	}

	/* ---- Test 6: Match and extract capture groups ---- */
	{
		int res = afc_regexp_match(re, "contact user@example.com please", 0);
		print_res("match email string",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		/* Get the full match (group 0) */
		char *sub0 = afc_string_new(256);
		afc_regexp_get_sub_string(re, sub0, 0);
		print_res("group 0: full match",
			(void *)"user@example.com",
			(void *)sub0,
			1);

		/* Get capture group 1 (username) */
		char *sub1 = afc_string_new(256);
		afc_regexp_get_sub_string(re, sub1, 1);
		print_res("group 1: username",
			(void *)"user",
			(void *)sub1,
			1);

		/* Get capture group 2 (domain) */
		char *sub2 = afc_string_new(256);
		afc_regexp_get_sub_string(re, sub2, 2);
		print_res("group 2: domain",
			(void *)"example",
			(void *)sub2,
			1);

		/* Get capture group 3 (tld) */
		char *sub3 = afc_string_new(256);
		afc_regexp_get_sub_string(re, sub3, 3);
		print_res("group 3: tld",
			(void *)"com",
			(void *)sub3,
			1);

		afc_string_delete(sub0);
		afc_string_delete(sub1);
		afc_string_delete(sub2);
		afc_string_delete(sub3);
	}

	print_row();

	/* ---- Test 7: Replace - single occurrence ---- */
	{
		char *dest = afc_string_new(1024);
		int res = afc_regexp_replace(re, dest, "cat and dog", "cat", "bird", FALSE);
		print_res("replace single",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
		print_res("replaced 'cat'->'bird'",
			(void *)"bird and dog",
			(void *)dest,
			1);
		afc_string_delete(dest);
	}

	/* ---- Test 8: Replace - global (replace all) ---- */
	{
		char *dest = afc_string_new(1024);
		int res = afc_regexp_replace(re, dest, "aaa bbb aaa ccc aaa", "aaa", "xxx", TRUE);
		print_res("replace all",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
		print_res("all 'aaa'->'xxx'",
			(void *)"xxx bbb xxx ccc xxx",
			(void *)dest,
			1);
		afc_string_delete(dest);
	}

	/* ---- Test 9: Replace with no match (string unchanged) ---- */
	{
		char *dest = afc_string_new(1024);
		afc_regexp_replace(re, dest, "hello world", "zzz", "yyy", FALSE);
		print_res("replace no match",
			(void *)"hello world",
			(void *)dest,
			1);
		afc_string_delete(dest);
	}

	print_row();

	/* ---- Test 10: Case-insensitive matching ---- */
	{
		afc_regexp_set_options(re, AFC_REGEXP_OPT_NOCASE);
		int res = afc_regexp_compile(re, "hello");
		print_res("compile nocase 'hello'",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		res = afc_regexp_match(re, "HELLO World", 0);
		print_res("match 'HELLO' nocase",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		/* Verify the matched text */
		char *sub = afc_string_new(256);
		afc_regexp_get_sub_string(re, sub, 0);
		print_res("nocase match text",
			(void *)"HELLO",
			(void *)sub,
			1);
		afc_string_delete(sub);

		/* Reset options back to default (case sensitive) */
		afc_regexp_set_options(re, 0);
	}

	print_row();

	/* ---- Test 11: Clear and re-use ---- */
	{
		int res = afc_regexp_clear(re);
		print_res("clear -> NO_ERROR",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		/* After clear, compile a new pattern */
		res = afc_regexp_compile(re, "\\d+");
		print_res("compile '\\d+' after clear",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		/* Match a string with digits */
		res = afc_regexp_match(re, "abc 123 def", 0);
		print_res("match digits",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		char *sub = afc_string_new(256);
		afc_regexp_get_sub_string(re, sub, 0);
		print_res("digit match -> '123'",
			(void *)"123",
			(void *)sub,
			1);
		afc_string_delete(sub);
	}

	print_row();

	/* ---- Test 12: Match with start position offset ---- */
	{
		afc_regexp_compile(re, "\\d+");

		/* First match starting from 0 */
		int res = afc_regexp_match(re, "abc 12 def 34 ghi", 0);
		print_res("first match at pos 0",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		char *sub = afc_string_new(256);
		afc_regexp_get_sub_string(re, sub, 0);
		print_res("first digits -> '12'",
			(void *)"12",
			(void *)sub,
			1);

		/* Get position of first match to skip past it */
		RegExpPos pos;
		afc_regexp_get_pos(re, 0, &pos);

		/* Second match starting after the first */
		res = afc_regexp_match(re, "abc 12 def 34 ghi", pos.end);
		print_res("second match at offset",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		afc_regexp_get_sub_string(re, sub, 0);
		print_res("second digits -> '34'",
			(void *)"34",
			(void *)sub,
			1);

		afc_string_delete(sub);
	}

	/* ---- Test 13: Match with NULL/empty string ---- */
	{
		afc_regexp_compile(re, "test");
		int res = afc_regexp_match(re, NULL, 0);
		print_res("match NULL string",
			(void *)(long)1,
			(void *)(long)res,
			0);

		res = afc_regexp_match(re, "", 0);
		print_res("match empty string",
			(void *)(long)1,
			(void *)(long)res,
			0);
	}

	print_summary();

	/* Cleanup */
	afc_regexp_delete(re);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
