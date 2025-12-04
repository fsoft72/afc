#ifndef AFC_HTTP_CLIENT_H
#define AFC_HTTP_CLIENT_H
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "afc.h"

/* HttpClient'Magic' value: 'HTTP' */
#define AFC_HTTP_CLIENT_MAGIC ( 'H' << 24 | 'T' << 16 | 'T' << 8 | 'P' )

/* HttpClient Base  */
#define AFC_HTTP_CLIENT_BASE 0x1000

/* Maximum redirects to follow */
#define AFC_HTTP_CLIENT_MAX_REDIRECTS 10

// HTTP Client tags for configuration
enum {
	AFC_HTTP_CLIENT_TAG_HOST = AFC_HTTP_CLIENT_BASE + 100,
	AFC_HTTP_CLIENT_TAG_PORT,
	AFC_HTTP_CLIENT_TAG_TIMEOUT,
	AFC_HTTP_CLIENT_TAG_FOLLOW_REDIRECTS,
	AFC_HTTP_CLIENT_TAG_MAX_REDIRECTS,
	AFC_HTTP_CLIENT_TAG_USE_SSL
};

// HTTP Client error codes
enum {
	AFC_HTTP_CLIENT_ERR_REQUEST = AFC_HTTP_CLIENT_BASE,
	AFC_HTTP_CLIENT_ERR_GETRESP,
	AFC_HTTP_CLIENT_ERR_PARSE_URL,
	AFC_HTTP_CLIENT_ERR_INVALID_STATUS,
	AFC_HTTP_CLIENT_ERR_TOO_MANY_REDIRECTS,
	AFC_HTTP_CLIENT_ERR_NO_MEMORY,
	AFC_HTTP_CLIENT_ERR_INVALID_METHOD
};

struct afc_http_client
{
	unsigned long magic;     /* HttpClient Magic Value */
	InetClient * inet;       /* InetClient for network operations */

	char * host;             /* Current host */
	int port;                /* Current port */
	BOOL isconnected;        /* Connection status */
	BOOL use_ssl;            /* SSL/TLS flag */

	// Request data
	Dictionary * req_headers;  /* Request headers */
	char * req_body;           /* Request body */
	int req_body_len;          /* Request body length */

	// Response data
	int status_code;           /* HTTP status code */
	char * status_message;     /* HTTP status message */
	Dictionary * resp_headers; /* Response headers */
	char * resp_body;          /* Response body */
	int resp_body_len;         /* Response body length */

	// Configuration
	int timeout;               /* Timeout in seconds */
	BOOL follow_redirects;     /* Follow 3xx redirects */
	int max_redirects;         /* Maximum redirects to follow */

	// Internal buffers
	char * buf;                /* General purpose buffer */
	char * tmp;                /* Temporary buffer */
};

typedef struct afc_http_client HttpClient;

/* Function Prototypes */
#define afc_http_client_delete(hc)   \
	if (hc)                          \
	{                                \
		_afc_http_client_delete(hc); \
		hc = NULL;                   \
	}

// Core functions
HttpClient * afc_http_client_new ( void );
int _afc_http_client_delete ( HttpClient * hc );
int afc_http_client_clear ( HttpClient * hc );

// Configuration functions
#define afc_http_client_set_tags(hc, first, ...) _afc_http_client_set_tags(hc, first, ##__VA_ARGS__, AFC_TAG_END)
int _afc_http_client_set_tags(HttpClient * hc, int first_tag, ...);
int afc_http_client_set_tag(HttpClient * hc, int tag, void * val);

// Request header management
int afc_http_client_set_header(HttpClient * hc, const char * name, const char * value);
int afc_http_client_clear_headers(HttpClient * hc);

// HTTP method functions
int afc_http_client_get(HttpClient * hc, const char * url);
int afc_http_client_post(HttpClient * hc, const char * url, const char * body, int body_len);
int afc_http_client_put(HttpClient * hc, const char * url, const char * body, int body_len);
int afc_http_client_patch(HttpClient * hc, const char * url, const char * body, int body_len);
int afc_http_client_delete_url(HttpClient * hc, const char * url);
int afc_http_client_head(HttpClient * hc, const char * url);
int afc_http_client_options(HttpClient * hc, const char * url);

// Generic request function
int afc_http_client_request(HttpClient * hc, const char * method, const char * url, const char * body, int body_len);

// Response access functions
int afc_http_client_get_status_code(HttpClient * hc);
char * afc_http_client_get_status_message(HttpClient * hc);
char * afc_http_client_get_response_body(HttpClient * hc);
int afc_http_client_get_response_body_len(HttpClient * hc);
Dictionary * afc_http_client_get_response_headers(HttpClient * hc);
char * afc_http_client_get_response_header(HttpClient * hc, const char * name);

// Connection management
int afc_http_client_close(HttpClient * hc);

#endif
