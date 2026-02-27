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
 * test_http_client.c - Comprehensive tests for the HttpClient module.
 *
 * Tests cover:
 *   - afc_http_client_new() / afc_http_client_delete() lifecycle
 *   - Verification that object is not NULL after creation
 *   - Magic number verification
 *   - Default field values after creation
 *   - afc_http_client_set_tag() with TIMEOUT, FOLLOW_REDIRECTS, MAX_REDIRECTS, USE_SSL
 *   - afc_http_client_set_tags() macro with multiple tags
 *   - afc_http_client_set_header() for custom request headers
 *   - afc_http_client_clear_headers()
 *   - afc_http_client_clear()
 *   - Multiple create/delete cycles for stability
 *
 * NOTE: No actual HTTP connections are made in these tests.
 */

#include "test_utils.h"
#include "../src/http_client.h"

/* Expected magic number computed from the 'HTTP' character sequence. */
#define EXPECTED_MAGIC ('H' << 24 | 'T' << 16 | 'T' << 8 | 'P')

/* Number of create/delete cycles for stability testing. */
#define CYCLE_COUNT 100

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ===== afc_http_client_new() lifecycle tests ===== */

	/* Create a new HttpClient and verify it is not NULL. */
	HttpClient *hc = afc_http_client_new();
	print_res("new() not NULL",
		(void *)(long)1,
		(void *)(long)(hc != NULL),
		0);

	/* Verify the magic number matches the expected value. */
	print_res("magic number correct",
		(void *)(long)EXPECTED_MAGIC,
		(void *)(long)hc->magic,
		0);

	print_row();

	/* ===== Default field values after afc_http_client_new() ===== */

	/* Verify the internal InetClient was allocated. */
	print_res("inet allocated",
		(void *)(long)1,
		(void *)(long)(hc->inet != NULL),
		0);

	/* Verify the request headers dictionary was allocated. */
	print_res("req_headers allocated",
		(void *)(long)1,
		(void *)(long)(hc->req_headers != NULL),
		0);

	/* Verify the response headers dictionary was allocated. */
	print_res("resp_headers allocated",
		(void *)(long)1,
		(void *)(long)(hc->resp_headers != NULL),
		0);

	/* Verify internal buffers were allocated. */
	print_res("buf allocated",
		(void *)(long)1,
		(void *)(long)(hc->buf != NULL),
		0);

	print_res("tmp allocated",
		(void *)(long)1,
		(void *)(long)(hc->tmp != NULL),
		0);

	/* Verify default port is 80. */
	print_res("default port 80",
		(void *)(long)80,
		(void *)(long)hc->port,
		0);

	/* Verify default SSL flag is FALSE. */
	print_res("default use_ssl FALSE",
		(void *)(long)FALSE,
		(void *)(long)hc->use_ssl,
		0);

	/* Verify default connection status is FALSE. */
	print_res("default isconnected F",
		(void *)(long)FALSE,
		(void *)(long)hc->isconnected,
		0);

	/* Verify default timeout is 0. */
	print_res("default timeout 0",
		(void *)(long)0,
		(void *)(long)hc->timeout,
		0);

	/* Verify default follow_redirects is TRUE. */
	print_res("default follow_redir T",
		(void *)(long)TRUE,
		(void *)(long)hc->follow_redirects,
		0);

	/* Verify default max_redirects is AFC_HTTP_CLIENT_MAX_REDIRECTS (10). */
	print_res("default max_redir 10",
		(void *)(long)AFC_HTTP_CLIENT_MAX_REDIRECTS,
		(void *)(long)hc->max_redirects,
		0);

	/* Verify default status_code is 0. */
	print_res("default status_code 0",
		(void *)(long)0,
		(void *)(long)hc->status_code,
		0);

	/* Verify host is NULL by default. */
	print_res("default host NULL",
		(void *)(long)1,
		(void *)(long)(hc->host == NULL),
		0);

	print_row();

	/* ===== afc_http_client_set_tag() - TIMEOUT ===== */

	/* Set timeout to 30 seconds and verify. */
	int res = afc_http_client_set_tag(hc, AFC_HTTP_CLIENT_TAG_TIMEOUT, (void *)(long)30);
	print_res("set_tag TIMEOUT ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("timeout is 30",
		(void *)(long)30,
		(void *)(long)hc->timeout,
		0);

	print_row();

	/* ===== afc_http_client_set_tag() - FOLLOW_REDIRECTS ===== */

	/* Disable redirect following and verify. */
	res = afc_http_client_set_tag(hc, AFC_HTTP_CLIENT_TAG_FOLLOW_REDIRECTS, (void *)(long)FALSE);
	print_res("set_tag FOLLOW ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("follow_redir FALSE",
		(void *)(long)FALSE,
		(void *)(long)hc->follow_redirects,
		0);

	/* Re-enable redirect following. */
	res = afc_http_client_set_tag(hc, AFC_HTTP_CLIENT_TAG_FOLLOW_REDIRECTS, (void *)(long)TRUE);
	print_res("set_tag FOLLOW T ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("follow_redir TRUE",
		(void *)(long)TRUE,
		(void *)(long)hc->follow_redirects,
		0);

	print_row();

	/* ===== afc_http_client_set_tag() - MAX_REDIRECTS ===== */

	/* Set max redirects to 5 and verify. */
	res = afc_http_client_set_tag(hc, AFC_HTTP_CLIENT_TAG_MAX_REDIRECTS, (void *)(long)5);
	print_res("set_tag MAX_REDIR ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("max_redir is 5",
		(void *)(long)5,
		(void *)(long)hc->max_redirects,
		0);

	print_row();

	/* ===== afc_http_client_set_tag() - USE_SSL ===== */

	/* Enable SSL and verify. */
	res = afc_http_client_set_tag(hc, AFC_HTTP_CLIENT_TAG_USE_SSL, (void *)(long)TRUE);
	print_res("set_tag USE_SSL ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("use_ssl is TRUE",
		(void *)(long)TRUE,
		(void *)(long)hc->use_ssl,
		0);

	/* Disable SSL. */
	res = afc_http_client_set_tag(hc, AFC_HTTP_CLIENT_TAG_USE_SSL, (void *)(long)FALSE);
	print_res("set_tag SSL F ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("use_ssl is FALSE",
		(void *)(long)FALSE,
		(void *)(long)hc->use_ssl,
		0);

	print_row();

	/* ===== afc_http_client_set_tags() macro test ===== */

	/* Set multiple tags at once using the convenience macro. */
	res = afc_http_client_set_tags(hc,
		AFC_HTTP_CLIENT_TAG_TIMEOUT, (void *)(long)60,
		AFC_HTTP_CLIENT_TAG_USE_SSL, (void *)(long)TRUE,
		AFC_HTTP_CLIENT_TAG_MAX_REDIRECTS, (void *)(long)3);
	print_res("set_tags ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("timeout after set_tags",
		(void *)(long)60,
		(void *)(long)hc->timeout,
		0);

	print_res("use_ssl after set_tags",
		(void *)(long)TRUE,
		(void *)(long)hc->use_ssl,
		0);

	print_res("max_redir after tags",
		(void *)(long)3,
		(void *)(long)hc->max_redirects,
		0);

	print_row();

	/* ===== afc_http_client_set_header() tests ===== */

	/* Set a custom Content-Type header and verify return code. */
	res = afc_http_client_set_header(hc, "Content-Type", "application/json");
	print_res("set_header CT ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* Set a custom Authorization header and verify return code. */
	res = afc_http_client_set_header(hc, "Authorization", "Bearer token123");
	print_res("set_header Auth ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* Set a custom User-Agent header and verify return code. */
	res = afc_http_client_set_header(hc, "User-Agent", "AFC-Test/1.0");
	print_res("set_header UA ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* set_header with NULL pointer should return error. */
	res = afc_http_client_set_header(hc, NULL, "value");
	print_res("set_header NULL name",
		(void *)(long)AFC_ERR_NULL_POINTER,
		(void *)(long)res,
		0);

	res = afc_http_client_set_header(hc, "name", NULL);
	print_res("set_header NULL value",
		(void *)(long)AFC_ERR_NULL_POINTER,
		(void *)(long)res,
		0);

	print_row();

	/* ===== afc_http_client_clear_headers() tests ===== */

	/* Clear all custom headers and verify return code. */
	res = afc_http_client_clear_headers(hc);
	print_res("clear_headers ret",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* Clear headers with NULL should return error. */
	res = afc_http_client_clear_headers(NULL);
	print_res("clear_headers NULL",
		(void *)(long)AFC_ERR_NULL_POINTER,
		(void *)(long)res,
		0);

	print_row();

	/* ===== afc_http_client_clear() tests ===== */

	/* Clear should return AFC_ERR_NO_ERROR on a valid object. */
	res = afc_http_client_clear(hc);
	print_res("clear() ret OK",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* After clear, status_code should be reset to 0. */
	print_res("status_code after clr",
		(void *)(long)0,
		(void *)(long)hc->status_code,
		0);

	/* After clear, resp_body_len should be reset to 0. */
	print_res("resp_body_len after clr",
		(void *)(long)0,
		(void *)(long)hc->resp_body_len,
		0);

	/* Clear with NULL should return AFC_ERR_NULL_POINTER. */
	res = afc_http_client_clear(NULL);
	print_res("clear(NULL) ret",
		(void *)(long)AFC_ERR_NULL_POINTER,
		(void *)(long)res,
		0);

	print_row();

	/* ===== afc_http_client_delete() test ===== */

	/* Delete the object (macro sets hc to NULL). */
	afc_http_client_delete(hc);
	print_res("delete sets NULL",
		(void *)(long)1,
		(void *)(long)(hc == NULL),
		0);

	print_row();

	/* ===== Multiple create/delete cycles for stability ===== */

	/* Rapidly create and destroy HttpClient objects. */
	int cycles_ok = 1;
	for (int i = 0; i < CYCLE_COUNT; i++)
	{
		HttpClient *tmp = afc_http_client_new();
		if (tmp == NULL)
		{
			cycles_ok = 0;
			break;
		}
		afc_http_client_delete(tmp);
	}

	print_res("100 create/delete OK",
		(void *)(long)1,
		(void *)(long)cycles_ok,
		0);

	/* Full lifecycle: create, configure tags, set headers, clear, and delete. */
	HttpClient *hc2 = afc_http_client_new();
	afc_http_client_set_tags(hc2,
		AFC_HTTP_CLIENT_TAG_TIMEOUT, (void *)(long)15,
		AFC_HTTP_CLIENT_TAG_USE_SSL, (void *)(long)TRUE);
	afc_http_client_set_header(hc2, "Accept", "text/html");
	afc_http_client_clear_headers(hc2);
	res = afc_http_client_clear(hc2);
	print_res("full lifecycle clear",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	afc_http_client_delete(hc2);
	print_res("full lifecycle del",
		(void *)(long)1,
		(void *)(long)(hc2 == NULL),
		0);

	print_summary();

	/* Cleanup the AFC base object. */
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
