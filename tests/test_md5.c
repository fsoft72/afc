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
 * test_md5.c - Comprehensive tests for the MD5 hashing class.
 *
 * Tests cover:
 *   - Object creation and deletion
 *   - MD5 of empty string
 *   - MD5 of "abc"
 *   - MD5 of "hello world"
 *   - MD5 of a single character
 *   - MD5 of a longer known string
 *   - afc_md5_clear() and re-use of the same instance
 *   - Incremental updates (feeding data in chunks)
 */

#include "test_utils.h"
#include "../src/md5.h"

/* Well-known MD5 test vectors (RFC 1321) */
#define MD5_EMPTY        "d41d8cd98f00b204e9800998ecf8427e"
#define MD5_ABC          "900150983cd24fb0d6963f7d28e17f72"
#define MD5_HELLO_WORLD  "5eb63bbbe01eeed093cb22bb8f5acdc3"
#define MD5_SINGLE_A     "0cc175b9c0f1b6a831c399e269772661"
#define MD5_MESSAGE      "f96b697d7cb7938d525a2f31aaf161d0"  /* MD5("message digest") */
#define MD5_ALPHA_LOWER  "c3fcd3d76192e4007dfb496cca67e13b"  /* MD5("abcdefghijklmnopqrstuvwxyz") */

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	const char *digest;

	/* ---- Test 1: Object creation ---- */
	MD5 *m = afc_md5_new();
	print_res("md5_new() not NULL",
		(void *)(long)1,
		(void *)(long)(m != NULL),
		0);

	print_row();

	/* ---- Test 2: MD5 of empty string ---- */
	/* Feed zero bytes, then digest immediately */
	digest = afc_md5_digest(m);
	print_res("MD5('') empty string",
		(void *)MD5_EMPTY,
		(void *)digest,
		1);

	/* ---- Test 3: Clear and compute MD5 of "abc" ---- */
	afc_md5_clear(m);
	afc_md5_update(m, (unsigned char *)"abc", 3);
	digest = afc_md5_digest(m);
	print_res("MD5('abc')",
		(void *)MD5_ABC,
		(void *)digest,
		1);

	/* ---- Test 4: MD5 of "hello world" ---- */
	afc_md5_clear(m);
	afc_md5_update(m, (unsigned char *)"hello world", 11);
	digest = afc_md5_digest(m);
	print_res("MD5('hello world')",
		(void *)MD5_HELLO_WORLD,
		(void *)digest,
		1);

	print_row();

	/* ---- Test 5: MD5 of single character "a" ---- */
	afc_md5_clear(m);
	afc_md5_update(m, (unsigned char *)"a", 1);
	digest = afc_md5_digest(m);
	print_res("MD5('a')",
		(void *)MD5_SINGLE_A,
		(void *)digest,
		1);

	/* ---- Test 6: MD5 of "message digest" ---- */
	afc_md5_clear(m);
	afc_md5_update(m, (unsigned char *)"message digest", 14);
	digest = afc_md5_digest(m);
	print_res("MD5('message digest')",
		(void *)MD5_MESSAGE,
		(void *)digest,
		1);

	/* ---- Test 7: MD5 of full lowercase alphabet ---- */
	afc_md5_clear(m);
	afc_md5_update(m, (unsigned char *)"abcdefghijklmnopqrstuvwxyz", 26);
	digest = afc_md5_digest(m);
	print_res("MD5('a..z')",
		(void *)MD5_ALPHA_LOWER,
		(void *)digest,
		1);

	print_row();

	/* ---- Test 8: Clear and re-use - verify state reset ---- */
	/* After computing a hash, clear should reset state to produce correct results */
	afc_md5_clear(m);
	digest = afc_md5_digest(m);
	print_res("clear then digest -> empty",
		(void *)MD5_EMPTY,
		(void *)digest,
		1);

	/* ---- Test 9: Incremental update - feed data in chunks ---- */
	/* MD5("hello world") should be the same whether fed in one call or two */
	afc_md5_clear(m);
	afc_md5_update(m, (unsigned char *)"hello", 5);
	afc_md5_update(m, (unsigned char *)" ", 1);
	afc_md5_update(m, (unsigned char *)"world", 5);
	digest = afc_md5_digest(m);
	print_res("incremental 'hello world'",
		(void *)MD5_HELLO_WORLD,
		(void *)digest,
		1);

	/* ---- Test 10: Incremental update - single byte at a time ---- */
	afc_md5_clear(m);
	afc_md5_update(m, (unsigned char *)"a", 1);
	afc_md5_update(m, (unsigned char *)"b", 1);
	afc_md5_update(m, (unsigned char *)"c", 1);
	digest = afc_md5_digest(m);
	print_res("incremental 'a'+'b'+'c'",
		(void *)MD5_ABC,
		(void *)digest,
		1);

	/* ---- Test 11: afc_md5_clear() returns AFC_ERR_NO_ERROR ---- */
	{
		int res = afc_md5_clear(m);
		print_res("clear returns NO_ERROR",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
	}

	print_summary();

	/* Cleanup */
	afc_md5_delete(m);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
