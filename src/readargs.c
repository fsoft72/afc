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
@config
	TITLE:     ReadArgs
	VERSION:   1.01
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
@endnode
*/

#include "readargs.h"

static char afc_readargs_switches[] = "xAKNSM";
static const char class_name[] = "ReadArgs";

static int afc_readargs_internal_parse_template(ReadArgs *rdargs, const char *template);
static int afc_readargs_internal_add_template(ReadArgs *rdargs, char *tok);
static void afc_readargs_internal_parse_string(ReadArgs *rdargs, char *txt);
static void afc_readargs_internal_find_quotes(ReadArgs *rdargs, char *str);
static int afc_readargs_internal_fill_required(ReadArgs *rdargs);
static int afc_readargs_internal_fill_keyword(ReadArgs *rdarg);
static int afc_readargs_internal_fill_switch(ReadArgs *rdarg);
static int afc_readargs_internal_fill_all_the_rest(ReadArgs *rdarg);
static int afc_readargs_internal_fill_names(ReadArgs *rdarg);
static int afc_readargs_internal_fill_multi(ReadArgs *rdarg);
static char *afc_readargs_internal_get_first_element(ReadArgs *rdarg, short is_numeric);
static void *afc_readargs_internal_get_keyword(ReadArgs *rdarg, char *key, short is_switch, short is_numeric);
static void afc_readargs_internal_replace_chars(ReadArgs *rdarg, char *str, char c1, char c2);
static struct afc_readargs_data *afc_readargs_data_new(void);
static void afc_readargs_data_delete(struct afc_readargs_data *rddata);

// {{{ docs
/*
@node quote
	 *A Y2K virus might shut down our entire network over the new year. As a precaution, we will shut down our entire network over the new year.*

		Anonymous
@endnode

@node intro
This is a class that reproduces almost faithfully (and some what /enhancing/) the *AmigaDOS*'s ReadArgs system call.
AmigaDOS ReadArgs is a powerfull commands, mainly used to parse command line arguments from user input. But who uses Amiga
knows that ReadArgs is even more than that. It is a flexible way to parse text lines with great power and very customizable.

..NOTE:
	ReadArgs is a very complex command present in the Amiga OS.
	Even if I have tried to make it as similar as possible, maybe some bugs
	are liying somewhere. If you are familiar with the AmigaDOS ReadArgs system call
	and find any bug in my port, please, fill free to contact me at the e-mail
	addresses shown below, pointing me what ReadArgs should do and why, and I'll
	try to fix it as soon as possible.


Overview
========

	ReadArgs parses a text string according to a template that is
	passed to it. A template consists of a list of *fields*.
	Fields in the template are separated by commas.  To get the results
	of ReadArgs, you call the afc_readargs_get_by_name() method passing the field name
	or, alternatively, afc_readargs_get_by_pos() passing the field index value (one entry per field in the template),
	starting from 0 for the first field and so on...

	Exactly what is put in a given entry by ReadArgs depends on the type of field.  The default is an AFC string.

	*Fields* can be followed by *modifiers*, which specify things such as
	the type of the field.  Modifiers are specified by following the
	field with a '/' and a single character modifier.  Multiple modifiers
	can be specified by using multiple '/'s.

	Please, see afc_readargs_parse() for a full list of modifiers and their implementation.

-----
Usage
-----

	The use of ReadArgs is quite simple. You define, inside a *template*, the way
	ReadArgs should parse your text string, then you just have to call the afc_readargs_parse()
	method to have it parsed for you. A result code is returned, if it is "0" then the parsersing
	successfully succeded, while if it is different, then something went wrong.

	Here there are some examples:

Example 1
---------

	Suppose you want to create a *copy* shell command. You can create a template like this one:
	``SOURCE/A, DEST/A``

	and (supposing you want to copy *fileA* to *fileB*, the text string could be written in these ways::

		fileA fileB
		DEST fileB fileA
		fileB SOURCE fileA
		SOURCE fileA DEST fileB

	As you can see, ReadArgs is incredibly flexible in parsing strings.


Example 2
----------

	Suppose now that you want to get in input a list of files and to copy them into a destination directory,
	maybe prompting the user for overwrite existing files. A good idea could be create a template in which the
	user can decide *before* starting the copy process whether he/she wants to be prompted for overwriting or
	not. You should declare a template like this:

	``SOURCE/M/A, TO/K/A, OVERWRITE/S, NOOVERWRITE/S``

	In this way, if the user writes a string like:
	``fileA fileB fileC fileD TO destdir/ OVERWRITE``

	ReadArgs will return you a vector containing all the files (A,B,C,D) and you'll know that the overwrite process
	can take place without asking the user any permission.
@endnode
*/
// }}}
// {{{ afc_readargs_new ()
/*
@node afc_readargs_new

		 NAME: afc_readargs_new () - Initializes a new ReadArgs object.

			 SYNOPSIS: ReadArgs * afc_readargs_new ()

		DESCRIPTION: Use this command to inizialize a List object.

		INPUT: NONE

	RESULTS: an initialized ReadArgs structure.

			 SEE ALSO: afc_readargs_delete()
@endnode
*/
ReadArgs *afc_readargs_new()
{
	TRY(ReadArgs *)

	ReadArgs *rdargs = (ReadArgs *)afc_malloc(sizeof(ReadArgs));

	if (rdargs == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "rdargs", NULL);
	rdargs->magic = AFC_READARGS_MAGIC; // Set the Magic value

	// Create the List for the Template Fields
	if ((rdargs->fields = afc_list_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "fields", NULL);

	// Create the List for the string tokens
	if ((rdargs->str = afc_list_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "str", NULL);

	// Create the main string Splitter
	if ((rdargs->global_split = afc_string_list_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "global_split", NULL);

	// Create the secondary string splitter
	if ((rdargs->local_split = afc_string_list_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "local_split", NULL);

	RETURN(rdargs);

	EXCEPT
	afc_readargs_delete(rdargs);

	FINALLY
	ENDTRY
}
// }}}
// {{{ afc_readargs_delete ( rda )
/*
@node afc_readargs_delete

		 NAME: afc_readargs_delete () - Dispose a ReadArgs class.

			 SYNOPSIS: int afc_readargs_delete ( ReadArgs * rdargs )

		DESCRIPTION: Use this method to delete an object's instance.

		INPUT: - rdargs - Pointer to a *valid* ReadArgs instance.

	RESULTS: - On success AFC_ERR_NO_ERROR
				   - On failure, another error value

			 SEE ALSO: afc_readargs_new()
@endnode
*/
int _afc_readargs_delete(ReadArgs *rdargs)
{
	int res;

	if ((res = afc_readargs_clear(rdargs)) != AFC_ERR_NO_ERROR)
		return (res);

	afc_list_delete(rdargs->str);
	afc_list_delete(rdargs->fields);
	afc_string_list_delete(rdargs->global_split);
	afc_string_list_delete(rdargs->local_split);

	afc_free(rdargs);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_readargs_parse ( rda, template, text )
/*
@node afc_readargs_parse

		 NAME: afc_readargs_parse ( rdargs, template, text ) - Parses a string with the given template

			 SYNOPSIS: int afc_readargs_parse ( ReadArgs * rdargs, const char * template, const char * text )

		DESCRIPTION: This is the main method. Using this method you can parse the string with a
				   given template.

		INPUT: - rdargs   - Pointer to a *valid* ReadArgs instance.
				   - template - The template to be used to parse the given string.
								The template string is a list of one or more fields separated by commas,
								with zero or more attributes separated by the "/" char.
								Valid attributes are:
								  + S - Field is a switch. It may be either set of left out.
								  + N - Field is a number. Strings are not allowed.
								  + A - Field is required. If it is left out, the parse fails.
								  + K - The keyword must be given when filling the option.
								  + M - Multiple strings. The result is stored inside a special List class.

				   - text     - The string to parse


	RESULTS: - On success AFC_ERR_NO_ERROR
				   - On failure, another error value

			NOTES: - There may be only one *M* option in a template.

			 SEE ALSO: - afc_readargs_get_by_name()
				   - afc_readargs_get_by_pos()
@endnode
*/
int afc_readargs_parse(ReadArgs *rdargs, const char *template, const char *text)
{
	afc_readargs_clear(rdargs);

	if (text == NULL)
		return (AFC_ERR_NO_ERROR);
	if (strlen(text) == 0)
		return (AFC_ERR_NO_ERROR);

	rdargs->buffer = afc_string_dup(text);
	afc_string_trim(rdargs->buffer);

	afc_readargs_internal_parse_template(rdargs, template);

	afc_readargs_internal_parse_string(rdargs, rdargs->buffer);

	// printf ( "1. %d\n", afc_list_len ( rdargs->str ) );

	afc_readargs_internal_fill_names(rdargs);

	// printf ( "2. %d\n", afc_list_len ( rdargs->str ) );

	if (afc_readargs_internal_fill_keyword(rdargs) != AFC_ERR_NO_ERROR)
		return (AFC_LOG(AFC_LOG_ERROR, AFC_READARGS_ERR_MISSING_KEYWORD, "Keyword is missing", NULL));

	// printf ( "3. %d\n", afc_list_len ( rdargs->str ) );

	afc_readargs_internal_fill_switch(rdargs);

	// printf ( "4. %d\n", afc_list_len ( rdargs->str ) );

	afc_readargs_internal_fill_required(rdargs);

	// printf ( "5. %d\n", afc_list_len ( rdargs->str ) );

	afc_readargs_internal_fill_all_the_rest(rdargs);

	// printf ( "6. %d\n", afc_list_len ( rdargs->str ) );

	afc_readargs_internal_fill_multi(rdargs);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_readargs_get_by_name ( rda, name )
/*
@node afc_readargs_get_by_name

		 NAME: afc_readargs_get_by_name ( rdargs, name ) - Returns a parsed named field

			 SYNOPSIS: void * afc_readargs_get_by_name ( ReadArgs * rdargs, const char * name )

		DESCRIPTION: This method returns the desired field. The name provieded is the same
				   of the one given in the template string in the afc_readargs_parse() call.

		INPUT: - rdargs   - Pointer to a *valid* ReadArgs instance.
				   - name     - Name of the field to retrieve

	RESULTS: - The desired field or NULL in case of errors.

			 SEE ALSO: - afc_readargs_get_by_name()
				   - afc_readargs_get_by_pos()
@endnode
*/
void *afc_readargs_get_by_name(ReadArgs *rdargs, const char *name)
{
	struct afc_readargs_data *arg;

	arg = (struct afc_readargs_data *)afc_list_first(rdargs->fields);
	while (arg)
	{
		if (strcasecmp(name, arg->name) == 0)
		{
			if (arg->multi == NULL)
			{
				if (arg->is_switch)
				{
					if ((BOOL)(int)(long)arg->data == TRUE)
						return ((void *)TRUE);
					else
						return ((void *)FALSE);
				}
				else
					return (arg->data);
			}
			else
				return (arg->multi);
		}

		arg = (struct afc_readargs_data *)afc_list_next(rdargs->fields);
	}

	return (NULL);
}
// }}}
// {{{ afc_readargs_get_by_pos ( rdargs, pos )
/*
@node afc_readargs_get_by_pos

		 NAME: afc_readargs_get_by_pos ( rdargs, pos ) - Returns a parsed field

			 SYNOPSIS: void * afc_readargs_get_by_pos ( ReadArgs * rdargs, int pos )

		DESCRIPTION: This method returns the desired field. The position provided is the
				   same ordinal position of the field in the template string.

		INPUT: - rdargs   - Pointer to a *valid* ReadArgs instance.
				   - pos      - Ordinal position of the field.

	RESULTS: - The desired field or NULL in case of errors.

			 SEE ALSO: - afc_readargs_get_by_name()
				   - afc_readargs_get_by_name()
@endnode
*/
void *afc_readargs_get_by_pos(ReadArgs *rdargs, int pos)
{
	struct afc_readargs_data *arg;

	if ((arg = (struct afc_readargs_data *)afc_list_item(rdargs->fields, pos)) == NULL)
		return (NULL);

	if (arg->multi == NULL)
	{
		if (arg->is_switch)
		{
			if ((BOOL)(int)(long)arg->data == TRUE)
				return ((void *)TRUE);
			else
				return ((void *)FALSE);
		}
		else
			return (arg->data);
	}

	return (arg->multi);
}
// }}}
// {{{ afc_readargs_clear ( rdargs )
/*
@node afc_readargs_clear

		 NAME: afc_readargs_clear ( rdargs ) - Frees all unused memory

			 SYNOPSIS: void * afc_readargs_clear ( ReadArgs * rdargs )

		DESCRIPTION: This method clears all data inside the ReadArgs instance,
				   except the main classes. After this call, all subsequent calls
				   to afc_readargs_get_by_name() or afc_readargs_get_by_pos() will return NULL,
				   until a new afc_readargs_parse() is done.

		INPUT: - rdargs   - Pointer to a *valid* ReadArgs instance.

	RESULTS: - On success AFC_ERR_NO_ERROR
				   - On error another error value

			 SEE ALSO:

@endnode
*/
int afc_readargs_clear(ReadArgs *rdargs)
{
	struct afc_readargs_data *data;
	char *s;

	if (rdargs == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));
	if (rdargs->magic != AFC_READARGS_MAGIC)
		return (AFC_LOG_FAST(AFC_ERR_INVALID_POINTER));

	if (rdargs->buffer)
		afc_string_delete(rdargs->buffer);

	rdargs->buffer = NULL;

	data = (struct afc_readargs_data *)afc_list_first(rdargs->fields);
	while (data)
	{
		afc_readargs_data_delete(data);

		data = (struct afc_readargs_data *)afc_list_next(rdargs->fields);
	}

	s = afc_list_first(rdargs->str);
	while (s)
	{
		afc_string_delete(s);
		s = afc_list_next(rdargs->str);
	}

	afc_list_clear(rdargs->fields);
	afc_list_clear(rdargs->str);

	afc_string_list_clear(rdargs->global_split);
	afc_string_list_clear(rdargs->local_split);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_readargs_parse_cmd_line ( rdargs, template, argc, argv )
/*
@node afc_readargs_parse_cmd_line

		 NAME: afc_readargs_parse_cmd_line ( rdargs, template, argc, argv ) - Parses params passed from the command line

		 SYNOPSIS: void * afc_readargs_parse_cmd_line ( ReadArgs * rdargs, char * template, int argc, char * argv [] )

	  DESCRIPTION: This method helps you parsing the parameters passed from the command line with ReadArgs.
			   Simply provide the /argc/ and /argv/ values from the main function, along with the desired
			   /template/ and the work is done.

			   Since this method is mainly aimed to help you parse command line options, it also handles "help requests" from
			   the user command line. If the user passes one of the tokens shown below as a command line argument, ReadArgs will
			   stop (without actually parsing the command line) with the return code *AFC_READARGS_ERR_HELP_REQUESTED*.
			   In this case, you should provide a nice help message to the user.
			   Recognized "help tokens" are: -h, -help, --help, -?, ?.
			   This should not generate any "clash" with standard ReadArgs arguments since they do not have any "-" before them.

		INPUT: - rdargs     - Pointer to a *valid* ReadArgs instance.
			   - template   - The template to parse the command line parameters with.
			   - argc       - Number of parameters to parse
					   - argv       - values of parameters

		  RESULTS: - On success AFC_ERR_NO_ERROR
				   - On error another error value

				NOTES: this function parses *all* command line arguments, so the file name of the program is included
			   (and it is always the program name). So, if you want to be sure that your command line is parsed
					   correctly, please keep in mind to add a specific entry for the command.
					   For example, you can add a "CMD/A" as the first parameter of your template

		 SEE ALSO: - afc_readargs_parse()

@endnode
*/
int afc_readargs_parse_cmd_line(ReadArgs *rdargs, char *template, int argc, char *argv[])
{
	int slen = 0;
	int t;
	char *s;
	short do_quotes;
	int res;

	// First of all, compute the length of the string to hold all command line args
	// We add three more chars per item just to be sure to be able to handle "" and spaces
	for (t = 0; t < argc; t++)
	{
		s = argv[t];

		slen += (strlen(s)) + 3;

		if ((strcasecmp(s, "-h") == 0) || (strcasecmp(s, "--help") == 0) || (strcasecmp(s, "-help") == 0) ||
			(strcasecmp(s, "?") == 0) || (strcasecmp(s, "-?") == 0))
			return (AFC_READARGS_ERR_HELP_REQUESTED);
	}

	// Then allocate the new string
	s = afc_string_new(slen);

	// Copy all values in the string
	for (t = 0; t < argc; t++)
	{
		// printf ( "Val: %s\n", argv [ t ] );

		do_quotes = (short)(int)(long)index(argv[t], ' ');

		if (do_quotes)
			afc_string_add(s, "\"", 1);
		afc_string_add(s, argv[t], ALL);
		afc_string_add(s, " ", 1);
		if (do_quotes)
			afc_string_add(s, "\"", 1);
	}

	// Parse the string with the given template
	res = afc_readargs_parse(rdargs, template, s);

	// Free the temporary string
	afc_string_delete(s);

	// Return (the result returned bu the parse)
	return (res);
}
// }}}

/* ---------------------------------------------
	 INTERNAL FUNCTIONS
----------------------------------------------- */
// {{{ afc_readargs_internal_parse_template ( rdargs, template )
static int afc_readargs_internal_parse_template(ReadArgs *rdargs, const char *template)
{
	char *token;
	int res;

	if ((res = afc_string_list_split(rdargs->global_split, template, " ,	")) != AFC_ERR_NO_ERROR)
		return (res);

	token = afc_string_list_first(rdargs->global_split);

	while (token)
	{
		afc_readargs_internal_add_template(rdargs, token);
		token = afc_string_list_next(rdargs->global_split);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_readargs_internal_add_template ( rdargs, tok )
static int afc_readargs_internal_add_template(ReadArgs *rdargs, char *tok)
{
	int v, res;
	struct afc_readargs_data *data = afc_readargs_data_new();
	char *token, *name;

	if (data == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NO_MEMORY));

	if ((res = afc_string_list_split(rdargs->local_split, tok, "/")) != AFC_ERR_NO_ERROR)
		return (res);

	name = afc_string_list_first(rdargs->local_split);
	token = afc_string_list_next(rdargs->local_split);

	while (token)
	{
		v = (int)((index(afc_readargs_switches, (int)token[0]))-afc_readargs_switches);
		switch (v)
		{
		case AFC_READARGS_MODE_REQUIRED:
			data->is_required = TRUE;
			break;
		case AFC_READARGS_MODE_KEYWORD:
			data->need_keyword = TRUE;
			data->is_keyword = TRUE;
			break;
		case AFC_READARGS_MODE_NUMERIC:
			data->is_numeric = TRUE;
			break;
		case AFC_READARGS_MODE_SWITCH:
			data->is_switch = TRUE;
			break;
		case AFC_READARGS_MODE_MULTI:
			data->multi = afc_list_new();
			break;
		}

		token = afc_string_list_next(rdargs->local_split);
	}

	afc_string_copy(data->name, name, ALL);
	afc_list_add(rdargs->fields, data, AFC_LIST_ADD_TAIL);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_readargs_internal_parse_string ( rdargs, txt )
static void afc_readargs_internal_parse_string(ReadArgs *rdargs, char *txt)
{
	char *token;
	int res;

	afc_readargs_internal_find_quotes(rdargs, txt);

	if ((res = afc_string_list_split(rdargs->global_split, txt, " =")) != AFC_ERR_NO_ERROR)
		return;

	token = afc_string_list_first(rdargs->global_split);

	while (token)
	{
		if (afc_string_len(token))
		{
			afc_list_add(rdargs->str, afc_string_dup(token), AFC_LIST_ADD_TAIL);
		}

		token = afc_string_list_next(rdargs->global_split);
	}
}
// }}}
// {{{ afc_readargs_internal_find_quotes ( rdargs, str )
static void afc_readargs_internal_find_quotes(ReadArgs *rdargs, char *str)
{
	int t = 0;
	short inside = FALSE;

	while (str[t])
	{
		if (inside)
		{
			if (str[t] == '"')
			{
				inside = FALSE;
				str[t] = ' ';
			}
			else if (str[t] == ' ')
				str[t] = 1;
		}
		else
		{
			if (str[t] == '"')
			{
				inside = TRUE;
				str[t] = ' ';
			}

			if (str[t] == '	')
				str[t] = ' '; // Change tabs to spaces
		}
		t++;
	}
}
// }}}
// {{{ afc_readargs_internal_fill_required ( rdargs )
static int afc_readargs_internal_fill_required(ReadArgs *rdargs)
{
	struct afc_readargs_data *arg;
	void *s;
	long num;

	arg = (struct afc_readargs_data *)afc_list_first(rdargs->fields);

	while (arg)
	{
		if ((arg->is_required) && (arg->data == NULL) && (arg->multi == NULL))
		{
			if ((s = afc_readargs_internal_get_keyword(rdargs, arg->name, arg->is_switch, arg->is_numeric)) == NULL)
			{
				if ((s = afc_readargs_internal_get_first_element(rdargs, arg->is_numeric)) == NULL)
					return (AFC_LOG(AFC_LOG_ERROR, AFC_READARGS_ERR_REQUIRED, "Element required but NULL", arg->name));

				if (arg->is_numeric)
				{
					num = strtol((char *)s, NULL, 0);
					if (errno)
						return (AFC_LOG(AFC_LOG_ERROR, AFC_READARGS_ERR_NOT_A_NUMBER, strerror(errno), s));

					arg->data = (void *)num;

					afc_string_delete(s);
				}
				else
					arg->data = s;
			}
		}

		arg = (struct afc_readargs_data *)afc_list_next(rdargs->fields);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_readargs_internal_fill_keyword ( rdarg )
static int afc_readargs_internal_fill_keyword(ReadArgs *rdarg)
{
	struct afc_readargs_data *arg;
	void *s;

	arg = (struct afc_readargs_data *)afc_list_first(rdarg->fields);

	while (arg)
	{
		if ((arg->need_keyword) && (arg->data == NULL))
		{
			if ((s = afc_readargs_internal_get_keyword(rdarg, arg->name, arg->is_switch, arg->is_numeric)) != NULL)
				arg->data = s;
			// else
			// return ( AFC_READARGS_ERR_MISSING_KEYWORD );
		}

		arg = (struct afc_readargs_data *)afc_list_next(rdarg->fields);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_readargs_internal_fill_switch ( rdarg )
static int afc_readargs_internal_fill_switch(ReadArgs *rdarg)
{
	struct afc_readargs_data *arg;
	void *s;

	arg = (struct afc_readargs_data *)afc_list_first(rdarg->fields);

	while (arg)
	{
		if ((arg->is_switch) && (arg->data == NULL))
		{
			if ((s = afc_readargs_internal_get_keyword(rdarg, arg->name, arg->is_switch, FALSE)) != NULL)
				arg->data = (void *)TRUE;
			else
				arg->data = NULL;
		}

		arg = (struct afc_readargs_data *)afc_list_next(rdarg->fields);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_readargs_internal_fill_all_the_rest ( rdarg )
static int afc_readargs_internal_fill_all_the_rest(ReadArgs *rdarg)
{
	struct afc_readargs_data *arg;

	arg = (struct afc_readargs_data *)afc_list_first(rdarg->fields);

	while (arg)
	{
		if ((arg->data == NULL) && (arg->multi == NULL) && (arg->is_keyword == FALSE))
			if ((arg->data = afc_readargs_internal_get_first_element(rdarg, arg->is_numeric)) == NULL)
				return (AFC_ERR_NO_ERROR);

		arg = (struct afc_readargs_data *)afc_list_next(rdarg->fields);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_readargs_internal_fill_names ( rdargs )
static int afc_readargs_internal_fill_names(ReadArgs *rdarg)
{
	struct afc_readargs_data *arg;

	arg = (struct afc_readargs_data *)afc_list_first(rdarg->fields);

	while (arg)
	{
		if ((arg->data == NULL) && (arg->multi == NULL))
			arg->data = afc_readargs_internal_get_keyword(rdarg, arg->name, arg->is_switch, arg->is_numeric);

		arg = (struct afc_readargs_data *)afc_list_next(rdarg->fields);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_readargs_internal_fill_multi ( rdarg )
static int afc_readargs_internal_fill_multi(ReadArgs *rdarg)
{
	void *s;
	struct afc_readargs_data *arg;

	arg = (struct afc_readargs_data *)afc_list_first(rdarg->fields);

	while (arg)
	{
		if (arg->multi != NULL)
		{
			while ((s = afc_readargs_internal_get_first_element(rdarg, arg->is_numeric)) != NULL)
				afc_list_add(arg->multi, s, AFC_LIST_ADD_TAIL);

			return (AFC_ERR_NO_ERROR);
		}
		arg = (struct afc_readargs_data *)afc_list_next(rdarg->fields);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_readargs_internal_get_first_element ( rdarg, is_numeric )
static char *afc_readargs_internal_get_first_element(ReadArgs *rdarg, short is_numeric)
{
	char *s;
	char *g;

	if (afc_list_is_empty(rdarg->str))
		return (NULL);

	s = (char *)afc_list_first(rdarg->str);

	afc_list_del(rdarg->str);
	afc_readargs_internal_replace_chars(rdarg, s, 1, ' ');

	if (is_numeric)
	{
		g = s;
		s = (char *)strtol(s, NULL, 0);

		afc_string_delete(g);
	}

	return (s);
}
// }}}
// {{{ afc_readargs_internal_get_keyword ( rdarg, key, is_switch, is_numeric )
static void *afc_readargs_internal_get_keyword(ReadArgs *rdarg, char *key, short is_switch, short is_numeric)
{
	char *s;
	int num = 0;

	s = (char *)afc_list_first(rdarg->str);
	while (s)
	{
		// If the element taken from the list is equal
		// to the key we were searching for ...
		if (strcasecmp(s, key) == 0)
		{
			// free the string inside the List
			afc_string_delete(s);

			// Delete the string inside the List (and get the next one)
			s = (char *)afc_list_del(rdarg->str);

			// If the key was a switch, we simply return a TRUE value
			if (is_switch)
			{
				return ((void *)TRUE);
			}
			else
			{
				// if the data was not null we must parse it
				if (s != NULL)
				{
					// Remove the item from the list of items
					afc_list_del(rdarg->str);
					// Convert (char) 1 to space
					afc_readargs_internal_replace_chars(rdarg, s, 1, ' ');
				}
			}

			// If the key is numeric value
			if (is_numeric)
			{
				if (s)
				{
					// We get the numeric rappresentation of it
					num = strtol((char *)s, NULL, 0);
					// And delete the string
					afc_string_delete(s);
				}

				// Returning the number
				return ((void *)(long int)num);
			}

			// Else, we return the string
			return ((void *)s);
		}

		s = (char *)afc_list_next(rdarg->str);
	}

	// If we haven't found anything, and the arg was a SWITCH, then
	// we have to set the data to some value (-2)
	if (is_switch)
		return ((void *)-2);

	return (NULL);
}
// }}}
// {{{ afc_readargs_internal_replace_chars ( rdarg, str, c1, c2 )
static void afc_readargs_internal_replace_chars(ReadArgs *rdarg, char *str, char c1, char c2)
{
	int t = 0;

	if (str == NULL)
		return;

	while (str[t])
	{
		if (str[t] == c1)
			str[t] = c2;
		t++;
	}
}
// }}}
// {{{ afc_readargs_data_new ()
static struct afc_readargs_data *afc_readargs_data_new()
{
	struct afc_readargs_data *rddata = (struct afc_readargs_data *)afc_malloc(sizeof(struct afc_readargs_data));

	if (rddata == NULL)
		return (NULL);

	rddata->name = afc_string_new(AFC_READARGS_MAX_FIELD_NAME);

	if (rddata->name == NULL)
	{
		afc_readargs_data_delete(rddata);
		return (NULL);
	}

	return (rddata);
}
// }}}
// {{{ afc_readargs_data_delete ()
static void afc_readargs_data_delete(struct afc_readargs_data *rddata)
{
	char *s;

	if (rddata == NULL)
		return;

	if (rddata->name)
		afc_string_delete(rddata->name);
	if (rddata->multi)
	{
		s = afc_list_first(rddata->multi);
		while (s)
		{
			afc_string_delete(s);
			s = afc_list_next(rddata->multi);
		}
		afc_list_delete(rddata->multi);
	}

	if ((rddata->is_switch == FALSE) && (rddata->is_numeric == FALSE) && (rddata->data))
		afc_string_delete(rddata->data);

	afc_free(rddata);
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
int main()
{
	ReadArgs *rdarg = afc_readargs_new();
	struct afc_list *node = NULL;

	printf("Res1: %d\n",
		   afc_readargs_parse(rdarg, "COMMAND/A,NOCASE/S,TEXT/A,MAXBYTES/K/N",
							  "Search \"Pippo Pluto Paperino\" MAXBYTES=2048"));

	printf("Command: %s\n", (char *)afc_readargs_get_by_pos(rdarg, 0));
	printf("NoCase:	%d\n", (int)afc_readargs_get_by_pos(rdarg, 1));
	printf("Search:	%s\n", (char *)afc_readargs_get_by_pos(rdarg, 2));
	printf("MaxByt:	%ld\n", (long)afc_readargs_get_by_pos(rdarg, 3));

	printf("Res2: %d\n",
		   afc_readargs_parse(rdarg, "COMMAND/A,NOCASE/S,TEXT/A,MAXBYTES/K/N",
							  "Search NOCASE MAXBYTES 0x1000 \"Pippo Pluto Paperino\""));

	printf("Command: %s\n", (char *)afc_readargs_get_by_pos(rdarg, 0));
	printf("NoCase:	%d\n", (char *)afc_readargs_get_by_pos(rdarg, 1));
	printf("Search:	%s\n", (char *)afc_readargs_get_by_pos(rdarg, 2));
	printf("MaxByt:	%ld\n", (long)afc_readargs_get_by_pos(rdarg, 3));

	return (0);
}
// }}}
#endif
