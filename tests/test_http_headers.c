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
 * test_http_headers.c - Regression test: a server flooding response headers
 * must not cause unbounded memory growth in the HttpClient.
 *
 * Spawns a local TCP server (thread) that replies with far more headers than
 * AFC_HTTP_CLIENT_MAX_HEADERS, then verifies the request fails cleanly.
 */

#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "test_utils.h"
#include "../src/http_client.h"

static int srv_port = 0;

static void *flood_server(void *arg)
{
	int fd = *(int *)arg;
	int client;
	char buf[1024];
	int i;

	client = accept(fd, NULL, NULL);
	if (client < 0)
		return NULL;

	/* Read the request (single read is enough for this test) */
	read(client, buf, sizeof(buf));

	send(client, "HTTP/1.1 200 OK\r\n", 17, MSG_NOSIGNAL);

	/* Flood: more headers than AFC_HTTP_CLIENT_MAX_HEADERS allows */
	for (i = 0; i < AFC_HTTP_CLIENT_MAX_HEADERS + 50; i++)
	{
		int n = snprintf(buf, sizeof(buf), "X-Flood-%d: %d\r\n", i, i);
		send(client, buf, n, MSG_NOSIGNAL);
	}
	send(client, "\r\n", 2, MSG_NOSIGNAL);
	send(client, "done", 4, MSG_NOSIGNAL);

	close(client);
	return NULL;
}

int main(void)
{
	AFC *afc = afc_new();
	HttpClient *hc;
	pthread_t th;
	struct sockaddr_in addr;
	int fd;
	char url[64];
	int res;

	test_header();

	/* Local server on an ephemeral port */
	fd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = 0;
	bind(fd, (struct sockaddr *)&addr, sizeof(addr));
	listen(fd, 1);
	{
		socklen_t l = sizeof(addr);
		getsockname(fd, (struct sockaddr *)&addr, &l);
		srv_port = ntohs(addr.sin_port);
	}
	print_res("server bound", (void *)1, (void *)(long)(srv_port > 0), 0);

	pthread_create(&th, NULL, flood_server, &fd);

	hc = afc_http_client_new();
	afc_http_client_set_tag(hc, AFC_HTTP_CLIENT_TAG_TIMEOUT, (void *)(long)10);

	snprintf(url, sizeof(url), "http://127.0.0.1:%d/", srv_port);
	res = afc_http_client_get(hc, url);

	/* The flood must be rejected with an error, not swallowed */
	print_res("header flood rejected", (void *)1, (void *)(long)(res != AFC_ERR_NO_ERROR), 0);

	/* And the client must not have stored all flooded headers */
	print_res("stored headers capped",
		(void *)1,
		(void *)(long)(afc_dictionary_num_items(hc->resp_headers) <= AFC_HTTP_CLIENT_MAX_HEADERS),
		0);

	pthread_join(th, NULL);
	close(fd);
	afc_http_client_delete(hc);

	print_summary();
	afc_delete(afc);
	return get_test_failures();
}
