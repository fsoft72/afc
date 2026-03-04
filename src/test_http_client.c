/*
 * Test program for HttpClient
 */
#include <stdio.h>
#include <string.h>
#include "afc.h"
#include "http_client.h"

int main(int argc, char * argv[])
{
	AFC * afc = afc_new();
	HttpClient * hc = afc_http_client_new();
	int res;

	if (!hc)
	{
		fprintf(stderr, "Failed to create HttpClient\n");
		return 1;
	}

	printf("=== HTTP Client Test Suite ===\n\n");

	// Test 1: Simple GET request
	printf("Test 1: HTTP GET request to httpbin.org\n");
	printf("----------------------------------------\n");
	res = afc_http_client_get(hc, "http://httpbin.org/get");
	if (res == AFC_ERR_NO_ERROR)
	{
		printf("Status: %d %s\n",
			afc_http_client_get_status_code(hc),
			afc_http_client_get_status_message(hc));
		printf("Body length: %d bytes\n", afc_http_client_get_response_body_len(hc));
		printf("Body preview: %.200s...\n\n", afc_http_client_get_response_body(hc));
	}
	else
	{
		printf("Request failed with error: %d\n\n", res);
	}

	// Test 2: HTTPS request
	printf("Test 2: HTTPS GET request\n");
	printf("----------------------------------------\n");
	res = afc_http_client_get(hc, "https://httpbin.org/get");
	if (res == AFC_ERR_NO_ERROR)
	{
		printf("Status: %d %s\n",
			afc_http_client_get_status_code(hc),
			afc_http_client_get_status_message(hc));
		printf("Body length: %d bytes\n", afc_http_client_get_response_body_len(hc));
		printf("Body preview: %.200s...\n\n", afc_http_client_get_response_body(hc));
	}
	else
	{
		printf("Request failed with error: %d\n\n", res);
	}

	// Test 3: POST request with body
	printf("Test 3: HTTP POST request with JSON body\n");
	printf("----------------------------------------\n");
	const char * json_body = "{\"test\":\"data\",\"value\":123}";
	afc_http_client_set_header(hc, "Content-Type", "application/json");
	res = afc_http_client_post(hc, "http://httpbin.org/post", json_body, strlen(json_body));
	if (res == AFC_ERR_NO_ERROR)
	{
		printf("Status: %d %s\n",
			afc_http_client_get_status_code(hc),
			afc_http_client_get_status_message(hc));
		printf("Body length: %d bytes\n", afc_http_client_get_response_body_len(hc));
		printf("Body preview: %.200s...\n\n", afc_http_client_get_response_body(hc));
	}
	else
	{
		printf("Request failed with error: %d\n\n", res);
	}

	// Test 4: Custom headers
	printf("Test 4: GET request with custom headers\n");
	printf("----------------------------------------\n");
	afc_http_client_clear_headers(hc);
	afc_http_client_set_header(hc, "User-Agent", "AFC-HttpClient/1.0");
	afc_http_client_set_header(hc, "X-Custom-Header", "TestValue");
	res = afc_http_client_get(hc, "http://httpbin.org/headers");
	if (res == AFC_ERR_NO_ERROR)
	{
		printf("Status: %d %s\n",
			afc_http_client_get_status_code(hc),
			afc_http_client_get_status_message(hc));
		printf("Body preview: %.200s...\n\n", afc_http_client_get_response_body(hc));
	}
	else
	{
		printf("Request failed with error: %d\n\n", res);
	}

	// Test 5: Redirect following
	printf("Test 5: Redirect following\n");
	printf("----------------------------------------\n");
	res = afc_http_client_get(hc, "http://httpbin.org/redirect/2");
	if (res == AFC_ERR_NO_ERROR)
	{
		printf("Status: %d %s\n",
			afc_http_client_get_status_code(hc),
			afc_http_client_get_status_message(hc));
		printf("Redirect successfully followed!\n");
		printf("Body preview: %.200s...\n\n", afc_http_client_get_response_body(hc));
	}
	else
	{
		printf("Request failed with error: %d\n\n", res);
	}

	// Test 6: Timeout configuration
	printf("Test 6: Timeout configuration (5 second timeout)\n");
	printf("----------------------------------------\n");
	afc_http_client_set_tags(hc,
		AFC_HTTP_CLIENT_TAG_TIMEOUT, (void *)5,
		AFC_TAG_END);
	res = afc_http_client_get(hc, "http://httpbin.org/delay/2");
	if (res == AFC_ERR_NO_ERROR)
	{
		printf("Status: %d %s\n",
			afc_http_client_get_status_code(hc),
			afc_http_client_get_status_message(hc));
		printf("Request completed within timeout\n\n");
	}
	else
	{
		printf("Request failed with error: %d (timeout or other error)\n\n", res);
	}

	// Test 7: Response headers
	printf("Test 7: Reading response headers\n");
	printf("----------------------------------------\n");
	res = afc_http_client_get(hc, "http://httpbin.org/response-headers?X-Test-Header=TestValue");
	if (res == AFC_ERR_NO_ERROR)
	{
		printf("Status: %d %s\n",
			afc_http_client_get_status_code(hc),
			afc_http_client_get_status_message(hc));

		char * content_type = afc_http_client_get_response_header(hc, "content-type");
		if (content_type)
			printf("Content-Type: %s\n", content_type);

		char * server = afc_http_client_get_response_header(hc, "server");
		if (server)
			printf("Server: %s\n", server);

		printf("\n");
	}
	else
	{
		printf("Request failed with error: %d\n\n", res);
	}

	printf("=== All tests completed ===\n");

	afc_http_client_delete(hc);
	afc_delete(afc);

	return 0;
}
