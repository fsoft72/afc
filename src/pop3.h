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
#ifndef AFC_POP3_H
#define AFC_POP3_H
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "afc.h"

/* POP3'Magic' value: 'POP3' */
#define AFC_POP3_MAGIC ('P' << 24 | 'O' << 16 | 'P' << 8 | '3')

/* POP3 Base  */
#define AFC_POP3_BASE 0x1000

enum
{
	AFC_POP3_TAG_HOST = AFC_POP3_BASE + 1,
	AFC_POP3_TAG_PORT,
	AFC_POP3_TAG_LOGIN,
	AFC_POP3_TAG_PASSWD
};

enum
{
	AFC_POP3_ERR_PROTOCOL = AFC_POP3_BASE
};

struct afc_pop3_msg_data
{
	int id;
	int size;

	char *subject;
	char *from;
};

typedef struct afc_pop3_msg_data POP3MsgData;

struct afc_pop3
{
	unsigned long magic; /* POP3 Magic Value */

	InetClient *ic;

	FILE *fd; // File Descriptor

	char *host;	  // Host Name
	char *port;	  // Port Name (default: 110)
	char *login;  // Login
	char *passwd; // Password

	char *buf; // Buffer
	char *tmp; // Temp Buffer

	StringNode *sn;	 // Long Response
	HashMaster *msg; // Hash Table of Messages (populated by 'LIST')

	int tot_messages; // Number of messages in mailbox
	int tot_size;	  // Size of mailbox
};

typedef struct afc_pop3 POP3;

/* Function Prototypes */
#define afc_pop3_delete(p3)   \
	if (p3)                   \
	{                         \
		_afc_pop3_delete(p3); \
		p3 = NULL;            \
	}

POP3 *afc_pop3_new(void);
int _afc_pop3_delete(POP3 *p);
int afc_pop3_clear(POP3 *p);

#define afc_pop3_set_tags(p, first, ...) _afc_pop3_set_tags(p, first, ##__VA_ARGS__, AFC_TAG_END)
int _afc_pop3_set_tags(POP3 *p, int first_tag, ...);
int afc_pop3_set_tag(POP3 *p, int tag, void *val);
int afc_pop3_connect(POP3 *p);
int afc_pop3_login(POP3 *p);
int afc_pop3_stat(POP3 *p);
int afc_pop3_get_list(POP3 *p);
int afc_pop3_retr(POP3 *p, int num);
int afc_pop3_dele(POP3 *p, int num);
int afc_pop3_noop(POP3 *p);
int afc_pop3_rset(POP3 *p);
int afc_pop3_quit(POP3 *p);
int afc_pop3_top(POP3 *p, int msg, int lines);
#endif
