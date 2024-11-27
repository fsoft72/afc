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
#ifndef AFC_CMD_PARSER_H
#define AFC_CMD_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "base.h"
#include "list.h"
#include "dictionary.h"
#include "dynamic_class.h"
#include "dynamic_class_master.h"
#include "stringnode.h"
#include "hash_master.h"
#include "readargs.h"

/*****************************
		 GUI PARSER
******************************/

/* AFC afc_cmd_parser Magic Number */
#define AFC_CMD_PARSER_MAGIC ('C' << 24 | 'M' << 16 | 'D' << 8 | 'P')

/* AFC afc_cmd_parser Base value for constants */
#define AFC_CMD_PARSER_BASE 0xf000

/* Errors for afc_cmd_parser */
enum
{
	AFC_CMD_PARSER_ERR_NO_SCRIPT = AFC_CMD_PARSER_BASE + 1, /* No string has been passed, so there's nothing to do */
	AFC_CMD_PARSER_ERR_CMD_UNKNOWN,							/* command unknown = no callback for that command */
	AFC_CMD_PARSER_ERR_UNMATCHED_OPEN_BRACKET,				/* Unmatched open bracket in script */
	AFC_CMD_PARSER_ERR_UNMATCHED_CLOSE_BRACKET,				/* Unmatched close bracket in script */
	AFC_CMD_PARSER_ERR_TAG_UNKNOWN							/* Tag unknown */
};

/* type of token */
enum
{
	AFC_CMD_PARSER_TOKEN_OPEN = 1,
	AFC_CMD_PARSER_TOKEN_CLOSE
};

/* max length of token name */
#define AFC_CMD_PARSER_TOKEN_MAX_NAMELEN 50
#define AFC_CMD_PARSER_MAX_BUFFER 256

// values for afc_cmd_parser_internal_set_skip
#define AFC_CMD_PARSER_SKIP_FIRST 1
#define AFC_CMD_PARSER_SKIP_SECOND 2

// CMD Parser Tags
#define AFC_CMD_PARSER_TAG_DEFAULT_TEMPLATE AFC_CMD_PARSER_BASE + 1

/* formal parameters: userdata */
typedef int (*CommandParserCallbackStartFunction)(void *);
typedef int (*CommandParserCallbackEndFunction)(void *);

/* forward definitions */
struct afc_cmd_parser;
typedef struct afc_cmd_parser CommandParser;

typedef int (*CommandParserBuiltinFunction)(CommandParser *, char *, List *);
typedef int (*CommandParserFunction)(CommandParser *, List *);

struct afc_cmd_parser_callback
{
	char *name;
	CommandParserCallbackStartFunction start_fun;
	CommandParserCallbackEndFunction end_fun;
	char *args_template;
};

typedef struct afc_cmd_parser_callback CommandParserCallback;

struct afc_cmd_parser_token
{
	char *name;
	int type;
};

struct afc_cmd_parser
{
	unsigned long magic;
	List *callbacks;
	Dictionary *classes;
	void *userdata;
	StringNode *stack;
	struct afc_cmd_parser_token *token;
	ReadArgs *rdargs;
	DynamicClassMaster *dynmast;
	char *buffer;			// Generic buffer AFC_CMD_PARSER_MAX_BUFFER long
	char *default_template; // Default template
	Dictionary *builtins;
	Dictionary *functions;
	int skip_block;
	int stack_depth;
};

/************************
 CommandParser functions
*************************/
#define afc_cmd_parser_delete(cmd)   \
	if (cmd)                         \
	{                                \
		_afc_cmd_parser_delete(cmd); \
		cmd = NULL;                  \
	}

CommandParser *afc_cmd_parser_new(void);
int afc_cmd_parser_clear(CommandParser *);
int _afc_cmd_parser_delete(CommandParser *);
int afc_cmd_parser_add_callback(CommandParser *, const char *, void *, void *, const char *);
int afc_cmd_parser_parse_string(CommandParser *, const char *, void *);
void *afc_cmd_parser_arg_get_by_name(CommandParser *, char *);
void *afc_cmd_parser_arg_get_by_pos(CommandParser *, int);
int afc_cmd_parser_add_commands(CommandParser *, DynamicClassMaster *);

#define afc_cmd_parser_set_tags(cmd, first, ...) _afc_cmd_parser_set_tags(cmd, first, ##__VA_ARGS__, AFC_TAG_END)
int _afc_cmd_parser_set_tags(CommandParser *, int, ...);
int afc_cmd_parser_set_tag(CommandParser *, int, void *);
int afc_cmd_parser_function_set(CommandParser *, char *, CommandParserFunction);
int afc_cmd_parser_function_del(CommandParser *, char *);
#endif
