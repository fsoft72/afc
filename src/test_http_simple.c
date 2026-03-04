/*
 * Simple test program for HttpClient
 */
#include <stdio.h>
#include <string.h>
#include "afc.h"
#include "http_client.h"

int main(int argc, char * argv[])
{
	AFC * afc = afc_new();
	HttpClient * hc;
	int res;

	printf("=== HTTP Client Simple Test ===\n\n");

	// Create HttpClient
	printf("Creating HttpClient...\n");
	hc = afc_http_client_new();
	if (!hc)
	{
		fprintf(stderr, "Failed to create HttpClient\n");
		afc_delete(afc);
		return 1;
	}
	printf("HttpClient created successfully\n\n");

	// Test URL parsing and basic GET
	printf("Test: HTTP GET request to example.com\n");
	printf("--------------------------------------\n");
	res = afc_http_client_get(hc, "http://example.com/");
	printf("Request returned: %d\n", res);

	if (res == AFC_ERR_NO_ERROR)
	{
		int status = afc_http_client_get_status_code(hc);
		char * message = afc_http_client_get_status_message(hc);
		int body_len = afc_http_client_get_response_body_len(hc);
		char * body = afc_http_client_get_response_body(hc);

		printf("Status Code: %d\n", status);
		printf("Status Message: %s\n", message ? message : "(null)");
		printf("Body Length: %d bytes\n", body_len);

		if (body && body_len > 0)
		{
			int preview_len = body_len < 300 ? body_len : 300;
			printf("Body Preview: %.*s", preview_len, body);
			if (body_len > 300)
				printf("... [truncated]");
			printf("\n");
		}

		// Check response headers
		char * content_type = afc_http_client_get_response_header(hc, "content-type");
		if (content_type)
			printf("Content-Type: %s\n", content_type);

		printf("\nSUCCESS: HTTP GET request completed\n");
	}
	else
	{
		fprintf(stderr, "Request failed with error code: %d\n", res);
		printf("FAILED: HTTP GET request failed\n");
	}

	printf("\n");

	// Test timeout configuration
	printf("Test: Setting timeout to 10 seconds\n");
	printf("--------------------------------------\n");
	res = afc_http_client_set_tags(hc,
		AFC_HTTP_CLIENT_TAG_TIMEOUT, (void *)10,
		AFC_TAG_END);
	printf("Timeout set result: %d\n", res);
	printf("\n");

	// Test custom headers
	printf("Test: Setting custom headers\n");
	printf("--------------------------------------\n");
	res = afc_http_client_set_header(hc, "User-Agent", "AFC-HttpClient/1.0-Test");
	printf("Set User-Agent result: %d\n", res);
	res = afc_http_client_set_header(hc, "Accept", "text/html");
	printf("Set Accept result: %d\n", res);
	printf("\n");

	// Cleanup
	printf("Cleaning up...\n");
	afc_http_client_delete(hc);
	afc_delete(afc);
	printf("Cleanup complete\n\n");

	printf("=== All tests completed successfully ===\n");

	return 0;
}
