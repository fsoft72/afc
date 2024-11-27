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
#ifndef AFC_DIRMASTER_H
#define AFC_DIRMASTER_H

/*  $VER: DirMaster 1.01						  */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>

#include "base.h"
#include "array.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* AFC DirMaster Magic Value: DIRM */
#define AFC_DIRMASTER_MAGIC ('D' << 24 | 'I' << 16 | 'R' << 8 | 'M')

/* Base DirMaster Valie */
#define AFC_DIRMASTER_BASE 0x4000

	enum
	{
		AFC_DIRMASTER_ERR_DIR_NOT_FOUND = AFC_DIRMASTER_BASE + 1
	};

	enum
	{
		AFC_DIRMASTER_TAG_DATE_FORMAT = AFC_DIRMASTER_BASE + 1,
		AFC_DIRMASTER_TAG_SIZE_FORMAT,
		AFC_DIRMASTER_TAG_SIZE_DECIMALS,
		AFC_DIRMASTER_TAG_CONV_DATE_MODIFY,
		AFC_DIRMASTER_TAG_CONV_DATE_ACCESS,
		AFC_DIRMASTER_TAG_CONV_DATE_CHANGE,
		AFC_DIRMASTER_TAG_CONV_USER,
		AFC_DIRMASTER_TAG_CONV_MODE,
		AFC_DIRMASTER_TAG_CONV_GROUP,
		AFC_DIRMASTER_TAG_SORT_FIELD,
		AFC_DIRMASTER_TAG_SORT_CASE_INSENSITIVE,
		AFC_DIRMASTER_TAG_SORT_INVERTED
	};

	/* Date Format */

	enum
	{
		DATEFORMAT_DD_MM_YYYY = 0,
		DATEFORMAT_MM_DD_YYYY,
		DATEFORMAT_HH_MM,
		DATEFORMAT_HH_MM_SS,
		DATEFORMAT_DD_MM_YYYY_HH_MM,
		DATEFORMAT_MM_DD_YYYY_HH_MM
	};

	/* Size Format */
	enum
	{
		SIZEFORMAT_BYTES = 0,
		SIZEFORMAT_HUMAN, /* Human readable sizes, with 1K corresponding to 1024 bytes */
		SIZEFORMAT_HUMAN_1000
	}; /* Human readable sizes, with 1K corresponding to 1000 bytes */

	/* FIELDS CONSTANTS */
	enum
	{
		FINFO_NAME = 0,
		FINFO_MODE,
		FINFO_USER,
		FINFO_GROUP,
		FINFO_DATE_ACCESS,
		FINFO_DATE_MODIFY,
		FINFO_DATE_CHANGE,
		FINFO_SIZE
	};

	enum
	{
		FINFO_KIND_UNKN = 0,
		FINFO_KIND_FILE,
		FINFO_KIND_DIR,
		FINFO_KIND_LINK
	};

	struct afc_dirmaster; // Defined later

	struct _internalSortInfo
	{
		int field;
		short inverted;
		short case_insensitive;
	};

	struct _file_info
	{
		char name[NAME_MAX]; /* File name (with no path) */

		char cmode[16];	 /* string containing the 'mode' of the file (eg. '-rwxr-xr-x') */
		char cuser[10];	 /* string containing the user name                             */
		char cgroup[10]; /* string containing the group name                            */

		char caccess[20];
		char cmodify[20];
		char cchange[20];
		char csize[10];

		short hidden; /* whether this is an hidden file or not */

		short selected; /* whether this file/dir is selected or not */

		unsigned long size; /* Size of file in bytes */
		unsigned int kind;	/* See FINFO_KIND_*   */

		struct stat *st; /* Relative Stat structure */

		void *info; /* Free for personal use */

		struct afc_dirmaster *dm; /* Link to the managing DirMaster */
	};

	typedef struct _file_info FileInfo;

	struct afc_dirmaster
	{
		unsigned long magic; /* AFC DirMaster Magic Number */

		Array *am;				 /* Pointer to our super-class */
		unsigned long errorcode; /* Last error code */

		char *current_dir; // Current directory in memory

		int date_format;   /* See DATEFORMAT_* defines  */
		int size_format;   /* See SIZEFORMAT_* defines  */
		int size_decimals; /* Number of digits after the "." (only for HUMAN* format ) */

		short conv_date_access; /* These are three flags to select whether to convert dates to chars */
		short conv_date_change;
		short conv_date_modify;

		short conv_mode;  // Flag T/F. If T, then mode is converted from numerical value to its corresponding string
		short conv_user;  // Flag T/F. If T, then user is converted from id to string
		short conv_group; // Flag T/F. If T, then group is converted from id to string

		struct _internalSortInfo isi;
	};

	typedef struct afc_dirmaster DirMaster;

#define afc_dirmaster_delete(dm)   \
	if (dm)                        \
	{                              \
		_afc_dirmaster_delete(dm); \
		dm = NULL;                 \
	}

	struct afc_dirmaster *afc_dirmaster_new(void);

	int _afc_dirmaster_delete(DirMaster *);
#define afc_dirmaster_set_tags(dm, first, ...) _afc_dirmaster_set_tags(dm, first, ##__VA_ARGS__, AFC_TAG_END)
	int _afc_dirmaster_set_tags(DirMaster *dm, int first_tag, ...);
	int afc_dirmaster_set_tag(DirMaster *dm, int tag, void *val);

	int afc_dirmaster_scan_dir(DirMaster *, const char *);

#define afc_dirmaster_first(d) (FileInfo *)(d ? afc_array_first(d->am) : NULL)
#define afc_dirmaster_last(d) (FileInfo *)(d ? afc_array_last(d->am) : NULL)
#define afc_dirmaster_prev(d) (FileInfo *)(d ? afc_array_prev(d->am) : NULL)
#define afc_dirmaster_item(d, n) (FileInfo *)(d ? afc_array_item(d->am, n) : NULL)
#define afc_dirmaster_succ(d) (FileInfo *)(d ? afc_array_next(d->am) : NULL)
#define afc_dirmaster_next(d) (FileInfo *)(d ? afc_array_next(d->am) : NULL)
#define afc_dirmaster_len(d) (d ? afc_array_len(d->am) : 0)
#define afc_dirmaster_num_items(d) (d ? afc_array_len(d->am) : 0)
#define afc_dirmaster_before_first(d) (d ? afc_array_before_first(d->am) : 0)
#define afc_dirmaster_is_empty(d) (d ? afc_array_is_empty(d->am) : TRUE)
#define afc_dirmaster_pos(d) (d ? afc_array_pos(d->am) : 0)

	// FileInfo *     afc_dirmaster_obj(DirMaster * );
	// FileInfo *     afc_dirmaster_next(DirMaster * );
	// FileInfo *     afc_dirmaster_first(DirMaster * );
	// FileInfo *     afc_dirmaster_prev(DirMaster * );
	// FileInfo *     afc_dirmaster_last(DirMaster * );
	// unsigned long  afc_dirmaster_num_items(DirMaster * );
	// int            afc_dirmaster_before_first ( DirMaster * dm );
	// FileInfo *     afc_dirmaster_item(DirMaster *, unsigned long);
	// BOOL           afc_dirmaster_is_empty(DirMaster * );
	// unsigned long           afc_dirmaster_pos(DirMaster * );

	FileInfo *afc_dirmaster_del(DirMaster *);
	int afc_dirmaster_clear(DirMaster *);
	FileInfo *afc_dirmaster_sort(DirMaster *, int first_tag, ...);
	int afc_dirmaster_get_parent(DirMaster *dm, char *dest);
	FileInfo *afc_dirmaster_add_item(DirMaster *, char *, char *, struct stat *);
	FileInfo *afc_dirmaster_search(DirMaster *dm, char *name, BOOL no_case);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
