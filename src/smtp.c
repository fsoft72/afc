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
#include <stdint.h>
#include "smtp.h"

/*
@config
	TITLE:     SMTP
	VERSION:   1.00
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
@endnode
*/

static const char class_name[] = "SMTP";

// {{{ afc_smtp_new ()
/*
@node afc_smtp_new

		   NAME: afc_smtp_new () - Initializes a new SMTP instance.

	   SYNOPSIS: SMTP * afc_smtp_new ()

	DESCRIPTION: This function initializes a new SMTP instance.

		  INPUT: NONE

		RESULTS: a valid initialized SMTP structure. NULL in case of errors.

	   SEE ALSO: - afc_smtp_delete()

@endnode
*/
SMTP *afc_smtp_new(void)
{
	TRY(SMTP *)

	SMTP *smtp = (SMTP *)afc_malloc(sizeof(SMTP));

	if (smtp == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "smtp", NULL);

	smtp->magic = AFC_SMTP_MAGIC;

	if ((smtp->ic = afc_inet_client_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "inet_client", NULL);

	if ((smtp->buf = afc_string_new(1024)) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "buf", NULL);

	if ((smtp->tmp = afc_string_new(1024)) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "tmp", NULL);

	// Default port is 25 (standard SMTP)
	smtp->port = afc_string_new(5);
	afc_string_copy(smtp->port, "25", ALL);

	smtp->use_tls = FALSE;
	smtp->use_ssl = FALSE;
	smtp->auth_method = AFC_SMTP_AUTH_NONE;
	smtp->authenticated = FALSE;
	smtp->connected = FALSE;

	RETURN(smtp);

	EXCEPT
	afc_smtp_delete(smtp);

	FINALLY

	ENDTRY
}
// }}}
// {{{ afc_smtp_delete ( smtp )
/*
@node afc_smtp_delete

		   NAME: afc_smtp_delete ( smtp )  - Disposes a valid SMTP instance.

	   SYNOPSIS: int afc_smtp_delete ( SMTP * smtp)

	DESCRIPTION: This function frees an already alloc'd SMTP structure.

		  INPUT: - smtp  - Pointer to a valid afc_smtp class.

		RESULTS: should be AFC_ERR_NO_ERROR

		  NOTES: - this method calls: afc_smtp_clear()

	   SEE ALSO: - afc_smtp_new()
			 - afc_smtp_clear()
@endnode
*/
int _afc_smtp_delete(SMTP *smtp)
{
	int afc_res;

	if ((afc_res = afc_smtp_clear(smtp)) != AFC_ERR_NO_ERROR)
		return (afc_res);

	afc_inet_client_delete(smtp->ic);
	afc_string_delete(smtp->buf);
	afc_string_delete(smtp->tmp);
	afc_string_delete(smtp->port);

	if (smtp->host)
		afc_string_delete(smtp->host);
	if (smtp->username)
		afc_string_delete(smtp->username);
	if (smtp->password)
		afc_string_delete(smtp->password);
	if (smtp->from)
		afc_string_delete(smtp->from);
	if (smtp->to)
		afc_string_delete(smtp->to);
	if (smtp->subject)
		afc_string_delete(smtp->subject);

	afc_free(smtp);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_smtp_clear ( smtp )
/*
@node afc_smtp_clear

		   NAME: afc_smtp_clear ( smtp )  - Clears all stored data

	   SYNOPSIS: int afc_smtp_clear ( SMTP * smtp)

	DESCRIPTION: Use this function to clear all stored data in the current smtp instance.

		  INPUT: - smtp    - Pointer to a valid afc_smtp instance.

		RESULTS: should be AFC_ERR_NO_ERROR

	   SEE ALSO: - afc_smtp_delete()

@endnode
*/
int afc_smtp_clear(SMTP *smtp)
{
	if (smtp == NULL)
		return (AFC_ERR_NULL_POINTER);

	if (smtp->magic != AFC_SMTP_MAGIC)
		return (AFC_ERR_INVALID_POINTER);

	smtp->authenticated = FALSE;
	smtp->connected = FALSE;

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_smtp_set_tags ( smtp, first_tag, ... )
/*
@node afc_smtp_set_tags

		   NAME: afc_smtp_set_tags ( smtp, first_tag, ... )  - Set tags for SMTP

	   SYNOPSIS: int afc_smtp_set_tags ( SMTP * smtp, int first_tag, ... )

	DESCRIPTION: Use this function to set tags for the SMTP instance.

		  INPUT: - smtp    - Pointer to a valid afc_smtp instance.
		 - first_tag - First tag to set
		 - ... - Tag values

		RESULTS: should be AFC_ERR_NO_ERROR

	   SEE ALSO: - afc_smtp_set_tag()

@endnode
*/
int _afc_smtp_set_tags(SMTP *smtp, int first_tag, ...)
{
	va_list tags;
	int tag;
	void *val;
	int res = AFC_ERR_NO_ERROR;

	va_start(tags, first_tag);

	tag = first_tag;

	while (tag != AFC_TAG_END)
	{
		val = va_arg(tags, void *);

		if ((res = afc_smtp_set_tag(smtp, tag, val)) != AFC_ERR_NO_ERROR)
			break;

		tag = va_arg(tags, int);
	}

	va_end(tags);

	return res;
}
// }}}
// {{{ afc_smtp_set_tag ( smtp, tag, val )
/*
@node afc_smtp_set_tag

		   NAME: afc_smtp_set_tag ( smtp, tag, val )  - Set a single tag

	   SYNOPSIS: int afc_smtp_set_tag ( SMTP * smtp, int tag, void * val )

	DESCRIPTION: Use this function to set a single tag for the SMTP instance.

		  INPUT: - smtp    - Pointer to a valid afc_smtp instance.
		 - tag   - Tag to set
		 - val   - Tag value

		RESULTS: should be AFC_ERR_NO_ERROR

	   SEE ALSO: - afc_smtp_set_tags()

@endnode
*/
int afc_smtp_set_tag(SMTP *smtp, int tag, void *val)
{
	if (!smtp)
		return AFC_ERR_NULL_POINTER;

	switch (tag)
	{
	case AFC_SMTP_TAG_HOST:
		if (smtp->host)
			afc_string_delete(smtp->host);
		smtp->host = afc_string_dup((char *)val);
		break;

	case AFC_SMTP_TAG_PORT:
		afc_string_copy(smtp->port, (char *)val, ALL);
		break;

	case AFC_SMTP_TAG_USERNAME:
		if (smtp->username)
			afc_string_delete(smtp->username);
		smtp->username = afc_string_dup((char *)val);
		break;

	case AFC_SMTP_TAG_PASSWORD:
		if (smtp->password)
			afc_string_delete(smtp->password);
		smtp->password = afc_string_dup((char *)val);
		break;

	case AFC_SMTP_TAG_FROM:
		if (smtp->from)
			afc_string_delete(smtp->from);
		smtp->from = afc_string_dup((char *)val);
		break;

	case AFC_SMTP_TAG_TO:
		if (smtp->to)
			afc_string_delete(smtp->to);
		smtp->to = afc_string_dup((char *)val);
		break;

	case AFC_SMTP_TAG_SUBJECT:
		if (smtp->subject)
			afc_string_delete(smtp->subject);
		smtp->subject = afc_string_dup((char *)val);
		break;

	case AFC_SMTP_TAG_USE_TLS:
		smtp->use_tls = (BOOL)(long)val;
		break;

	case AFC_SMTP_TAG_USE_SSL:
		smtp->use_ssl = (BOOL)(long)val;
		break;

	case AFC_SMTP_TAG_AUTH_METHOD:
		smtp->auth_method = (int)(long)val;
		break;

	default:
		return AFC_LOG(AFC_LOG_ERROR, AFC_ERR_UNSUPPORTED_TAG, "Unsupported tag", NULL);
	}

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ _afc_smtp_get_response ( smtp )
/*
 * _afc_smtp_get_response - Read SMTP server response
 *
 * Returns: SMTP response code (e.g., 220, 250, etc.) or negative on error
 */
int _afc_smtp_get_response(SMTP *smtp)
{
	int res;

	if (!smtp || smtp->magic != AFC_SMTP_MAGIC)
		return AFC_LOG(AFC_LOG_ERROR, AFC_ERR_INVALID_POINTER, "Invalid SMTP object", NULL);

	afc_string_clear(smtp->buf);

	if ((res = afc_inet_client_get(smtp->ic)) != AFC_ERR_NO_ERROR)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_PROTOCOL, "Failed to get response", NULL);

	// Copy response from InetClient buffer to SMTP buffer
	afc_string_copy(smtp->buf, smtp->ic->buf, ALL);

	// SMTP responses start with a 3-digit code
	if (afc_string_len(smtp->buf) < 3)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_INVALID_RESPONSE, "Invalid response", smtp->buf);

	// Extract response code safely
	char code_str[4];
	snprintf(code_str, sizeof(code_str), "%.3s", smtp->buf);

	return atoi(code_str);
}
// }}}
// {{{ _afc_smtp_send_command ( smtp, cmd )
/*
 * _afc_smtp_send_command - Send SMTP command and get response
 *
 * Returns: SMTP response code or negative on error
 */
int _afc_smtp_send_command(SMTP *smtp, const char *cmd)
{
	if (!smtp || smtp->magic != AFC_SMTP_MAGIC)
		return AFC_LOG(AFC_LOG_ERROR, AFC_ERR_INVALID_POINTER, "Invalid SMTP object", NULL);

	afc_string_make(smtp->tmp, "%s\r\n", cmd);

	if (afc_inet_client_send(smtp->ic, smtp->tmp, 0) != AFC_ERR_NO_ERROR)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_SEND_FAILED, "Failed to send command", cmd);

	return _afc_smtp_get_response(smtp);
}
// }}}
// {{{ afc_smtp_connect ( smtp )
/*
@node afc_smtp_connect

		   NAME: afc_smtp_connect ( smtp )  - Connect to SMTP server

	   SYNOPSIS: int afc_smtp_connect ( SMTP * smtp )

	DESCRIPTION: Connect to the SMTP server and perform initial handshake.

		  INPUT: - smtp    - Pointer to a valid afc_smtp instance.

		RESULTS: - AFC_ERR_NO_ERROR on success
		 - Error code on failure

	   SEE ALSO: - afc_smtp_quit()

@endnode
*/
int afc_smtp_connect(SMTP *smtp)
{
	int res;
	int port;

	if (!smtp)
		return AFC_ERR_NULL_POINTER;
	if (!smtp->host)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_CONNECT, "No host specified", NULL);

	port = atoi(smtp->port);

	// Open connection
	if ((res = afc_inet_client_open(smtp->ic, smtp->host, port)) != AFC_ERR_NO_ERROR)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_CONNECT, "Failed to connect", smtp->host);

	// If using direct SSL (port 465), enable SSL now
	if (smtp->use_ssl)
	{
		if ((res = afc_inet_client_enable_ssl(smtp->ic)) != AFC_ERR_NO_ERROR)
			return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_CONNECT, "Failed to enable SSL", NULL);
	}

	// Get server greeting (220)
	if ((res = _afc_smtp_get_response(smtp)) != 220)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_PROTOCOL, "Invalid server greeting", smtp->buf);

	// Send EHLO command
	if ((res = _afc_smtp_send_command(smtp, "EHLO localhost")) != 250)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_PROTOCOL, "EHLO failed", smtp->buf);

	smtp->connected = TRUE;

	// If using STARTTLS (port 587), upgrade to TLS now
	if (smtp->use_tls)
	{
		if ((res = _afc_smtp_send_command(smtp, "STARTTLS")) != 220)
			return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_TLS_REQUIRED, "STARTTLS failed", smtp->buf);

		if ((res = afc_inet_client_start_tls(smtp->ic)) != AFC_ERR_NO_ERROR)
			return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_TLS_REQUIRED, "Failed to start TLS", NULL);

		// Send EHLO again after STARTTLS
		if ((res = _afc_smtp_send_command(smtp, "EHLO localhost")) != 250)
			return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_PROTOCOL, "EHLO after STARTTLS failed", smtp->buf);
	}

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ _afc_smtp_auth_plain ( smtp )
/*
 * _afc_smtp_auth_plain - Perform AUTH PLAIN authentication
 *
 * Returns: AFC_ERR_NO_ERROR on success, error code on failure
 */
int _afc_smtp_auth_plain(SMTP *smtp)
{
	int res;
	char *auth_str = NULL;
	char *encoded = NULL;
	char *clean = NULL;
	char *cmd = NULL;
	unsigned long user_len, pass_len, auth_len;
	Base64 *b64 = NULL;

	if (!smtp || smtp->magic != AFC_SMTP_MAGIC)
		return AFC_LOG(AFC_LOG_ERROR, AFC_ERR_INVALID_POINTER, "Invalid SMTP object", NULL);

	// Defensive NULL checks (caller should verify but check anyway)
	if (!smtp->username || !smtp->password)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_AUTH, "Missing credentials", NULL);

	user_len = afc_string_len(smtp->username);
	pass_len = afc_string_len(smtp->password);

	// Check for integer overflow before allocation
	if (user_len > SIZE_MAX - pass_len - 2)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_AUTH, "Credentials too long", NULL);

	// AUTH PLAIN format: base64("\0username\0password")
	auth_len = 1 + user_len + 1 + pass_len;
	auth_str = afc_string_new(auth_len + 1);
	if (!auth_str)
		return AFC_LOG(AFC_LOG_ERROR, AFC_ERR_NO_MEMORY, "Failed to allocate auth_str", NULL);

	// Build auth string: \0username\0password using safer memcpy
	auth_str[0] = '\0';
	memcpy(auth_str + 1, smtp->username, user_len);
	auth_str[1 + user_len] = '\0';
	memcpy(auth_str + 1 + user_len + 1, smtp->password, pass_len);

	// Base64 encode
	encoded = afc_string_new(auth_len * 2 + 10);
	if (!encoded)
	{
		afc_string_delete(auth_str);
		return AFC_LOG(AFC_LOG_ERROR, AFC_ERR_NO_MEMORY, "Failed to allocate encoded", NULL);
	}

	b64 = afc_base64_new();
	if (!b64)
	{
		afc_string_delete(auth_str);
		afc_string_delete(encoded);
		return AFC_LOG(AFC_LOG_ERROR, AFC_ERR_NO_MEMORY, "Failed to create Base64", NULL);
	}

	afc_base64_encode(b64,
					  AFC_BASE64_TAG_MEM_IN, auth_str,
					  AFC_BASE64_TAG_MEM_IN_SIZE, auth_len,
					  AFC_BASE64_TAG_MEM_OUT, encoded,
					  AFC_BASE64_TAG_MEM_OUT_SIZE, auth_len * 2 + 10,
					  AFC_TAG_END);
	afc_base64_delete(b64);
	afc_string_delete(auth_str);

	// Remove trailing CRLF added by base64 encoder
	afc_string_trim(encoded);

	// Remove internal CRLF if any (base64 splits lines at 76 chars)
	clean = afc_string_new(afc_string_max(encoded));
	if (!clean)
	{
		afc_string_delete(encoded);
		return AFC_LOG(AFC_LOG_ERROR, AFC_ERR_NO_MEMORY, "Failed to allocate clean", NULL);
	}
	afc_string_replace_all(clean, encoded, "\r\n", "");
	afc_string_copy(encoded, clean, ALL);
	afc_string_delete(clean);

	// Send AUTH PLAIN command
	cmd = afc_string_new(auth_len * 2 + 20);
	if (!cmd)
	{
		afc_string_delete(encoded);
		return AFC_LOG(AFC_LOG_ERROR, AFC_ERR_NO_MEMORY, "Failed to allocate cmd", NULL);
	}
	afc_string_make(cmd, "AUTH PLAIN %s", encoded);
	afc_string_delete(encoded);

	res = _afc_smtp_send_command(smtp, cmd);
	afc_string_delete(cmd);

	if (res != 235)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_AUTH, "AUTH PLAIN failed", smtp->buf);

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ _afc_smtp_auth_login ( smtp )
/*
 * _afc_smtp_auth_login - Perform AUTH LOGIN authentication
 *
 * Returns: AFC_ERR_NO_ERROR on success, error code on failure
 */
int _afc_smtp_auth_login(SMTP *smtp)
{
	int res;
	char *encoded_user, *encoded_pass;
	Base64 *b64;
	int user_len, pass_len;

	if (!smtp || smtp->magic != AFC_SMTP_MAGIC)
		return AFC_LOG(AFC_LOG_ERROR, AFC_ERR_INVALID_POINTER, "Invalid SMTP object", NULL);

	// Send AUTH LOGIN command
	if ((res = _afc_smtp_send_command(smtp, "AUTH LOGIN")) != 334)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_AUTH, "AUTH LOGIN failed", smtp->buf);

	// Send base64-encoded username
	user_len = afc_string_len(smtp->username);
	encoded_user = afc_string_new(user_len * 2 + 10);
	b64 = afc_base64_new();
	afc_base64_encode(b64,
					  AFC_BASE64_TAG_MEM_IN, smtp->username,
					  AFC_BASE64_TAG_MEM_IN_SIZE, user_len,
					  AFC_BASE64_TAG_MEM_OUT, encoded_user,
					  AFC_BASE64_TAG_MEM_OUT_SIZE, user_len * 2 + 10,
					  AFC_TAG_END);
	afc_base64_delete(b64);

	// Remove trailing CRLF added by base64 encoder
	afc_string_trim(encoded_user);
	
	// Remove internal CRLF
	char *clean_user = afc_string_new(afc_string_max(encoded_user));
	afc_string_replace_all(clean_user, encoded_user, "\r\n", "");
	afc_string_copy(encoded_user, clean_user, ALL);
	afc_string_delete(clean_user);

	if ((res = _afc_smtp_send_command(smtp, encoded_user)) != 334)
	{
		afc_string_delete(encoded_user);
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_AUTH, "Username rejected", smtp->buf);
	}
	afc_string_delete(encoded_user);

	// Send base64-encoded password
	pass_len = afc_string_len(smtp->password);
	encoded_pass = afc_string_new(pass_len * 2 + 10);
	b64 = afc_base64_new();
	afc_base64_encode(b64,
					  AFC_BASE64_TAG_MEM_IN, smtp->password,
					  AFC_BASE64_TAG_MEM_IN_SIZE, pass_len,
					  AFC_BASE64_TAG_MEM_OUT, encoded_pass,
					  AFC_BASE64_TAG_MEM_OUT_SIZE, pass_len * 2 + 10,
					  AFC_TAG_END);
	afc_base64_delete(b64);

	// Remove trailing CRLF added by base64 encoder
	afc_string_trim(encoded_pass);
	
	// Remove internal CRLF
	char *clean_pass = afc_string_new(afc_string_max(encoded_pass));
	afc_string_replace_all(clean_pass, encoded_pass, "\r\n", "");
	afc_string_copy(encoded_pass, clean_pass, ALL);
	afc_string_delete(clean_pass);

	if ((res = _afc_smtp_send_command(smtp, encoded_pass)) != 235)
	{
		afc_string_delete(encoded_pass);
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_AUTH, "Password rejected", smtp->buf);
	}
	afc_string_delete(encoded_pass);

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ afc_smtp_authenticate ( smtp )
/*
@node afc_smtp_authenticate

		   NAME: afc_smtp_authenticate ( smtp )  - Authenticate with SMTP server

	   SYNOPSIS: int afc_smtp_authenticate ( SMTP * smtp )

	DESCRIPTION: Authenticate with the SMTP server using configured credentials.

		  INPUT: - smtp    - Pointer to a valid afc_smtp instance.

		RESULTS: - AFC_ERR_NO_ERROR on success
		 - Error code on failure

	   SEE ALSO: - afc_smtp_connect()

@endnode
*/
int afc_smtp_authenticate(SMTP *smtp)
{
	int res;

	if (!smtp)
		return AFC_ERR_NULL_POINTER;
	if (!smtp->connected)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_AUTH, "Not connected", NULL);
	if (!smtp->username || !smtp->password)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_AUTH, "No credentials", NULL);

	switch (smtp->auth_method)
	{
	case AFC_SMTP_AUTH_PLAIN:
		res = _afc_smtp_auth_plain(smtp);
		break;

	case AFC_SMTP_AUTH_LOGIN:
		res = _afc_smtp_auth_login(smtp);
		break;

	default:
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_AUTH, "Unsupported auth method", NULL);
	}

	if (res == AFC_ERR_NO_ERROR)
		smtp->authenticated = TRUE;

	return res;
}
// }}}
// {{{ afc_smtp_send ( smtp, message )
/*
@node afc_smtp_send

		   NAME: afc_smtp_send ( smtp, message )  - Send email message

	   SYNOPSIS: int afc_smtp_send ( SMTP * smtp, const char * message )

	DESCRIPTION: Send an email message through the SMTP server.

		  INPUT: - smtp    - Pointer to a valid afc_smtp instance.
		 - message - The complete email message (headers + body)

		RESULTS: - AFC_ERR_NO_ERROR on success
		 - Error code on failure

	   SEE ALSO: - afc_smtp_send_simple()

@endnode
*/
int afc_smtp_send(SMTP *smtp, const char *message)
{
	int res;

	if (!smtp)
		return AFC_ERR_NULL_POINTER;
	if (!smtp->connected)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_SEND_FAILED, "Not connected", NULL);
	if (!smtp->from)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_NO_SENDER, "No sender specified", NULL);
	if (!smtp->to)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_NO_RECIPIENTS, "No recipients specified", NULL);

	// MAIL FROM
	char *mail_from_cmd = afc_string_new(256);
	afc_string_make(mail_from_cmd, "MAIL FROM:<%s>", smtp->from);
	res = _afc_smtp_send_command(smtp, mail_from_cmd);
	afc_string_delete(mail_from_cmd);
	if (res != 250)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_SEND_FAILED, "MAIL FROM failed", smtp->buf);

	// RCPT TO (support multiple recipients separated by comma)
	char *to_list = afc_string_dup(smtp->to);
	char *saveptr = NULL;
	char *recipient = strtok_r(to_list, ",", &saveptr);
	char *rcpt_to_cmd = afc_string_new(256);
	while (recipient)
	{
		// Trim whitespace
		while (*recipient == ' ')
			recipient++;

		afc_string_make(rcpt_to_cmd, "RCPT TO:<%s>", recipient);
		if ((res = _afc_smtp_send_command(smtp, rcpt_to_cmd)) != 250)
		{
			afc_string_delete(rcpt_to_cmd);
			afc_string_delete(to_list);
			return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_SEND_FAILED, "RCPT TO failed", smtp->buf);
		}

		recipient = strtok_r(NULL, ",", &saveptr);
	}
	afc_string_delete(rcpt_to_cmd);
	afc_string_delete(to_list);

	// DATA command
	if ((res = _afc_smtp_send_command(smtp, "DATA")) != 354)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_SEND_FAILED, "DATA failed", smtp->buf);

	// Send message
	if (afc_inet_client_send(smtp->ic, message, 0) != AFC_ERR_NO_ERROR)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_SEND_FAILED, "Failed to send message", NULL);

	// End with CRLF.CRLF
	if ((res = _afc_smtp_send_command(smtp, "\r\n.")) != 250)
		return AFC_LOG(AFC_LOG_ERROR, AFC_SMTP_ERR_SEND_FAILED, "Message not accepted", smtp->buf);

	return AFC_ERR_NO_ERROR;
}
// }}}
// {{{ afc_smtp_send_simple ( smtp, from, to, subject, body )
/*
@node afc_smtp_send_simple

		   NAME: afc_smtp_send_simple ( smtp, from, to, subject, body )  - Send simple email

	   SYNOPSIS: int afc_smtp_send_simple ( SMTP * smtp, const char * from, const char * to, const char * subject, const char * body )

	DESCRIPTION: Send a simple text email. This is a convenience function that builds
			 the message headers automatically.

		  INPUT: - smtp    - Pointer to a valid afc_smtp instance.
		 - from    - Sender email address
		 - to      - Recipient email address(es), comma-separated
		 - subject - Email subject
		 - body    - Email body (plain text)

		RESULTS: - AFC_ERR_NO_ERROR on success
		 - Error code on failure

	   SEE ALSO: - afc_smtp_send()

@endnode
*/
int afc_smtp_send_simple(SMTP *smtp, const char *from, const char *to, const char *subject, const char *body)
{
	char *message;
	int res;

	if (!smtp)
		return AFC_ERR_NULL_POINTER;

	// Set FROM, TO, SUBJECT
	afc_smtp_set_tags(smtp,
					  AFC_SMTP_TAG_FROM, from,
					  AFC_SMTP_TAG_TO, to,
					  AFC_SMTP_TAG_SUBJECT, subject);

	// Build message with headers
	message = afc_string_new(4096);
	afc_string_make(message, "From: %s\r\n", from);
	afc_string_add(message, "To: ", ALL);
	afc_string_add(message, to, ALL);
	afc_string_add(message, "\r\n", ALL);
	afc_string_add(message, "Subject: ", ALL);
	afc_string_add(message, subject, ALL);
	afc_string_add(message, "\r\n", ALL);
	afc_string_add(message, "Content-Type: text/plain; charset=UTF-8\r\n", ALL);
	afc_string_add(message, "\r\n", ALL);
	afc_string_add(message, body, ALL);

	res = afc_smtp_send(smtp, message);

	afc_string_delete(message);

	return res;
}
// }}}
// {{{ afc_smtp_quit ( smtp )
/*
@node afc_smtp_quit

		   NAME: afc_smtp_quit ( smtp )  - Close SMTP connection

	   SYNOPSIS: int afc_smtp_quit ( SMTP * smtp )

	DESCRIPTION: Send QUIT command and close the connection to the SMTP server.

		  INPUT: - smtp    - Pointer to a valid afc_smtp instance.

		RESULTS: - AFC_ERR_NO_ERROR on success

	   SEE ALSO: - afc_smtp_connect()

@endnode
*/
int afc_smtp_quit(SMTP *smtp)
{
	if (!smtp)
		return AFC_ERR_NULL_POINTER;

	if (smtp->connected)
	{
		_afc_smtp_send_command(smtp, "QUIT");
		afc_inet_client_close(smtp->ic);
		smtp->connected = FALSE;
		smtp->authenticated = FALSE;
	}

	return AFC_ERR_NO_ERROR;
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
int main(int argc, char *argv[])
{
	AFC *afc = afc_new();
	SMTP *smtp = afc_smtp_new();

	if (smtp == NULL)
	{
		fprintf(stderr, "Init of class SMTP failed.\n");
		return (1);
	}

	// Example usage for Amazon SES
	printf("SMTP class initialized successfully.\n");
	printf("To use with Amazon SES, configure:\n");
	printf("  Host: email-smtp.us-east-1.amazonaws.com\n");
	printf("  Port: 587 (STARTTLS) or 465 (SSL)\n");
	printf("  Auth: PLAIN or LOGIN\n");

	afc_smtp_delete(smtp);
	afc_delete(afc);

	return (0);
}
// }}}
#endif
