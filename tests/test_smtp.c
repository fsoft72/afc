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
 * test_smtp.c - Comprehensive tests for the SMTP client module.
 *
 * Tests cover:
 *   - afc_smtp_new() / afc_smtp_delete() lifecycle
 *   - Verification that object is not NULL after creation
 *   - Magic number verification
 *   - Default field values after creation
 *   - afc_smtp_set_tag() with HOST, PORT, USERNAME, PASSWORD, USE_TLS, USE_SSL, AUTH_METHOD
 *   - afc_smtp_set_tags() macro with multiple tags
 *   - afc_smtp_clear()
 *   - Multiple create/delete cycles for stability
 *
 * NOTE: No actual SMTP connections are made in these tests.
 */

#include "test_utils.h"
#include "../src/smtp.h"

/* Expected magic number computed from the 'SMTP' character sequence. */
#define EXPECTED_MAGIC ('S' << 24 | 'M' << 16 | 'T' << 8 | 'P')

/* Number of create/delete cycles for stability testing. */
#define CYCLE_COUNT 100

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ===== afc_smtp_new() lifecycle tests ===== */

	/* Create a new SMTP object and verify it is not NULL. */
	SMTP *smtp = afc_smtp_new();
	print_res("new() not NULL",
		(void *)(long)1,
		(void *)(long)(smtp != NULL),
		0);

	/* Verify the magic number matches the expected value. */
	print_res("magic number correct",
		(void *)(long)EXPECTED_MAGIC,
		(void *)(long)smtp->magic,
		0);

	print_row();

	/* ===== Default field values after afc_smtp_new() ===== */

	/* Verify the internal InetClient was allocated. */
	print_res("ic allocated",
		(void *)(long)1,
		(void *)(long)(smtp->ic != NULL),
		0);

	/* Verify the internal buffer was allocated. */
	print_res("buf allocated",
		(void *)(long)1,
		(void *)(long)(smtp->buf != NULL),
		0);

	/* Verify the temporary buffer was allocated. */
	print_res("tmp allocated",
		(void *)(long)1,
		(void *)(long)(smtp->tmp != NULL),
		0);

	/* Verify default port is "25". */
	print_res("default port 25",
		(void *)"25",
		(void *)smtp->port,
		1);

	/* Verify default TLS flag is FALSE. */
	print_res("default use_tls FALSE",
		(void *)(long)FALSE,
		(void *)(long)smtp->use_tls,
		0);

	/* Verify default SSL flag is FALSE. */
	print_res("default use_ssl FALSE",
		(void *)(long)FALSE,
		(void *)(long)smtp->use_ssl,
		0);

	/* Verify default auth method is AFC_SMTP_AUTH_NONE. */
	print_res("default auth NONE",
		(void *)(long)AFC_SMTP_AUTH_NONE,
		(void *)(long)smtp->auth_method,
		0);

	/* Verify default authenticated flag is FALSE. */
	print_res("default authenticated F",
		(void *)(long)FALSE,
		(void *)(long)smtp->authenticated,
		0);

	/* Verify default connected flag is FALSE. */
	print_res("default connected FALSE",
		(void *)(long)FALSE,
		(void *)(long)smtp->connected,
		0);

	/* Verify host is NULL by default (not configured). */
	print_res("default host NULL",
		(void *)(long)1,
		(void *)(long)(smtp->host == NULL),
		0);

	/* Verify username is NULL by default. */
	print_res("default username NULL",
		(void *)(long)1,
		(void *)(long)(smtp->username == NULL),
		0);

	/* Verify password is NULL by default. */
	print_res("default password NULL",
		(void *)(long)1,
		(void *)(long)(smtp->password == NULL),
		0);

	print_row();

	/* ===== afc_smtp_set_tag() - HOST ===== */

	/* Set the host and verify the return code. */
	int res = afc_smtp_set_tag(smtp, AFC_SMTP_TAG_HOST, (void *)"smtp.example.com");
	print_res("set_tag HOST ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* Verify the host value was stored correctly. */
	print_res("host value",
		(void *)"smtp.example.com",
		(void *)smtp->host,
		1);

	print_row();

	/* ===== afc_smtp_set_tag() - PORT ===== */

	/* Set the port to 587 (STARTTLS) and verify. */
	res = afc_smtp_set_tag(smtp, AFC_SMTP_TAG_PORT, (void *)"587");
	print_res("set_tag PORT ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("port value 587",
		(void *)"587",
		(void *)smtp->port,
		1);

	print_row();

	/* ===== afc_smtp_set_tag() - USERNAME and PASSWORD ===== */

	/* Set the username and verify. */
	res = afc_smtp_set_tag(smtp, AFC_SMTP_TAG_USERNAME, (void *)"testuser");
	print_res("set_tag USERNAME ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("username value",
		(void *)"testuser",
		(void *)smtp->username,
		1);

	/* Set the password and verify. */
	res = afc_smtp_set_tag(smtp, AFC_SMTP_TAG_PASSWORD, (void *)"secret123");
	print_res("set_tag PASSWORD ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("password value",
		(void *)"secret123",
		(void *)smtp->password,
		1);

	print_row();

	/* ===== afc_smtp_set_tag() - TLS, SSL, AUTH_METHOD ===== */

	/* Enable STARTTLS and verify. */
	res = afc_smtp_set_tag(smtp, AFC_SMTP_TAG_USE_TLS, (void *)(long)TRUE);
	print_res("set_tag USE_TLS ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("use_tls is TRUE",
		(void *)(long)TRUE,
		(void *)(long)smtp->use_tls,
		0);

	/* Enable direct SSL and verify. */
	res = afc_smtp_set_tag(smtp, AFC_SMTP_TAG_USE_SSL, (void *)(long)TRUE);
	print_res("set_tag USE_SSL ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("use_ssl is TRUE",
		(void *)(long)TRUE,
		(void *)(long)smtp->use_ssl,
		0);

	/* Set AUTH method to PLAIN and verify. */
	res = afc_smtp_set_tag(smtp, AFC_SMTP_TAG_AUTH_METHOD, (void *)(long)AFC_SMTP_AUTH_PLAIN);
	print_res("set_tag AUTH PLAIN ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("auth is PLAIN",
		(void *)(long)AFC_SMTP_AUTH_PLAIN,
		(void *)(long)smtp->auth_method,
		0);

	/* Set AUTH method to LOGIN and verify. */
	res = afc_smtp_set_tag(smtp, AFC_SMTP_TAG_AUTH_METHOD, (void *)(long)AFC_SMTP_AUTH_LOGIN);
	print_res("set_tag AUTH LOGIN ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("auth is LOGIN",
		(void *)(long)AFC_SMTP_AUTH_LOGIN,
		(void *)(long)smtp->auth_method,
		0);

	print_row();

	/* ===== afc_smtp_set_tag() - FROM, TO, SUBJECT ===== */

	/* Set the sender address. */
	res = afc_smtp_set_tag(smtp, AFC_SMTP_TAG_FROM, (void *)"sender@example.com");
	print_res("set_tag FROM ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("from value",
		(void *)"sender@example.com",
		(void *)smtp->from,
		1);

	/* Set the recipient address. */
	res = afc_smtp_set_tag(smtp, AFC_SMTP_TAG_TO, (void *)"recipient@example.com");
	print_res("set_tag TO ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("to value",
		(void *)"recipient@example.com",
		(void *)smtp->to,
		1);

	/* Set the subject. */
	res = afc_smtp_set_tag(smtp, AFC_SMTP_TAG_SUBJECT, (void *)"Test Subject");
	print_res("set_tag SUBJECT ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("subject value",
		(void *)"Test Subject",
		(void *)smtp->subject,
		1);

	print_row();

	/* ===== afc_smtp_set_tags() macro test ===== */

	/* Overwrite host and port using the convenience macro. */
	res = afc_smtp_set_tags(smtp,
		AFC_SMTP_TAG_HOST, (void *)"mail.test.org",
		AFC_SMTP_TAG_PORT, (void *)"465");
	print_res("set_tags ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("host after set_tags",
		(void *)"mail.test.org",
		(void *)smtp->host,
		1);

	print_res("port after set_tags",
		(void *)"465",
		(void *)smtp->port,
		1);

	print_row();

	/* ===== afc_smtp_clear() tests ===== */

	/* Clear should return AFC_ERR_NO_ERROR on a valid object. */
	res = afc_smtp_clear(smtp);
	print_res("clear() ret OK",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* After clear, authenticated should be FALSE. */
	print_res("authenticated after clr",
		(void *)(long)FALSE,
		(void *)(long)smtp->authenticated,
		0);

	/* After clear, connected should be FALSE. */
	print_res("connected after clear",
		(void *)(long)FALSE,
		(void *)(long)smtp->connected,
		0);

	/* Clear with NULL should return AFC_ERR_NULL_POINTER. */
	res = afc_smtp_clear(NULL);
	print_res("clear(NULL) ret",
		(void *)(long)AFC_ERR_NULL_POINTER,
		(void *)(long)res,
		0);

	print_row();

	/* ===== afc_smtp_delete() test ===== */

	/* Delete the object (macro sets smtp to NULL). */
	afc_smtp_delete(smtp);
	print_res("delete sets NULL",
		(void *)(long)1,
		(void *)(long)(smtp == NULL),
		0);

	print_row();

	/* ===== Multiple create/delete cycles for stability ===== */

	/* Rapidly create and destroy SMTP objects. */
	int cycles_ok = 1;
	for (int i = 0; i < CYCLE_COUNT; i++)
	{
		SMTP *tmp = afc_smtp_new();
		if (tmp == NULL)
		{
			cycles_ok = 0;
			break;
		}
		afc_smtp_delete(tmp);
	}

	print_res("100 create/delete OK",
		(void *)(long)1,
		(void *)(long)cycles_ok,
		0);

	/* Full lifecycle: create, configure all tags, clear, and delete. */
	SMTP *smtp2 = afc_smtp_new();
	afc_smtp_set_tags(smtp2,
		AFC_SMTP_TAG_HOST, (void *)"ses.amazonaws.com",
		AFC_SMTP_TAG_PORT, (void *)"587",
		AFC_SMTP_TAG_USERNAME, (void *)"AKIAEXAMPLE",
		AFC_SMTP_TAG_PASSWORD, (void *)"wJalrXUtnFEMI",
		AFC_SMTP_TAG_USE_TLS, (void *)(long)TRUE,
		AFC_SMTP_TAG_AUTH_METHOD, (void *)(long)AFC_SMTP_AUTH_LOGIN);

	res = afc_smtp_clear(smtp2);
	print_res("full lifecycle clear",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	afc_smtp_delete(smtp2);
	print_res("full lifecycle del",
		(void *)(long)1,
		(void *)(long)(smtp2 == NULL),
		0);

	print_summary();

	/* Cleanup the AFC base object. */
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
