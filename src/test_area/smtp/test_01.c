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
 * SMTP Test - Send email using settings from settings.txt
 *
 * This test reads SMTP configuration from settings.txt and sends a test email.
 * Copy settings.txt.example to settings.txt and configure your SMTP settings.
 */

#include "../test_utils.h"
#include <string.h>

/*
 * _read_settings - Read settings from settings.txt file
 *
 * Reads key=value pairs from settings.txt, ignoring comments and empty lines.
 * Returns a Dictionary with all settings.
 */
static Dictionary *_read_settings(AFC *afc)
{
	FILE *fp;
	Dictionary *dict;
	char line[1024];
	char *key, *value, *p;

	dict = afc_dictionary_new();
	if (!dict) return NULL;

	fp = fopen("settings.txt", "r");
	if (!fp) {
		printf("ERROR: Cannot open settings.txt\n");
		printf("Copy settings.txt.example to settings.txt and configure your SMTP settings.\n");
		afc_dictionary_delete(dict);
		return NULL;
	}

	while (fgets(line, sizeof(line), fp)) {
		// Remove trailing newline
		p = strchr(line, '\n');
		if (p) *p = '\0';

		// Skip comments and empty lines
		if (line[0] == '#' || line[0] == '\0') continue;

		// Skip whitespace-only lines
		p = line;
		while (*p && (*p == ' ' || *p == '\t')) p++;
		if (*p == '\0') continue;

		// Split on '='
		key = line;
		value = strchr(line, '=');
		if (!value) continue;

		*value = '\0';
		value++;

		// Trim whitespace from key
		p = key + strlen(key) - 1;
		while (p > key && (*p == ' ' || *p == '\t')) {
			*p = '\0';
			p--;
		}

		// Trim whitespace from value
		while (*value && (*value == ' ' || *value == '\t')) value++;
		p = value + strlen(value) - 1;
		while (p > value && (*p == ' ' || *p == '\t')) {
			*p = '\0';
			p--;
		}

		afc_dictionary_set(dict, key, value);
	}

	fclose(fp);
	return dict;
}

/*
 * _get_auth_method - Convert auth method string to enum value
 */
static int _get_auth_method(const char *method)
{
	if (!method) return AFC_SMTP_AUTH_PLAIN;
	if (strcmp(method, "auth_login") == 0) return AFC_SMTP_AUTH_LOGIN;
	if (strcmp(method, "auth_plain") == 0) return AFC_SMTP_AUTH_PLAIN;
	if (strcmp(method, "auth_none") == 0) return AFC_SMTP_AUTH_NONE;
	return AFC_SMTP_AUTH_PLAIN;
}

int main()
{
	AFC *afc;
	SMTP *smtp;
	Dictionary *settings;
	const char *host, *port, *username, *password;
	const char *from, *to, *auth_method_str;
	const char *use_tls_str, *use_ssl_str;
	int auth_method;
	BOOL use_tls = FALSE;
	BOOL use_ssl = FALSE;
	int res;

	test_header();

	afc = afc_new();
	if (!afc) {
		printf("FATAL: Cannot create AFC instance\n");
		return 1;
	}

	// Read settings
	printf("Reading settings from settings.txt...\n");
	settings = _read_settings(afc);
	if (!settings) {
		afc_delete(afc);
		return 1;
	}

	// Get configuration
	host = afc_dictionary_get(settings, "host");
	port = afc_dictionary_get(settings, "port");
	username = afc_dictionary_get(settings, "username");
	password = afc_dictionary_get(settings, "password");
	from = afc_dictionary_get(settings, "from");
	to = afc_dictionary_get(settings, "to");
	auth_method_str = afc_dictionary_get(settings, "auth_method");
	use_tls_str = afc_dictionary_get(settings, "use_tls");
	use_ssl_str = afc_dictionary_get(settings, "use_ssl");

	// Validate required settings
	if (!host || !port || !username || !password || !from || !to) {
		printf("ERROR: Missing required settings in settings.txt\n");
		printf("Required: host, port, username, password, from, to\n");
		afc_dictionary_delete(settings);
		afc_delete(afc);
		return 1;
	}

	// Parse TLS/SSL flags
	if (use_tls_str && strcmp(use_tls_str, "1") == 0) use_tls = TRUE;
	if (use_ssl_str && strcmp(use_ssl_str, "1") == 0) use_ssl = TRUE;

	// Get auth method
	auth_method = _get_auth_method(auth_method_str);

	// Display configuration
	printf("\n--- SMTP Configuration ---\n");
	printf("Host: %s\n", host);
	printf("Port: %s\n", port);
	printf("Username: %s\n", username);
	printf("From: %s\n", from);
	printf("To: %s\n", to);
	printf("Use TLS: %s\n", use_tls ? "YES" : "NO");
	printf("Use SSL: %s\n", use_ssl ? "YES" : "NO");
	printf("Auth Method: %s\n", auth_method_str ? auth_method_str : "auth_plain");
	printf("-------------------------\n\n");

	// Create SMTP instance
	smtp = afc_smtp_new();
	if (!smtp) {
		printf("FATAL: Cannot create SMTP instance\n");
		afc_dictionary_delete(settings);
		afc_delete(afc);
		return 1;
	}

	// Configure SMTP
	printf("Configuring SMTP client...\n");
	afc_smtp_set_tags(smtp,
		AFC_SMTP_TAG_HOST, host,
		AFC_SMTP_TAG_PORT, port,
		AFC_SMTP_TAG_USERNAME, username,
		AFC_SMTP_TAG_PASSWORD, password,
		AFC_SMTP_TAG_USE_TLS, (void *)use_tls,
		AFC_SMTP_TAG_USE_SSL, (void *)use_ssl,
		AFC_SMTP_TAG_AUTH_METHOD, (void *)auth_method);

	// Connect
	printf("Connecting to SMTP server...\n");
	res = afc_smtp_connect(smtp);
	if (res != AFC_ERR_NO_ERROR) {
		printf("ERROR: Failed to connect to SMTP server (error: %d)\n", res);
		afc_smtp_delete(smtp);
		afc_dictionary_delete(settings);
		afc_delete(afc);
		return 1;
	}
	printf("Connected successfully!\n");

	// Authenticate
	printf("Authenticating...\n");
	res = afc_smtp_authenticate(smtp);
	if (res != AFC_ERR_NO_ERROR) {
		printf("ERROR: Authentication failed (error: %d)\n", res);
		afc_smtp_quit(smtp);
		afc_smtp_delete(smtp);
		afc_dictionary_delete(settings);
		afc_delete(afc);
		return 1;
	}
	printf("Authenticated successfully!\n");

	// Send test email
	printf("\nSending test email...\n");
	res = afc_smtp_send_simple(smtp,
		from,
		to,
		"AFC SMTP Test - Success!",
		"This is a test email sent from AFC SMTP library.\n\n"
		"If you are reading this, the SMTP client is working correctly!\n\n"
		"Test details:\n"
		"- Host: Email server configured in settings.txt\n"
		"- TLS/SSL: Secure connection established\n"
		"- Authentication: Successful\n\n"
		"AFC - Advanced Foundation Classes\n"
		"https://github.com/fsoft72/afc\n");

	if (res != AFC_ERR_NO_ERROR) {
		printf("ERROR: Failed to send email (error: %d)\n", res);
	} else {
		printf("Email sent successfully!\n");
		print_res("afc_smtp_send_simple", (void *)AFC_ERR_NO_ERROR, (void *)res, 0);
	}

	// Quit
	printf("\nClosing connection...\n");
	afc_smtp_quit(smtp);

	// Cleanup
	afc_smtp_delete(smtp);
	afc_dictionary_delete(settings);

	print_summary();

	afc_delete(afc);

	return 0;
}
