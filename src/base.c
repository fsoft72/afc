/* AFC - The  Advanced Foundation Classes
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
	1.20	- Added Logging on external files
		  Added afc_dprintf function.

	1.11	- Added the atexit() callback. Now AFC automatically free themselves on exit
*/

#include "base.h"
#include "string.h"
#include "mem_tracker.h"

// {{{ docs
/*
@config
	TITLE:     AFCBase
	VERSION:   1.20
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
@endnode

@node quote
	 *Programming today is a race between software engineers striving to build bigger and better idiot-proof programs, and the*
	 *Universe trying to produce bigger and better idiots. So far, the Universe is winning.*

		Rich Cook
@endnode

@node intro

AFC Base is the base class of all AFC classes.
It must be instantiated in all your application in order to be able to use any AFC class.
The class offers some good (and standardized) utilities, such as afc_log() and afc_debug() that will help
you develop your programs.
Since all AFC classes can reference an AFC Base, they gain the same flexibility and standardization in
sending log and debug messages, and the application can give the user the ability to set the debug and info
level he/she wants.
You can create a new instance of AFC Base by calling afc_new(), and remember to free it before exiting
with the standard call afc_delete().


-----------------
Utility Functions
-----------------

AFC Base offers some utility functions that can be used inside any program taking advatages of AFC classes.
As mentioned before, take a look at afc_log(), afc_debug() and afc_log_fast() functions. But there are also
some macro you should take under consideration. In this section we'll examine deeply AFC debugging and logging
functions, to let you have a glance of what you can do with them.

Feedback to the user and developer
----------------------------------

AFC Base standardized the way feedback should be outputted to the user and/or developer, by giving always the same
aspect on error/warning/log messages. This will ease the pain of understand what's going on in the backyard and helps
better understanding the various aspect of a program.

All we're going to discuss here is already implemented on standard AFC (maybe, something is missing, but the main parts
are already all there). All these things can apply to your own classes and applications too, without any problem.

Logging problems
----------------

A log message appears when something inside the application went wrong. AFC implement different levels of severity,
from a simple message defined with AFC_LOG_MESSAGE to the critical almost-blocking error, defined with AFC_LOG_CRITICAL.
Depending by the setting of the AFC Base in use, an AFC_LOG_CRITICAL error can cause the application to abort immediately.
See afc_set_tag() for more info.


Loggin functions are described in deep detail on afc_log() and afc_log_fast() API documentation section. Here we'd like
to introduce you some macro (based on afc_log() and afc_log_fast() respectively) that can ease you even more.

- ``AFC_LOG (afc, level, error, descr, info)``
- ``AFC_LOG_FAST (afc, error)``
- ``AFC_LOG_FAST_INFO (afc, error, info)``

All these macro gets the same inputs, so we'll describe them only once:

afc
	Pointer to a valid (already allocated) AFC class.

level
	Log message error level. See afc_log() for more info.

error
	Error code related to the problem enocuntered. Every app can use its own.

descr
	Textual description of the error enocuntered.

info
	Optional string with additional info about the error.


Sending Debug Informations
--------------------------

To simple send debug messages to the user/developers, AFC Base offers two functions: afc_debug() and afc_debug_adv().
Please refer to the afc_debug() and afc_debug_adv() API pages for in deep documentation about these functions. What we
want to discuss here are two handy macros:

- ``AFC_DEBUG_FUNC(afc)``:  If debug level is set to AFC_DEBUG_EVENRYTHING, displays the current function name during program execution.
- ``AFC_DEBUG(afc,level,str)``: Writes a debug message on the standard error. Debug message level is defined by level
				and the message is stored in str.

@endnode
*/
// }}}

static const char class_name[] = "AFC Base";

struct afc_base *__internal_afc_base = NULL;

static int afc_internal_parse_tags(AFC *afc, int first_tag, va_list tags);
static void afc_internal_on_exit(void);

// {{{ afc_new ()
/*
@node afc_new

			 NAME: afc_new () - Initializes a new afc instance.

		 SYNOPSIS: AFC * afc_new ()

	  DESCRIPTION: This function initializes a new afc instance.

			INPUT: - NONE

		  RESULTS: a valid initialized afc structure. NULL in case of errors.

		 SEE ALSO: - afc_delete()

@endnode
*/
AFC *afc_new()
{
	AFC *afc = (AFC *)malloc(sizeof(AFC));

	if (afc == NULL)
		return (NULL);

	memset(afc, 0, sizeof(AFC));

	afc->magic = AFC_MAGIC;

	afc->start_log_level = 0;
	afc->fout = stderr;

	__internal_afc_base = afc;

	if ((afc->tmp_string = afc_string_new(255)) == NULL)
	{
		afc_log(afc, AFC_LOG_CRITICAL, AFC_ERR_NO_MEMORY, class_name, __FUNCTION__, "No memory to alloc a afc_string_new()", "size: 255 bytes");
		afc_delete(afc);
		return (NULL);
	}

	if ((afc->last_error = afc_string_new(255)) == NULL)
	{
		afc_log(afc, AFC_LOG_CRITICAL, AFC_ERR_NO_MEMORY, class_name, __FUNCTION__, "No memory for last_error", "size: 255 bytes");
		afc_delete(afc);
		return (NULL);
	}

	atexit(afc_internal_on_exit);

	return (afc);
}
// }}}
// {{{ afc_delete ( afc )
/*
@node afc_delete

			 NAME: afc_delete ( afc )  - Disposes a valid afc instance.

		 SYNOPSIS: int afc_delete ( AFC * base)

	  DESCRIPTION: This function frees an already alloc'd afc structure.

			INPUT: - afc  - Pointer to a valid afc class.

		  RESULTS: should be AFC_ERR_NO_ERROR

			NOTES: - this method calls: afc_clear()

		 SEE ALSO: - afc_new()
				   - afc_clear()
@endnode
*/
int afc_delete(AFC *afc)
{
	int afc_res;

	if ((afc_res = afc_clear(afc)) != AFC_ERR_NO_ERROR)
		return (afc_res);

	if (afc->tracker)
		afc_mem_tracker_delete(afc->tracker);
	afc->tracker = NULL;

	if (afc->tmp_string)
		afc_string_delete(afc->tmp_string);
	if (afc->last_error)
		afc_string_delete(afc->last_error);

	/* NOTE: any class contained in afc should be deleted here */

	// Remove the magic value
	afc->magic = 0;

	if ((afc->fout) && (afc->fout != stderr))
		fclose(afc->fout);

	free(afc);

	__internal_afc_base = NULL;

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_clear ( afc )
/*
@node afc_clear

			 NAME: afc_clear ( afc )  - Clears all stored data

		 SYNOPSIS: int afc_clear ( AFC * base)

	  DESCRIPTION: Use this function to clear all stored data in the current afc instance.

			INPUT: - afc    - Pointer to a valid afc instance.

		  RESULTS: should be AFC_ERR_NO_ERROR

		 SEE ALSO: - afc_delete()

@endnode
*/
int afc_clear(AFC *afc)
{
	if (afc == NULL)
		return (AFC_ERR_NULL_POINTER);

	if (afc->magic != AFC_MAGIC)
		return (AFC_ERR_INVALID_POINTER);

	if (afc->tmp_string)
		afc_string_clear(afc->tmp_string);
	if (afc->last_error)
		afc_string_clear(afc->last_error);

	/* Custom Clean-up code should go here */
	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_log ( afc, level, error, class_name, funct_name, descr, info )
/*
@node afc_log

			 NAME: afc_log ( afc, level, error, class_name, funct_name, descr, info ) - Writes a log message to the standard error

		 SYNOPSIS: int afc_log ( AFC * afc, int level, unsigned int error, const char * class_name, const char * funct_name, const char * descr, const char * info )

	  DESCRIPTION: This function writes a log text to the application standard error. It is best to use this function instead of
				   custom made error/log reporting routines because it takes care of a lot of things. For example, if a message is
				   below the default user desired log level, no message will be written to the standard error.

			INPUT: 	- afc    	- Pointer to a valid afc instance.
			- level  	- Log Level. It can be any of the following:
						+ AFC_LOG_MESSAGE  - Just a message, nothing really important
						+ AFC_LOG_NOTICE   - Something that the user can be interested in view
						+ AFC_LOG_WARNING  - A non serious error, just to let the user know
						+ AFC_LOG_ERROR    - A real error
						+ AFC_LOG_CRITICAL - An unrecoverable error occurred.  If AFC_TAG_LOG_EXIT_CRITICAL was set to TRUE,
									then the program will immediately quit.

			- error		- Your own error code to be returned.
			- class_name 	- The name of the class raising the error.
			- funct_name 	- The name of the function raising the error.
			- descr      	- A textual description of the error being raised.
			- info       	- Additional string for the error being raised.

		  RESULTS: 	this function returns the same error passed in the 'error' parameter. This is useful in cases where you want to
			write stuff like: return ( afc_log ( ... ) );

		 SEE ALSO: 	- afc_log_fast ()
			- afc_debug ()

@endnode
*/
int afc_log(AFC *afc, int level, unsigned int error, const char *class_name, const char *funct_name, const char *descr, const char *info)
{
	static char *str_level[] = {"MESSAGE", "NOTICE", "WARNING", "ERROR", "CRITICAL", NULL};

	if ((descr != NULL) && (afc->last_error != NULL))
		afc_string_copy(afc->last_error, descr, ALL);

	if (afc->fout == NULL)
		return (error);

	// printf ( "ERR: Level: %d - Start: %d - Class: %s - Descr: %s\n", level, afc->start_log_level, class_name, descr );

	if (level > AFC_LOG_CRITICAL)
		return (error);

	if (level < afc->start_log_level)
		return (error);

	fprintf(afc->fout, "------------------------ %s -------------------------\n", str_level[level]);

	if (class_name)
		fprintf(afc->fout, "Class: %s\n", class_name);
	if (funct_name)
		fprintf(afc->fout, "Funct: %s\n", funct_name);
	if (descr)
		fprintf(afc->fout, "Descr: %s\n", descr);
	if (info)
		fprintf(afc->fout, " Info: %s\n", info);

	fprintf(afc->fout, " Code: %x\n", error);

	if ((level == AFC_LOG_CRITICAL) && (afc->log_exit_critical))
		exit(1);

	return (error);
}
// }}}
// {{{ afc_log_fast ( afc, error, class_name, funct_name, info )
/*
@node afc_log_fast

			 NAME: afc_log_fast ( afc, error, class_name, funct_name, info ) - Writes a log message to the standard error

		 SYNOPSIS: int afc_log_fast ( AFC * afc, unsigned int error, const char * class_name, const char * funct_name, const char * info )

	  DESCRIPTION: This function is a /reduced/ version of the standard afc_log() one. It takes few arguments and it is able to
				   set the missing ones with defvault values. It is just a shorthand for the afc_log() since the original function
				   is called by afc_log_fast once it has decided what is the log level and the text description for the error raised.
				   You can call this function only with a defined set of /standard/ errors, briefely described below, but mainly all
				   the errors present in afc/base.h file.

			INPUT: 	- afc    	- Pointer to a valid afc instance.
			- error		- The error code to raise. It can be one of the following:
						+ AFC_ERR_NO_ERROR
						+ AFC_ERR_NO_MEMORY
						+ AFC_ERR_UNSUPPORTED_TAG
						+ AFC_ERR_INVALID_POINTER

			- class_name 	- The name of the class raising the error.
			- funct_name 	- The name of the function raising the error.
			- info       	- Additional string for the error being raised.

		  RESULTS: 	this function returns the same error passed in the 'error' parameter. This is useful in cases where you want to
			write stuff like: return ( afc_log_fast ( ... ) );

		 SEE ALSO: 	- afc_log ()
			- afc_debug ()
@endnode
*/
int afc_log_fast(AFC *afc, unsigned int error, const char *class_name, const char *funct_name, const char *info)
{
	static const char *descr[] = {"No Error", "No Memory", "NULL Pointer", "Invalid Pointer", "Unsupported Tag", NULL};
	const char *d;
	int level;

	switch (error)
	{
	case AFC_ERR_NO_ERROR:
		level = AFC_LOG_MESSAGE;
		break;

	case AFC_ERR_NO_MEMORY:
		level = AFC_LOG_CRITICAL;
		break;

	case AFC_ERR_UNSUPPORTED_TAG:
	case AFC_ERR_NULL_POINTER:
		level = AFC_LOG_WARNING;
		break;

	case AFC_ERR_INVALID_POINTER:
		level = AFC_LOG_ERROR;
		break;

	default:
		level = 1000;
		break;
	}

	if (level > AFC_LOG_CRITICAL)
		return (error);
	if (level < afc->start_log_level)
		return (error);

	if (error)
	{
		if (error < AFC_ERR_LAST_ERROR)
			d = descr[error];
		else
			d = descr[0];
	}
	else
		d = descr[0];

	afc_log(afc, level, error, class_name, funct_name, d, info);

	return (error);
}
// }}}
// {{{ afc_debug ( afc, level, class_name, message )
/*
@node afc_debug

			 NAME: 	afc_debug ( afc, level, class_name, message ) - Writes a debug message to the standard error

		 SYNOPSIS: 	int afc_debug ( AFC * afc, int level, const char * class_name, const char * message )

	  DESCRIPTION: 	This function writes a debug message to the standard error. Depending on the settings of the AFC Base class,
					messages passed to the afc_debug function may be shown or simply ignored. See afc_set_tag () for more
					info. If this function is not enough flexible for you, than you should take a look at afc_debug_adv()
			counterpart.

			INPUT: 	- afc    	- Pointer to a valid afc instance.
			- level		- The debug message level. Valid values are
						+ AFC_DEBUG_IMPORTANT - This is a very important message that needs to be shown
						+ AFC_DEBUG_STANDARD  - This is a standard-importance message.
						+ AFC_DEBUG_VERBOSE   - This is a very informative message.

			- class_name 	- The name of the class raising the error.
			- str        	- Message to show

		  RESULTS: should be AFC_ERR_NO_ERROR

		 SEE ALSO: 	- afc_debug_adv ()
			- afc_log ()
@endnode
*/
int afc_debug(AFC *afc, int level, const char *class_name, const char *str)
{
	if (level > afc->debug_level)
		return (AFC_ERR_NO_ERROR);
	if (afc->fout == NULL)
		return (AFC_ERR_NO_ERROR);

	fprintf(afc->fout, "DEBUG (%s): %s\n", class_name, str);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_debug_adv ( afc, level, class_name, fmt, ... )
/*
@node afc_debug_adv

			 NAME: afc_debug_adv ( afc, level, class_name, fmt, ... ) - Writes a debug message to the standard error

		 SYNOPSIS: int afc_debug_adv ( AFC * afc, int level, const char * class_name, const char * fmt, ... )

	  DESCRIPTION: This function writes a debug message to the standard error. Depending on the settings of the AFC Base class,
				   messages passed to the afc_debug function may be shown or simply ignored. See afc_set_tag () for more
				   info. The message is composed most likely the standard printf-family functions. If you are looking for
				   something easier to use, please consider afc_debug() function.

			INPUT: 	- afc    	- Pointer to a valid afc instance.
			- level		- The debug message level. Valid values are
						+ AFC_DEBUG_IMPORTANT - This is a very important message that needs to be shown
						+ AFC_DEBUG_STANDARD  - This is a standard-importance message.
						+ AFC_DEBUG_VERBOSE   - This is a very informative message.

			- class_name 	- The name of the class raising the error.
			- fmt        	- String format, specified in the same way you do with printf.
			- ...        	- All params needed by the provieded fmt.

		  RESULTS: should be AFC_ERR_NO_ERROR

		 SEE ALSO: 	- afc_debug ()
			- afc_log ()
@endnode
*/
int afc_debug_adv(AFC *afc, int level, const char *class_name, const char *fmt, ...)
{
	va_list args;

	if (level > afc->debug_level)
		return (AFC_ERR_NO_ERROR);
	if (afc->fout == NULL)
		return (AFC_ERR_NO_ERROR);

	va_start(args, fmt);

	fprintf(afc->fout, "DEBUG (%s): ", class_name);
	// Flawfinder: ignore
	vfprintf(afc->fout, fmt, args);
	fprintf(afc->fout, "\n");

	va_end(args);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_set_tags ( afc, first_tag, ... )
/*
@node afc_set_tags

			 NAME: 	afc_set_tags ( afc, first_tag, ... ) - Sets valid tags

		 SYNOPSIS: 	int afc_set_tags ( AFC * afc, int first_tag, ... )

	  DESCRIPTION: 	This function sets a list of tags in the current class. For a list of valid tags, please
					see afc_set_tag() function.

			INPUT: 	- afc        - Pointer to a valid afc instance.
			- first_tag  - First tag to be set
			- ...        - Tags and values to be set

		  RESULTS: 	should be AFC_ERR_NO_ERROR

		 SEE ALSO: 	- afc_set_tag ()
@endnode
*/
int _afc_set_tags(AFC *afc, int first_tag, ...)
{
	va_list tags;

	va_start(tags, first_tag);
	afc_internal_parse_tags(afc, first_tag, tags);
	va_end(tags);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_set_tag ( afc, tag, value )
/*
@node afc_set_tag

			 NAME: afc_set_tag ( afc, tag, value ) - Set a tag

		 SYNOPSIS: int afc_set_tag ( AFC * afc, int tag, void * val )

	  DESCRIPTION: This function sets a tag in the current class.

			INPUT: 	- afc    - Pointer to a valid afc instance.
			- tag    - Tag to be set. Valid tags are:
					+ AFC_TAG_DEBUG_LEVEL       - This tag determines the debug level. Valid values can be:

						* AFC_DEBUG_NONE      - No debug messages are shown
						* AFC_DEBUG_IMPORTANT - Only important debug messages are shown
						* AFC_DEBUG_STANDARD  - All messages that are at least marked as "standard" will be shown
						* AFC_DEBUG_VERBOSE   - All messages that are at least "verbose" will be shown
						* AFC_DEBUG_EVERYTHING - All messages are shown

					+ AFC_TAG_LOG_EXIT_CRITICAL - This tag determines whether, in occurrence of a
						critical error, the application should quit or not.  Valid values are:

						* TRUE - The application will quit when a critical error appears
						* FALSE - The application will continue to work

					+ AFC_TAG_LOG_LEVEL - Sets the log level. Values can be:

						* AFC_LOG_MESSAGE - This is just a simple message, not very important
						* AFC_LOG_NOTICE	- This is something the user would like to know
						* AFC_LOG_WARNING - This is not an error, but is something that needs to be fixed anyway
						* AFC_LOG_ERROR		- This is really an error
						* AFC_LOG_CRITICAL - This error is blocking

					+ AFC_TAG_SHOW_MALLOCS 	- Show MemTracker Mallocs (only when MemTracker is active)

						* TRUE		- Mallocs are shown
						* FALSE		- Mallocs are not shown

					+ AFC_TAG_SHOW_FREES 	- Show MemTracker frees (only when MemTracker is active)

						* TRUE		- Frees are shown
						* FALSE		- Frees are not shown

					+ AFC_TAG_OUTPUT_FILE	- Sets the output file used for debug functions. Valid values can be:

						* an already opened FILE pointer. You must handle file opening and closing.
						* By passing a NULL value, you completely turn off debug messages.

			- val    - Value to set to the tag

		  RESULTS: should be AFC_ERR_NO_ERROR

		 SEE ALSO: - afc_set_tags ()
@endnode
*/
int afc_set_tag(AFC *afc, int tag, void *val)
{
	switch (tag)
	{
	case AFC_TAG_LOG_LEVEL:
		afc->start_log_level = (int)(long)val;
		break;

	case AFC_TAG_LOG_EXIT_CRITICAL:
		afc->log_exit_critical = (short)(int)(long)val;
		break;

	case AFC_TAG_DEBUG_LEVEL:
		afc->debug_level = (int)(long)val;
		break;

	case AFC_TAG_SHOW_MALLOCS:
		if (afc->tracker)
			afc->tracker->show_mallocs = (short)(int)(long)val;
		break;

	case AFC_TAG_SHOW_FREES:
		if (afc->tracker)
			afc->tracker->show_frees = (short)(int)(long)val;
		break;

	case AFC_TAG_OUTPUT_FILE:
		afc->fout = (FILE *)val;
		break;
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_malloc ( size )
/*
@node afc_malloc

			 NAME: afc_malloc ( size ) - Allocates memory

		 SYNOPSIS: void * afc_malloc ( size_t size )

	  DESCRIPTION: 	This function is a wrapper around the standard malloc(3) function.
			It offers just some enhancements, that are:
				- Automatic error message in case of no memory avaible.
				- Memory is cleared before returning the pointer.

			INPUT: - size   - Size (in bytes) of the memory chunk you want to alloc.

		  RESULTS: should be a valid pointer to the memory just alloc'd, or NULL in case of errors.

		 SEE ALSO: - afc_free()
@endnode
*/
void *_afc_malloc(size_t size, const char *file_name, const char *func_name, const unsigned int line)
{
	void *mem;

	if (__internal_afc_base->tracker == NULL)
	{
		mem = malloc(size);
	}
	else
	{
		mem = afc_mem_tracker_malloc(__internal_afc_base->tracker, size, file_name, func_name, line);
	}

	if (mem == NULL)
	{
		AFC_LOG_FAST(AFC_ERR_NO_MEMORY);
		return NULL;
	}

	memset(mem, 0, size);
	return mem;
}
// }}}
// {{{ afc_free ( addr )
/*
@node afc_free

			 NAME: afc_free ( mem ) - Frees some memory

		 SYNOPSIS: void afc_free ( void * mem )

	  DESCRIPTION: This function is a wrapper around the standard free(3) function.
		 Please, use this function in pair with all afc_malloc() calls you will do.

			INPUT: - mem   - Pointer of memory to release to the system.

		  RESULTS: NONE

		NOTES: - This function handles NULL pointer

		 SEE ALSO: - afc_malloc()
@endnode
*/
void _afc_free(void *mem, const char *file, const char *func, const unsigned int line)
{
	char *m;

	if (!mem)
	{
		_afc_dprintf("%s::%s NULL pointer from %s::%s (%d)\n", __FILE__, __FUNCTION__, file, func, line);
		return;
	}

	m = mem;
	m[0] = '\0';

	if (__internal_afc_base == NULL || __internal_afc_base->tracker == NULL) /*hardening against uninit base pointer*/
		free(mem);
	else
		_afc_mem_tracker_free(__internal_afc_base->tracker, mem, file, func, line);
}
// }}}
// {{{ afc_realloc ( addr )
/*
@node afc_realloc

			 NAME: afc_realloc ( mem ) - Frees some memory

		 SYNOPSIS: void afc_realloc ( void * mem )

	  DESCRIPTION: This function is a wrapper around the standard realloc(3) function.
		 Please, use this function in pair with all afc_malloc() calls you will do.

			INPUT: - mem   - Pointer of memory to release to the system.

		  RESULTS: NONE

		NOTES: - This function handles NULL pointer

		 SEE ALSO: - afc_malloc()
			   - afc_free()
@endnode
*/
void *_afc_realloc(void *mem, size_t size, const char *file, const char *func, const unsigned int line)
{
	void *new_addr;

	if (!mem)
	{
		_afc_dprintf("%s::%s NULL pointer from %s::%s (%d)\n", __FILE__, __FUNCTION__, file, func, line);
		return NULL;
	}

	new_addr = realloc(mem, size);

	if (new_addr == NULL)
	{
		AFC_LOG_FAST(AFC_ERR_NO_MEMORY);
		return NULL;
	}

	if (__internal_afc_base == NULL || __internal_afc_base->tracker == NULL) /*hardening against uninit base pointer*/
		return new_addr;
	else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuse-after-free"
		_afc_mem_tracker_update_size(__internal_afc_base->tracker, mem, new_addr, size, file, func, line);
#pragma GCC diagnostic pop

	return new_addr;
}
// }}}
// {{{ afc_track_mallocs ( afc ) ************
MemTracker *afc_track_mallocs(AFC *afc)
{
	if (afc->tracker != NULL)
		return afc->tracker;

	afc->tracker = afc_mem_tracker_new();

	return afc->tracker;
}
// }}}
// {{{ afc_dprintf ( fmt, ... ) ************
int _afc_dprintf(const char *fmt, ...)
{
	va_list args;
	FILE *f;

	if (__internal_afc_base == NULL)
		return (AFC_ERR_NO_ERROR);

	f = __internal_afc_base->fout;

	if (f == NULL)
		return (AFC_ERR_NO_ERROR);

	va_start(args, fmt);

	vfprintf(f, fmt, args);
	fflush(f);

	va_end(args);

	fflush(f);

	return (AFC_ERR_NO_ERROR);
}
// }}}

/* =======================================================================================================
	INTERNAL FUNCTIONS
======================================================================================================= */

// {{{ afc_internal_parse_tags ( afc, first_tag, tags )
static int afc_internal_parse_tags(AFC *afc, int first_tag, va_list tags)
{
	unsigned int tag;
	void *val;

	tag = first_tag;

	while (tag != AFC_TAG_END)
	{
		val = va_arg(tags, void *);
		afc_set_tag(afc, tag, val);
		tag = va_arg(tags, int);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_internal_on_exit ()
static void afc_internal_on_exit(void)
{
	if (__internal_afc_base)
		afc_delete(__internal_afc_base);
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
int main(int argc, char *argv[])
{
	AFC *afc = afc_new();

	if (afc == NULL)
	{
		fprintf(stderr, "Init of class AFC failed.\n");
		return (1);
	}

	afc_set_tags(afc, AFC_TAG_LOG_LEVEL, AFC_LOG_CRITICAL, AFC_TAG_END);

	afc_log(afc, 0, 0, "Test", __FUNCTION__, "Just a test message", "Info");

	afc_delete(afc);

	return (0);
}
// }}}
#endif
