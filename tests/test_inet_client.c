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
 * test_inet_client.c - Comprehensive tests for the InetClient networking module.
 *
 * Tests cover:
 *   - afc_inet_client_new() / afc_inet_client_delete() lifecycle
 *   - Verification that object is not NULL after creation
 *   - Magic number verification
 *   - afc_inet_client_set_tag() with AFC_INET_CLIENT_TAG_TIMEOUT
 *   - afc_inet_client_set_tag() with AFC_INET_CLIENT_TAG_USE_SSL
 *   - afc_inet_client_clear()
 *   - Multiple create/delete cycles for stability
 *
 * NOTE: No actual network connections are made in these tests.
 */

#include "test_utils.h"
#include "../src/inet_client.h"

/* Expected magic number computed from the 'ICLI' character sequence. */
#define EXPECTED_MAGIC ('I' << 24 | 'C' << 16 | 'L' << 8 | 'I')

/* Number of create/delete cycles for stability testing. */
#define CYCLE_COUNT 100

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ===== afc_inet_client_new() lifecycle tests ===== */

	/* Create a new InetClient and verify it is not NULL. */
	InetClient *ic = afc_inet_client_new();
	print_res("new() not NULL",
		(void *)(long)1,
		(void *)(long)(ic != NULL),
		0);

	/* Verify the magic number matches the expected value. */
	print_res("magic number correct",
		(void *)(long)EXPECTED_MAGIC,
		(void *)(long)ic->magic,
		0);

	/* Verify default SSL flag is FALSE. */
	print_res("default use_ssl FALSE",
		(void *)(long)FALSE,
		(void *)(long)ic->use_ssl,
		0);

	/* Verify default timeout is 0 (no timeout). */
	print_res("default timeout 0",
		(void *)(long)0,
		(void *)(long)ic->timeout,
		0);

	/* Verify default socket fd is -1 (not connected). */
	print_res("default sockfd -1",
		(void *)(long)-1,
		(void *)(long)ic->sockfd,
		0);

	/* Verify the internal buffer was allocated. */
	print_res("buf allocated",
		(void *)(long)1,
		(void *)(long)(ic->buf != NULL),
		0);

	print_row();

	/* ===== afc_inet_client_set_tag() tests ===== */

	/* Set timeout to 30 seconds and verify. */
	int res = afc_inet_client_set_tag(ic, AFC_INET_CLIENT_TAG_TIMEOUT, (void *)(long)30);
	print_res("set_tag TIMEOUT ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* Verify the timeout value was stored correctly. */
	print_res("timeout is 30",
		(void *)(long)30,
		(void *)(long)ic->timeout,
		0);

	/* Change timeout to 60 seconds and verify. */
	res = afc_inet_client_set_tag(ic, AFC_INET_CLIENT_TAG_TIMEOUT, (void *)(long)60);
	print_res("set_tag TIMEOUT 60 ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("timeout is 60",
		(void *)(long)60,
		(void *)(long)ic->timeout,
		0);

	/* Set timeout back to 0 (no timeout). */
	res = afc_inet_client_set_tag(ic, AFC_INET_CLIENT_TAG_TIMEOUT, (void *)(long)0);
	print_res("set_tag TIMEOUT 0 ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("timeout is 0",
		(void *)(long)0,
		(void *)(long)ic->timeout,
		0);

	print_row();

	/* ===== SSL tag tests ===== */

	/* Enable SSL via set_tag and verify. */
	res = afc_inet_client_set_tag(ic, AFC_INET_CLIENT_TAG_USE_SSL, (void *)(long)TRUE);
	print_res("set_tag USE_SSL ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("use_ssl is TRUE",
		(void *)(long)TRUE,
		(void *)(long)ic->use_ssl,
		0);

	/* Disable SSL and verify. */
	res = afc_inet_client_set_tag(ic, AFC_INET_CLIENT_TAG_USE_SSL, (void *)(long)FALSE);
	print_res("set_tag SSL FALSE ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("use_ssl is FALSE",
		(void *)(long)FALSE,
		(void *)(long)ic->use_ssl,
		0);

	print_row();

	/* ===== afc_inet_client_set_tags() macro test ===== */

	/* Set multiple tags at once using the convenience macro. */
	res = afc_inet_client_set_tags(ic,
		AFC_INET_CLIENT_TAG_TIMEOUT, (void *)(long)45,
		AFC_INET_CLIENT_TAG_USE_SSL, (void *)(long)TRUE);
	print_res("set_tags ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("timeout is 45",
		(void *)(long)45,
		(void *)(long)ic->timeout,
		0);

	print_res("use_ssl is TRUE",
		(void *)(long)TRUE,
		(void *)(long)ic->use_ssl,
		0);

	print_row();

	/* ===== afc_inet_client_clear() tests ===== */

	/* Clear should return AFC_ERR_NO_ERROR on a valid object. */
	res = afc_inet_client_clear(ic);
	print_res("clear() ret OK",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* Clear with NULL should return AFC_ERR_NULL_POINTER. */
	res = afc_inet_client_clear(NULL);
	print_res("clear(NULL) ret",
		(void *)(long)AFC_ERR_NULL_POINTER,
		(void *)(long)res,
		0);

	print_row();

	/* ===== afc_inet_client_delete() test ===== */

	/* Delete the object (macro sets ic to NULL). */
	afc_inet_client_delete(ic);
	print_res("delete sets NULL",
		(void *)(long)1,
		(void *)(long)(ic == NULL),
		0);

	print_row();

	/* ===== Multiple create/delete cycles for stability ===== */

	/* Repeatedly create and destroy InetClient objects to check for leaks or crashes. */
	int cycles_ok = 1;
	for (int i = 0; i < CYCLE_COUNT; i++)
	{
		InetClient *tmp = afc_inet_client_new();
		if (tmp == NULL)
		{
			cycles_ok = 0;
			break;
		}
		afc_inet_client_delete(tmp);
	}

	print_res("100 create/delete OK",
		(void *)(long)1,
		(void *)(long)cycles_ok,
		0);

	/* Create, configure, clear, and delete to test full lifecycle. */
	InetClient *ic2 = afc_inet_client_new();
	afc_inet_client_set_tag(ic2, AFC_INET_CLIENT_TAG_TIMEOUT, (void *)(long)10);
	afc_inet_client_set_tag(ic2, AFC_INET_CLIENT_TAG_USE_SSL, (void *)(long)TRUE);
	res = afc_inet_client_clear(ic2);
	print_res("full lifecycle clear",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	afc_inet_client_delete(ic2);
	print_res("full lifecycle del",
		(void *)(long)1,
		(void *)(long)(ic2 == NULL),
		0);

	print_summary();

	/* Cleanup the AFC base object. */
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
