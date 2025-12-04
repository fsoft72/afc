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
#ifndef AFC_SMTP_H
#define AFC_SMTP_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "afc.h"

/* SMTP Magic value: 'SMTP' */
#define AFC_SMTP_MAGIC ('S' << 24 | 'M' << 16 | 'T' << 8 | 'P')

/* SMTP Base */
#define AFC_SMTP_BASE 0x2000

// SMTP tags for configuration
enum
{
	AFC_SMTP_TAG_HOST = AFC_SMTP_BASE + 1,
	AFC_SMTP_TAG_PORT,
	AFC_SMTP_TAG_USERNAME,
	AFC_SMTP_TAG_PASSWORD,
	AFC_SMTP_TAG_FROM,
	AFC_SMTP_TAG_TO,
	AFC_SMTP_TAG_SUBJECT,
	AFC_SMTP_TAG_USE_TLS,		// Use STARTTLS (port 587)
	AFC_SMTP_TAG_USE_SSL,		// Use direct SSL (port 465)
	AFC_SMTP_TAG_AUTH_METHOD	// AUTH method (PLAIN, LOGIN)
};

// SMTP error codes
enum
{
	AFC_SMTP_ERR_PROTOCOL = AFC_SMTP_BASE,
	AFC_SMTP_ERR_CONNECT,
	AFC_SMTP_ERR_AUTH,
	AFC_SMTP_ERR_INVALID_RESPONSE,
	AFC_SMTP_ERR_SEND_FAILED,
	AFC_SMTP_ERR_NO_RECIPIENTS,
	AFC_SMTP_ERR_NO_SENDER,
	AFC_SMTP_ERR_TLS_REQUIRED
};

// SMTP authentication methods
enum
{
	AFC_SMTP_AUTH_NONE = 0,
	AFC_SMTP_AUTH_PLAIN,
	AFC_SMTP_AUTH_LOGIN
};

struct afc_smtp
{
	unsigned long magic; /* SMTP Magic Value */

	InetClient *ic; // InetClient for network operations

	char *host;		// SMTP server hostname
	char *port;		// SMTP server port (default: 25, TLS: 587, SSL: 465)
	char *username; // Authentication username
	char *password; // Authentication password

	char *from;	   // Sender email address
	char *to;	   // Recipient email addresses (comma-separated)
	char *subject; // Email subject

	char *buf; // Response buffer
	char *tmp; // Temporary buffer

	BOOL use_tls;	  // Use STARTTLS
	BOOL use_ssl;	  // Use direct SSL
	int auth_method;  // Authentication method

	BOOL authenticated; // Flag: authenticated successfully
	BOOL connected;		// Flag: connected to server
};

typedef struct afc_smtp SMTP;

/* Function Prototypes */
#define afc_smtp_delete(smtp)	\
	if (smtp)					\
	{							\
		_afc_smtp_delete(smtp); \
		smtp = NULL;			\
	}

// Core functions
SMTP *afc_smtp_new(void);
int _afc_smtp_delete(SMTP *smtp);
int afc_smtp_clear(SMTP *smtp);

// Configuration
#define afc_smtp_set_tags(smtp, first, ...) _afc_smtp_set_tags(smtp, first, ##__VA_ARGS__, AFC_TAG_END)
int _afc_smtp_set_tags(SMTP *smtp, int first_tag, ...);
int afc_smtp_set_tag(SMTP *smtp, int tag, void *val);

// Connection and authentication
int afc_smtp_connect(SMTP *smtp);
int afc_smtp_authenticate(SMTP *smtp);
int afc_smtp_quit(SMTP *smtp);

// Email sending
int afc_smtp_send(SMTP *smtp, const char *message);
int afc_smtp_send_simple(SMTP *smtp, const char *from, const char *to, const char *subject, const char *body);

// Internal helper functions (not for direct use)
int _afc_smtp_get_response(SMTP *smtp);
int _afc_smtp_send_command(SMTP *smtp, const char *cmd);
int _afc_smtp_send_data(SMTP *smtp, const char *data);
int _afc_smtp_auth_plain(SMTP *smtp);
int _afc_smtp_auth_login(SMTP *smtp);

#endif
