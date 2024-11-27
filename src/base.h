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
#ifndef AFC_H
#define AFC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "mem_tracker.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Constants */
#ifndef true
#define true (~0)
#endif

#ifndef false
#define false 0
#endif

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

#ifndef MINGW
#define BOOL char
#else
#define BOOL int
#endif

#define AFC_TAG_END 0xDEADBEEF

/* AFC Magic Number */
#define AFC_MAGIC ('B' << 24 | 'A' << 16 | 'S' << 8 | 'E')

/* AFC value for constants */
#define AFC_BASE 0xFF00

	/* Errors for afc */

	enum
	{
		AFC_ERR_NO_ERROR = 0,	   /* No error occurred                       */
		AFC_NO_ERR = 0,			   /* Same as AFC_ERR_NO_ERROR 		   */
		AFC_ERR_NO_MEMORY,		   /* No memory for required operation        */
		AFC_ERR_NULL_POINTER,	   /* Null pointer passed as argument         */
		AFC_ERR_INVALID_POINTER,   /* Pointer passed is not of the right kind */
		AFC_ERR_INVALID_LOG_LEVEL, /* Pointer passed is not of the right kind */
		AFC_ERR_UNSUPPORTED_TAG,   /* Tag passed is not supported by the class */

		AFC_ERR_LAST_ERROR
	};

	enum
	{
		AFC_LOG_MESSAGE = 0,
		AFC_LOG_NOTICE,
		AFC_LOG_WARNING,
		AFC_LOG_ERROR,
		AFC_LOG_CRITICAL
	};

	enum
	{
		AFC_DEBUG_NONE = 0,	 // Nothing is show for debug
		AFC_DEBUG_IMPORTANT, // Only very important things are shown in debug
		AFC_DEBUG_STANDARD,	 // Standard debug messages are shown
		AFC_DEBUG_VERBOSE,	 // Show much more info
		AFC_DEBUG_EVERYTHING // Everything possible is shown
	};

	enum
	{
		AFC_TAG_LOG_LEVEL = AFC_BASE + 1,
		AFC_TAG_LOG_EXIT_CRITICAL,
		AFC_TAG_DEBUG_LEVEL,
		AFC_TAG_SHOW_MALLOCS,
		AFC_TAG_SHOW_FREES,
		AFC_TAG_OUTPUT_FILE
	};

#define AFC_LOG(level, error, descr, info) afc_log(__internal_afc_base, level, error, class_name, __FUNCTION__, descr, info)
#define AFC_LOG_FAST(error) afc_log_fast(__internal_afc_base, error, class_name, __FUNCTION__, NULL)
#define AFC_LOG_FAST_INFO(error, info) afc_log_fast(__internal_afc_base, error, class_name, __FUNCTION__, info)

#define AFC_CLASS_MAGIC(class) ((AFC *)class)->magic

#define AFC_CLASS_TYPE(buf, magic)    \
	buf[0] = (magic >> 24);           \
	buf[1] = (magic >> 16) & 0x00ff;  \
	buf[2] = (magic >> 8) & 0x0000ff; \
	buf[3] = (magic & 0x000000ff);    \
	buf[4] = 0;

#define AFC_CLASS_NAME(buf, class) AFC_CLASS_TYPE(buf, AFC_CLASS_MAGIC(class))

#define AFC_STR_ERROR() __internal_afc_base->last_error

	struct afc_base
	{
		unsigned long int magic;

		int start_log_level;	 // Log level to show the info from. See AFC_LOG_* ( default: 0 )
		short log_exit_critical; // Flag T/F. If T and the log level is CRITICAL, the program will exit ( default: FALSE )
		int debug_level;		 // Debug level (to show some msg on stderr). See AFC_DEBUG_*

		char *tmp_string; //
		char *last_error; // Last error by afc_log()

		struct _memtrack *tracker;

		FILE *fout; // Output file handler (used for loggin functions)
	};

	typedef struct afc_base AFC;

	extern struct afc_base *__internal_afc_base; // A pointer to the AFC alloc'd

	AFC *afc_new(void);
	int afc_delete(AFC *);
	int afc_clear(AFC *);

	int afc_log(AFC *afc, int level, unsigned int error, const char *class_name, const char *funct_name, const char *descr, const char *info);
	int afc_log_fast(AFC *afc, unsigned int error, const char *class_name, const char *funct_name, const char *info);
#define afc_set_tags(afc, first_tag, ...) _afc_set_tags(afc, first_tag, ##__VA_ARGS__, AFC_TAG_END)
	int _afc_set_tags(AFC *afc, int first_tag, ...);
	int afc_set_tag(AFC *afc, int tag, void *val);

	int afc_debug(AFC *afc, int level, const char *class_name, const char *str);
	int afc_debug_adv(AFC *afc, int level, const char *class_name, const char *fmt, ...);

#define afc_malloc(size) _afc_malloc(size, __FILE__, __FUNCTION__, __LINE__)
#define afc_realloc(mem, size) _afc_realloc(mem, size, __FILE__, __FUNCTION__, __LINE__)

	void *_afc_malloc(size_t size, const char *file_name, const char *func_name, const unsigned int line);
	void _afc_free(void *mem, const char *file, const char *func, const unsigned int line);
	void *_afc_realloc(void *mem, size_t size, const char *file, const char *func, const unsigned int line);

	struct _memtrack *afc_track_mallocs(AFC *afc);

	int _afc_dprintf(const char *fmt, ...);

#define afc_free(mem) _afc_free(mem, __FILE__, __FUNCTION__, __LINE__)

#ifdef DEBUG
#define F_IN() afc_dprintf(">>> Enter: %s\n", __FUNCTION__);
#define F_OUT() afc_dprintf("<<< Leave: %s\n", __FUNCTION__);
#define AFC_DEBUG_FUNC() afc_debug(__internal_afc_base, AFC_DEBUG_EVERYTHING, class_name, __FUNCTION__)
#define AFC_DEBUG(level, str) afc_debug(__internal_afc_base, level, class_name, str)
#define afc_dprintf(fmt, ...) _afc_dprintf((fmt), ##__VA_ARGS__)
#else
#define F_IN()
#define F_OUT()
#define AFC_DEBUG_FUNC()
#define AFC_DEBUG(level, str)
#define afc_dprintf(fmt, ...)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
