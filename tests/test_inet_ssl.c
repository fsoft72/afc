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
 * test_inet_ssl.c - TLS hardening tests for the InetClient module.
 *
 * Tests cover:
 *   - TLS handshake with a valid certificate succeeds (example.com)
 *   - TLS handshake with a self-signed certificate is rejected
 *     (self-signed.badssl.com)
 *   - TLS handshake with a mismatched hostname is rejected
 *     (wrong.host.badssl.com)
 *   - Host name is stored on open() for SNI / verification
 *
 * NOTE: these tests require internet access.
 */

#include "test_utils.h"
#include "../src/inet_client.h"

static int test_valid_cert(void)
{
	InetClient *ic = afc_inet_client_new();
	int res;

	print_res("new() for valid cert", (void *)1, (void *)(long)(ic != NULL), 0);
	if (!ic)
		return 1;

	afc_inet_client_set_tag(ic, AFC_INET_CLIENT_TAG_TIMEOUT, (void *)(long)10);

	res = afc_inet_client_open(ic, "example.com", 443);
	print_res("open example.com:443", (void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* The host name must be stored for SNI / verification */
	print_res("host stored", (void *)"example.com", (void *)(ic->host ? ic->host : ""), 1);

	res = afc_inet_client_enable_ssl(ic);
	print_res("enable_ssl valid cert", (void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	print_res("use_ssl flag set", (void *)(long)TRUE, (void *)(long)ic->use_ssl, 0);

	afc_inet_client_close(ic);
	afc_inet_client_delete(ic);

	return 0;
}

static int test_rejected_cert(const char *host, const char *label)
{
	InetClient *ic = afc_inet_client_new();
	int res;

	if (!ic)
		return 1;

	afc_inet_client_set_tag(ic, AFC_INET_CLIENT_TAG_TIMEOUT, (void *)(long)10);

	res = afc_inet_client_open(ic, host, 443);
	print_res("open badssl host", (void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	res = afc_inet_client_enable_ssl(ic);
	/* Handshake or post-handshake verification must fail */
	print_res((char *)label, (void *)1, (void *)(long)(res != AFC_ERR_NO_ERROR), 0);

	print_res("use_ssl stays FALSE", (void *)(long)FALSE, (void *)(long)ic->use_ssl, 0);

	afc_inet_client_close(ic);
	afc_inet_client_delete(ic);

	return 0;
}

int main(void)
{
	AFC *afc = afc_new();

	test_header();

	test_valid_cert();
	print_row();

	test_rejected_cert("self-signed.badssl.com", "self-signed cert rejected");
	print_row();

	test_rejected_cert("wrong.host.badssl.com", "wrong hostname rejected");
	print_row();

	print_summary();
	afc_delete(afc);
	return get_test_failures();
}
