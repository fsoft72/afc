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
 * test_cgi_manager.c - Comprehensive tests for the AFC CGIManager module.
 *
 * Tests cover creation/deletion, content type setting, cookie set/get,
 * cookie domain/path/expire configuration, header string generation,
 * the clear function, and tag-based cookie handling toggle.
 *
 * Note: No actual CGI environment (REQUEST_METHOD, etc.) is available,
 * so we test what is possible without real HTTP environment variables.
 */

#include "test_utils.h"
#include "../src/cgi_manager.h"

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ----------------------------------------------------------------
	 * 1. Creation and initial state
	 * ---------------------------------------------------------------- */
	CGIManager *cgi = afc_cgi_manager_new();
	print_res("cgi_manager_new != NULL",
		(void *)(long)1, (void *)(long)(cgi != NULL), 0);

	/* Default content type should be "text/html" */
	print_res("default content_type",
		"text/html", cgi->content_type, 1);

	/* Default method should be UNDEF (no CGI env available) */
	print_res("default method == UNDEF",
		(void *)(long)AFC_CGI_MANAGER_METHOD_UNDEF,
		(void *)(long)cgi->method, 0);

	/* Cookies should not be handled by default */
	print_res("handle_cookies default false",
		(void *)(long)0, (void *)(long)cgi->handle_cookies, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 2. afc_cgi_manager_set_content_type()
	 * ---------------------------------------------------------------- */
	int res = afc_cgi_manager_set_content_type(cgi, "application/json");
	print_res("set_content_type returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);
	print_res("content_type == json",
		"application/json", cgi->content_type, 1);

	/* Reset to text/html for further tests */
	afc_cgi_manager_set_content_type(cgi, "text/html");
	print_res("content_type reset to html",
		"text/html", cgi->content_type, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 3. afc_cgi_manager_set_cookie() and afc_cgi_manager_get_cookie()
	 * ---------------------------------------------------------------- */
	res = afc_cgi_manager_set_cookie(cgi, "session_id", "abc123");
	print_res("set_cookie returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* Setting a cookie should auto-enable cookie handling */
	print_res("handle_cookies auto-enabled",
		(void *)(long)TRUE, (void *)(long)cgi->handle_cookies, 0);

	/* Retrieve the cookie (keys are uppercased internally) */
	char *val = afc_cgi_manager_get_cookie(cgi, "session_id");
	print_res("get_cookie session_id",
		"abc123", val, 1);

	/* Case-insensitive retrieval (key is uppercased internally) */
	val = afc_cgi_manager_get_cookie(cgi, "SESSION_ID");
	print_res("get_cookie UPPERCASE key",
		"abc123", val, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 4. Multiple cookies
	 * ---------------------------------------------------------------- */
	afc_cgi_manager_set_cookie(cgi, "user", "johndoe");
	afc_cgi_manager_set_cookie(cgi, "theme", "dark");

	val = afc_cgi_manager_get_cookie(cgi, "user");
	print_res("get_cookie user",
		"johndoe", val, 1);

	val = afc_cgi_manager_get_cookie(cgi, "theme");
	print_res("get_cookie theme",
		"dark", val, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 5. Overwrite existing cookie value
	 * ---------------------------------------------------------------- */
	afc_cgi_manager_set_cookie(cgi, "theme", "light");
	val = afc_cgi_manager_get_cookie(cgi, "theme");
	print_res("overwrite cookie theme",
		"light", val, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 6. Get non-existent cookie returns NULL
	 * ---------------------------------------------------------------- */
	val = afc_cgi_manager_get_cookie(cgi, "nonexistent");
	print_res("get missing cookie == NULL",
		(void *)(long)0, (void *)(long)(val != NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 7. afc_cgi_manager_set_cookie_domain()
	 * ---------------------------------------------------------------- */
	res = afc_cgi_manager_set_cookie_domain(cgi, ".example.com");
	print_res("set_cookie_domain returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);
	print_res("cookies_domain value",
		".example.com", cgi->cookies_domain, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 8. afc_cgi_manager_set_cookie_path()
	 * ---------------------------------------------------------------- */
	res = afc_cgi_manager_set_cookie_path(cgi, "/myapp");
	print_res("set_cookie_path returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);
	print_res("cookies_path value",
		"/myapp", cgi->cookies_path, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 9. afc_cgi_manager_set_cookie_expire()
	 * ---------------------------------------------------------------- */
	res = afc_cgi_manager_set_cookie_expire(cgi, 7);
	print_res("set_cookie_expire returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* The expire string should be non-empty after setting days */
	int expire_len = afc_string_len(cgi->cookies_expire);
	print_res("cookies_expire non-empty",
		(void *)(long)1, (void *)(long)(expire_len > 0), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 10. afc_cgi_manager_get_header_str() with cookies
	 * ---------------------------------------------------------------- */
	char *header_buf = afc_string_new(16384);
	res = afc_cgi_manager_get_header_str(cgi, header_buf);
	print_res("get_header_str returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* Header should contain Content-type */
	int has_content_type = (strstr(header_buf, "Content-type: text/html") != NULL);
	print_res("header has Content-type",
		(void *)(long)1, (void *)(long)has_content_type, 0);

	/* Header should contain Set-Cookie directives */
	int has_set_cookie = (strstr(header_buf, "Set-Cookie:") != NULL);
	print_res("header has Set-Cookie",
		(void *)(long)1, (void *)(long)has_set_cookie, 0);

	/* Header should contain the domain */
	int has_domain = (strstr(header_buf, "domain=.example.com") != NULL);
	print_res("header has domain",
		(void *)(long)1, (void *)(long)has_domain, 0);

	/* Header should contain the path */
	int has_path = (strstr(header_buf, "path=/myapp") != NULL);
	print_res("header has path",
		(void *)(long)1, (void *)(long)has_path, 0);

	afc_string_delete(header_buf);

	print_row();

	/* ----------------------------------------------------------------
	 * 11. afc_cgi_manager_set_tag() - toggle cookie handling
	 * ---------------------------------------------------------------- */
	res = afc_cgi_manager_set_tag(cgi,
		AFC_CGI_MANAGER_TAG_HANDLE_COOKIES, (void *)(long)FALSE);
	print_res("set_tag disable cookies OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);
	print_res("handle_cookies now false",
		(void *)(long)0, (void *)(long)cgi->handle_cookies, 0);

	/* Re-enable */
	afc_cgi_manager_set_tag(cgi,
		AFC_CGI_MANAGER_TAG_HANDLE_COOKIES, (void *)(long)TRUE);
	print_res("handle_cookies re-enabled",
		(void *)(long)TRUE, (void *)(long)cgi->handle_cookies, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 12. afc_cgi_manager_clear()
	 * ---------------------------------------------------------------- */
	res = afc_cgi_manager_clear(cgi);
	print_res("clear returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* After clear, is_post_read should be FALSE */
	print_res("is_post_read after clear",
		(void *)(long)FALSE, (void *)(long)cgi->is_post_read, 0);

	/* are_headers_set should be FALSE after clear */
	print_res("are_headers_set after clear",
		(void *)(long)FALSE, (void *)(long)cgi->are_headers_set, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 13. Edge case: clear on NULL pointer
	 * ---------------------------------------------------------------- */
	res = afc_cgi_manager_clear(NULL);
	print_res("clear(NULL) returns err",
		(void *)(long)1, (void *)(long)(res != AFC_ERR_NO_ERROR), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 14. Header string without cookies (handle_cookies off)
	 * ---------------------------------------------------------------- */
	afc_cgi_manager_set_content_type(cgi, "text/plain");
	cgi->handle_cookies = FALSE;

	header_buf = afc_string_new(16384);
	res = afc_cgi_manager_get_header_str(cgi, header_buf);
	print_res("header_str no cookies OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	has_content_type = (strstr(header_buf, "Content-type: text/plain") != NULL);
	print_res("header plain content type",
		(void *)(long)1, (void *)(long)has_content_type, 0);

	/* Should NOT contain Set-Cookie when handle_cookies is off */
	has_set_cookie = (strstr(header_buf, "Set-Cookie:") != NULL);
	print_res("no Set-Cookie in header",
		(void *)(long)0, (void *)(long)has_set_cookie, 0);

	afc_string_delete(header_buf);

	/* ----------------------------------------------------------------
	 * Cleanup and summary
	 * ---------------------------------------------------------------- */
	print_summary();

	afc_cgi_manager_delete(cgi);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
