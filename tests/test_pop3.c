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
 * test_pop3.c - Comprehensive tests for the POP3 client module.
 *
 * Tests cover:
 *   - afc_pop3_new() / afc_pop3_delete() lifecycle
 *   - Verification that object is not NULL after creation
 *   - Magic number verification
 *   - Default field values after creation
 *   - afc_pop3_set_tag() with HOST, PORT, LOGIN, PASSWD tags
 *   - afc_pop3_set_tags() macro with multiple tags
 *   - afc_pop3_clear()
 *   - Multiple create/delete cycles for stability
 *
 * NOTE: No actual POP3 connections are made in these tests.
 */

#include "test_utils.h"
#include "../src/pop3.h"

/* Expected magic number computed from the 'POP3' character sequence. */
#define EXPECTED_MAGIC ('P' << 24 | 'O' << 16 | 'P' << 8 | '3')

/* Number of create/delete cycles for stability testing. */
#define CYCLE_COUNT 100

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ===== afc_pop3_new() lifecycle tests ===== */

	/* Create a new POP3 object and verify it is not NULL. */
	POP3 *p3 = afc_pop3_new();
	print_res("new() not NULL",
		(void *)(long)1,
		(void *)(long)(p3 != NULL),
		0);

	/* Verify the magic number matches the expected value. */
	print_res("magic number correct",
		(void *)(long)EXPECTED_MAGIC,
		(void *)(long)p3->magic,
		0);

	print_row();

	/* ===== Default field values after afc_pop3_new() ===== */

	/* Verify the internal InetClient was allocated. */
	print_res("ic allocated",
		(void *)(long)1,
		(void *)(long)(p3->ic != NULL),
		0);

	/* Verify the string list was allocated. */
	print_res("sn allocated",
		(void *)(long)1,
		(void *)(long)(p3->sn != NULL),
		0);

	/* Verify the internal buffer was allocated. */
	print_res("buf allocated",
		(void *)(long)1,
		(void *)(long)(p3->buf != NULL),
		0);

	/* Verify the temporary buffer was allocated. */
	print_res("tmp allocated",
		(void *)(long)1,
		(void *)(long)(p3->tmp != NULL),
		0);

	/* Verify the message hash table was allocated. */
	print_res("msg hash allocated",
		(void *)(long)1,
		(void *)(long)(p3->msg != NULL),
		0);

	/* Verify default port is "110". */
	print_res("default port 110",
		(void *)"110",
		(void *)p3->port,
		1);

	/* Verify host is NULL by default (not configured). */
	print_res("default host NULL",
		(void *)(long)1,
		(void *)(long)(p3->host == NULL),
		0);

	/* Verify login is NULL by default. */
	print_res("default login NULL",
		(void *)(long)1,
		(void *)(long)(p3->login == NULL),
		0);

	/* Verify passwd is NULL by default. */
	print_res("default passwd NULL",
		(void *)(long)1,
		(void *)(long)(p3->passwd == NULL),
		0);

	print_row();

	/* ===== afc_pop3_set_tag() - HOST ===== */

	/* Set the host and verify the return code. */
	int res = afc_pop3_set_tag(p3, AFC_POP3_TAG_HOST, (void *)"pop.example.com");
	print_res("set_tag HOST ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* Verify the host value was stored correctly. */
	print_res("host value",
		(void *)"pop.example.com",
		(void *)p3->host,
		1);

	print_row();

	/* ===== afc_pop3_set_tag() - PORT ===== */

	/* Set the port to 995 (POP3S) and verify. */
	res = afc_pop3_set_tag(p3, AFC_POP3_TAG_PORT, (void *)"995");
	print_res("set_tag PORT ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("port value 995",
		(void *)"995",
		(void *)p3->port,
		1);

	print_row();

	/* ===== afc_pop3_set_tag() - LOGIN ===== */

	/* Set the login name and verify. */
	res = afc_pop3_set_tag(p3, AFC_POP3_TAG_LOGIN, (void *)"user@example.com");
	print_res("set_tag LOGIN ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("login value",
		(void *)"user@example.com",
		(void *)p3->login,
		1);

	print_row();

	/* ===== afc_pop3_set_tag() - PASSWD ===== */

	/* Set the password and verify. */
	res = afc_pop3_set_tag(p3, AFC_POP3_TAG_PASSWD, (void *)"mysecretpass");
	print_res("set_tag PASSWD ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("passwd value",
		(void *)"mysecretpass",
		(void *)p3->passwd,
		1);

	print_row();

	/* ===== afc_pop3_set_tags() macro test ===== */

	/* Overwrite host and login using the convenience macro. */
	res = afc_pop3_set_tags(p3,
		AFC_POP3_TAG_HOST, (void *)"mail.test.org",
		AFC_POP3_TAG_LOGIN, (void *)"admin");
	print_res("set_tags ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("host after set_tags",
		(void *)"mail.test.org",
		(void *)p3->host,
		1);

	print_res("login after set_tags",
		(void *)"admin",
		(void *)p3->login,
		1);

	print_row();

	/* ===== Overwrite existing tags ===== */

	/* Verify that setting a tag again overwrites the previous value. */
	afc_pop3_set_tag(p3, AFC_POP3_TAG_HOST, (void *)"new.host.com");
	print_res("host overwrite",
		(void *)"new.host.com",
		(void *)p3->host,
		1);

	afc_pop3_set_tag(p3, AFC_POP3_TAG_PASSWD, (void *)"newpass");
	print_res("passwd overwrite",
		(void *)"newpass",
		(void *)p3->passwd,
		1);

	print_row();

	/* ===== afc_pop3_clear() tests ===== */

	/* Clear should return AFC_ERR_NO_ERROR on a valid object. */
	res = afc_pop3_clear(p3);
	print_res("clear() ret OK",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* After clear, host should be NULL. */
	print_res("host NULL after clear",
		(void *)(long)1,
		(void *)(long)(p3->host == NULL),
		0);

	/* After clear, login should be NULL. */
	print_res("login NULL after clear",
		(void *)(long)1,
		(void *)(long)(p3->login == NULL),
		0);

	/* After clear, passwd should be NULL. */
	print_res("passwd NULL after clr",
		(void *)(long)1,
		(void *)(long)(p3->passwd == NULL),
		0);

	/* After clear, port should be reset to "110". */
	print_res("port 110 after clear",
		(void *)"110",
		(void *)p3->port,
		1);

	/* Clear with NULL should return AFC_ERR_NULL_POINTER. */
	res = afc_pop3_clear(NULL);
	print_res("clear(NULL) ret",
		(void *)(long)AFC_ERR_NULL_POINTER,
		(void *)(long)res,
		0);

	print_row();

	/* ===== afc_pop3_delete() test ===== */

	/* Delete the object (macro sets p3 to NULL). */
	afc_pop3_delete(p3);
	print_res("delete sets NULL",
		(void *)(long)1,
		(void *)(long)(p3 == NULL),
		0);

	print_row();

	/* ===== Multiple create/delete cycles for stability ===== */

	/* Rapidly create and destroy POP3 objects. */
	int cycles_ok = 1;
	for (int i = 0; i < CYCLE_COUNT; i++)
	{
		POP3 *tmp = afc_pop3_new();
		if (tmp == NULL)
		{
			cycles_ok = 0;
			break;
		}
		afc_pop3_delete(tmp);
	}

	print_res("100 create/delete OK",
		(void *)(long)1,
		(void *)(long)cycles_ok,
		0);

	/* Full lifecycle: create, configure all tags, clear, and delete. */
	POP3 *p3b = afc_pop3_new();
	afc_pop3_set_tags(p3b,
		AFC_POP3_TAG_HOST, (void *)"pop.fulltest.com",
		AFC_POP3_TAG_PORT, (void *)"995",
		AFC_POP3_TAG_LOGIN, (void *)"testlogin",
		AFC_POP3_TAG_PASSWD, (void *)"testpass");

	res = afc_pop3_clear(p3b);
	print_res("full lifecycle clear",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	afc_pop3_delete(p3b);
	print_res("full lifecycle del",
		(void *)(long)1,
		(void *)(long)(p3b == NULL),
		0);

	print_summary();

	/* Cleanup the AFC base object. */
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
