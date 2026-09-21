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
 * test_sigpipe.c - Regression test: writing to a closed socket must return
 * an error instead of killing the process with SIGPIPE.
 *
 * Without MSG_NOSIGNAL in afc_inet_client_send(), this test process would
 * be terminated by SIGPIPE before reaching the checks below.
 */

#include <sys/socket.h>
#include <unistd.h>

#include "test_utils.h"
#include "../src/inet_client.h"

int main(void)
{
	AFC *afc = afc_new();
	InetClient *ic;
	int sv[2];
	int res;

	test_header();

	ic = afc_inet_client_new();
	print_res("inet_client new", (void *)1, (void *)(long)(ic != NULL), 0);
	if (!ic)
	{
		print_summary();
		afc_delete(afc);
		return 1;
	}

	/* Connected socket pair; close the peer immediately */
	res = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
	print_res("socketpair created", (void *)(long)0, (void *)(long)res, 0);
	close(sv[1]);

	ic->sockfd = sv[0];

	/* First send may succeed (buffers not yet aware of the close) */
	afc_inet_client_send(ic, "first", 0);

	/* Second send must fail with an error, NOT kill us with SIGPIPE */
	res = afc_inet_client_send(ic, "second", 0);
	print_res("send to closed socket errors", (void *)1, (void *)(long)(res != AFC_ERR_NO_ERROR), 0);

	/* Reaching this line at all proves SIGPIPE did not kill the process */
	print_res("process survived SIGPIPE", (void *)1, (void *)1, 0);

	close(sv[0]);
	ic->sockfd = -1;
	afc_inet_client_delete(ic);

	print_summary();
	afc_delete(afc);
	return get_test_failures();
}
