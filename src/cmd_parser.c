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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "cmd_parser.h"

static const char class_name[] = "CommandParser";

static struct afc_cmd_parser_token *afc_cmd_parser_internal_token_new(void);
static int afc_cmd_parser_internal_token_delete(struct afc_cmd_parser_token *token);
static int afc_cmd_parser_internal_token_clear(struct afc_cmd_parser_token *token);
static int afc_cmd_parser_internal_token_set_name(struct afc_cmd_parser_token *token, char *name);
static int afc_cmd_parser_internal_del_callback(CommandParser *cmdparser, CommandParserCallback *cb);
static int afc_cmd_parser_internal_get_next_token(CommandParser *cmdparser, char **script);
static int afc_cmd_parser_internal_get_token_args(CommandParser *cmdparser, char **script, char *template);
static int afc_cmd_parser_internal_string_purge(CommandParser *cmdparser, char *s);
static int afc_cmd_parser_internal_set_skip(CommandParser *cmdparser, int howmany);
static int afc_cmd_parser_internal_goto_block_end(CommandParser *cmdparser, char **script);
static int afc_cmd_parser_internal_keyword_if(CommandParser *cmdparser, char *script, List *args);
static int afc_cmd_parser_internal_function_expr(CommandParser *cmdparser, List *args);
static int afc_cmd_parser_internal_add_builtins(CommandParser *cmdparser);

/*
@node intro

The CommandParser is a class used to execute scripts than can contain whatever command the user defines. The
scripts have a common structure, but there's no predefined instruction that the parser recognizes: the user defines
the instructions as plugins and then adds them to the parser's internal list before "running" a script through the
parser.

@endnode
*/

/*
@node afc_cmd_parser_new

			 NAME: afc_cmd_parser_new ()    - Initializes a new afc_cmd_parser instance.

		 SYNOPSIS: Easygui * afc_cmd_parser_new ( void )

	  DESCRIPTION: This function initializes a new CommandParser instance.

			INPUT: - NONE

		  RESULTS: a valid inizialized CommandParser structure. NULL in case of errors.

		 SEE ALSO: - afc_cmd_parser_delete()
				 - afc_cmd_parser_clear()
@endnode
*/
CommandParser *afc_cmd_parser_new(void)
{
	CommandParser *cmdparser = (CommandParser *)afc_malloc(sizeof(CommandParser));

	if (cmdparser == NULL)
	{
		AFC_LOG_FAST(AFC_ERR_NO_MEMORY);
		return (NULL);
	}

	cmdparser->magic = AFC_CMD_PARSER_MAGIC;

	if ((cmdparser->callbacks = afc_list_new()) == NULL)
	{
		AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "callbacks");
		afc_cmd_parser_delete(cmdparser);
		return (NULL);
	}

	if ((cmdparser->classes = afc_dictionary_new()) == NULL)
	{
		AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "classes");
		afc_cmd_parser_delete(cmdparser);
		return (NULL);
	}

	if ((cmdparser->stack = afc_stringnode_new()) == NULL)
	{
		AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "stack");
		afc_cmd_parser_delete(cmdparser);
		return (NULL);
	}

	if ((cmdparser->token = afc_cmd_parser_internal_token_new()) == NULL)
	{
		AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "token");
		afc_cmd_parser_delete(cmdparser);
		return (NULL);
	}

	if ((cmdparser->rdargs = afc_readargs_new()) == NULL)
	{
		AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "readargs");
		afc_cmd_parser_delete(cmdparser);
		return (NULL);
	}

	if ((cmdparser->buffer = afc_string_new(AFC_CMD_PARSER_MAX_BUFFER)) == NULL)
	{
		AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "buffer");
		afc_cmd_parser_delete(cmdparser);
		return (NULL);
	}

	if ((cmdparser->functions = afc_dictionary_new()) == NULL)
	{
		AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "functions");
		afc_cmd_parser_delete(cmdparser);
		return (NULL);
	}

	if ((cmdparser->builtins = afc_dictionary_new()) == NULL)
	{
		AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "builtins");
		afc_cmd_parser_delete(cmdparser);
		return (NULL);
	}

	// add some builtin commands and functions
	afc_cmd_parser_internal_add_builtins(cmdparser);

	return (cmdparser);
}

/*
@node afc_cmd_parser_delete

			 NAME: afc_cmd_parser_delete ( cmdparser )  - Disposes a valid afc_cmd_parser instance.

		 SYNOPSIS: int afc_cmd_parser_delete ( CommandParser * cmdparser )

	  DESCRIPTION: This function frees an already alloc'd CommandParser structure.

		   INPUT: - cmdparser  - Pointer to a valid CommandParser class.

		  RESULTS: should be AFC_ERR_NO_ERROR

			NOTES: - this method calls: afc_cmd_parser_clear()

		 SEE ALSO: - afc_cmd_parser_new()
				   - afc_cmd_parser_clear()
@endnode
*/
int _afc_cmd_parser_delete(CommandParser *cmdparser)
{
	int afc_res;

	if ((afc_res = afc_cmd_parser_clear(cmdparser)) != AFC_ERR_NO_ERROR)
		return (afc_res);

	/* NOTE: any class contained in afc_cmd_parser should be deleted here */

	if (cmdparser->callbacks)
		afc_list_delete(cmdparser->callbacks);
	if (cmdparser->classes)
		afc_dictionary_delete(cmdparser->classes);
	if (cmdparser->stack)
		afc_stringnode_delete(cmdparser->stack);
	if (cmdparser->token)
		afc_cmd_parser_internal_token_delete(cmdparser->token);
	if (cmdparser->rdargs)
		afc_readargs_delete(cmdparser->rdargs);
	if (cmdparser->buffer)
		afc_string_delete(cmdparser->buffer);
	if (cmdparser->default_template)
		afc_string_delete(cmdparser->default_template);
	if (cmdparser->builtins)
		afc_dictionary_delete(cmdparser->builtins);
	if (cmdparser->functions)
		afc_dictionary_delete(cmdparser->functions);

	afc_free(cmdparser);

	return (AFC_ERR_NO_ERROR);
}

/*
@node afc_cmd_parser_clear

			 NAME: afc_cmd_parser_clear ( cmdparser )  - Clears all stored data

		 SYNOPSIS: int afc_cmd_parser_clear ( CommandParser * cmdparser )

	  DESCRIPTION: Use this function to clear all stored data in the current CommandParser instance.

			INPUT: - cmdparser    - Pointer to a valid CommandParser instance.

		  RESULTS: should be AFC_ERR_NO_ERROR

		 SEE ALSO: - afc_cmd_parser_delete()

@endnode
*/
int afc_cmd_parser_clear(CommandParser *cmdparser)
{
	CommandParserCallback *cb = NULL;
	DynamicClass *dyn = NULL;

	if (cmdparser == NULL)
		return (AFC_ERR_NULL_POINTER);

	if (cmdparser->magic != AFC_CMD_PARSER_MAGIC)
		return (AFC_ERR_INVALID_POINTER);

	/* Custom Clean-up code should go here */

	if (cmdparser->callbacks)
	{
		cb = (CommandParserCallback *)afc_list_first(cmdparser->callbacks);
		while (cb != NULL)
		{
			afc_cmd_parser_internal_del_callback(cmdparser, cb);
			cb = afc_list_next(cmdparser->callbacks);
		}
		afc_list_clear(cmdparser->callbacks);
	}

	if (cmdparser->classes)
	{
		dyn = afc_dictionary_first(cmdparser->classes);
		while (dyn != NULL)
		{
			afc_dynamic_class_master_delete_instance(cmdparser->dynmast, dyn);
			dyn = afc_dictionary_next(cmdparser->classes);
		}
		afc_dictionary_clear(cmdparser->classes);
	}

	if (cmdparser->stack)
		afc_stringnode_clear(cmdparser->stack);
	if (cmdparser->token)
		afc_cmd_parser_internal_token_clear(cmdparser->token);
	if (cmdparser->rdargs)
		afc_readargs_clear(cmdparser->rdargs);

	if (cmdparser->default_template)
		afc_string_clear(cmdparser->default_template);

	if (cmdparser->functions)
		afc_dictionary_clear(cmdparser->functions);

	return (AFC_ERR_NO_ERROR);
}

/*
@node afc_cmd_parser_add_callback

	   NAME: afc_cmd_parser_add_callback ( cmdparser, name, startCBack, endCBack, template ) - Adds a new Callback

   SYNOPSIS: int afc_cmd_parser_add_callback ( CommandParser * cmdparser, const char * name, void ( * startCBack ), void ( * endCBack ), const char * template )

DESCRIPTION: adds a callback to the CommandParser callback list. Before a token is recognized by the parser you
			 have to add it to the parser's internal list specifying the two functions to call when the token
			 is encountered in the parsed string.

			 The use of this function is DEPRECATED: you should use the afc_cmd_parser_add_commands() function to add
			 all the commands the parser should recognize.

	  INPUT: - cmdparser - pointer to a valid CommandParser instance;
			 - name      - name of a token to be added;
			 - startCBack - pointer to the function to be called when the opening bracket of the specified token is
							found;
			 - endCBack   - pointer to the function to be called when the closing bracket of the specified token is
							found;
			 - template   - string describing how to parse the token's parameters.

	RESULTS: - AFC_ERR_NO_MEMORY if the callback couldn't be allocated;
			 - AFC_ERR_NO_ERROR otherwise.

   SEE ALSO: - afc_cmd_parser_add_commands()

@endnode
*/
int afc_cmd_parser_add_callback(CommandParser *cmdparser, const char *name, void(*scBack), void(*ecBack), const char *templ)
{
	CommandParserCallback *cb = NULL;

	if (cmdparser == NULL)
		return (AFC_ERR_NULL_POINTER);
	if (scBack == NULL)
		return (AFC_ERR_NULL_POINTER);
	if (name == NULL)
		return (AFC_ERR_NULL_POINTER);

	cb = (CommandParserCallback *)afc_malloc(sizeof(CommandParserCallback));
	if (cb == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NO_MEMORY));

	if ((cb->name = afc_string_dup((char *)name)) == NULL)
	{
		afc_cmd_parser_internal_del_callback(cmdparser, cb);
		return (AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "callback name"));
	}

	cb->start_fun = scBack;
	cb->end_fun = ecBack;

	if (cmdparser->default_template)
		afc_string_make(cmdparser->buffer, "%s %s", cmdparser->default_template, templ);
	else
		afc_string_copy(cmdparser->buffer, templ, ALL);

	if (afc_string_len(cmdparser->buffer))
	{
		if ((cb->args_template = afc_string_dup((char *)cmdparser->buffer)) == NULL)
		{
			afc_cmd_parser_internal_del_callback(cmdparser, cb);
			return (AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "callback template"));
		}
	}

	afc_list_add(cmdparser->callbacks, cb, AFC_LIST_ADD_TAIL);

	return (AFC_ERR_NO_ERROR);
}

/*
@node afc_cmd_parser_parse_string

	   NAME: afc_cmd_parser_parse_string ( cmdparser, script, userdata ) - Parse a string of CmdParser commands

   SYNOPSIS: int afc_cmd_parser_parse_string ( CommandParser * cmdparser, const char * script, void * userdata )

DESCRIPTION: this function is used to start the parsing of a string containing commands previously added to the
			 parser using the afc_cmd_parser_add_commands() function.
			 This function gets the string to be parsed and, in the /userdata/ parameter, a custom structure to
			 be passed to all callback functions. In fact, you are supposed to allocate a structure in which
			 to put all the data you need to pass to every callback function: then, you specify such a structure
			 when calling the afc_cmd_parser_parse_string() function.

	  INPUT: - cmdparser - a valid CommandParser instance;
			 - script    - the script to be parsed;
			 - userdata  - a custom structure (can be almost anything) that the parser should pass as the first parameter
						   to every callback function.

	RESULTS: - AFC_CMD_PARSER_ERR_NO_SCRIPT if the script is empty;
			 - AFC_CMD_PARSER_ERR_CMD_UNKNOWN if the parser encounters an unknown command in the script;
			 - AFC_CMD_PARSER_ERR_UNMATCHED_OPEN_BRACKET if an open bracket has no corresponding closed bracket;
			 - AFC_CMD_PARSER_ERR_UNMATCHED_CLOSE_BRACKET if a closed bracket has no corresponding open bracket;
			 - AFC_ERR_NO_ERROR if all went ok.

@endnode
*/
int afc_cmd_parser_parse_string(CommandParser *cmdparser, const char *script, void *userdata)
{
	char *myscript = NULL;
	char *myscript2 = NULL;
	CommandParserCallback *cb = NULL;
	char found = false;
	int quit = AFC_ERR_NO_ERROR;
	DynamicClass *dyn = NULL;
	CommandParserBuiltinFunction func = NULL;

	if (cmdparser == NULL)
		return (AFC_ERR_NULL_POINTER);

	if ((script == NULL) || (afc_string_len((char *)script) == 0))
		return (AFC_LOG(AFC_LOG_ERROR, AFC_CMD_PARSER_ERR_NO_SCRIPT, "No script defined", NULL));

	/* the string containing the script is duplicated to preserve the orignal one */
	if ((myscript = afc_string_dup((char *)script)) == NULL)
		return (AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "script"));

	afc_cmd_parser_internal_string_purge(cmdparser, myscript);

	/* we use a copy of the string pointer to preserve the original pointer (needed to free the string) */
	myscript2 = myscript;

	cmdparser->userdata = userdata;

	while ((strcmp(myscript2, "") != 0) && (quit == AFC_ERR_NO_ERROR))
	{
		if ((quit = afc_cmd_parser_internal_get_next_token(cmdparser, &myscript2)) == AFC_ERR_NO_ERROR)
		{

			if (cmdparser->token->type == AFC_CMD_PARSER_TOKEN_OPEN)
			{
				cb = (CommandParserCallback *)afc_list_first(cmdparser->callbacks);
				found = false;

				/* check if the command is an internal one, and call it */
				if ((func = afc_dictionary_get(cmdparser->builtins, cmdparser->token->name)) != NULL)
				{
					found = true;
					afc_stringnode_add(cmdparser->stack, cmdparser->token->name, AFC_STRINGNODE_ADD_TAIL);

					if ((afc_cmd_parser_internal_get_token_args(cmdparser, &myscript2, "ARGS/M")) == AFC_ERR_NO_ERROR)
						quit = func(cmdparser, myscript2, (List *)afc_cmd_parser_arg_get_by_name(cmdparser, "ARGS"));
				}

				while ((cb != NULL) && (found == false))
				{

					if (strcmp(cb->name, cmdparser->token->name) == 0)
					{
						dyn = afc_dictionary_get(cmdparser->classes, cb->name);

						if ((cb->args_template != NULL) && (strcmp(cb->args_template, "") != 0))
							quit = afc_cmd_parser_internal_get_token_args(cmdparser, &myscript2, cb->args_template);

						afc_stringnode_add(cmdparser->stack, cmdparser->token->name, AFC_STRINGNODE_ADD_TAIL);

						if (quit == AFC_ERR_NO_ERROR)
						{
							if (cb->start_fun != NULL)
								quit = afc_dynamic_class_execute(dyn, "open_callback", cmdparser->userdata, AFC_DYNAMIC_CLASS_ARG_END);
						}

						found = true;
					}
					cb = (CommandParserCallback *)afc_list_next(cmdparser->callbacks);
				} /* while  cb != NULL */

				if (found == false)
				{
					AFC_LOG(AFC_LOG_ERROR, AFC_CMD_PARSER_ERR_CMD_UNKNOWN, "Command unknown", cmdparser->token->name);
					quit = AFC_CMD_PARSER_ERR_CMD_UNKNOWN;
				}

			} /* if TOKEN_OPEN */
			else
			{
				cb = (CommandParserCallback *)afc_list_first(cmdparser->callbacks);
				found = false;
				while ((cb != NULL) && (found == false))
				{
					if (strcmp(cb->name, cmdparser->token->name) == 0)
					{
						dyn = afc_dictionary_get(cmdparser->classes, cb->name);

						if (cb->end_fun != NULL)
							quit = afc_dynamic_class_execute(dyn, "close_callback", cmdparser->userdata, AFC_DYNAMIC_CLASS_ARG_END);
						found = true;
					}
					cb = (CommandParserCallback *)afc_list_next(cmdparser->callbacks);
				} /* while cb != NULL */
			} /* else TOKEN_CLOSE */

		} /* if get_next_token */
	} /* while myscript != "" */

	// stack must be empty, otherwise some open bracket is unmatched (= no corresponding closed bracket)
	if (afc_stringnode_len(cmdparser->stack) > 0)
	{
		quit = AFC_CMD_PARSER_ERR_UNMATCHED_OPEN_BRACKET;
		AFC_LOG(AFC_LOG_ERROR, AFC_CMD_PARSER_ERR_UNMATCHED_OPEN_BRACKET, "Unmatched open bracket", NULL);
	}

	if (myscript)
		afc_string_delete(myscript);

	return (quit);
}

/*
@node afc_cmd_parser_arg_get_by_name

	   NAME: afc_cmd_parser_arg_get_by_name ( cmdparser, name ) - Returns a parameter's value by name

   SYNOPSIS: void * afc_cmd_parser_arg_get_by_name ( CommandParser * cmdparser, char * name )

DESCRIPTION: after the parameters of a command have been parsed, this function returns the value of
			 the parameter with name /name/.

	  INPUT: - cmdparser - pointer to a valid CommandParser instance;
			 - name      - name of the parameter whose value is needed.

	RESULTS: the value of the chosen parameter, otherwise NULL if the parameter has not been found.

   SEE ALSO: - afc_cmd_parser_arg_get_by_pos()

@endnode
*/
void *afc_cmd_parser_arg_get_by_name(CommandParser *cmdparser, char *name)
{
	if (cmdparser == NULL)
		return (NULL);
	if (name == NULL)
		return (NULL);

	return afc_readargs_get_by_name(cmdparser->rdargs, name);
}

/*
@node afc_cmd_parser_arg_get_by_pos

	   NAME: afc_cmd_parser_arg_get_by_pos ( cmdparser, pos ) - Gets a parameter's value by pos

   SYNOPSIS: void * afc_cmd_parser_arg_get_by_pos ( CommandParser * cmdparser, int pos )

DESCRIPTION: after the parameters of a command have been parsed, this function returns the value of
			 the parameter appearing at position /pos/.

	  INPUT: - cmdparser - pointer to a valid CommandParser instance;
			 - pos       - position of the parameter whose value is needed.

	RESULTS: the value of the chosen parameter, otherwise NULL if the parameter has not been found.

   SEE ALSO: afc_cmd_parser_arg_get_by_name()

@endnode
*/
void *afc_cmd_parser_arg_get_by_pos(CommandParser *cmdparser, int pos)
{
	if (cmdparser == NULL)
		return (NULL);

	return afc_readargs_get_by_pos(cmdparser->rdargs, pos);
}

/*
@node afc_cmd_parser_add_commands

	   NAME: afc_cmd_parser_add_commands ( cmdparser, dynmast ) - Adds new tokens using a Dynamic Class Master

   SYNOPSIS: int afc_cmd_parser_add_commands ( CommandParser * cmdparser, DynamicClassMaster * dynmast )

DESCRIPTION: this functions is used to add to the parser all the commands (tokens) it should recognize
			 while parsing a script.
			 The commands are structured as "plugins", or "dynamic classes": the /dynmast/ parameter
			 is a pointer to a DynamicClassMaster object holding all the plugins. Thus, the
			 afc_cmd_parser_add_commands() function cycles through the plugins therein and queries them
			 for some standard methods they must (or may) have; if all is ok then the parser stores the name
			 of the plugin as the name of a command.

			 The methods a plugin may define are recognized by name and are:
			 - get_template - this is the only one the plugin *must* define: it returns the string
							  describing how the command parameters are to be parsed;
			 - open_callback - the function to be called when the opening bracket of the command is
							   found;
			 - close_callback - the function to be called when the closing bracket of the command
								is found.

			 If one of /open_callback/ and /close_callback/ is not defined then the parser does nothing
			 when the opening or closing bracket of a token is found.
			 On the contrary, even if the token has no arguments to be parsed the /get_template/ function
			 must be defined: in such a case it should return the empty string.

			 The prototypes of the preceding functions are:
			 - int get_template ( DynamicClass * dyn )
			 - int open_callback ( DynamicClass * dyn )
			 - int close_callback ( DynamicClass * dyn )

			 The /get_template/ function must store the template's string in the /result/ field of the
			 DynamicClass /dyn/. The other callbacks are passed the pointer to the /userdata/ structure,
			 specified when calling the afc_cmd_parser_parse_string() function, through the first item of
			 the /dyn->args/ parameter list.

	 INPUT: - cmdparser - Pointer to a *valid* CommandParser instance
		 - dynmas    - Pointer to the Dynamic Class Master to get the methods from.

	RESULTS: - AFC_ERR_NO_MEMORY if there's no memory to acclomplish one of the tasks;
			 - AFC_DYNAMIC_CLASS_ERR_METHOD_NOT_FOUND if the /get_template/ method has not been found;
			 - AFC_ERR_NO_ERROR if all is ok.

   SEE ALSO: - afc_cmd_parser_parse_string()

@endnode
*/
int afc_cmd_parser_add_commands(CommandParser *cmdparser, DynamicClassMaster *dynmast)
{
	DynamicClass *dyn = NULL;
	DynamicClassMethod start_cb = NULL;
	DynamicClassMethod end_cb = NULL;
	int afc_res = AFC_ERR_NO_ERROR;
	void *foo = NULL;
	char *class_name = NULL;

	if ((cmdparser == NULL) || (dynmast == NULL))
		return AFC_LOG_FAST(AFC_ERR_NULL_POINTER);

	/* store the pointer to the dynamic class master */
	cmdparser->dynmast = dynmast;

	foo = afc_dictionary_first(dynmast->classes);
	while ((foo != NULL) && (afc_res == AFC_ERR_NO_ERROR))
	{
		class_name = (char *)afc_dictionary_get_key(dynmast->classes);

		if ((dyn = afc_dynamic_class_master_new_instance(dynmast, class_name)) != NULL)
		{
			afc_dictionary_set(cmdparser->classes, class_name, dyn);

			start_cb = (DynamicClassMethod)afc_dynamic_class_find_method(dyn, "open_callback");
			end_cb = (DynamicClassMethod)afc_dynamic_class_find_method(dyn, "close_callback");

			if ((afc_res = afc_dynamic_class_execute(dyn, "get_template", AFC_DYNAMIC_CLASS_ARG_END)) == AFC_ERR_NO_ERROR)
			{
				afc_res = afc_cmd_parser_add_callback(cmdparser, class_name, start_cb, end_cb, (char *)dyn->result);
			}
			else
				AFC_LOG_FAST_INFO(afc_res, "plugin method get_template not found");
		}
		else
		{
			AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "new plugin instance");
			break;
		}

		foo = afc_dictionary_next(dynmast->classes);
	} /* while */

	return (afc_res);
}

/*
@node afc_cmd_parser_set_tags

	   NAME: afc_cmd_parser_set_tags ( cmdparser, first_tag, ... ) - Set tags

   SYNOPSIS: int afc_cmd_parser_set_tags ( CommandParser * cmdparser, int first_tag, ... )

DESCRIPTION: this method is used to set some attributes of the CommandParser object. Each
			 attribute to be set is passed as a tag name, tag value pair; the list *must* be ended with
			 the AFC_TAG_END tag.

			 For the list of valid tags see the afc_cmd_parser_set_tag() function.

	 INPUT: - cmdparser - pointer to a valid CommandParser instance;
			 - first_tag, ... - list of tag name, tag value pairs.

	RESULTS: - AFC_ERR_NO_ERROR

   SEE ALSO: afc_cmd_parser_set_tag()

@endnode
*/
int _afc_cmd_parser_set_tags(CommandParser *cmdparser, int first_tag, ...)
{
	void *val;
	va_list args;
	unsigned int tag;

	tag = first_tag;

	va_start(args, first_tag);

	while (tag != AFC_TAG_END)
	{
		val = va_arg(args, void *);

		afc_cmd_parser_set_tag(cmdparser, tag, val);

		tag = va_arg(args, int);
	}

	va_end(args);

	return (AFC_ERR_NO_ERROR);
}

/*
@node afc_cmd_parser_set_tag

	   NAME: afc_cmd_parser_set_tag ( cmdparser, tag, val ) - Set a single tag

   SYNOPSIS: int afc_cmd_parser_set_tag ( CommandParser * cmdparser, int tag, void * val )

DESCRIPTION: sets the value of a CommandParser attribute. The attribute to be modified is
			 specified with a tag, while its new value goes in the /val/ parameter.

			 Valid tags are:
			   - AFC_CMD_PARSER_TAG_DEFAULT_TEMPLATE - its value is a string specifying the template
				 parameters to be appended to the head of the template of each command (thus, they are
				 parsed before any other parameter of a command).

	 INPUT: - cmdparser - a valid CommandParser instance;
			 - tag - tag identifying the attribute to be modified;
			 - val - value of the attribute.

	RESULTS: - AFC_CMD_PARSER_ERR_TAG_UNKNOWN if the tag specified is wrong;
			 - AFC_ERR_NO_ERROR otherwise.

   SEE ALSO: - afc_cmd_parser_set_tags()

@endnode
*/
int afc_cmd_parser_set_tag(CommandParser *cmdparser, int tag, void *val)
{
	int afc_res = AFC_ERR_NO_ERROR;

	switch (tag)
	{
	case AFC_CMD_PARSER_TAG_DEFAULT_TEMPLATE:
		if (cmdparser->default_template)
			afc_string_delete(cmdparser->default_template);
		cmdparser->default_template = afc_string_dup((char *)val);
		break;

	default:
		afc_res = AFC_CMD_PARSER_ERR_TAG_UNKNOWN;
		break;
	}

	return (afc_res);
}

/*
@node afc_cmd_parser_function_set

	   NAME: afc_cmd_parser_function_set ( cmdparser, name, func ) - Sets a new function

   SYNOPSIS: int afc_cmd_parser_function_set ( CommandParser * cmdparser, char * name, CommandParserFunction func )

DESCRIPTION: adds a new function to the command parser. Functions are up to now used only by the
			 "if" internal keyword. For example, if you have added a function "foo", then in
			 your script you can use the function to be evaluated by an "if" statement:

			 (if foo arg1 arg2
			   (then block)
			   (else block)
			 )

			 A valid function should accept two parameters: a CommandParser instance and
			 a List instance containing the arguments passed to the function:

			 int foo ( CommandParser * cmdparser, List * args )

	 INPUT: - cmdparser - a valid CommandParser instance
			- name - name of the function
			- func - pointer to a CommandParserFunction

   RESULTS: AFC_ERR_NO_ERROR

  SEE ALSO: afc_cmd_parser_function_del()

@endnode
*/
int afc_cmd_parser_function_set(CommandParser *cmdparser, char *name, CommandParserFunction func)
{
	if ((cmdparser == NULL) || (name == NULL))
		return (AFC_ERR_NULL_POINTER);

	afc_dictionary_set(cmdparser->functions, name, func);

	return (AFC_ERR_NO_ERROR);
}

/*
@node afc_cmd_parser_function_del

	   NAME: afc_cmd_parser_function_del ( cmdparser, name ) - Deletes a function

   SYNOPSIS: int afc_cmd_parser_function_del ( CommandParser * cmdparser, char * name )

DESCRIPTION: removes a function from the CommandParser.

	  INPUT: - cmdparser - a valid CommandParser instance
			 - name - name of the function to be removed

	RESULTS: AFC_ERR_NO_ERROR

   SEE ALSO: afc_cmd_parser_function_set()

@endnode
*/
int afc_cmd_parser_function_del(CommandParser *cmdparser, char *name)
{
	if ((cmdparser == NULL) || (name == NULL))
		return (AFC_ERR_NULL_POINTER);

	if (afc_dictionary_get(cmdparser->functions, name) != NULL)
		afc_dictionary_del(cmdparser->functions);

	return (AFC_ERR_NO_ERROR);
}

/*********************************
 CommandParser internal functions
*********************************/

/*
NAME: afc_cmd_parser_internal_add_builtins

*/
static int afc_cmd_parser_internal_add_builtins(CommandParser *cmdparser)
{
	// builtin commands
	afc_dictionary_set(cmdparser->builtins, "if", afc_cmd_parser_internal_keyword_if);

	// builtin functions
	afc_dictionary_set(cmdparser->functions, "expr", afc_cmd_parser_internal_function_expr);

	return (AFC_ERR_NO_ERROR);
}

/*
NAME: afc_cmd_parser_internal_token_new

*/
static struct afc_cmd_parser_token *afc_cmd_parser_internal_token_new()
{
	struct afc_cmd_parser_token *token;

	token = (struct afc_cmd_parser_token *)afc_malloc(sizeof(struct afc_cmd_parser_token));
	if (token == NULL)
	{
		AFC_LOG_FAST(AFC_ERR_NO_MEMORY);
		return (NULL);
	}

	if ((token->name = afc_string_new(AFC_CMD_PARSER_TOKEN_MAX_NAMELEN)) == NULL)
	{
		AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "name");
		afc_cmd_parser_internal_token_delete(token);
		return (NULL);
	}

	return (token);
}

/*
NAME: afc_cmd_parser_internal_token_delete

*/
static int afc_cmd_parser_internal_token_delete(struct afc_cmd_parser_token *token)
{
	int afc_res;

	if ((afc_res = afc_cmd_parser_internal_token_clear(token)) != AFC_ERR_NO_ERROR)
		return (afc_res);

	if (token->name)
		afc_string_delete(token->name);
	afc_free(token);

	return (AFC_ERR_NO_ERROR);
}

/*
NAME: afc_cmd_parser_internal_token_clear

*/
static int afc_cmd_parser_internal_token_clear(struct afc_cmd_parser_token *token)
{
	if (token == NULL)
		return (AFC_ERR_NULL_POINTER);

	afc_string_clear(token->name);

	token->type = 0;

	return (AFC_ERR_NO_ERROR);
}

/*
NAME: afc_cmd_parser_internal_token_set_name

*/
static int afc_cmd_parser_internal_token_set_name(struct afc_cmd_parser_token *token, char *name)
{
	if (token == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));
	if (name == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));

	if (strlen(name) > afc_string_max(token->name))
	{
		afc_string_delete(token->name);
		token->name = afc_string_dup(name);
	}
	else
		afc_string_copy(token->name, name, ALL);

	return (AFC_ERR_NO_ERROR);
}

/*
NAME: afc_cmd_parser_internal_del_callback

*/
static int afc_cmd_parser_internal_del_callback(CommandParser *cmdparser, CommandParserCallback *cb)
{
	if (cb == NULL)
		return (AFC_ERR_NULL_POINTER);

	if (cb->name)
		afc_string_delete(cb->name);
	if (cb->args_template)
		afc_string_delete(cb->args_template);
	afc_free(cb);

	return (AFC_ERR_NO_ERROR);
}

/*
NAME: afc_cmd_parser_internal_get_next_token

*/
static int afc_cmd_parser_internal_get_next_token(CommandParser *cmdparser, char **script)
{
	char *data = NULL;
	char *bra = NULL;
	char *ket = NULL;
	char *space = NULL;
	char *token_end = NULL;
	char old_char = '\0';
	char *last_token = NULL;
	int afc_res = AFC_ERR_NO_ERROR;

	if (cmdparser == NULL)
		return (AFC_ERR_NULL_POINTER);

	/* if we have to skip the next block, then we jump to its end */
	if (cmdparser->skip_block == 1)
		afc_cmd_parser_internal_goto_block_end(cmdparser, script);

	afc_cmd_parser_internal_token_clear(cmdparser->token);

	/* search for the first open bracket */
	data = *script;
	bra = index(data, '(');
	ket = index(data, ')');

	if (ket != NULL)
	{
		if ((bra == NULL) || ((bra != NULL) && (ket < bra)))
		{
			if ((last_token = afc_stringnode_last(cmdparser->stack)) != NULL)
			{
				strcpy(cmdparser->token->name, last_token);
				afc_stringnode_del(cmdparser->stack);
				cmdparser->token->type = AFC_CMD_PARSER_TOKEN_CLOSE;

				/* if we have to skip some block, we know we have reached the end of a block if the current stack depth is
				   equal to stack_depth: then we decrement the value of the skip_block parameter */
				if ((cmdparser->skip_block > 0) && ((int)afc_stringnode_len(cmdparser->stack) == cmdparser->stack_depth))
				{
					cmdparser->skip_block = cmdparser->skip_block - 1;
				}
			}
			else
			{
				// the closed bracket has no corresponding open bracket
				afc_res = AFC_CMD_PARSER_ERR_UNMATCHED_CLOSE_BRACKET;
				AFC_LOG(AFC_LOG_ERROR, AFC_CMD_PARSER_ERR_UNMATCHED_CLOSE_BRACKET, "Unmatched close bracket", NULL);
			}

			*script = ket + 1;

			/* DEBUG */
#ifdef TEST_CLASS
			fprintf(stderr, "TOKEN_CLOSE:  <%s>\n", cmdparser->token->name);
#endif

			return (afc_res);
		}
	}

	if (bra != NULL)
	{
		bra++;
		space = index(bra, ' ');
		ket = index(bra, ')');

		token_end = ((ket != NULL) && (ket < space)) ? ket : space;

		if (token_end != NULL)
		{
			old_char = *token_end;
			*token_end = '\0';
			afc_cmd_parser_internal_token_set_name(cmdparser->token, bra);
			cmdparser->token->type = AFC_CMD_PARSER_TOKEN_OPEN;
			*token_end = old_char;
			*script = token_end;

			/* DEBUG */
#ifdef TEST_CLASS
			fprintf(stderr, "TOKEN_OPEN:   <%s>\n", cmdparser->token->name);
#endif
		}
		else
			*script = index(data, '\0');
	}

	if ((ket == NULL) && (bra == NULL))
		*script = index(data, '\0');

	return (AFC_ERR_NO_ERROR);
}

/*
NAME: afc_cmd_parser_internal_get_token_args

*/
static int afc_cmd_parser_internal_get_token_args(CommandParser *cmdparser, char **script, char *template)
{
	char *data = *script;
	char *start = *script;
	char *bra = NULL;
	char *ket = NULL;
	int afc_res = 0;
	char *the_end = NULL;
	char old_char = '\0';

	if (cmdparser == NULL)
		return (AFC_ERR_NULL_POINTER);
	if (template == NULL)
		return (AFC_ERR_NULL_POINTER);

	bra = index(start, '(');
	ket = index(start, ')');

	the_end = NULL;

	if (bra != NULL)
	{
		if ((ket != NULL) && (ket < bra))
			the_end = ket;
		else
			the_end = bra;
	}
	else
	{
		if (ket != NULL)
			the_end = ket;
		else
			the_end = index(data, '\0');
	}

	old_char = *the_end;
	*the_end = '\0';

	/* DEBUG */
#ifdef TEST_CLASS
	fprintf(stderr, "TOKEN_ARG(S): <%s>\n", data);
#endif

	if ((afc_res = afc_readargs_parse(cmdparser->rdargs, template, data)) != AFC_ERR_NO_ERROR)
		return (afc_res);

	*the_end = old_char;
	*script = the_end;

	return (AFC_ERR_NO_ERROR);
}

/*
NAME: afc_cmd_parser_internal_goto_block_end

*/
static int afc_cmd_parser_internal_goto_block_end(CommandParser *cmdparser, char **script)
{
	char *data = *script;
	char *bra = NULL;
	char *ket = NULL;
	int count = 0;

	bra = index(data, '(');
	ket = index(data, ')');

	if (bra != NULL)
	{
		// if a closing bracket comes first, we stop on it and exit
		if ((ket != NULL) && (ket < bra))
		{
			*script = ket;
			return (AFC_ERR_NO_ERROR);
		}

		// the block starts on the next open bracket
		data = bra;

		count = 1;
		while ((count > 0) && (*data != '\0'))
		{
			data++;
			if (*data == '(')
				count++;
			if (*data == ')')
				count--;
		}

		if (*data != '\0')
			data++;
		*script = data;
	}
	else if (ket != NULL)
		*script = ket;

	/* we reached the end of the block, so we decrement the skip_block counter */
	cmdparser->skip_block = cmdparser->skip_block - 1;

	return (AFC_ERR_NO_ERROR);
}

/*
NAME: afc_cmd_parser_internal_string_purge

*/
static int afc_cmd_parser_internal_string_purge(CommandParser *cmdparser, char *s)
{
	char *cr = NULL;

	if (s == NULL)
		return (AFC_ERR_NULL_POINTER);

	while ((cr = index(s, '\n')) != NULL)
		*cr = ' ';

	return (AFC_ERR_NO_ERROR);
}

/*
NAME: afc_cmd_parser_internal_set_skip

DESCRIPTION: sets two parameters used by get_next_token to skip some blocks of code.
The parameters have the following meanings:

- skip_block - number, from the current position in the script, of the block to be
			   skipped; e.g., 1 = skip first block, 2 = skip second block from here;
- stack_depth - current stack "depth": it used by get_next_token to know when a block
				ends. A block ends when the stack depth returns to the value
				stored in stack_depth.

*/
static int afc_cmd_parser_internal_set_skip(CommandParser *cmdparser, int howmany)
{
	if (cmdparser == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));

	switch (howmany)
	{
	case AFC_CMD_PARSER_SKIP_FIRST:
		cmdparser->skip_block = 1;
		break;

	case AFC_CMD_PARSER_SKIP_SECOND:
		cmdparser->skip_block = 2;
		break;
	}

	cmdparser->stack_depth = afc_stringnode_len(cmdparser->stack);

	return (AFC_ERR_NO_ERROR);
}

/*
NAME: afc_cmd_parser_internal_keyword_if

*/
static int afc_cmd_parser_internal_keyword_if(CommandParser *cmdparser, char *script, List *args)
{
	char *func_name = NULL;
	CommandParserFunction func = NULL;
	int res = 0;

	if ((cmdparser == NULL) || (args == NULL))
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));

	if ((func_name = (char *)afc_list_first(args)) != NULL)
	{
		afc_list_del(args);
		if ((func = afc_dictionary_get(cmdparser->functions, func_name)) != NULL)
			res = func(cmdparser, args);

		// FIXED: non liberavi func_name quando avevi gi� "scaricato" la stringa dal list
		afc_string_delete(func_name);
	}

	if (res == false)
		afc_cmd_parser_internal_set_skip(cmdparser, AFC_CMD_PARSER_SKIP_FIRST);
	else
		afc_cmd_parser_internal_set_skip(cmdparser, AFC_CMD_PARSER_SKIP_SECOND);

	return (AFC_ERR_NO_ERROR);
}

/*
NAME: afc_cmd_parser_internal_function_expr

*/
static int afc_cmd_parser_internal_function_expr(CommandParser *cmdparser, List *args)
{
	if (atoi((char *)afc_list_first(args)) > 0)
		return (true);
	else
		return (false);
}

/* ------------------------------------------------- */

#ifdef TEST_CLASS
struct somedata
{
	CommandParser *cmdp;
};

// window class
int window_cb_open(DynamicClass *dyn)
{
	struct somedata *mydata = (struct somedata *)afc_array_first(dyn->args);
	char *w = (char *)afc_cmd_parser_arg_get_by_name(mydata->cmdp, "W");
	char *h = (char *)afc_cmd_parser_arg_get_by_name(mydata->cmdp, "H");
	char *title = (char *)afc_cmd_parser_arg_get_by_name(mydata->cmdp, "TITLE");

	printf("window open - title: <%s>, width: <%s>, height: <%s>\n", title, w, h);

	return (AFC_ERR_NO_ERROR);
}

int window_cb_close(DynamicClass *dyn)
{
	printf("window close\n");

	return (AFC_ERR_NO_ERROR);
}

int window_get_template(DynamicClass *dyn)
{
	dyn->result = "TITLE W H";
	return (AFC_ERR_NO_ERROR);
}

DynamicClass *window_new_instance()
{
	DynamicClass *dyn = afc_dynamic_class_new();

	if (dyn == NULL)
		return NULL;

	afc_dynamic_class_add_method(dyn, "open_callback", NULL, window_cb_open);
	afc_dynamic_class_add_method(dyn, "close_callback", NULL, window_cb_close);
	afc_dynamic_class_add_method(dyn, "get_template", NULL, window_get_template);

	return (dyn);
}

int window_del_instance(DynamicClass *dyn)
{
	if (dyn != NULL)
		afc_dynamic_class_delete(dyn);

	return (AFC_ERR_NO_ERROR);
}

// button class
int button_cb_open(DynamicClass *dyn)
{
	struct somedata *mydata = (struct somedata *)afc_array_first(dyn->args);
	char *name = (char *)afc_cmd_parser_arg_get_by_name(mydata->cmdp, "NAME");
	char *label = (char *)afc_cmd_parser_arg_get_by_name(mydata->cmdp, "LABEL");
	char *callback = (char *)afc_cmd_parser_arg_get_by_name(mydata->cmdp, "CB");

	printf("button open - name: <%s>, label: <%s>, cb: <%s>\n", name, label, callback);

	return (AFC_ERR_NO_ERROR);
}

int button_cb_close(DynamicClass *dyn)
{
	printf("button close\n");

	return (AFC_ERR_NO_ERROR);
}

int button_get_template(DynamicClass *dyn)
{
	dyn->result = "NAME LABEL CB";
	return (AFC_ERR_NO_ERROR);
}

DynamicClass *button_new_instance()
{
	DynamicClass *dyn = afc_dynamic_class_new();

	if (dyn == NULL)
		return NULL;

	afc_dynamic_class_add_method(dyn, "open_callback", NULL, button_cb_open);
	afc_dynamic_class_add_method(dyn, "close_callback", NULL, button_cb_close);
	afc_dynamic_class_add_method(dyn, "get_template", NULL, button_get_template);

	return (dyn);
}

int button_del_instance(DynamicClass *dyn)
{
	if (dyn != NULL)
		afc_dynamic_class_delete(dyn);

	return (AFC_ERR_NO_ERROR);
}

// eqrows class
int eqrows_cb_open(DynamicClass *dyn)
{
	printf("eqrows open\n");

	return (AFC_ERR_NO_ERROR);
}

int eqrows_cb_close(DynamicClass *dyn)
{
	printf("eqrows close\n");

	return (AFC_ERR_NO_ERROR);
}

int eqrows_get_template(DynamicClass *dyn)
{
	dyn->result = "";
	return (AFC_ERR_NO_ERROR);
}

DynamicClass *eqrows_new_instance()
{
	DynamicClass *dyn = afc_dynamic_class_new();

	if (dyn == NULL)
		return NULL;

	afc_dynamic_class_add_method(dyn, "open_callback", NULL, eqrows_cb_open);
	afc_dynamic_class_add_method(dyn, "close_callback", NULL, eqrows_cb_close);
	afc_dynamic_class_add_method(dyn, "get_template", NULL, eqrows_get_template);

	return (dyn);
}

int eqrows_del_instance(DynamicClass *dyn)
{
	if (dyn != NULL)
		afc_dynamic_class_delete(dyn);

	return (AFC_ERR_NO_ERROR);
}

// separator class
int separator_get_template(DynamicClass *dyn)
{
	dyn->result = "NAME";
	return (AFC_ERR_NO_ERROR);
}

int separator_cb_open(DynamicClass *dyn)
{
	printf("separator!\n");

	return (AFC_ERR_NO_ERROR);
}

DynamicClass *separator_new_instance()
{
	DynamicClass *dyn = afc_dynamic_class_new();

	if (dyn == NULL)
		return NULL;

	afc_dynamic_class_add_method(dyn, "open_callback", NULL, separator_cb_open);
	// afc_dynamic_class_add_method ( dyn, "close_callback", NULL, separator_cb_close );
	afc_dynamic_class_add_method(dyn, "get_template", NULL, separator_get_template);

	return (dyn);
}

int separator_del_instance(DynamicClass *dyn)
{
	if (dyn != NULL)
		afc_dynamic_class_delete(dyn);

	return (AFC_ERR_NO_ERROR);
}

/*****************
  MAIN
******************/

int main()
{
	CommandParser *cmdp = afc_cmd_parser_new();
	DynamicClassMaster *dcm = afc_dynamic_class_master_new();
	AFC *afc = afc_new();
	char *stuff;
	int afc_res = AFC_ERR_NO_ERROR;
	struct somedata *mydata = NULL;

	mydata = (struct somedata *)afc_malloc(sizeof(struct somedata));
	mydata->cmdp = cmdp;

	stuff = afc_string_dup(
		"(window \"This is a title\" 640 480"
		"(button b hello! clicked)"
		"(eqrows"
		"(button c Wow NULL)"
		"(separator)"
		")"
		"(if expr 1"
		"(eqrows"
		"(button a foo NULL)"
		"(button d ddd NULL)"
		")"
		"(eqrows"
		"(separator)"
		"(separator)"
		")"
		")"
		")");

	afc_dynamic_class_master_add(dcm, "window", NULL, window_new_instance, window_del_instance, NULL);
	afc_dynamic_class_master_add(dcm, "button", NULL, button_new_instance, button_del_instance, NULL);
	afc_dynamic_class_master_add(dcm, "eqrows", NULL, eqrows_new_instance, eqrows_del_instance, NULL);
	afc_dynamic_class_master_add(dcm, "separator", NULL, separator_new_instance, separator_del_instance, NULL);

	if ((afc_res = afc_cmd_parser_add_commands(cmdp, dcm)) == AFC_ERR_NO_ERROR)
	{
		afc_res = afc_cmd_parser_parse_string(cmdp, stuff, mydata);
	}

	printf("res: %x\n", afc_res);

	/* The order is: first cmd_parser deletes the plugin' instances, then class_master deletes itself.
	   If class_master is deleted first, then it will dispose all the plugins still used by cmd_parser,
	 generating some errors. */
	afc_cmd_parser_delete(cmdp);
	afc_dynamic_class_master_delete(dcm);

	afc_free(mydata);
	afc_string_delete(stuff);

	afc_delete(afc);

	exit(0);
}

#endif
