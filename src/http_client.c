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
#include <stdarg.h>
#include <ctype.h>
#include "http_client.h"

static const char class_name[] = "HttpClient";

// Internal functions
static int _afc_http_client_parse_url(const char * url, char ** protocol, char ** host, int * port, char ** path);
static int _afc_http_client_send_request(HttpClient * hc, const char * method, const char * path, const char * body, int body_len);
static int _afc_http_client_read_response(HttpClient * hc);
static int _afc_http_client_parse_status_line(HttpClient * hc, const char * line);
static int _afc_http_client_parse_headers(HttpClient * hc, FILE * fd);
static int _afc_http_client_read_body(HttpClient * hc, FILE * fd);
static int _afc_http_client_handle_redirect(HttpClient * hc, const char * method, const char * body, int body_len, int redirect_count);

// {{{ afc_http_client_new ()
/*
@node afc_http_client_new

           NAME: afc_http_client_new () - Initializes a new HttpClient instance.

       SYNOPSIS: HttpClient * afc_http_client_new ()

    DESCRIPTION: This function initializes a new HttpClient instance.

          INPUT: NONE

        RESULTS: a valid initialized HttpClient structure. NULL in case of errors.

       SEE ALSO: - afc_http_client_delete()

@endnode
*/
HttpClient * afc_http_client_new(void)
{
	TRY(HttpClient *)

	HttpClient * hc = (HttpClient *) afc_malloc(sizeof(HttpClient));

	if (!hc)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "hc", NULL);

	hc->magic = AFC_HTTP_CLIENT_MAGIC;

	if (!(hc->inet = afc_inet_client_new()))
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "inet", NULL);

	if (!(hc->req_headers = afc_dictionary_new()))
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "req_headers", NULL);

	if (!(hc->resp_headers = afc_dictionary_new()))
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "resp_headers", NULL);

	if (!(hc->buf = afc_string_new(4096)))
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "buf", NULL);

	if (!(hc->tmp = afc_string_new(4096)))
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "tmp", NULL);

	hc->host = NULL;
	hc->port = 80;
	hc->isconnected = FALSE;
	hc->use_ssl = FALSE;

	hc->req_body = NULL;
	hc->req_body_len = 0;

	hc->status_code = 0;
	hc->status_message = NULL;
	hc->resp_body = NULL;
	hc->resp_body_len = 0;

	hc->timeout = 0;
	hc->follow_redirects = TRUE;
	hc->max_redirects = AFC_HTTP_CLIENT_MAX_REDIRECTS;

	RETURN(hc);

	EXCEPT
	if (hc) afc_http_client_delete(hc);

	FINALLY

	ENDTRY
}
// }}}
// {{{ afc_http_client_delete ( hc )
/*
@node afc_http_client_delete

           NAME: afc_http_client_delete ( hc )  - Disposes a valid HttpClient instance.

       SYNOPSIS: int afc_http_client_delete ( HttpClient * hc)

    DESCRIPTION: This function frees an already alloc'd HttpClient structure.

          INPUT: - hc  - Pointer to a valid afc_http_client class.

        RESULTS: should be AFC_ERR_NO_ERROR

          NOTES: - this method calls: afc_http_client_clear()

       SEE ALSO: - afc_http_client_new()
                 - afc_http_client_clear()
@endnode
*/
int _afc_http_client_delete(HttpClient * hc)
{
	int afc_res;

	if ((afc_res = afc_http_client_clear(hc)) != AFC_ERR_NO_ERROR)
		return afc_res;

	afc_inet_client_delete(hc->inet);
	afc_dictionary_delete(hc->req_headers);
	afc_dictionary_delete(hc->resp_headers);
	afc_string_delete(hc->buf);
	afc_string_delete(hc->tmp);

	if (hc->host) afc_string_delete(hc->host);
	if (hc->status_message) afc_string_delete(hc->status_message);
	if (hc->resp_body) afc_string_delete(hc->resp_body);

	afc_free(hc);

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ afc_http_client_clear ( hc )
/*
@node afc_http_client_clear

           NAME: afc_http_client_clear ( hc )  - Clears all stored data

       SYNOPSIS: int afc_http_client_clear ( HttpClient * hc)

    DESCRIPTION: Use this function to clear all stored data in the current hc instance.

          INPUT: - hc    - Pointer to a valid afc_http_client instance.

        RESULTS: should be AFC_ERR_NO_ERROR

       SEE ALSO: - afc_http_client_delete()

@endnode
*/
int afc_http_client_clear(HttpClient * hc)
{
	if (!hc) return AFC_ERR_NULL_POINTER;

	if (hc->magic != AFC_HTTP_CLIENT_MAGIC) return AFC_ERR_INVALID_POINTER;

	if (hc->isconnected)
	{
		afc_inet_client_close(hc->inet);
		hc->isconnected = FALSE;
	}

	if (hc->inet) afc_inet_client_clear(hc->inet);

	if (hc->req_headers) afc_dictionary_clear(hc->req_headers);
	if (hc->resp_headers) afc_dictionary_clear(hc->resp_headers);

	if (hc->status_message)
	{
		afc_string_delete(hc->status_message);
		hc->status_message = NULL;
	}

	if (hc->resp_body)
	{
		afc_string_delete(hc->resp_body);
		hc->resp_body = NULL;
	}

	hc->status_code = 0;
	hc->resp_body_len = 0;

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ afc_http_client_set_tags ( hc, first_tag, ... )
/*
@node afc_http_client_set_tags

           NAME: afc_http_client_set_tags ( hc, first_tag, ... )  - Set tags for HttpClient

       SYNOPSIS: int afc_http_client_set_tags ( HttpClient * hc, int first_tag, ... )

    DESCRIPTION: Use this function to set tags for the HttpClient instance.

          INPUT: - hc    - Pointer to a valid afc_http_client instance.
                 - first_tag - First tag to set
                 - ... - Tag values

        RESULTS: should be AFC_ERR_NO_ERROR

       SEE ALSO: - afc_http_client_set_tag()

@endnode
*/
int _afc_http_client_set_tags(HttpClient * hc, int first_tag, ...)
{
	va_list tags;
	int tag;
	void * val;
	int res = AFC_ERR_NO_ERROR;

	va_start(tags, first_tag);

	tag = first_tag;

	while (tag != AFC_TAG_END)
	{
		val = va_arg(tags, void *);

		if ((res = afc_http_client_set_tag(hc, tag, val)) != AFC_ERR_NO_ERROR)
			break;

		tag = va_arg(tags, int);
	}

	va_end(tags);

	return res;
}
// }}}
// {{{ afc_http_client_set_tag ( hc, tag, val )
/*
@node afc_http_client_set_tag

           NAME: afc_http_client_set_tag ( hc, tag, val )  - Set a single tag

       SYNOPSIS: int afc_http_client_set_tag ( HttpClient * hc, int tag, void * val )

    DESCRIPTION: Use this function to set a single tag for the HttpClient instance.

          INPUT: - hc    - Pointer to a valid afc_http_client instance.
                 - tag   - Tag to set
                 - val   - Tag value

        RESULTS: should be AFC_ERR_NO_ERROR

       SEE ALSO: - afc_http_client_set_tags()

@endnode
*/
int afc_http_client_set_tag(HttpClient * hc, int tag, void * val)
{
	if (!hc) return AFC_ERR_NULL_POINTER;

	switch (tag)
	{
	case AFC_HTTP_CLIENT_TAG_TIMEOUT:
		hc->timeout = (int)(long)val;
		break;

	case AFC_HTTP_CLIENT_TAG_FOLLOW_REDIRECTS:
		hc->follow_redirects = (BOOL)(long)val;
		break;

	case AFC_HTTP_CLIENT_TAG_MAX_REDIRECTS:
		hc->max_redirects = (int)(long)val;
		break;

	case AFC_HTTP_CLIENT_TAG_USE_SSL:
		hc->use_ssl = (BOOL)(long)val;
		break;

	default:
		return AFC_LOG(AFC_LOG_ERROR, AFC_ERR_UNSUPPORTED_TAG, "Unsupported tag", NULL);
	}

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ afc_http_client_set_header ( hc, name, value )
/*
@node afc_http_client_set_header

           NAME: afc_http_client_set_header ( hc, name, value )  - Set a request header

       SYNOPSIS: int afc_http_client_set_header ( HttpClient * hc, const char * name, const char * value )

    DESCRIPTION: Use this function to set a custom request header.

          INPUT: - hc    - Pointer to a valid afc_http_client instance.
                 - name  - Header name
                 - value - Header value

        RESULTS: should be AFC_ERR_NO_ERROR

@endnode
*/
int afc_http_client_set_header(HttpClient * hc, const char * name, const char * value)
{
	if (!hc) return AFC_ERR_NULL_POINTER;
	if (!name || !value) return AFC_ERR_NULL_POINTER;

	return afc_dictionary_set(hc->req_headers, name, afc_string_dup(value));
}
// }}}
// {{{ afc_http_client_clear_headers ( hc )
/*
@node afc_http_client_clear_headers

           NAME: afc_http_client_clear_headers ( hc )  - Clear all request headers

       SYNOPSIS: int afc_http_client_clear_headers ( HttpClient * hc )

    DESCRIPTION: Use this function to clear all custom request headers.

          INPUT: - hc    - Pointer to a valid afc_http_client instance.

        RESULTS: should be AFC_ERR_NO_ERROR

@endnode
*/
int afc_http_client_clear_headers(HttpClient * hc)
{
	if (!hc) return AFC_ERR_NULL_POINTER;

	return afc_dictionary_clear(hc->req_headers);
}
// }}}
// {{{ afc_http_client_get ( hc, url )
/*
@node afc_http_client_get

           NAME: afc_http_client_get ( hc, url )  - Perform HTTP GET request

       SYNOPSIS: int afc_http_client_get ( HttpClient * hc, const char * url )

    DESCRIPTION: Performs an HTTP GET request to the specified URL.

          INPUT: - hc  - Pointer to a valid afc_http_client instance.
                 - url - Full URL to request

        RESULTS: should be AFC_ERR_NO_ERROR

@endnode
*/
int afc_http_client_get(HttpClient * hc, const char * url)
{
	return afc_http_client_request(hc, "GET", url, NULL, 0);
}
// }}}
// {{{ afc_http_client_post ( hc, url, body, body_len )
/*
@node afc_http_client_post

           NAME: afc_http_client_post ( hc, url, body, body_len )  - Perform HTTP POST request

       SYNOPSIS: int afc_http_client_post ( HttpClient * hc, const char * url, const char * body, int body_len )

    DESCRIPTION: Performs an HTTP POST request to the specified URL with a body.

          INPUT: - hc       - Pointer to a valid afc_http_client instance.
                 - url      - Full URL to request
                 - body     - Request body data
                 - body_len - Length of body data

        RESULTS: should be AFC_ERR_NO_ERROR

@endnode
*/
int afc_http_client_post(HttpClient * hc, const char * url, const char * body, int body_len)
{
	return afc_http_client_request(hc, "POST", url, body, body_len);
}
// }}}
// {{{ afc_http_client_put ( hc, url, body, body_len )
/*
@node afc_http_client_put

           NAME: afc_http_client_put ( hc, url, body, body_len )  - Perform HTTP PUT request

       SYNOPSIS: int afc_http_client_put ( HttpClient * hc, const char * url, const char * body, int body_len )

    DESCRIPTION: Performs an HTTP PUT request to the specified URL with a body.

          INPUT: - hc       - Pointer to a valid afc_http_client instance.
                 - url      - Full URL to request
                 - body     - Request body data
                 - body_len - Length of body data

        RESULTS: should be AFC_ERR_NO_ERROR

@endnode
*/
int afc_http_client_put(HttpClient * hc, const char * url, const char * body, int body_len)
{
	return afc_http_client_request(hc, "PUT", url, body, body_len);
}
// }}}
// {{{ afc_http_client_patch ( hc, url, body, body_len )
/*
@node afc_http_client_patch

           NAME: afc_http_client_patch ( hc, url, body, body_len )  - Perform HTTP PATCH request

       SYNOPSIS: int afc_http_client_patch ( HttpClient * hc, const char * url, const char * body, int body_len )

    DESCRIPTION: Performs an HTTP PATCH request to the specified URL with a body.

          INPUT: - hc       - Pointer to a valid afc_http_client instance.
                 - url      - Full URL to request
                 - body     - Request body data
                 - body_len - Length of body data

        RESULTS: should be AFC_ERR_NO_ERROR

@endnode
*/
int afc_http_client_patch(HttpClient * hc, const char * url, const char * body, int body_len)
{
	return afc_http_client_request(hc, "PATCH", url, body, body_len);
}
// }}}
// {{{ afc_http_client_delete_url ( hc, url )
/*
@node afc_http_client_delete_url

           NAME: afc_http_client_delete_url ( hc, url )  - Perform HTTP DELETE request

       SYNOPSIS: int afc_http_client_delete_url ( HttpClient * hc, const char * url )

    DESCRIPTION: Performs an HTTP DELETE request to the specified URL.

          INPUT: - hc  - Pointer to a valid afc_http_client instance.
                 - url - Full URL to request

        RESULTS: should be AFC_ERR_NO_ERROR

@endnode
*/
int afc_http_client_delete_url(HttpClient * hc, const char * url)
{
	return afc_http_client_request(hc, "DELETE", url, NULL, 0);
}
// }}}
// {{{ afc_http_client_head ( hc, url )
/*
@node afc_http_client_head

           NAME: afc_http_client_head ( hc, url )  - Perform HTTP HEAD request

       SYNOPSIS: int afc_http_client_head ( HttpClient * hc, const char * url )

    DESCRIPTION: Performs an HTTP HEAD request to the specified URL.

          INPUT: - hc  - Pointer to a valid afc_http_client instance.
                 - url - Full URL to request

        RESULTS: should be AFC_ERR_NO_ERROR

@endnode
*/
int afc_http_client_head(HttpClient * hc, const char * url)
{
	return afc_http_client_request(hc, "HEAD", url, NULL, 0);
}
// }}}
// {{{ afc_http_client_options ( hc, url )
/*
@node afc_http_client_options

           NAME: afc_http_client_options ( hc, url )  - Perform HTTP OPTIONS request

       SYNOPSIS: int afc_http_client_options ( HttpClient * hc, const char * url )

    DESCRIPTION: Performs an HTTP OPTIONS request to the specified URL.

          INPUT: - hc  - Pointer to a valid afc_http_client instance.
                 - url - Full URL to request

        RESULTS: should be AFC_ERR_NO_ERROR

@endnode
*/
int afc_http_client_options(HttpClient * hc, const char * url)
{
	return afc_http_client_request(hc, "OPTIONS", url, NULL, 0);
}
// }}}
// {{{ afc_http_client_request ( hc, method, url, body, body_len )
/*
@node afc_http_client_request

           NAME: afc_http_client_request ( hc, method, url, body, body_len )  - Perform HTTP request

       SYNOPSIS: int afc_http_client_request ( HttpClient * hc, const char * method, const char * url, const char * body, int body_len )

    DESCRIPTION: Performs an HTTP request with the specified method, URL, and optional body.
                 This function handles URL parsing, connection, request sending, and response parsing.
                 It also handles automatic redirects if follow_redirects is enabled.

          INPUT: - hc       - Pointer to a valid afc_http_client instance.
                 - method   - HTTP method (GET, POST, PUT, DELETE, PATCH, HEAD, OPTIONS)
                 - url      - Full URL to request (e.g., "http://example.com/path")
                 - body     - Request body data (can be NULL)
                 - body_len - Length of body data (0 if no body)

        RESULTS: should be AFC_ERR_NO_ERROR

@endnode
*/
int afc_http_client_request(HttpClient * hc, const char * method, const char * url, const char * body, int body_len)
{
	TRY(int)

	char * protocol = NULL;
	char * host = NULL;
	int port = 0;
	char * path = NULL;
	int res;

	if (!hc) RAISE_RC(AFC_LOG_ERROR, AFC_ERR_NULL_POINTER, "HttpClient is NULL", "", AFC_ERR_NULL_POINTER);
	if (!method || !url) RAISE_RC(AFC_LOG_ERROR, AFC_ERR_NULL_POINTER, "Method or URL is NULL", "", AFC_ERR_NULL_POINTER);

	// Parse the URL
	res = _afc_http_client_parse_url(url, &protocol, &host, &port, &path);
	if (res != AFC_ERR_NO_ERROR)
		RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_PARSE_URL, "Failed to parse URL", url, res);

	// Determine if we need SSL
	BOOL use_ssl = FALSE;
	if (protocol && strcmp(protocol, "https") == 0)
		use_ssl = TRUE;

	// Check if we need to reconnect
	BOOL need_reconnect = FALSE;
	if (!hc->isconnected)
		need_reconnect = TRUE;
	else if (hc->host && strcmp(hc->host, host) != 0)
		need_reconnect = TRUE;
	else if (hc->port != port)
		need_reconnect = TRUE;
	else if (hc->use_ssl != use_ssl)
		need_reconnect = TRUE;

	// Reconnect if needed
	if (need_reconnect)
	{
		if (hc->isconnected)
		{
			afc_inet_client_close(hc->inet);
			hc->isconnected = FALSE;
		}

		// Update host and port
		if (hc->host) afc_string_delete(hc->host);
		hc->host = afc_string_dup(host);
		hc->port = port;
		hc->use_ssl = use_ssl;

		// Configure timeout on inet_client
		if (hc->timeout > 0)
			afc_inet_client_set_tags(hc->inet, AFC_INET_CLIENT_TAG_TIMEOUT, (void *)(long)hc->timeout, AFC_TAG_END);

		// Open connection
		res = afc_inet_client_open(hc->inet, host, port);
		if (res != AFC_ERR_NO_ERROR)
			RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_REQUEST, "Failed to connect", host, res);

		// Enable SSL if needed
		if (use_ssl)
		{
			res = afc_inet_client_enable_ssl(hc->inet);
			if (res != AFC_ERR_NO_ERROR)
				RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_REQUEST, "Failed to enable SSL", host, res);
		}

		hc->isconnected = TRUE;
	}

	// Clear previous response data
	if (hc->resp_headers) afc_dictionary_clear(hc->resp_headers);
	if (hc->status_message)
	{
		afc_string_delete(hc->status_message);
		hc->status_message = NULL;
	}
	if (hc->resp_body)
	{
		afc_string_delete(hc->resp_body);
		hc->resp_body = NULL;
	}
	hc->status_code = 0;
	hc->resp_body_len = 0;

	// Send request
	res = _afc_http_client_send_request(hc, method, path, body, body_len);
	if (res != AFC_ERR_NO_ERROR)
		RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_REQUEST, "Failed to send request", path, res);

	// Read response
	res = _afc_http_client_read_response(hc);
	if (res != AFC_ERR_NO_ERROR)
		RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_GETRESP, "Failed to read response", "", res);

	// Handle redirects
	if (hc->follow_redirects && hc->status_code >= 300 && hc->status_code < 400)
	{
		res = _afc_http_client_handle_redirect(hc, method, body, body_len, 0);
		if (res != AFC_ERR_NO_ERROR)
			RAISE_RC(AFC_LOG_ERROR, res, "Failed to handle redirect", "", res);
	}

	RETURN(AFC_ERR_NO_ERROR);

	EXCEPT

	FINALLY
	if (protocol) afc_string_delete(protocol);
	if (host) afc_string_delete(host);
	if (path) afc_string_delete(path);

	ENDTRY
}
// }}}
// {{{ afc_http_client_close ( hc )
int afc_http_client_close(HttpClient * hc)
{
	if (!hc) return AFC_ERR_NULL_POINTER;

	if (hc->isconnected)
	{
		afc_inet_client_close(hc->inet);
		hc->isconnected = FALSE;
	}

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ afc_http_client_get_status_code ( hc )
/*
@node afc_http_client_get_status_code

           NAME: afc_http_client_get_status_code ( hc )  - Get HTTP status code

       SYNOPSIS: int afc_http_client_get_status_code ( HttpClient * hc )

    DESCRIPTION: Returns the HTTP status code from the last response.

          INPUT: - hc - Pointer to a valid afc_http_client instance.

        RESULTS: HTTP status code (e.g., 200, 404, 500) or 0 if no response

@endnode
*/
int afc_http_client_get_status_code(HttpClient * hc)
{
	if (!hc) return 0;
	return hc->status_code;
}
// }}}
// {{{ afc_http_client_get_status_message ( hc )
/*
@node afc_http_client_get_status_message

           NAME: afc_http_client_get_status_message ( hc )  - Get HTTP status message

       SYNOPSIS: char * afc_http_client_get_status_message ( HttpClient * hc )

    DESCRIPTION: Returns the HTTP status message from the last response.

          INPUT: - hc - Pointer to a valid afc_http_client instance.

        RESULTS: Status message string (e.g., "OK", "Not Found") or NULL

@endnode
*/
char * afc_http_client_get_status_message(HttpClient * hc)
{
	if (!hc) return NULL;
	return hc->status_message;
}
// }}}
// {{{ afc_http_client_get_response_body ( hc )
/*
@node afc_http_client_get_response_body

           NAME: afc_http_client_get_response_body ( hc )  - Get response body

       SYNOPSIS: char * afc_http_client_get_response_body ( HttpClient * hc )

    DESCRIPTION: Returns the response body from the last request.

          INPUT: - hc - Pointer to a valid afc_http_client instance.

        RESULTS: Response body string or NULL

@endnode
*/
char * afc_http_client_get_response_body(HttpClient * hc)
{
	if (!hc) return NULL;
	return hc->resp_body;
}
// }}}
// {{{ afc_http_client_get_response_body_len ( hc )
/*
@node afc_http_client_get_response_body_len

           NAME: afc_http_client_get_response_body_len ( hc )  - Get response body length

       SYNOPSIS: int afc_http_client_get_response_body_len ( HttpClient * hc )

    DESCRIPTION: Returns the length of the response body from the last request.

          INPUT: - hc - Pointer to a valid afc_http_client instance.

        RESULTS: Length of response body or 0

@endnode
*/
int afc_http_client_get_response_body_len(HttpClient * hc)
{
	if (!hc) return 0;
	return hc->resp_body_len;
}
// }}}
// {{{ afc_http_client_get_response_headers ( hc )
/*
@node afc_http_client_get_response_headers

           NAME: afc_http_client_get_response_headers ( hc )  - Get response headers

       SYNOPSIS: Dictionary * afc_http_client_get_response_headers ( HttpClient * hc )

    DESCRIPTION: Returns the Dictionary containing all response headers.

          INPUT: - hc - Pointer to a valid afc_http_client instance.

        RESULTS: Dictionary of headers or NULL

@endnode
*/
Dictionary * afc_http_client_get_response_headers(HttpClient * hc)
{
	if (!hc) return NULL;
	return hc->resp_headers;
}
// }}}
// {{{ afc_http_client_get_response_header ( hc, name )
/*
@node afc_http_client_get_response_header

           NAME: afc_http_client_get_response_header ( hc, name )  - Get a specific response header

       SYNOPSIS: char * afc_http_client_get_response_header ( HttpClient * hc, const char * name )

    DESCRIPTION: Returns the value of a specific response header.

          INPUT: - hc   - Pointer to a valid afc_http_client instance.
                 - name - Header name to retrieve

        RESULTS: Header value or NULL if not found

@endnode
*/
char * afc_http_client_get_response_header(HttpClient * hc, const char * name)
{
	if (!hc || !name) return NULL;
	return (char *)afc_dictionary_get(hc->resp_headers, name);
}
// }}}

/* Internal Functions */

// {{{ _afc_http_client_parse_url ( url, protocol, host, port, path )
/*
 * Parse a URL into its components
 * URL format: [protocol://]host[:port][/path]
 * Returns: AFC_ERR_NO_ERROR on success
 */
static int _afc_http_client_parse_url(const char * url, char ** protocol, char ** host, int * port, char ** path)
{
	char * url_copy;
	char * p;
	char * colon;
	char * slash;

	if (!url) return AFC_ERR_NULL_POINTER;

	url_copy = afc_string_dup(url);

	*protocol = NULL;
	*host = NULL;
	*port = 80;
	*path = NULL;

	p = url_copy;

	// Extract protocol
	if (strstr(p, "://"))
	{
		*protocol = p;
		p = strstr(p, "://");
		*p = '\0';
		p += 3;
		*protocol = afc_string_dup(*protocol);
	}

	// Extract host and optional port
	slash = strchr(p, '/');
	if (slash)
	{
		*slash = '\0';
		*path = afc_string_dup(slash + 1);
	}

	// Check for port in host
	colon = strchr(p, ':');
	if (colon)
	{
		*colon = '\0';
		*port = atoi(colon + 1);
	}
	else
	{
		// Default ports
		if (*protocol && strcmp(*protocol, "https") == 0)
			*port = 443;
		else
			*port = 80;
	}

	*host = afc_string_dup(p);

	// Default path
	if (!*path)
		*path = afc_string_dup("");

	afc_string_delete(url_copy);

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ _afc_http_client_send_request ( hc, method, path, body, body_len )
/*
 * Send an HTTP request
 */
static int _afc_http_client_send_request(HttpClient * hc, const char * method, const char * path, const char * body, int body_len)
{
	TRY(int)

	char * key;
	char * val;
	int res;

	// Build request line
	afc_string_clear(hc->buf);
	afc_string_make(hc->buf, "%s /%s HTTP/1.1\r\n", method, path);

	res = afc_inet_client_send(hc->inet, hc->buf, afc_string_len(hc->buf));
	if (res != AFC_ERR_NO_ERROR)
		RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_REQUEST, "Failed to send request line", hc->buf, res);

	// Send Host header (required for HTTP/1.1)
	afc_string_clear(hc->buf);
	afc_string_make(hc->buf, "Host: %s\r\n", hc->host);
	res = afc_inet_client_send(hc->inet, hc->buf, afc_string_len(hc->buf));
	if (res != AFC_ERR_NO_ERROR)
		RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_REQUEST, "Failed to send Host header", hc->buf, res);

	// Send Content-Length if body is present
	if (body && body_len > 0)
	{
		afc_string_clear(hc->buf);
		afc_string_make(hc->buf, "Content-Length: %d\r\n", body_len);
		res = afc_inet_client_send(hc->inet, hc->buf, afc_string_len(hc->buf));
		if (res != AFC_ERR_NO_ERROR)
			RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_REQUEST, "Failed to send Content-Length", hc->buf, res);
	}

	// Send custom headers
	if ((val = (char *)afc_dictionary_first(hc->req_headers)))
	{
		do
		{
			key = afc_dictionary_get_key(hc->req_headers);

			afc_string_clear(hc->buf);
			afc_string_make(hc->buf, "%s: %s\r\n", key, val);

			res = afc_inet_client_send(hc->inet, hc->buf, afc_string_len(hc->buf));
			if (res != AFC_ERR_NO_ERROR)
				RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_REQUEST, "Failed to send header", hc->buf, res);

		} while ((val = (char *)afc_dictionary_succ(hc->req_headers)));
	}

	// Send empty line to end headers
	res = afc_inet_client_send(hc->inet, "\r\n", 2);
	if (res != AFC_ERR_NO_ERROR)
		RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_REQUEST, "Failed to send header terminator", "", res);

	// Send body if present
	if (body && body_len > 0)
	{
		res = afc_inet_client_send(hc->inet, body, body_len);
		if (res != AFC_ERR_NO_ERROR)
			RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_REQUEST, "Failed to send body", "", res);
	}

	RETURN(AFC_ERR_NO_ERROR);

	EXCEPT

	FINALLY

	ENDTRY
}
// }}}
// {{{ _afc_http_client_read_response ( hc )
/*
 * Read HTTP response (status line, headers, and body)
 */
static int _afc_http_client_read_response(HttpClient * hc)
{
	TRY(int)

	FILE * fd;
	int res;

	fd = afc_inet_client_get_file(hc->inet);
	if (!fd)
		RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_GETRESP, "Failed to get file descriptor", "", AFC_ERR_NULL_POINTER);

	// Read status line
	if (!fgets(hc->buf, afc_string_max(hc->buf), fd))
		RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_GETRESP, "Failed to read status line", "", AFC_INET_CLIENT_ERR_RECEIVE);

	afc_string_trim(hc->buf);

	res = _afc_http_client_parse_status_line(hc, hc->buf);
	if (res != AFC_ERR_NO_ERROR)
		RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_INVALID_STATUS, "Failed to parse status line", hc->buf, res);

	// Read headers
	res = _afc_http_client_parse_headers(hc, fd);
	if (res != AFC_ERR_NO_ERROR)
		RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_GETRESP, "Failed to parse headers", "", res);

	// Read body (if not HEAD request)
	res = _afc_http_client_read_body(hc, fd);
	if (res != AFC_ERR_NO_ERROR)
		RAISE_RC(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_GETRESP, "Failed to read body", "", res);

	RETURN(AFC_ERR_NO_ERROR);

	EXCEPT

	FINALLY

	ENDTRY
}
// }}}
// {{{ _afc_http_client_parse_status_line ( hc, line )
/*
 * Parse HTTP status line: "HTTP/1.1 200 OK"
 */
static int _afc_http_client_parse_status_line(HttpClient * hc, const char * line)
{
	char * line_copy;
	char * p;
	char * status_str;
	char * message;

	if (!line) return AFC_ERR_NULL_POINTER;

	line_copy = afc_string_dup(line);

	// Skip "HTTP/x.x "
	p = strchr(line_copy, ' ');
	if (!p)
	{
		afc_string_delete(line_copy);
		return AFC_HTTP_CLIENT_ERR_INVALID_STATUS;
	}
	p++;

	// Extract status code
	status_str = p;
	p = strchr(p, ' ');
	if (p)
	{
		*p = '\0';
		p++;
		message = p;
	}
	else
	{
		message = "";
	}

	hc->status_code = atoi(status_str);

	if (hc->status_message)
		afc_string_delete(hc->status_message);
	hc->status_message = afc_string_dup(message);

	afc_string_delete(line_copy);

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ _afc_http_client_parse_headers ( hc, fd )
/*
 * Parse HTTP response headers
 */
static int _afc_http_client_parse_headers(HttpClient * hc, FILE * fd)
{
	char * line;
	char * colon;
	char * name;
	char * value;

	line = afc_string_new(1024);

	while (fgets(line, afc_string_max(line), fd))
	{
		afc_string_trim(line);

		// Empty line marks end of headers
		if (afc_string_len(line) == 0)
			break;

		// Parse "Name: Value"
		colon = strchr(line, ':');
		if (!colon)
			continue;

		*colon = '\0';
		name = line;
		value = colon + 1;

		// Trim leading space from value
		while (*value == ' ')
			value++;

		// Store header (convert name to lowercase for case-insensitive lookup)
		afc_string_lower(name);
		afc_dictionary_set(hc->resp_headers, name, afc_string_dup(value));
	}

	afc_string_delete(line);

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ _afc_http_client_read_body ( hc, fd )
/*
 * Read HTTP response body
 * Handles both Content-Length and chunked transfer encoding
 */
static int _afc_http_client_read_body(HttpClient * hc, FILE * fd)
{
	char * content_length_str;
	char * transfer_encoding;
	int content_length;
	int bytes_read;
	char chunk_size_str[32];
	int chunk_size;

	// Check for Content-Length
	content_length_str = (char *)afc_dictionary_get(hc->resp_headers, "content-length");
	transfer_encoding = (char *)afc_dictionary_get(hc->resp_headers, "transfer-encoding");

	// Initialize response body
	if (hc->resp_body)
		afc_string_delete(hc->resp_body);
	hc->resp_body = afc_string_new(4096);
	hc->resp_body_len = 0;

	// Handle chunked encoding
	if (transfer_encoding && strstr(transfer_encoding, "chunked"))
	{
		while (1)
		{
			// Read chunk size line
			if (!fgets(chunk_size_str, sizeof(chunk_size_str), fd))
				break;

			chunk_size = (int)strtol(chunk_size_str, NULL, 16);
			if (chunk_size <= 0)
				break;

			// Read chunk data
			while (chunk_size > 0)
			{
				int to_read = chunk_size < afc_string_max(hc->buf) ? chunk_size : afc_string_max(hc->buf);
				bytes_read = fread(hc->buf, 1, to_read, fd);
				if (bytes_read <= 0)
					break;

				hc->buf[bytes_read] = '\0';
				afc_string_add(hc->resp_body, hc->buf, ALL);
				hc->resp_body_len += bytes_read;
				chunk_size -= bytes_read;
			}

			// Read trailing CRLF after chunk data
			fgets(chunk_size_str, sizeof(chunk_size_str), fd);
		}
	}
	// Handle Content-Length
	else if (content_length_str)
	{
		content_length = atoi(content_length_str);

		while (content_length > 0)
		{
			int to_read = content_length < afc_string_max(hc->buf) ? content_length : afc_string_max(hc->buf);
			bytes_read = fread(hc->buf, 1, to_read, fd);
			if (bytes_read <= 0)
				break;

			hc->buf[bytes_read] = '\0';
			afc_string_add(hc->resp_body, hc->buf, ALL);
			hc->resp_body_len += bytes_read;
			content_length -= bytes_read;
		}
	}
	// No Content-Length and no chunked encoding - read until connection closes
	else
	{
		while ((bytes_read = fread(hc->buf, 1, afc_string_max(hc->buf), fd)) > 0)
		{
			hc->buf[bytes_read] = '\0';
			afc_string_add(hc->resp_body, hc->buf, ALL);
			hc->resp_body_len += bytes_read;
		}
	}

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ _afc_http_client_handle_redirect ( hc, method, body, body_len, redirect_count )
/*
 * Handle HTTP redirect (3xx status codes)
 */
static int _afc_http_client_handle_redirect(HttpClient * hc, const char * method, const char * body, int body_len, int redirect_count)
{
	char * location;
	int res;

	if (redirect_count >= hc->max_redirects)
		return AFC_LOG(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_TOO_MANY_REDIRECTS, "Too many redirects", NULL);

	location = (char *)afc_dictionary_get(hc->resp_headers, "location");
	if (!location)
		return AFC_LOG(AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_GETRESP, "Redirect without Location header", NULL);

	// For 301, 302, 303, change method to GET (except for 307, 308)
	if (hc->status_code == 303 ||
	    ((hc->status_code == 301 || hc->status_code == 302) && strcmp(method, "GET") != 0 && strcmp(method, "HEAD") != 0))
	{
		method = "GET";
		body = NULL;
		body_len = 0;
	}

	// Follow redirect
	res = afc_http_client_request(hc, method, location, body, body_len);
	if (res != AFC_ERR_NO_ERROR)
		return res;

	// Check if we got another redirect
	if (hc->status_code >= 300 && hc->status_code < 400)
		return _afc_http_client_handle_redirect(hc, method, body, body_len, redirect_count + 1);

	return AFC_ERR_NO_ERROR;
}
// }}}

#ifdef TEST_CLASS
int main(int argc, char * argv[])
{
	AFC * afc = afc_new();
	HttpClient * hc = afc_http_client_new();

	if (!hc)
	{
		fprintf(stderr, "Init of class HttpClient failed.\n");
		return 1;
	}

	// Test GET request
	printf("Testing HTTP GET request...\n");
	int res = afc_http_client_get(hc, "http://httpbin.org/get");
	if (res == AFC_ERR_NO_ERROR)
	{
		printf("Status: %d %s\n",
			afc_http_client_get_status_code(hc),
			afc_http_client_get_status_message(hc));
		printf("Body:\n%s\n", afc_http_client_get_response_body(hc));
	}

	afc_http_client_delete(hc);
	afc_delete(afc);

	return 0;
}
#endif
