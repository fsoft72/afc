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
#ifndef AFC_INET_CLIENT_H
#define AFC_INET_CLIENT_H
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
// #include <resolv.h>
#include <netdb.h>
#include <stdlib.h>
#include <malloc.h>
#include <arpa/inet.h>

#include "base.h"
#include "strings.h"
#include "exceptions.h"

/* InetClient'Magic' value: 'IBSE' */
#define AFC_INET_CLIENT_MAGIC ('I' << 24 | 'C' << 16 | 'L' << 8 | 'I')

/* InetClient Base  */
#define AFC_INET_CLIENT_BASE 0x1000

enum
{
	AFC_INET_CLIENT_ERR_SOCKET = AFC_INET_CLIENT_BASE + 1,
	AFC_INET_CLIENT_ERR_RESOLVE,
	AFC_INET_CLIENT_ERR_HOST_UNKNOWN,
	AFC_INET_CLIENT_ERR_CONNECT,
	AFC_INET_CLIENT_ERR_RECEIVE,
	AFC_INET_CLIENT_ERR_END_OF_STREAM,
	AFC_INET_CLIENT_ERR_SEND
};

struct afc_inet_client
{
	unsigned long magic; /* InetClient Magic Value */

	int sockfd;					  /* Socket File Descriptor */
	struct sockaddr_in dest_addr; /* Destination Address */

	char *buf; /* Data Buffer */

	FILE *fd; /* File Descriptor */
};

typedef struct afc_inet_client InetClient;

/* Function Prototypes */
#define afc_inet_client_delete(ic)   \
	if (ic)                          \
	{                                \
		_afc_inet_client_delete(ic); \
		ic = NULL;                   \
	}

InetClient *afc_inet_client_new(void);
int _afc_inet_client_delete(InetClient *ic);
int afc_inet_client_clear(InetClient *ic);
int afc_inet_client_open(InetClient *ic, char *site_name, int port);
int afc_inet_client_close(InetClient *ic);
struct hostent *afc_inet_client_resolve(InetClient *ic, char *site_name);
int afc_inet_client_get(InetClient *ic);
int afc_inet_client_send(InetClient *ic, const char *str, int len);
FILE *afc_inet_client_get_file(InetClient *ic);
#endif
