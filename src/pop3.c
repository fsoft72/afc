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
#include "pop3.h"

/*
@config
	TITLE:     POP3
	VERSION:   1.00
	AUTHOR:	   Fabio Rotondo - fabio@rotondo.it
@endnode
*/

static const char class_name[] = "POP3";

static int afc_pop3_internal_get_response(POP3 *p);
static int afc_pop3_internal_put(POP3 *p, const char *msg);
static int afc_pop3_internal_get_line(POP3 *p);
static int afc_pop3_internal_get_multi_line(POP3 *p);
static int afc_pop3_internal_cmd(POP3 *p, const char *cmd, short multi);

static int afc_pop3_internal_del_msg(HashMaster *hm, void *msg);

// {{{ afc_pop3_new ()
/*
@node afc_pop3_new

		   NAME: afc_pop3_new () - Initializes a new POP3 instance.

	   SYNOPSIS: POP3 * afc_pop3_new ()

	DESCRIPTION: This function initializes a new POP3 instance.

		  INPUT: NONE

		RESULTS: a valid inizialized POP3 structure. NULL in case of errors.

	   SEE ALSO: - afc_pop3_delete()

@endnode
*/
POP3 *afc_pop3_new(void)
{
	TRY(POP3 *)

	POP3 *p = (POP3 *)afc_malloc(sizeof(POP3));

	if (p == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "pop3", NULL);
	p->magic = AFC_POP3_MAGIC;

	if ((p->ic = afc_inet_client_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "inet_client", NULL);
	if ((p->sn = afc_string_list_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "sn", NULL);
	if ((p->tmp = afc_string_new(1024)) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "tmp", NULL);
	if ((p->buf = afc_string_new(1024)) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "buf", NULL);
	if ((p->msg = afc_hash_master_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "msg", NULL);

	afc_hash_master_set_clear_func(p->msg, afc_pop3_internal_del_msg);

	p->port = afc_string_new(5);
	afc_string_copy(p->port, "110", ALL);

	RETURN(p);

	EXCEPT
	afc_pop3_delete(p);

	FINALLY

	ENDTRY
}
// }}}
// {{{ afc_pop3_delete ( p )
/*
@node afc_pop3_delete

		   NAME: afc_pop3_delete ( p )  - Disposes a valid POP3 instance.

	   SYNOPSIS: int afc_pop3_delete ( POP3 * p)

	DESCRIPTION: This function frees an already alloc'd POP3 structure.

		  INPUT: - p  - Pointer to a valid afc_pop3 class.

		RESULTS: should be AFC_ERR_NO_ERROR

		  NOTES: - this method calls: afc_pop3_clear()

	   SEE ALSO: - afc_pop3_new()
				 - afc_pop3_clear()
@endnode
*/
int _afc_pop3_delete(POP3 *p)
{
	int afc_res;

	if ((afc_res = afc_pop3_clear(p)) != AFC_ERR_NO_ERROR)
		return (afc_res);

	afc_inet_client_delete(p->ic);
	afc_string_list_delete(p->sn);
	afc_string_delete(p->tmp);
	afc_string_delete(p->buf);
	afc_string_delete(p->port);
	afc_free(p);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_pop3_clear ( p )
/*
@node afc_pop3_clear

		   NAME: afc_pop3_clear ( p )  - Clears all stored data

	   SYNOPSIS: int afc_pop3_clear ( POP3 * p)

	DESCRIPTION: Use this function to clear all stored data in the current p instance.

		  INPUT: - p    - Pointer to a valid afc_pop3 instance.

		RESULTS: should be AFC_ERR_NO_ERROR

	   SEE ALSO: - afc_pop3_delete()

@endnode
*/
int afc_pop3_clear(POP3 *p)
{
	if (p == NULL)
		return (AFC_ERR_NULL_POINTER);

	if (p->magic != AFC_POP3_MAGIC)
		return (AFC_ERR_INVALID_POINTER);
	if (p->ic)
		afc_inet_client_clear(p->ic);
	if (p->sn)
		afc_string_list_clear(p->sn);
	if (p->tmp)
		afc_string_clear(p->tmp);
	if (p->buf)
		afc_string_clear(p->buf);

	if (p->port)
		afc_string_copy(p->port, "110", ALL);

	if (p->host)
		afc_string_delete(p->host);
	if (p->login)
		afc_string_delete(p->login);
	if (p->passwd)
		afc_string_delete(p->passwd);

	p->host = NULL;
	p->login = NULL;
	p->passwd = NULL;

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_pop3_set_tags ( p, first_tag, ... ) ************
int _afc_pop3_set_tags(POP3 *p, int first_tag, ...)
{
	va_list tags;
	unsigned int tag;
	void *val;

	va_start(tags, first_tag);

	tag = first_tag;

	while (tag != AFC_TAG_END)
	{
		val = va_arg(tags, void *);

		afc_pop3_set_tag(p, tag, val);

		tag = va_arg(tags, int);
	}

	va_end(tags);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_pop3_set_tag ( p, tag, val ) ***************
int afc_pop3_set_tag(POP3 *p, int tag, void *val)
{
	switch (tag)
	{
	case AFC_POP3_TAG_HOST:
		if (p->host)
			afc_string_delete(p->host);
		p->host = afc_string_dup((char *)val);
		break;

	case AFC_POP3_TAG_PORT:
		afc_string_copy(p->port, (char *)val, ALL);
		break;

	case AFC_POP3_TAG_LOGIN:
		if (p->login)
			afc_string_delete(p->login);
		p->login = afc_string_dup((char *)val);
		break;

	case AFC_POP3_TAG_PASSWD:
		if (p->passwd)
			afc_string_delete(p->passwd);
		p->passwd = afc_string_dup((char *)val);
		break;
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_pop3_connect ( p ) ************
int afc_pop3_connect(POP3 *p)
{
	int res;

	if ((res = afc_inet_client_open(p->ic, p->host, atoi(p->port))) != AFC_ERR_NO_ERROR)
		return (res);

	p->fd = afc_inet_client_get_file(p->ic);

	afc_pop3_internal_get_response(p);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_pop3_login ( p ) ***********
int afc_pop3_login(POP3 *p)
{
	int res;

	afc_dprintf("Sending user: %s\n", p->login);

	afc_string_make(p->tmp, "USER %s", p->login);
	if ((res = afc_pop3_internal_cmd(p, p->tmp, FALSE)) != AFC_ERR_NO_ERROR)
		return (res);

	afc_dprintf("Sending password: %s\n", p->passwd);
	afc_string_make(p->tmp, "PASS %s", p->passwd);
	if ((res = afc_pop3_internal_cmd(p, p->tmp, FALSE)) != AFC_ERR_NO_ERROR)
		return (res);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_pop3_stat ( p ) *************
int afc_pop3_stat(POP3 *p)
{
	int res;
	char *s;

	if ((res = afc_pop3_internal_cmd(p, "STAT", FALSE)) != AFC_ERR_NO_ERROR)
		return (res);

	afc_string_list_split(p->sn, p->buf, " ");

	// NOTA: la parte "s = ..." e' stata aggiunta per evitare il warning del GCC 4.1:
	// null argument where non-null required
	p->tot_messages = atoi((s = afc_string_list_item(p->sn, 1)));
	p->tot_size = atoi((s = afc_string_list_item(p->sn, 2)));

	afc_dprintf("Messages: %d\nSize: %d\n", p->tot_messages, p->tot_size);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_pop3_get_list ( p ) **********
int afc_pop3_get_list(POP3 *p)
{
	int res;
	char *s, *pos;
	POP3MsgData *msg;
	short subject, sender;

	afc_hash_master_clear(p->msg);

	if ((res = afc_pop3_internal_cmd(p, "LIST", TRUE)) != AFC_ERR_NO_ERROR)
		return (res);

	s = afc_string_list_first(p->sn);

	while (s)
	{
		pos = strchr(s, ' ');
		*pos = '\0';

		msg = afc_malloc(sizeof(POP3MsgData));

		msg->id = atoi(s);
		msg->size = atoi(pos + 1);

		afc_hash_master_add(p->msg, msg->id, msg);

		s = afc_string_list_next(p->sn);
	}

	// Retrieve msg subject and sender

	msg = afc_hash_master_first(p->msg);

	while (msg)
	{
		subject = sender = FALSE;

		afc_pop3_top(p, msg->id, 0);

		s = afc_string_list_first(p->sn);
		while (s)
		{
			if (afc_string_comp(s, "Subject:", 8) == 0)
			{
				msg->subject = afc_string_dup(s + 9);
				subject = TRUE;
			}

			if (afc_string_comp(s, "From:", 5) == 0)
			{
				msg->from = afc_string_dup(s + 6);
				sender = TRUE;
			}

			if (subject && sender)
				break;

			s = afc_string_list_next(p->sn);
		}

		msg = afc_hash_master_next(p->msg);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_pop3_retr ( p, num ) *********
int afc_pop3_retr(POP3 *p, int num)
{
	// if ( afc_hash_master_find ( p->msg, num ) == NULL ) return ( AFC_ERR_NO_ERROR );

	afc_string_make(p->tmp, "RETR %d", num);
	return (afc_pop3_internal_cmd(p, p->tmp, TRUE));
}
// }}}
// {{{ afc_pop3_dele ( p, num ) ********
int afc_pop3_dele(POP3 *p, int num)
{
	// if ( afc_hash_master_find ( p->msg, num ) == NULL ) return ( AFC_ERR_NO_ERROR );

	afc_string_make(p->tmp, "DELE %d", num);
	return (afc_pop3_internal_cmd(p, p->tmp, FALSE));
}
// }}}
// {{{ afc_pop3_noop ( p ) *******
int afc_pop3_noop(POP3 *p) { return (afc_pop3_internal_cmd(p, "NOOP", FALSE)); }
// }}}
// {{{ afc_pop3_rset ( p ) *******
int afc_pop3_rset(POP3 *p) { return (afc_pop3_internal_cmd(p, "RSET", FALSE)); }
// }}}
// {{{ afc_pop3_quit ( p ) *******
int afc_pop3_quit(POP3 *p) { return (afc_pop3_internal_cmd(p, "QUIT", FALSE)); }
// }}}
// {{{ afc_pop3_top ( p, msg, lines ) ********
int afc_pop3_top(POP3 *p, int msg, int lines)
{
	// if ( afc_hash_master_find ( p->msg, msg ) == NULL ) return ( AFC_ERR_NO_ERROR );

	afc_string_make(p->tmp, "TOP %d %d", msg, lines);
	return (afc_pop3_internal_cmd(p, p->tmp, TRUE));
}
// }}}

/* =====================================================================================
	INTERNAL FUNCTIONS
===================================================================================== */
// {{{ afc_pop3_internal_get_response ( p )
static int afc_pop3_internal_get_response(POP3 *p)
{
	int res;

	if ((res = afc_pop3_internal_get_line(p)) != AFC_ERR_NO_ERROR)
		return (res);

	afc_dprintf("RESP: '%s'\n", p->buf);

	if (p->buf[0] != '+')
		return (AFC_LOG(AFC_LOG_ERROR, AFC_POP3_ERR_PROTOCOL, "Protocol Error", p->buf));

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_pop3_internal_put ( p, msg )
static int afc_pop3_internal_put(POP3 *p, const char *msg)
{
	int res;

	afc_dprintf("PUT: %s\n", msg);

	if ((res = afc_inet_client_send(p->ic, msg, 0)) != AFC_ERR_NO_ERROR)
		return (res);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_pop3_internal_get_line ( p )
static int afc_pop3_internal_get_line(POP3 *p)
{
	afc_string_fget(p->buf, p->fd);
	afc_string_trim(p->buf);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_pop3_internal_get_multi_line ( p )
static int afc_pop3_internal_get_multi_line(POP3 *p)
{
	int res;

	if ((res = afc_pop3_internal_get_response(p)) != AFC_ERR_NO_ERROR)
		return (res);

	afc_string_list_clear(p->sn);

	afc_pop3_internal_get_line(p);

	while (afc_string_comp(p->buf, ".", 1) != 0)
	{
		afc_dprintf("LINE: '%s'\n", p->buf);

		afc_string_list_add(p->sn, p->buf, AFC_STRING_LIST_ADD_TAIL);
		afc_pop3_internal_get_line(p);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_pop3_internal_cmd ( p, cmd, multi )
static int afc_pop3_internal_cmd(POP3 *p, const char *cmd, short multi)
{
	int res;
	char *str;

	str = afc_string_new(strlen(cmd) + 4);
	afc_string_copy(str, cmd, ALL);
	afc_string_add(str, "\r\n", ALL);

	res = afc_pop3_internal_put(p, str);
	afc_string_delete(str);

	if (res != AFC_ERR_NO_ERROR)
		return (res);

	if (multi)
		return (afc_pop3_internal_get_multi_line(p));

	return (afc_pop3_internal_get_response(p));
}
// }}}
// {{{ afc_pop3_internal_del_msg ( hm, m )
static int afc_pop3_internal_del_msg(HashMaster *hm, void *m)
{
	POP3MsgData *msg = m;

	if (msg->subject)
		afc_string_delete(msg->subject);
	if (msg->from)
		afc_string_delete(msg->from);

	afc_free(msg);

	return (AFC_ERR_NO_ERROR);
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
void dump_messages(POP3 *p)
{
	POP3MsgData *msg;

	msg = afc_hash_master_first(p->msg);

	while (msg)
	{
		printf("MSG ID: %d - Size: %d - From: %s - Subject: %s\n", msg->id, msg->size, msg->from, msg->subject);
		msg = afc_hash_master_next(p->msg);
	}
}

int main(int argc, char *argv[])
{
	AFC *afc = afc_new();
	POP3 *p = afc_pop3_new();

	if (p == NULL)
	{
		fprintf(stderr, "Init of class POP3 failed.\n");
		return (1);
	}

	afc_pop3_set_tags(p, AFC_POP3_TAG_LOGIN, "prova@encefalo.it",
					  AFC_POP3_TAG_PASSWD, "prova",
					  AFC_POP3_TAG_HOST, "mail.encefalo.it",
					  AFC_TAG_END);

	afc_pop3_connect(p);

	afc_pop3_login(p);
	afc_pop3_stat(p);
	// afc_pop3_get_list ( p );

	dump_messages(p);

	afc_pop3_retr(p, 2);
	afc_pop3_dele(p, 2);

	afc_pop3_noop(p);
	afc_pop3_rset(p);

	afc_pop3_quit(p);

	afc_pop3_delete(p);
	afc_delete(afc);

	return (0);
}
// }}}
#endif
