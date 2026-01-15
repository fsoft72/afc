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
#include <stdarg.h>
#include "inet_client.h"

static const char class_name[] = "InetClient";

/*
@config
	TITLE:     InetClient
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
InetClient is a class that will help you create network clients to any kind of TCP/IP protocol.
@endnode

@node history
	- 1.00:		Initial Release
@endnode
*/
// }}}

// {{{ afc_inet_client_new ()
/*
@node afc_inet_client_new

		   NAME: afc_inet_client_new () - Initializes a new InetClient instance.

	   SYNOPSIS: InetClient * afc_inet_client_new ()

	DESCRIPTION: This function initializes a new InetClient instance.

		  INPUT: NONE

		RESULTS: a valid inizialized InetClient structure. NULL in case of errors.

	   SEE ALSO: - afc_inet_client_delete()

@endnode
*/
InetClient *afc_inet_client_new()
{
	TRY(InetClient *)

	InetClient *ic = (InetClient *)afc_malloc(sizeof(InetClient));

	if (ic == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "ic", NULL);

	ic->magic = AFC_INET_CLIENT_MAGIC;

	if ((ic->buf = afc_string_new(1024)) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "buf", NULL);

	ic->sockfd = -1; // Initialize to invalid fd
	ic->fd = NULL;
	ic->use_ssl = FALSE;
	ic->ssl_ctx = NULL;
	ic->ssl = NULL;
	ic->timeout = 0; // No timeout by default

	RETURN(ic);

	EXCEPT
	afc_inet_client_delete(ic);

	FINALLY

	ENDTRY
}
// }}}
// {{{ afc_inet_client_delete ( ic )
/*
@node afc_inet_client_delete

		   NAME: afc_inet_client_delete ( ic )  - Disposes a valid InetClient instance.

	   SYNOPSIS: int afc_inet_client_delete ( InetClient * ic)

	DESCRIPTION: This function frees an already alloc'd InetClient structure.

		  INPUT: - ic  - Pointer to a valid afc_inet_client class.

		RESULTS: should be AFC_ERR_NO_ERROR

		  NOTES: - this method calls: afc_inet_client_clear()

	   SEE ALSO: - afc_inet_client_new()
				 - afc_inet_client_clear()
@endnode
*/
int _afc_inet_client_delete(InetClient *ic)
{
	int afc_res;

	if (ic == NULL)
		return AFC_LOG_FAST(AFC_ERR_NULL_POINTER);

	if (ic->magic != AFC_INET_CLIENT_MAGIC)
		return AFC_LOG_FAST(AFC_ERR_INVALID_POINTER);

	if ((afc_res = afc_inet_client_clear(ic)) != AFC_ERR_NO_ERROR)
		return (afc_res);

	afc_inet_client_close(ic);

	afc_string_delete(ic->buf);
	afc_free(ic);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_inet_client_clear ( ic )
/*
@node afc_inet_client_clear

		   NAME: afc_inet_client_clear ( ic )  - Clears all stored data

	   SYNOPSIS: int afc_inet_client_clear ( InetClient * ic)

	DESCRIPTION: Use this function to clear all stored data in the current ic instance.

		  INPUT: - ic    - Pointer to a valid afc_inet_client instance.

		RESULTS: should be AFC_ERR_NO_ERROR

	   SEE ALSO: - afc_inet_client_delete()

@endnode
*/
int afc_inet_client_clear(InetClient *ic)
{
	if (ic == NULL)
		return (AFC_ERR_NULL_POINTER);

	if (ic->magic != AFC_INET_CLIENT_MAGIC)
		return (AFC_ERR_INVALID_POINTER);

	/* Custom Clean-up code should go here */

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_inet_client_open ( ic, url, port )
/*
@node afc_inet_client_open

		   NAME: afc_inet_client_open ( ic, url, port )  - Open a socket

	   SYNOPSIS: int afc_inet_client_open ( InetClient * ic, char * url, int port )

	DESCRIPTION: Use this function to open a connection to the given /url/ at the /port/ specified.

		  INPUT: - ic    - Pointer to a valid afc_inet_client instance.
		 - url	 - Url to open the socket to. It can be a valid internet address (eg. 194.80.12.12) or
			   a name to be resolved.
		 - port  - The port to be opened.

		RESULTS: should be AFC_ERR_NO_ERROR

	   SEE ALSO: - afc_inet_client_close()
			 - afc_inet_client_send()
			 - afc_inet_client_get()
@endnode
*/
int afc_inet_client_open(InetClient *ic, char *url, int port)
{
	struct hostent *h;
	struct timeval tv;

	if ((ic->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return (AFC_LOG(AFC_LOG_ERROR, AFC_INET_CLIENT_ERR_SOCKET, "Cannot Create the Socket", "socket() failed"));

	// Set timeout if configured
	if (ic->timeout > 0)
	{
		tv.tv_sec = ic->timeout;
		tv.tv_usec = 0;
		setsockopt(ic->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
		setsockopt(ic->sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));
	}

	if ((h = afc_inet_client_resolve(ic, url)) == NULL)
		return (AFC_LOG(AFC_LOG_ERROR, AFC_INET_CLIENT_ERR_HOST_UNKNOWN, "Unable to resolve the host", NULL));

	// printf ( "IP Addr: %s\n", inet_ntoa ( * ((struct in_addr *) h->h_addr ) ) );

	memset(&ic->dest_addr, 0, sizeof(struct sockaddr_in));

	ic->dest_addr.sin_family = AF_INET;	  // This is in host byte order
	ic->dest_addr.sin_port = htons(port); // short, network byte order
	ic->dest_addr.sin_addr = *((struct in_addr *)h->h_addr);

	// free the hostent obtained? (at the moment, it just segfaults...)
	// free ( h );

	if ((connect(ic->sockfd, (struct sockaddr *)&ic->dest_addr, sizeof(struct sockaddr))) == -1)
		return (AFC_LOG(AFC_LOG_ERROR, AFC_INET_CLIENT_ERR_CONNECT, "Connect() failed", strerror(errno)));

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_inet_client_close ( ic )
/*
@node afc_inet_client_close

		   NAME: afc_inet_client_close ( ic )  - Closes a connection

	   SYNOPSIS: int afc_inet_client_close ( InetClient * ic )

	DESCRIPTION: Use this function to close a connection previously opened with afc_inet_client_open().

		  INPUT: - ic    - Pointer to a valid afc_inet_client instance.

		RESULTS: should be AFC_ERR_NO_ERROR

	   SEE ALSO: - afc_inet_client_open()

@endnode
*/
int afc_inet_client_close(InetClient *ic)
{
	// Clean up SSL connection if active
	if (ic->ssl)
	{
		SSL_shutdown(ic->ssl);
		SSL_free(ic->ssl);
		ic->ssl = NULL;
	}

	if (ic->ssl_ctx)
	{
		SSL_CTX_free(ic->ssl_ctx);
		ic->ssl_ctx = NULL;
	}

	if (ic->sockfd >= 0)
		close(ic->sockfd);

	ic->sockfd = -1;
	ic->fd = NULL;

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_inet_client_resolve ( ic, url )
/*
@node afc_inet_client_resolve

		   NAME: afc_inet_client_resolve ( ic, url )  - Performs a DNS resolution of the given URL

	   SYNOPSIS: struct hostent afc_inet_client_resolve ( InetClient * ic, char * url )

	DESCRIPTION: This function resolves the given /url/ and returns an /hostent/ structure.

		  INPUT: - ic    - Pointer to a valid afc_inet_client instance.
		 - url   - URL to be resolved

		RESULTS: - an hostent structure on success.
		 - NULL on error.

	   SEE ALSO: - afc_inet_client_open()

@endnode
*/
struct hostent *afc_inet_client_resolve(InetClient *ic, char *url)
{
	struct hostent *h;

	if ((h = gethostbyname(url)) == NULL)
	{
		AFC_LOG(AFC_LOG_ERROR, AFC_INET_CLIENT_ERR_RESOLVE, "Cannot Resolve This Name", url);
		return (NULL);
	}

	return (h);
}
// }}}
// {{{ afc_inet_client_get ( ic )
/*
@node afc_inet_client_get

		   NAME: afc_inet_client_get ( ic )  - Gets some data from the connection

	   SYNOPSIS: int afc_inet_client_get ( InetClient * ic )

	DESCRIPTION: This function gets some data from the current connection, and fills the
		 internal /buf/ buffer. You can access the buffer by referencing /ic->buf/.

		  INPUT: - ic    - Pointer to a valid afc_inet_client instance.

		RESULTS: - AFC_ERR_NO_ERROR when everything was fine and /buf/ has been filled.
		 - AFC_INET_CLIENT_ERR_RECEIVE when there was something wrong in receiving the data.
		 - AFC_INET_CLIENT_ERR_END_OF_STREAM when the data has been finished.

	   SEE ALSO: - afc_inet_client_send()
		 - afc_inet_client_open()

@endnode
*/
int afc_inet_client_get(InetClient *ic)
{
	int bytes;

	// Reserve 1 byte for null terminator
	unsigned long max_read = afc_string_max(ic->buf) - 1;

	// Use SSL_read if SSL is enabled
	if (ic->use_ssl && ic->ssl)
	{
		if ((bytes = SSL_read(ic->ssl, ic->buf, max_read)) <= 0)
		{
			int ssl_err = SSL_get_error(ic->ssl, bytes);
			if (ssl_err == SSL_ERROR_ZERO_RETURN)
			{
				afc_string_clear(ic->buf);
				return (AFC_INET_CLIENT_ERR_END_OF_STREAM);
			}
			return (AFC_LOG(AFC_LOG_ERROR, AFC_INET_CLIENT_ERR_SSL_READ, "SSL_read() failed", NULL));
		}
	}
	else
	{
		if ((bytes = recv(ic->sockfd, ic->buf, max_read, 0)) == -1)
			return (AFC_LOG(AFC_LOG_ERROR, AFC_INET_CLIENT_ERR_RECEIVE, "recv() failed", NULL));
	}

	if (bytes == 0)
	{
		afc_string_clear(ic->buf);
		return (AFC_INET_CLIENT_ERR_END_OF_STREAM);
	}

	ic->buf[bytes] = '\0';
	afc_string_reset_len(ic->buf);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_inet_client_send ( ic, str, len )
/*
@node afc_inet_client_send

		   NAME: afc_inet_client_send ( ic, str, len )  - Sends some data through the connection

	   SYNOPSIS: int afc_inet_client_send ( InetClient * ic, const char * str, int len )

	DESCRIPTION: This function sends some data through the current connection. No data manipulation
		 or additional characters will be performed through the send.
		 If you are sending a normal string, you can set /len/ to 0 or -1, and the current
		 string length will be calculated by the function itself when needed.

		  INPUT: - ic    - Pointer to a valid afc_inet_client instance.
		 - str   - Data to be sent
		 - len   - Length of the data to be sent. If you are sending a string, put 0 or -1
			   and the string length will be computed automatically.

		RESULTS: - AFC_ERR_NO_ERROR when everything was fine and /buf/ has been filled.
		 - AFC_INET_CLIENT_ERR_RECEIVE when there was something wrong in receiving the data.
		 - AFC_INET_CLIENT_ERR_END_OF_STREAM when the data has been finished.

	   SEE ALSO: - afc_inet_client_get()
		 - afc_inet_client_open()

@endnode
*/
int afc_inet_client_send(InetClient *ic, const char *str, int len)
{
	if (len <= 0)
		len = strlen(str);

	// Use SSL_write if SSL is enabled
	if (ic->use_ssl && ic->ssl)
	{
		if (SSL_write(ic->ssl, str, len) <= 0)
			return (AFC_LOG(AFC_LOG_ERROR, AFC_INET_CLIENT_ERR_SSL_WRITE, "SSL_write() failed", NULL));
	}
	else
	{
		if (send(ic->sockfd, str, len, 0) == -1)
			return (AFC_LOG(AFC_LOG_ERROR, AFC_INET_CLIENT_ERR_SEND, "send() failed", NULL));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_inet_client_get_file ( ic )
FILE *afc_inet_client_get_file(InetClient *ic)
{
	if (ic->fd)
		return (ic->fd);

	return (ic->fd = fdopen(ic->sockfd, "r"));
}
// }}}
// {{{ afc_inet_client_set_tags ( ic, first_tag, ... )
/*
@node afc_inet_client_set_tags

		   NAME: afc_inet_client_set_tags ( ic, first_tag, ... )  - Set tags for InetClient

	   SYNOPSIS: int afc_inet_client_set_tags ( InetClient * ic, int first_tag, ... )

	DESCRIPTION: Use this function to set tags for the InetClient instance.

		  INPUT: - ic    - Pointer to a valid afc_inet_client instance.
		 - first_tag - First tag to set
		 - ... - Tag values

		RESULTS: should be AFC_ERR_NO_ERROR

	   SEE ALSO: - afc_inet_client_set_tag()

@endnode
*/
int _afc_inet_client_set_tags(InetClient *ic, int first_tag, ...)
{
	va_list tags;
	int tag;
	void *val;
	int res = AFC_ERR_NO_ERROR;

	if (ic == NULL)
		return AFC_LOG_FAST(AFC_ERR_NULL_POINTER);

	if (ic->magic != AFC_INET_CLIENT_MAGIC)
		return AFC_LOG_FAST(AFC_ERR_INVALID_POINTER);

	va_start(tags, first_tag);

	tag = first_tag;

	while (tag != AFC_TAG_END)
	{
		val = va_arg(tags, void *);

		if ((res = afc_inet_client_set_tag(ic, tag, val)) != AFC_ERR_NO_ERROR)
			break;

		tag = va_arg(tags, int);
	}

	va_end(tags);

	return res;
}
// }}}
// {{{ afc_inet_client_set_tag ( ic, tag, val )
/*
@node afc_inet_client_set_tag

		   NAME: afc_inet_client_set_tag ( ic, tag, val )  - Set a single tag

	   SYNOPSIS: int afc_inet_client_set_tag ( InetClient * ic, int tag, void * val )

	DESCRIPTION: Use this function to set a single tag for the InetClient instance.

		  INPUT: - ic    - Pointer to a valid afc_inet_client instance.
		 - tag   - Tag to set
		 - val   - Tag value

		RESULTS: should be AFC_ERR_NO_ERROR

	   SEE ALSO: - afc_inet_client_set_tags()

@endnode
*/
int afc_inet_client_set_tag(InetClient *ic, int tag, void *val)
{
	if (!ic)
		return AFC_ERR_NULL_POINTER;

	switch (tag)
	{
	case AFC_INET_CLIENT_TAG_USE_SSL:
		ic->use_ssl = (BOOL)(long)val;
		break;

	case AFC_INET_CLIENT_TAG_TIMEOUT:
		ic->timeout = (int)(long)val;
		break;

	default:
		return AFC_LOG(AFC_LOG_ERROR, AFC_ERR_UNSUPPORTED_TAG, "Unsupported tag", NULL);
	}

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ afc_inet_client_enable_ssl ( ic )
/*
@node afc_inet_client_enable_ssl

		   NAME: afc_inet_client_enable_ssl ( ic )  - Enable SSL for direct SSL connections

	   SYNOPSIS: int afc_inet_client_enable_ssl ( InetClient * ic )

	DESCRIPTION: Use this function to enable SSL on an already open socket (direct SSL/TLS).
		 This should be called after afc_inet_client_open() for connections like SMTPS on port 465.

		  INPUT: - ic    - Pointer to a valid afc_inet_client instance.

		RESULTS: - AFC_ERR_NO_ERROR on success
		 - AFC_INET_CLIENT_ERR_SSL_INIT on SSL context creation failure
		 - AFC_INET_CLIENT_ERR_SSL_CONNECT on SSL handshake failure

	   SEE ALSO: - afc_inet_client_start_tls()

@endnode
*/
int afc_inet_client_enable_ssl(InetClient *ic)
{
	if (!ic)
		return AFC_ERR_NULL_POINTER;

	// Initialize OpenSSL library
	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();

	// Create SSL context
	const SSL_METHOD *method = TLS_client_method();
	ic->ssl_ctx = SSL_CTX_new(method);
	if (!ic->ssl_ctx)
		return AFC_LOG(AFC_LOG_ERROR, AFC_INET_CLIENT_ERR_SSL_INIT, "SSL_CTX_new() failed", NULL);

	// Create SSL connection
	ic->ssl = SSL_new(ic->ssl_ctx);
	if (!ic->ssl)
	{
		SSL_CTX_free(ic->ssl_ctx);
		ic->ssl_ctx = NULL;
		return AFC_LOG(AFC_LOG_ERROR, AFC_INET_CLIENT_ERR_SSL_INIT, "SSL_new() failed", NULL);
	}

	// Attach the socket to SSL
	if (SSL_set_fd(ic->ssl, ic->sockfd) != 1)
	{
		SSL_free(ic->ssl);
		SSL_CTX_free(ic->ssl_ctx);
		ic->ssl = NULL;
		ic->ssl_ctx = NULL;
		return AFC_LOG(AFC_LOG_ERROR, AFC_INET_CLIENT_ERR_SSL_INIT, "SSL_set_fd() failed", NULL);
	}

	// Perform SSL handshake
	if (SSL_connect(ic->ssl) != 1)
	{
		SSL_free(ic->ssl);
		SSL_CTX_free(ic->ssl_ctx);
		ic->ssl = NULL;
		ic->ssl_ctx = NULL;
		return AFC_LOG(AFC_LOG_ERROR, AFC_INET_CLIENT_ERR_SSL_CONNECT, "SSL_connect() failed", NULL);
	}

	ic->use_ssl = TRUE;

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ afc_inet_client_start_tls ( ic )
/*
@node afc_inet_client_start_tls

		   NAME: afc_inet_client_start_tls ( ic )  - Upgrade connection to TLS (STARTTLS)

	   SYNOPSIS: int afc_inet_client_start_tls ( InetClient * ic )

	DESCRIPTION: Use this function to upgrade an existing plain connection to TLS.
		 This is used after sending a STARTTLS command (e.g., in SMTP on port 587).

		  INPUT: - ic    - Pointer to a valid afc_inet_client instance.

		RESULTS: - AFC_ERR_NO_ERROR on success
		 - AFC_INET_CLIENT_ERR_SSL_INIT on SSL context creation failure
		 - AFC_INET_CLIENT_ERR_SSL_CONNECT on SSL handshake failure

	   SEE ALSO: - afc_inet_client_enable_ssl()

@endnode
*/
int afc_inet_client_start_tls(InetClient *ic)
{
	// STARTTLS is the same as enable_ssl - just a semantic alias
	return afc_inet_client_enable_ssl(ic);
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
int main(int argc, char *argv[])
{
	AFC *afc = afc_new();
	InetClient *ic = afc_inet_client_new();

	if (ic == NULL)
	{
		fprintf(stderr, "Init of class InetClient failed.\n");
		return (1);
	}

	afc_inet_client_open(ic, "webbench.fabiosoft.it", 80);
	afc_inet_client_send(ic, "GET / HTTP/1.0\n\n");
	while (afc_inet_client_get(ic) == AFC_ERR_NO_ERROR)
	{
		printf("%s\n", ic->buf);
	}

	afc_inet_client_delete(ic);
	afc_delete(afc);

	return (0);
}
// }}}
#endif
