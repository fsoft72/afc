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
#ifndef AFC_INET_SERVER_H
#define AFC_INET_SERVER_H
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <stdlib.h>
#include <malloc.h>
#include <arpa/inet.h>

#include "base.h"
#include "strings.h"
#include "hash.h"
#include "exceptions.h"

/* InetServer'Magic' value: 'IBSE' */
#define AFC_INET_SERVER_MAGIC ('I' << 24 | 'B' << 16 | 'S' << 8 | 'E')

/* InetServer Base  */
#define AFC_INET_SERVER_BASE 0x1000

enum
{
	AFC_INET_SERVER_ERR_SOCKET = AFC_INET_SERVER_BASE + 1,
	AFC_INET_SERVER_ERR_CONNECT,
	AFC_INET_SERVER_ERR_RECEIVE,
	AFC_INET_SERVER_ERR_END_OF_STREAM,
	AFC_INET_SERVER_ERR_SEND
};

#define AFC_INET_SERVER_DEFAULT_BUFSIZE 256

struct afc_inet_server;
struct afc_inet_server_connection_data;

typedef int (*InetServerCBConnect)(struct afc_inet_server *is, struct afc_inet_server_connection_data *);
typedef int (*InetServerCBClose)(struct afc_inet_server *is, struct afc_inet_server_connection_data *);

typedef int (*InetServerCBReceive)(struct afc_inet_server *is, struct afc_inet_server_connection_data *);

struct afc_inet_server_connection_data
{
	int fd;

	struct afc_inet_server *is;

	char *buf;

	InetServerCBConnect cb_connect;
	InetServerCBClose cb_close;
	InetServerCBReceive cb_receive;

	void *data;
};

typedef struct afc_inet_server_connection_data InetConnData;

struct afc_inet_server
{
	unsigned long magic; /* InetServer Magic Value */

	fd_set master;				   /* Master FD set */
	fd_set read_fds;			   /* FD set used   */
	struct sockaddr_in myaddr;	   /* IP Addr of the server */
	struct sockaddr_in remoteaddr; /* IP Addr of the destination client */

	int fdmax;	  /* Max FD value (needed by select() */
	int listener; /* Listener FD */
	int newfd;	  /* FD of a new client connecting */

	Hash *hash; /* Keep track of connections data */

	int active; /* Used to find the current active connection */

	int bufsize; /* Size (in bytes) of the Connection Buffer Data */

	InetServerCBConnect cb_connect;
	InetServerCBClose cb_close;
	InetServerCBReceive cb_receive;

	void *data; /* Generic Data Pointer */
};

typedef struct afc_inet_server InetServer;

/* Function Prototypes */
#define afc_inet_server_delete(is)   \
	if (is)                          \
	{                                \
		_afc_inet_server_delete(is); \
		is = NULL;                   \
	}
InetServer *afc_inet_server_new(void);
int _afc_inet_server_delete(InetServer *ic);
int afc_inet_server_clear(InetServer *ic);
int afc_inet_server_create(InetServer *is, int port);
int afc_inet_server_close(InetServer *is);
int afc_inet_server_wait(InetServer *is);
int afc_inet_server_process(InetServer *is);
int afc_inet_server_send(InetServer *is, InetConnData *data, char *str);
int afc_inet_server_close_conn(InetServer *is, InetConnData *data);

#endif
