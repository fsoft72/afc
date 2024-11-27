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
#include "../../afc.h"

int set_name(InetConnData *data)
{
	char *s = strstr(data->buf, ":name");

	afc_string_copy(data->data, s + 6, ALL);

	afc_string_trim(data->data);

	return (AFC_ERR_NO_ERROR);
}

int inet_show_help(InetServer *is, InetConnData *data)
{
	char *buf;

	buf = "\n\n:bye                 - Esce dalla chat\n"
		  ":name (nome)         - Setta il proprio nome in chat\n"
		  ":quit                - Chiude tutte le connessioni e ferma il server\n\n"
		  ":help                - Mostra questo messaggio di help\n\n\n";

	afc_inet_server_send(is, data, buf);

	return (AFC_ERR_NO_ERROR);
}

int inet_send(InetServer *is, InetConnData *data, char *msg)
{
	InetConnData *dest;
	char *buf = afc_string_new(afc_string_len(data->buf) + 55);

	dest = afc_hash_first(is->hash);

	if (msg == NULL)
		msg = data->buf;
	afc_string_make(buf, "%s: %s\r\n", data->data, msg);

	while (dest != NULL)
	{
		if (dest != data)
			afc_inet_server_send(is, dest, buf);

		dest = afc_hash_next(is->hash);
	}

	return (AFC_ERR_NO_ERROR);
}

int inet_connect(InetServer *is, InetConnData *data)
{
	char *name;

	printf("New connection from %s on socket %d\n", inet_ntoa(is->remoteaddr.sin_addr), data->fd);

	name = afc_string_new(50);
	afc_string_make(name, "[No Name #%3.3d]", data->fd);

	data->data = name;

	return (AFC_ERR_NO_ERROR);
}

int inet_close(InetServer *is, InetConnData *data)
{
	char *name;

	printf("Closing connection from: %d\n", data->fd);

	name = data->data;

	if (name != NULL)
		afc_string_delete(name);

	return (AFC_ERR_NO_ERROR);
}

int inet_receive(InetServer *is, InetConnData *data)
{
	int *q;
	static char *cmds[] = {":quit", ":help", ":name", ":bye", NULL};
	int t = 0;

	afc_string_trim(data->buf);

	if (afc_string_len(data->buf) == 0)
		return (AFC_ERR_NO_ERROR);

	if (data->buf[0] != ':')
	{
		inet_send(is, data, NULL);
	}
	else
	{
		while (cmds[t] != NULL)
		{
			if (strstr(data->buf, cmds[t]) != NULL)
			{
				printf("Cmd: %d\n", t);

				switch (t)
				{
				case 0: // ":quit"
					q = is->data;
					*q = TRUE;
					break;

				case 1: // ":help"
					inet_show_help(is, data);
					break;

				case 2: // ":name"
					set_name(data);
					break;

				case 3: // ":bye"
					inet_send(is, data, "DISCONNECTED");
					afc_inet_server_close_conn(is, data);
					break;
				}

				return (AFC_ERR_NO_ERROR);
			}

			t++;
		}
	}

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
	for (;;)
	{
		afc_inet_server_wait(is);
		afc_inet_server_process(is);

		// printf ( "Quit: %d\n", quit );

		if (quit)
			break;
	}

	afc_inet_server_delete(is);
	afc_delete(afc);

	return (0);
}
