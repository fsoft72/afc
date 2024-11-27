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
#ifndef AFC_REGEXP_H
#define AFC_REGEXP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "pcre/pcre.h"

#include "base.h"
#include "nodemaster.h"
#include "string.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* AFC afc_regexp Magic Number */
#define AFC_REGEXP_MAGIC ('R' << 24 | 'E' << 16 | 'G' << 8 | 'X')

/* AFC afc_regexp Base value for constants */
#define AFC_REGEXP_BASE 0x6000

/* AFC afc_regexp Constants */
#define AFC_REGEXP_BUFFER 8192 /* Buffer length for substitution */

	/* Errors for afc_regexp */
	enum
	{
		AFC_REGEXP_ERR_COMPILING = AFC_REGEXP_BASE + 1, /* regcomp() failed                        */
		AFC_REGEXP_ERR_NOT_READY,						/* missing the set_expression() step       */
		AFC_REGEXP_ERR_NO_STORAGE,						/* missing the set_storage_size() step     */
		AFC_REGEXP_ERR_OUT_OF_BOUNDS,					/* missing the set_storage_size() step     */
		AFC_REGEXP_ERR_NO_MATCH							/* Last RE didn't match the string         */
	};

#define AFC_REGEXP_OPT_NOCASE PCRE_CASELESS			  // No case search
#define AFC_REGEXP_OPT_DOLLAR_END PCRE_DOLLAR_ENDONLY // '$' only matches the end of line
#define AFC_REGEXP_OPT_DOT_NEWLINE PCRE_DOTALL		  // '.' matches also newlines
#define AFC_REGEXP_OPT_EXTENDED PCRE_EXTENDED		  // Ignores all whitespaces and comments
#define AFC_REGEXP_OPT_MULTILINE PCRE_MULTILINE		  // Works on multi lines

#define AFC_REGEXP_MATCH_OPT_NOT_BOL PCRE_NOTBOL	// The '^' should not match begin of line
#define AFC_REGEXP_MATCH_OPT_NOT_EOL PCRE_NOTEOL	// The '$' should not match end of line
#define AFC_REGEXP_MATCH_OPT_NOT_EMPTY PCRE_NOEMPTY // Empty strings are no valid matches

#define AFC_REGEXP_MAX_OSIZE 99 // Max OVector Size (must be multiple of 3)

	struct afc_regexp
	{
		unsigned long magic;

		pcre *pattern;
		pcre_extra *hints;

		int options; // Options as set with "set_options"

		int matches; // Number of subpatterns matched

		int *ovector; /* Matching addresses of RegExp in string */
		int replaces; /* Number of replaces in the last replace() call */

		short ready;  /* Flag T/F to tell if the RegEx has been compiled or not */
		char *str;	  /* afc_string_new parsed */
		char *buffer; /* Destination buffer */
	};

	struct afc_regexp_pos
	{
		int start;
		int end;
	};

	typedef struct afc_regexp RegExp;
	typedef struct afc_regexp_pos RegExpPos;

#define afc_regexp_delete(re)   \
	if (re)                     \
	{                           \
		_afc_regexp_delete(re); \
		re = NULL;              \
	}

	struct afc_regexp *afc_regexp_new(void);
	int _afc_regexp_delete(RegExp *);
	int afc_regexp_clear(RegExp *);

	int afc_regexp_compile(RegExp *regexp, const char *str);
	int afc_regexp_match(RegExp *regexp, const char *str, const int startpos);
	int afc_regexp_set_options(RegExp *regexp, const int options);

	int afc_regexp_set_buffer(RegExp *, int);
	int afc_regexp_replace(RegExp *, char *, char *, char *, char *, short);
	int afc_regexp_compute_replace_size(RegExp *regexp, char *str, char *str_re, char *replace, short replace_all);
	int afc_regexp_get_sub_string(RegExp *regexp, char *dest, int pos);
	int afc_regexp_get_pos(RegExp *regexp, int pos, struct afc_regexp_pos *retval);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
