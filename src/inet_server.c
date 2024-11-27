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
#include "inet_server.h"

static const char class_name[] = "InetServer";

static InetConnData *afc_inet_server_create_conn_data(InetServer *is, int fd);

/*
@config
	TITLE:     InetServer
	VERSION:   1.00
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
@endnode
*/

// {{{ docs
/*
@node quote
	*Who are you going to believe, me or your own eyes?*

		Groucho Marx
@endnode

@node intro
InetServer is a class that will help you create network servers to any kind of TCP/IP protocol.
@endnode

@node history
	- 1.00:		Initial Release
@endnode
*/
// }}}

// {{{ afc_inet_server_new ()
/*
@node afc_inet_server_new

		   NAME: afc_inet_server_new () - Initializes a new InetServer instance.

	   SYNOPSIS: InetServer * afc_inet_server_new ()

	DESCRIPTION: This function initializes a new InetServer instance.

		  INPUT: NONE

		RESULTS: a valid inizialized InetServer structure. NULL in case of errors.

	   SEE ALSO: - afc_inet_server_delete()

@endnode
*/
InetServer *afc_inet_server_new()
{
	TRY(InetServer *)

	InetServer *is = (InetServer *)afc_malloc(sizeof(InetServer));

	if (is == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "is", NULL);
	is->magic = AFC_INET_SERVER_MAGIC;
	if ((is->hash = afc_hash_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "hash", NULL);
	is->bufsize = AFC_INET_SERVER_DEFAULT_BUFSIZE;

	RETURN(is);

	EXCEPT
	afc_inet_server_delete(is);

	FINALLY

	ENDTRY
}
// }}}
// {{{ afc_inet_server_delete ( is )
/*
@node afc_inet_server_delete

		   NAME: afc_inet_server_delete ( is )  - Disposes a valid InetServer instance.

	   SYNOPSIS: int afc_inet_server_delete ( InetServer * ic)

	DESCRIPTION: This function frees an already alloc'd InetServer structure.

		  INPUT: - is  - Pointer to a valid afc_inet_server class.

		RESULTS: should be AFC_ERR_NO_ERROR

		  NOTES: - this method calls: afc_inet_server_clear()

	   SEE ALSO: - afc_inet_server_new()
				 - afc_inet_server_clear()
@endnode
*/
int _afc_inet_server_delete(InetServer *is)
{
	int afc_res;

	if ((afc_res = afc_inet_server_clear(is)) != AFC_ERR_NO_ERROR)
		return (afc_res);

	afc_inet_server_close(is);
	afc_hash_delete(is->hash);
	afc_free(is);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_inet_server_clear ( is )
/*
@node afc_inet_server_clear

		   NAME: afc_inet_server_clear ( is )  - Clears all stored data

	   SYNOPSIS: int afc_inet_server_clear ( InetServer * ic)

	DESCRIPTION: Use this function to clear all stored data in the current is instance.

		  INPUT: - is    - Pointer to a valid afc_inet_server instance.

		RESULTS: should be AFC_ERR_NO_ERROR

	   SEE ALSO: - afc_inet_server_delete()

@endnode

*/
int afc_inet_server_clear(InetServer *is)
{
	if (is == NULL)
		return (AFC_ERR_NULL_POINTER);

	if (is->magic != AFC_INET_SERVER_MAGIC)
		return (AFC_ERR_INVALID_POINTER);

	/* Custom Clean-up code should go here */

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_inet_server_create ( is, port ) ***********
int afc_inet_server_create(InetServer *is, int port)
{
	int yes = 1;

	FD_ZERO(&is->master);
	FD_ZERO(&is->read_fds);

	// Create the listener
	if ((is->listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	// I can reuse the addr
	if (setsockopt(is->listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
	{
		perror("setsockopt");
		exit(1);
	}

	// bind
	is->myaddr.sin_family = AF_INET;
	is->myaddr.sin_addr.s_addr = INADDR_ANY;
	is->myaddr.sin_port = htons(port);
	memset(&(is->myaddr.sin_zero), '\0', 8);

	if (bind(is->listener, (struct sockaddr *)&is->myaddr, sizeof(is->myaddr)) == -1)
	{
		perror("bind");
		exit(1);
	}

	// listen (prepare for connection)
	if (listen(is->listener, 10) == -1)
	{
		perror("listen");
		exit(1);
	}

	// Aggiungo listener al master set
	FD_SET(is->listener, &is->master);

	// keep track of the biggest descr
	is->fdmax = is->listener; // it is the only one

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_inet_server_close ( is ) **********
int afc_inet_server_close(InetServer *is)
{
	InetConnData *data;
	// int t;

	data = afc_hash_first(is->hash);
	while (data)
	{
		afc_inet_server_close_conn(is, data);
		data = afc_hash_next(is->hash);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_inet_server_wait ( is ) ***************
int afc_inet_server_wait(InetServer *is)
{
	is->read_fds = is->master; // copy it

	is->active = 0;

	if (select(is->fdmax + 1, &is->read_fds, NULL, NULL, NULL) == -1)
	{
		perror("select");
		exit(1);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_inet_server_process ( is ) **************
int afc_inet_server_process(InetServer *is)
{
	unsigned int addrlen;
	int nbytes;
	int i;
	InetConnData *data;

	// cycle existing connections for data
	for (i = is->active; i <= is->fdmax; i++)
	{
		if (FD_ISSET(i, &is->read_fds))
		{
			if (i == is->listener) // New connection coming :-)
			{
				addrlen = sizeof(is->remoteaddr);

				if ((is->newfd = accept(is->listener, (struct sockaddr *)&is->remoteaddr, &addrlen)) == -1)
				{
					perror("accept");
				}
				else
				{
					if ((data = afc_inet_server_create_conn_data(is, is->newfd)) == NULL)
						return (AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "data"));

					FD_SET(is->newfd, &is->master); // Add newfd to the master set
					if (is->newfd > is->fdmax)
						is->fdmax = is->newfd; // keep track of the maximum fd
				}
			}
			else
			{
				data = afc_hash_find(is->hash, i);

				// Data from clients already connected
				if ((nbytes = recv(i, data->buf, afc_string_max(data->buf), 0)) <= 0)
				{
					// If 0 == connection closed, if -1 == error
					if (nbytes == 0)
					{
						printf("Socket %d closed\n", i);
					}
					else
					{
						printf("Socket %d error\n", i);
					}

					// Delete it from the fd set
					afc_inet_server_close_conn(is, data);
				}
				else
				{
					data->buf[nbytes] = '\0';
					afc_string_reset_len(data->buf);

					if (data->cb_receive)
						is->cb_receive(is, data);
				}
			}
		}
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_inet_server_send ( is, data, str ) ***********
int afc_inet_server_send(InetServer *is, InetConnData *data, char *str)
{
	if (FD_ISSET(data->fd, &is->master))
	{
		// We block sending messages to the listener
		if (data->fd == is->listener)
			return (AFC_ERR_NO_ERROR);

		if (send(data->fd, str, strlen(str), 0) == -1)
			return (AFC_LOG(AFC_LOG_ERROR, AFC_INET_SERVER_ERR_SEND, "send() failed", str));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_inet_server_close_conn ( is, data ) **********
int afc_inet_server_close_conn(InetServer *is, InetConnData *data)
{
	if (data->cb_close != NULL)
		data->cb_close(is, data);

	close(data->fd);
	FD_CLR(data->fd, &is->master);

	if (data->buf)
		afc_string_delete(data->buf);

	afc_free(data);
	afc_hash_del(is->hash);

	return (AFC_ERR_NO_ERROR);
}
// }}}

// INTERNALS
// {{{ afc_inet_server_create_conn_data ( is, fd )
static InetConnData *afc_inet_server_create_conn_data(InetServer *is, int fd)
{
	InetConnData *data = afc_malloc(sizeof(InetConnData));

	if (data == NULL)
	{
		AFC_LOG_FAST(AFC_ERR_NO_MEMORY);
		return (NULL);
	}

	if ((data->buf = afc_string_new(is->bufsize)) == NULL)
	{
		AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "buffer");
		afc_free(data);
		return (NULL);
	}

	// If successful, set the fd and all user data
	data->fd = fd;
	data->cb_connect = is->cb_connect;
	data->cb_close = is->cb_close;
	data->cb_receive = is->cb_receive;
	data->is = is;

	// Add it to the Hash Table
	afc_hash_add(is->hash, is->newfd, data);

	if ((is->cb_connect) != NULL)
		is->cb_connect(is, data);

	return (data);
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
int inet_connect(InetServer *is, InetConnData *data)
{
	printf("New connection from %s on socket %d\n", inet_ntoa(is->remoteaddr.sin_addr), data->fd);

	return (AFC_ERR_NO_ERROR);
}

int inet_close(InetServer *is, InetConnData *data)
{
	printf("Closing connection from: %d\n", data->fd);

	return (AFC_ERR_NO_ERROR);
}

int inet_receive(InetServer *is, InetConnData *data)
{
	int *q;

	printf("Data from socket: %d - %s\n", data->fd, data->buf);

	q = is->data;

	*q = TRUE;

	// afc_inet_server_delete ( is );

	return (AFC_ERR_NO_ERROR);
}

int main(int argc, char *argv[])
{
	AFC *afc = afc_new();
	InetServer *is = afc_inet_server_new();
	int quit = 0;

	if (is == NULL)
	{
		fprintf(stderr, "Init of class InetServer failed.\n");
		return (1);
	}

	is->cb_connect = inet_connect;
	is->cb_close = inet_close;
	is->cb_receive = inet_receive;

	is->data = &quit;

	afc_inet_server_create(is, 8080);
	while (!quit)
	{
		afc_inet_server_wait(is);
		afc_inet_server_process(is);

		printf("Quit: %d\n", quit);
	}

	afc_inet_server_delete(is);
	afc_delete(afc);

	return (0);
}
// }}}
#endif
