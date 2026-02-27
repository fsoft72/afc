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
 * test_ftp_client.c - Comprehensive tests for the FtpClient module.
 *
 * Tests cover:
 *   - afc_ftp_client_new() / afc_ftp_client_delete() lifecycle
 *   - Verification that object is not NULL after creation
 *   - Magic number verification
 *   - Default field values after creation
 *   - afc_ftp_client_clear()
 *   - Multiple create/delete cycles for stability
 *
 * NOTE: The FTP client has a very limited public API (new/delete/clear).
 *       No actual FTP connections are made in these tests.
 *       afc_ftp_client_delete() is a regular function (not a NULL-setting macro).
 */

#include "test_utils.h"
#include "../src/ftp_client.h"

/* Expected magic number computed from the 'FTP_' character sequence. */
#define EXPECTED_MAGIC ('F' << 24 | 'T' << 16 | 'P' << 8 | '_')

/* Number of create/delete cycles for stability testing. */
#define CYCLE_COUNT 100

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ===== afc_ftp_client_new() lifecycle tests ===== */

	/* Create a new FtpClient and verify it is not NULL. */
	FtpClient *fc = afc_ftp_client_new();
	print_res("new() not NULL",
		(void *)(long)1,
		(void *)(long)(fc != NULL),
		0);

	/* Verify the magic number matches the expected value. */
	print_res("magic number correct",
		(void *)(long)EXPECTED_MAGIC,
		(void *)(long)fc->magic,
		0);

	print_row();

	/* ===== Default field values after afc_ftp_client_new() ===== */

	/* Verify the internal InetClient was allocated. */
	print_res("inet allocated",
		(void *)(long)1,
		(void *)(long)(fc->inet != NULL),
		0);

	/* Verify the last_answer string was allocated. */
	print_res("last_answer allocated",
		(void *)(long)1,
		(void *)(long)(fc->last_answer != NULL),
		0);

	/* Verify default last_code is 0. */
	print_res("default last_code 0",
		(void *)(long)0,
		(void *)(long)fc->last_code,
		0);

	/* Verify default passive mode is FALSE. */
	print_res("default pasv FALSE",
		(void *)(long)FALSE,
		(void *)(long)fc->pasv,
		0);

	print_row();

	/* ===== afc_ftp_client_clear() tests ===== */

	/* Clear should return AFC_ERR_NO_ERROR on a valid object. */
	int res = afc_ftp_client_clear(fc);
	print_res("clear() ret OK",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* After clear, last_code should be 0. */
	print_res("last_code after clear",
		(void *)(long)0,
		(void *)(long)fc->last_code,
		0);

	/* After clear, passive mode should be FALSE. */
	print_res("pasv after clear",
		(void *)(long)FALSE,
		(void *)(long)fc->pasv,
		0);

	/* Verify the object is still valid after clear (magic intact). */
	print_res("magic after clear",
		(void *)(long)EXPECTED_MAGIC,
		(void *)(long)fc->magic,
		0);

	/* Verify inet is still valid after clear. */
	print_res("inet after clear",
		(void *)(long)1,
		(void *)(long)(fc->inet != NULL),
		0);

	/* Clear with NULL should return AFC_ERR_NULL_POINTER. */
	res = afc_ftp_client_clear(NULL);
	print_res("clear(NULL) ret",
		(void *)(long)AFC_ERR_NULL_POINTER,
		(void *)(long)res,
		0);

	print_row();

	/* ===== Double clear test ===== */

	/* Calling clear twice in a row should still succeed. */
	res = afc_ftp_client_clear(fc);
	print_res("1st clear() OK",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	res = afc_ftp_client_clear(fc);
	print_res("2nd clear() OK",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_row();

	/* ===== afc_ftp_client_delete() test ===== */

	/*
	 * Note: afc_ftp_client_delete() is a regular function, not a macro
	 * that sets the pointer to NULL. We test the return value instead.
	 */
	res = afc_ftp_client_delete(fc);
	print_res("delete() ret OK",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_row();

	/* ===== Multiple create/delete cycles for stability ===== */

	/* Rapidly create and destroy FtpClient objects to check for leaks or crashes. */
	int cycles_ok = 1;
	for (int i = 0; i < CYCLE_COUNT; i++)
	{
		FtpClient *tmp = afc_ftp_client_new();
		if (tmp == NULL)
		{
			cycles_ok = 0;
			break;
		}
		afc_ftp_client_delete(tmp);
	}

	print_res("100 create/delete OK",
		(void *)(long)1,
		(void *)(long)cycles_ok,
		0);

	/* Create, clear, delete to test full lifecycle. */
	FtpClient *fc2 = afc_ftp_client_new();
	print_res("2nd new() not NULL",
		(void *)(long)1,
		(void *)(long)(fc2 != NULL),
		0);

	res = afc_ftp_client_clear(fc2);
	print_res("full lifecycle clear",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	res = afc_ftp_client_delete(fc2);
	print_res("full lifecycle delete",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_row();

	/* ===== Multiple create + clear + delete cycles ===== */

	/* Stress test: create, clear, and delete in a loop. */
	int full_cycles_ok = 1;
	for (int i = 0; i < CYCLE_COUNT; i++)
	{
		FtpClient *tmp = afc_ftp_client_new();
		if (tmp == NULL)
		{
			full_cycles_ok = 0;
			break;
		}
		if (afc_ftp_client_clear(tmp) != AFC_ERR_NO_ERROR)
		{
			full_cycles_ok = 0;
			afc_ftp_client_delete(tmp);
			break;
		}
		afc_ftp_client_delete(tmp);
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
