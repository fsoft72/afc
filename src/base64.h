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
/*  AFCTools.h  $ 19/05/97 FR MT $  */

#ifndef AFC_BASE64_H
#define AFC_BASE64_H

#include <malloc.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "base.h"

#ifndef MINGW
#include <fnmatch.h>
#endif

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>

#ifdef MINGW
#include <fcntl.h>
#endif

enum
{
	AFC_BASE64_ERR_OUT_OF_MEM,
	AFC_BASE64_ERR_FILE_INPUT,
	AFC_BASE64_ERR_FILE_OUTPUT,
	AFC_BASE64_ERR_EOF,
	AFC_BASE64_OUT,
	AFC_BASE64_IN,

	AFC_BASE64_TAG_MEM_IN,
	AFC_BASE64_TAG_MEM_IN_SIZE,
	AFC_BASE64_TAG_MEM_OUT,
	AFC_BASE64_TAG_MEM_OUT_SIZE,
	AFC_BASE64_TAG_FILE_IN,
	AFC_BASE64_TAG_FILE_OUT
};

struct afc_base64
{
	unsigned long magic;

	unsigned int iocp;
	BOOL error_check;

	char *file_in;
	char *file_out;

	FILE *fin;
	FILE *fout;

	char *mem_in;
	char *mem_in_pos;
	unsigned int mem_in_size;

	char *mem_out;
	char *mem_out_pos;
	unsigned int mem_out_size;

	char io_buffer[4096];
	unsigned int size;

	char dtable[256];

	BOOL at_eof;

	int line_len;
};

#define LINELEN 256

typedef struct afc_base64 Base64;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	/* Function Prototypes */

	Base64 *afc_base64_new(void);
	int afc_base64_delete(Base64 *b64);
	int afc_base64_encode(Base64 *b64, int first_tag, ...);
	int afc_base64_decode(Base64 *b64, int first_tag, ...);
	int afc_base64_set_tag(Base64 *b64, int tag, void *val);
	int afc_base64_fwrite(Base64 *b64, const char *fname, int what);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
