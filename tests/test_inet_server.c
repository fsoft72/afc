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
 * test_inet_server.c - Comprehensive tests for the InetServer networking module.
 *
 * Tests cover:
 *   - afc_inet_server_new() / afc_inet_server_delete() lifecycle
 *   - Verification that object is not NULL after creation
 *   - Magic number verification
 *   - Default field values after creation
 *   - afc_inet_server_clear()
 *   - Multiple create/delete cycles for stability
 *
 * NOTE: No actual network connections or listening sockets are created.
 */

#include "test_utils.h"
#include "../src/inet_server.h"

/* Expected magic number computed from the 'IBSE' character sequence. */
#define EXPECTED_MAGIC ('I' << 24 | 'B' << 16 | 'S' << 8 | 'E')

/* Number of create/delete cycles for stability testing. */
#define CYCLE_COUNT 100

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ===== afc_inet_server_new() lifecycle tests ===== */

	/* Create a new InetServer and verify it is not NULL. */
	InetServer *is = afc_inet_server_new();
	print_res("new() not NULL",
		(void *)(long)1,
		(void *)(long)(is != NULL),
		0);

	/* Verify the magic number matches the expected value. */
	print_res("magic number correct",
		(void *)(long)EXPECTED_MAGIC,
		(void *)(long)is->magic,
		0);

	print_row();

	/* ===== Default field values after afc_inet_server_new() ===== */

	/* Verify the internal hash table was allocated. */
	print_res("hash allocated",
		(void *)(long)1,
		(void *)(long)(is->hash != NULL),
		0);

	/* Verify the default buffer size is AFC_INET_SERVER_DEFAULT_BUFSIZE (256). */
	print_res("default bufsize 256",
		(void *)(long)AFC_INET_SERVER_DEFAULT_BUFSIZE,
		(void *)(long)is->bufsize,
		0);

	/* Verify default callbacks are NULL. */
	print_res("cb_connect is NULL",
		(void *)(long)1,
		(void *)(long)(is->cb_connect == NULL),
		0);

	print_res("cb_close is NULL",
		(void *)(long)1,
		(void *)(long)(is->cb_close == NULL),
		0);

	print_res("cb_receive is NULL",
		(void *)(long)1,
		(void *)(long)(is->cb_receive == NULL),
		0);

	/* Verify data pointer is NULL by default. */
	print_res("data is NULL",
		(void *)(long)1,
		(void *)(long)(is->data == NULL),
		0);

	print_row();

	/* ===== afc_inet_server_clear() tests ===== */

	/* Clear should return AFC_ERR_NO_ERROR on a valid object. */
	int res = afc_inet_server_clear(is);
	print_res("clear() ret OK",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* Clear with NULL should return AFC_ERR_NULL_POINTER. */
	res = afc_inet_server_clear(NULL);
	print_res("clear(NULL) ret",
		(void *)(long)AFC_ERR_NULL_POINTER,
		(void *)(long)res,
		0);

	/* Verify the object is still valid after clear. */
	print_res("magic after clear",
		(void *)(long)EXPECTED_MAGIC,
		(void *)(long)is->magic,
		0);

	/* Verify hash is still valid after clear. */
	print_res("hash after clear",
		(void *)(long)1,
		(void *)(long)(is->hash != NULL),
		0);

	print_row();

	/* ===== afc_inet_server_delete() test ===== */

	/* Delete the object (macro sets is to NULL). */
	afc_inet_server_delete(is);
	print_res("delete sets NULL",
		(void *)(long)1,
		(void *)(long)(is == NULL),
		0);

	print_row();

	/* ===== Multiple create/delete cycles for stability ===== */

	/* Rapidly create and destroy InetServer objects to check for leaks or crashes. */
	int cycles_ok = 1;
	for (int i = 0; i < CYCLE_COUNT; i++)
	{
		InetServer *tmp = afc_inet_server_new();
		if (tmp == NULL)
		{
			cycles_ok = 0;
			break;
		}
		afc_inet_server_delete(tmp);
	}

	print_res("100 create/delete OK",
		(void *)(long)1,
		(void *)(long)cycles_ok,
		0);

	print_row();

	/* ===== Create, clear, delete full lifecycle ===== */

	/* Test a complete lifecycle: create, clear, delete. */
	InetServer *is2 = afc_inet_server_new();
	print_res("2nd new() not NULL",
		(void *)(long)1,
		(void *)(long)(is2 != NULL),
		0);

	res = afc_inet_server_clear(is2);
	print_res("2nd clear() ret OK",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	afc_inet_server_delete(is2);
	print_res("2nd delete sets NULL",
		(void *)(long)1,
		(void *)(long)(is2 == NULL),
		0);

	/* ===== Multiple create + clear + delete cycles ===== */

	/* Stress test: create, clear, and delete in a loop. */
	int full_cycles_ok = 1;
	for (int i = 0; i < CYCLE_COUNT; i++)
	{
		InetServer *tmp = afc_inet_server_new();
		if (tmp == NULL)
		{
			full_cycles_ok = 0;
			break;
		}
		if (afc_inet_server_clear(tmp) != AFC_ERR_NO_ERROR)
		{
			full_cycles_ok = 0;
			afc_inet_server_delete(tmp);
			break;
		}
		afc_inet_server_delete(tmp);
	}

	print_res("100 full cycles OK",
		(void *)(long)1,
		(void *)(long)full_cycles_ok,
		0);

	print_summary();

	/* Cleanup the AFC base object. */
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
