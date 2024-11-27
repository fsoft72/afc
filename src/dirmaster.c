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
	$VER: afc_dirmaster 2.00 - By Fabio Rotondo

	HISTORY:
		2.00 - Completely rewritten
*/

#include "dirmaster.h"

// {{{ docs
/*
@config
	TITLE:     DirMaster
	VERSION:   2.00
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
@endnode

@node quote

:Dark Helmet:
	I am your father's son's uncle's newphew's cousin's former roommate.

:Lone Star:
	So what does that make us?

:Dark Helmet:
	Absolutely nothing.

		Dark Helmet and Lone Star, *Spaceballs*
@endnode


@node intro
DirMaster is a class that's able to handle dirs and to store in memory their contents.
Main features of this class are:

- Ease of use: to scan a dir, simply call afc_dirmaster_scan_dir() function.
- Configurability: you can decide what info to retrieve and how.
- Sort can be done on different fields. See afc_dirmaster_sort().
- You can even add dir entries by hand.

To inizialize a new instance, simply call afc_dirmaster_new(), and to destroy it, call the
afc_dirmaster_delete().

To browse an already afc_dirmaster_scan_dir() scanned dir, you can use the usual APIs for double linked
list, with calls like afc_dirmaster_first(), afc_dirmaster_next() and so on
@endnode
*/
// }}}

static char *afc_dirmaster_size_bases[] = {
	"b", // Bytes
	"K", // KBytes
	"M", // Mega
	"G", // Giga
	"T", // Tera
	"Y"	 // Yota
};

static const char class_name[] = "DirMaster";

static char *afc_dirmaster_internal_date2string(char *str, unsigned long date, int format);
static char *afc_dirmaster_internal_mode2string(char *buf, unsigned long mode);
static void afc_dirmaster_internal_size2string(DirMaster *dm, char *dest, unsigned long size);
static int afc_dirmaster_internal_readd(DirMaster *dm, const char *path, int date_format);
static int afc_dirmaster_internal_sort_files(const void *a, const void *b);
static int afc_dirmaster_internal_parse_tags(DirMaster *dm, int first_tag, va_list tags);

// {{{ afc_dirmaster_new ()
/*
@node afc_dirmaster_new

		 NAME: afc_dirmaster_new () - Initializes a new Array object.

			 SYNOPSIS: DirMaster * afc_dirmaster_new ( afc )

		DESCRIPTION: Use this command to inizialize a DirMaster object.

		INPUT: - NONE

	RESULTS: an initialized Array structure.

			 SEE ALSO: afc_dirmaster_delete()
@endnode
*/
DirMaster *afc_dirmaster_new()
{
	TRY(DirMaster *)

	DirMaster *dm = afc_malloc(sizeof(DirMaster));

	if (dm == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "dirmaster", NULL);

	dm->magic = AFC_DIRMASTER_MAGIC;

	if ((dm->am = afc_array_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "array", NULL);

	if ((dm->current_dir = afc_string_new(1024)) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "curr_dir", NULL);

	/* DEFAULT VALUES */

	dm->date_format = DATEFORMAT_MM_DD_YYYY;
	dm->size_format = SIZEFORMAT_BYTES;
	dm->size_decimals = 2;

	dm->conv_date_modify = TRUE;
	dm->conv_date_access = FALSE;
	dm->conv_date_change = FALSE;

	RETURN(dm);

	EXCEPT
	afc_dirmaster_delete(dm);

	FINALLY

	ENDTRY
}
// }}}
// {{{ afc_dirmaster_delete ( dm )
/*
@node afc_dirmaster_delete

		NAME: afc_dirmaster_delete(dm) - Frees a DirMaster object

		SYNOPSIS: int afc_dirmaster_delete( DirMaster * dm )

	 DESCRIPTION: Use this command to free a DirMaster object.

		   INPUT: - dm - Pointer to a valid DirMaster instance

		 RESULTS: should return AFC_ERR_NO_ERROR.

		SEE ALSO: 	- afc_dirmaster_new()
			- afc_dirmaster_clear()
@endnode
*/
int _afc_dirmaster_delete(DirMaster *dm)
{
	int res;

	if ((res = afc_dirmaster_clear(dm)) != AFC_ERR_NO_ERROR)
		return (res);

	afc_array_delete(dm->am);
	afc_string_delete(dm->current_dir);

	afc_free(dm);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dirmaster_set_tags ( dm, first_tag, ... )
/*
@node afc_dirmaster_set_tags

		 NAME: afc_dirmaster_set_tags ( dm, first_tag, ... ) - Sets a list of tags

			 SYNOPSIS: int afc_dirmaster_set_tags ( DirMaster * dm, int first_tag, ... )

		DESCRIPTION: This functions sets a list of valid tags.
		 Please, see afc_dirmaster_set_tag() for a list of valid tag values.

		INPUT: - dm  - Pointer to a valid DirMaster instance
		 - ... - Tags to be set.

	RESULTS: should return AFC_ERR_NO_ERROR.

			 SEE ALSO: - afc_dirmaster_set_tag()
@endnode
*/
int _afc_dirmaster_set_tags(DirMaster *dm, int first_tag, ...)
{
	va_list tags;

	va_start(tags, first_tag);

	afc_dirmaster_internal_parse_tags(dm, first_tag, tags);

	va_end(tags);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dirmaster_set_tag ( dm, tag, value )
/*
@node afc_dirmaster_set_tag

		 NAME: afc_dirmaster_set_tag ( dm, tag, value ) - Set a tag

			 SYNOPSIS: int afc_dirmaster_set_tags ( DirMaster * dm, int tag, void * value )

		DESCRIPTION: This functions set a tag in the current instance.

		INPUT: - dm    - Pointer to a valid DirMaster instance
		 - tag   - Tag to set. Valid values are:
				 + AFC_DIRMASTER_TAG_DATE_FORMAT - The date format to use in rappresentation. Valid values are:
				   * DATEFORMAT_DD_MM_YYYY       - Standard European date format: day/month/year.
					 * DATEFORMAT_MM_DD_YYYY       - Standard American date format: month/day/year.
					 * DATEFORMAT_HH_MM            - Just hour and minutes are shown.
					 * DATEFORMAT_HH_MM_SS         - Just hour, minutes and seconds are shown.
							   * DATEFORMAT_DD_MM_YYYY_HH_MM - Standard European date plus hour and minutes.
					 * DATEFORMAT_MM_DD_YYYY_HH_MM - Standard American date plus hour and minutes.

				 + AFC_DIRMASTER_TAG_SIZE_FORMAT - The format to be used to show file and dirs size. Valid values are:
					 * SIZEFORMAT_BYTES        - The file size is shown in bytes
					 * SIZEFORMAT_HUMAN        - The file size is shown in human readable way, with 1K corresponding to
														   1024 bytes
					 * SIZEFORMAT_HUMAN_1000   - The file size is shown in human readable way, with 1K corresponding to
														   1000 bytes.

				 + AFC_DIRMASTER_TAG_SIZE_DECIMALS - The number of decimals to use in file size. This tag has effect
																 only if the AFC_DIRMASTER_TAG_SIZE_FORMAT is set to something
																 different from SIZEFORMAT_BYTES.

				 + AFC_DIRMASTER_TAG_CONV_DATE_MODIFY - A boolean value. If it is set to TRUE, then the file modification
																	date will be converted into the string specified by DATEFORMAT_*
																	values.

				 + AFC_DIRMASTER_TAG_CONV_DATE_ACCESS - A boolean value. If it is set to TRUE, then the file access
																	date will be converted into the string specified by DATEFORMAT_*
																	values.

				 + AFC_DIRMASTER_TAG_CONV_DATE_CHANGE - A boolean value. If it is set to TRUE, then the file change
																	date will be converted into the string specified by DATEFORMAT_*
																	values.

				 + AFC_DIRMASTER_TAG_CONV_USER  - A boolean value. If it is set to TRUE, then the file user id will
									be converted to the string rappresenting the user in the system.

				 + AFC_DIRMASTER_TAG_CONV_GROUP  - A boolean value. If it is set to TRUE, then the file group id will
									be converted to the string rappresenting the group in the system.

				 + AFC_DIRMASTER_TAG_CONV_MOVE  - A boolean value. If it is set to TRUE, then the file access mode will
									be converted to the string rappresenting it (eg. '-rwxr-xr-x' )

				 + AFC_DIRMASTER_TAG_SORT_FIELD - The field that will be used when sorting. Valid values are:
						* FINFO_NAME - Sort by file name
						* FINFO_MODE - Sort by file mode
					  * FINFO_USER - Sort by user id
						* FINFO_GROUP - Sort by group id
						* FINFO_DATE_ACCESS - Sort by date access
						* FINFO_DATE_MODIFY - Sort by date modification
						* FINFO_DATE_CHANGE - Sort by date change
						* FINFO_SIZE        - Sort by file size

				 + AFC_DIRMASTER_TAG_SORT_CASE_INSENSITIVE - A boolean value. If TRUE, the next sort will not consider
																		 case sense while comparing strings.

				 + AFC_DIRMASTER_TAG_SORT_INVERTED         - A boolean value. If TRUE, the next sort will return an
											 inverted list, eg. from Z to A and not A-Z.



		 - value - Value to set

	RESULTS: should return AFC_ERR_NO_ERROR.

			 SEE ALSO: - afc_dirmaster_set_tags()
@endnode
*/
int afc_dirmaster_set_tag(DirMaster *dm, int tag, void *val)
{

	switch (tag)
	{
	case AFC_DIRMASTER_TAG_DATE_FORMAT:
		dm->date_format = (int)(long)val;
		break;

	case AFC_DIRMASTER_TAG_SIZE_FORMAT:
		dm->size_format = (int)(long)val;
		break;

	case AFC_DIRMASTER_TAG_SIZE_DECIMALS:
		dm->size_decimals = (int)(long)val;
		break;

	case AFC_DIRMASTER_TAG_CONV_DATE_MODIFY:
		dm->conv_date_modify = (int)(long)val;
		break;

	case AFC_DIRMASTER_TAG_CONV_DATE_ACCESS:
		dm->conv_date_access = (int)(long)val;
		break;

	case AFC_DIRMASTER_TAG_CONV_DATE_CHANGE:
		dm->conv_date_change = (int)(long)val;
		break;

	case AFC_DIRMASTER_TAG_CONV_USER:
		dm->conv_user = (int)(long)val;
		break;

	case AFC_DIRMASTER_TAG_CONV_MODE:
		dm->conv_mode = (int)(long)val;
		break;

	case AFC_DIRMASTER_TAG_CONV_GROUP:
		dm->conv_group = (int)(long)val;
		break;

	case AFC_DIRMASTER_TAG_SORT_FIELD:
		dm->isi.field = (int)(long)val;
		break;

	case AFC_DIRMASTER_TAG_SORT_CASE_INSENSITIVE:
		dm->isi.case_insensitive = (int)(long)val;
		break;

	case AFC_DIRMASTER_TAG_SORT_INVERTED:
		dm->isi.inverted = (int)(long)val;
		break;
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dirmaster_scan_dir ( dm, dirname )
/*
@node afc_dirmaster_scan_dir

		 NAME: afc_dirmaster_scan_dir(dm, dirname) - Reads a dir inside the DirMaster

			 SYNOPSIS: int afc_dirmaster_scan_dir( DirMaster * dm, const char * dirname )

		DESCRIPTION: This function reads the contents of a dir inside a DirMaster object.

		INPUT: - dm       - Pointer to a valid DirMaster instance
				   - dirname  - Directory name (in full path) to scan

	RESULTS: should return AFC_ERR_NO_ERROR

			 SEE ALSO: - afc_dirmaster_clear()
@endnode
*/
int afc_dirmaster_scan_dir(DirMaster *dm, const char *dirname)
{
	FILE *f;

	afc_dirmaster_clear(dm);

	if (!(f = fopen(dirname, "r")))
		return (AFC_LOG(AFC_LOG_WARNING, AFC_DIRMASTER_ERR_DIR_NOT_FOUND, "Dir not found", dirname));

	fclose(f);

	afc_dirmaster_internal_readd(dm, dirname, dm->date_format);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dirmaster_obj ( dm )
/*
@node afc_dirmaster_obj

		 NAME: afc_dirmaster_obj (dm) - Returns the FileInfo structure of the current item

			 SYNOPSIS: FileInfo * afc_dirmaster_obj ( DirMaster * dm )

		DESCRIPTION: This function returns a pointer to the FileInfo structure of the current item in the list.
				   If the DirMaster is empty, it may be NULL.

		INPUT: - dm       - Pointer to a valid DirMaster instance

	RESULTS: Pointer to a valid FileInfo structure. It may be NULL.

			 SEE ALSO: - afc_array_obj()
@endnode
*/
FileInfo *afc_dirmaster_obj(DirMaster *dm)
{
	return ((FileInfo *)afc_array_obj(dm->am));
}
// }}}
// {{{ afc_dirmaster_is_empty ( dm )
/*
@node afc_dirmaster_is_empty

		 NAME: afc_dirmaster_is_empty (dm) - Checks if the DirMaster is empty

			 SYNOPSIS: short afc_dirmaster_is_empty ( DirMaster * dm )

		DESCRIPTION: This function checks whether the current DirMaster list contains some data
				   or not.


		INPUT: - dm       - Pointer to a valid DirMaster instance

	RESULTS: - TRUE     - The DirMaster is empty
		 - FALSE    - The DirMaster contains some data

			 SEE ALSO: - afc_array_is_empty()
@endnode
*/
/*
short afc_dirmaster_is_empty(DirMaster * dm)
{
	return ( afc_array_is_empty ( dm->am ) );
}
*/
// }}}
// {{{ afc_dirmaster_first ( dm )
/*
@node afc_dirmaster_first

		 NAME: afc_dirmaster_first (dm) - Gets the first element in the list

			 SYNOPSIS: FileInfo * afc_dirmaster_first ( DirMaster * dm )

		DESCRIPTION: This function returns the first element inside the current DirMaster.
		 If the list is empty, then a NULL value is returned.

		INPUT: - dm       - Pointer to a valid DirMaster instance

	RESULTS: a valid pointer to the first FileInfo structre in the list, or NULL if
				   the DirMaster is empty.

			 SEE ALSO: - afc_dirmaster_next()
				   - afc_dirmaster_last()
				   - afc_array_first()
@endnode
*/
/*
FileInfo * afc_dirmaster_first(DirMaster * dm)
{
	return ((FileInfo *)afc_array_first(dm->am));
}
*/
// }}}
// {{{ afc_dirmaster_next ( dm )
/*
@node afc_dirmaster_next

		 NAME: afc_dirmaster_next (dm) - Gets the next element in the list

			 SYNOPSIS: FileInfo * afc_dirmaster_next ( DirMaster * dm )

		DESCRIPTION: This function returns the next element in the list.

		INPUT: - dm       - Pointer to a valid DirMaster instance

	RESULTS: a valid pointer to the FileInfo struct in the list, or NULL if
				   the DirMaster is empty or you already reached the end of the list.

			 SEE ALSO: - afc_dirmaster_first()
				   - afc_dirmaster_last()
				   - afc_dirmaster_prev()
				   - afc_array_next()
@endnode
*/
/*
FileInfo * afc_dirmaster_next(DirMaster * dm)
{
	return ((FileInfo *)afc_array_next(dm->am));
}
*/
// }}}
// {{{ afc_dirmaster_prev ( dm )
/*
@node afc_dirmaster_prev

		 NAME: afc_dirmaster_prev (dm) - Gets the previous element in the list

			 SYNOPSIS: FileInfo * afc_dirmaster_prev ( DirMaster * dm )

		DESCRIPTION: This function returns the previous element in the list.

		INPUT: - dm       - Pointer to a valid DirMaster instance

	RESULTS: a valid pointer to the FileInfo struct in the list, or NULL if
				   the DirMaster is empty or you already reached the start of the list.

			 SEE ALSO: - afc_dirmaster_first()
				   - afc_dirmaster_last()
				   - afc_dirmaster_next()
				   - afc_array_prev()
@endnode
*/
/*
FileInfo * afc_dirmaster_prev(DirMaster * dm)
{
	return ((FileInfo *)afc_array_prev(dm->am));
}
*/
// }}}
// {{{ afc_dirmaster_del ( dm )
/*
@node afc_dirmaster_del

		 NAME: afc_dirmaster_del (dm) - Deletes the current item from the list.

			 SYNOPSIS: FileInfo * afc_dirmaster_del ( DirMaster * dm )

		DESCRIPTION: This function deletes the current item
				   a maximum of eight items on the internal stack.

		INPUT: - dm       - Pointer to a valid DirMaster instance
				   - autopos  - It is a boolean value:
				+ TRUE - The item is popped from the stack and become the current item in the list.
								  + FALSE - The item is just popped from the stack, but the current item will not change.

	RESULTS: a valid pointer to a FileInfo structure, or NULL if the stack is empty.

			 SEE ALSO:

@endnode
*/
FileInfo *afc_dirmaster_del(DirMaster *dm)
{
	FileInfo *s = NULL;

	if (afc_array_is_empty(dm->am))
		return (NULL);

	s = afc_dirmaster_obj(dm);
	afc_free(s->st);
	afc_free(s);

	return ((FileInfo *)afc_array_del(dm->am));
}
// }}}
// {{{ afc_dirmaster_last ( dm )
/*
@node afc_dirmaster_last

		 NAME: afc_dirmaster_last (dm) - Get the last element in the list

			 SYNOPSIS: FileInfo * afc_dirmaster_last ( DirMaster * dm )

		DESCRIPTION: This function returns the last element inside the current DirMaster.
		 If the list is empty, then a NULL value is returned.

		INPUT: - dm       - Pointer to a valid DirMaster instance

	RESULTS: a valid pointer to the last FileInfo structre in the list, or NULL if
				   the DirMaster is empty.

			 SEE ALSO: - afc_dirmaster_next()
				   - afc_dirmaster_last()
				   - afc_array_last()
@endnode
*/
/*
FileInfo * afc_dirmaster_last(DirMaster * dm)
{
	 return ((FileInfo *)afc_array_last(dm->am));
}
*/
// }}}
// {{{ afc_dirmaster_clear ( dm )
/*
@node afc_dirmaster_clear

		 NAME: afc_dirmaster_clear (dm) - Completely clears this DirMaster

			 SYNOPSIS: int afc_dirmaster_clear ( DirMaster * dm )

		DESCRIPTION: This function completely free this DirMaster instance.

		INPUT: - dm       - Pointer to a valid DirMaster instance

	RESULTS: should be AFC_ERR_NO_ERROR.

			 SEE ALSO:

@endnode
*/
int afc_dirmaster_clear(DirMaster *dm)
{
	FileInfo *s;

	if (dm == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));
	if (dm->magic != AFC_DIRMASTER_MAGIC)
		return (AFC_LOG_FAST(AFC_ERR_INVALID_POINTER));

	s = (FileInfo *)afc_array_first(dm->am);

	while (s != NULL)
	{
		afc_free(s->st);
		afc_free(s);
		s = (FileInfo *)afc_array_next(dm->am);
	}

	afc_array_clear(dm->am);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dirmaster_len ( dm )
/*
@node afc_dirmaster_len

		 NAME: afc_dirmaster_len (dm) - Return the number of items in list

			 SYNOPSIS: unsigned long afc_dirmaster_len ( DirMaster * dm )

		DESCRIPTION: This function returns the number of items in the list.

		INPUT: - dm       - Pointer to a valid DirMaster instance

	RESULTS: - the number of elements in list, or 0 if it is empty

			 SEE ALSO: - afc_array_len()

@endnode
*/
/*
unsigned long afc_dirmaster_len(DirMaster * dm)
{
	return (afc_array_len(dm->am));
}
*/
// }}}
// {{{ afc_dirmaster_item ( dm, n )
/*
@node afc_dirmaster_item

		 NAME: afc_dirmaster_item ( dm, n ) - Return the nth element

			 SYNOPSIS: FileInfo * afc_dirmaster_len ( DirMaster * dm, unsigned long n )

		DESCRIPTION: This function returns the desired /n/ item.

		INPUT: - dm       - Pointer to a valid DirMaster instance
		 - n        - The ordinal value rappresenting the element you want to
								retrieve. Count starts from 0.

	RESULTS: - the /FileInfo/ structure you wanted or NULL in case of errors.

			 SEE ALSO: - afc_array_item()

@endnode
*/
/*
FileInfo * afc_dirmaster_item(DirMaster * dm, unsigned long n)
{
	return ((FileInfo *)afc_array_item(dm->am, n));
}
*/
// }}}
// {{{ afc_dirmaster_search ( dm, name, no_case )
/*
@node afc_dirmaster_search

		 NAME: afc_dirmaster_search ( dm, name, no_case ) - Searches for a file name in the current DirMaster list

			 SYNOPSIS: FileInfo * afc_dirmaster_search ( DirMaster * dm, char * name, short no_case )

		DESCRIPTION: This function scans all DirMaster list in search of the /name/ provided. Search can be case sensitive
				   or not, depending on the value of /no_case/. This function does not support patterns.

		INPUT: - dm       - Pointer to a valid DirMaster instance
		 - name     - The name of the file you are looking for,
		 - no_case  - Flag T/F. If TRUE, case will not matter.

	RESULTS: - the /FileInfo/ structure you wanted or NULL in case of errors.

			 SEE ALSO:

@endnode
*/
FileInfo *afc_dirmaster_search(DirMaster *dm, char *name, BOOL no_case)
{
	FileInfo *fi;

	fi = afc_array_first(dm->am);

	while (fi)
	{
		if (no_case)
		{
			if (strcasecmp(fi->name, name) == 0)
				return (fi);
		}
		else
		{
			if (strcmp(fi->name, name) == 0)
				return (fi);
		}

		fi = afc_array_next(dm->am);
	}

	return (NULL);
}
// }}}
// {{{ afc_dirmaster_get_parent ( dm, dest )
/*
@node afc_dirmaster_get_parent

		 NAME: afc_dirmaster_get_parent ( dm, dest ) - Returns the parent of current dir

			 SYNOPSIS: int afc_dirmaster_get_parent ( DirMaster * dm, char * dest )

		DESCRIPTION: This function returns the parent of the current directory. If current dir is not set,
		 returns "/".

		INPUT: - dm       - Pointer to a valid DirMaster instance
		 - dest     - The destination string where to write the parent path. Must be an AFC string.

	RESULTS: - should be AFC_ERR_NO_ERROR.

		NOTES: - /dest/ string must be an AFC string allocated with afc_string_new()

			 SEE ALSO:

@endnode
*/
int afc_dirmaster_get_parent(DirMaster *dm, char *dest)
{
	char *str;
	char *x;
	int count = 0;

	if (afc_string_len(dm->current_dir) < 2)
	{
		afc_string_copy(dest, "/", ALL);
		return (AFC_ERR_NO_ERROR);
	}

	str = afc_string_dup(dm->current_dir);

	x = str + afc_string_len(str);

	while ((x >= str))
	{
		if (*x == '/')
			count++;

		if (count == 2)
			break;
		x--;
	}

	if (x != str)
		afc_string_copy(dest, str, x - str);
	else
		afc_string_copy(dest, "/", ALL);

	afc_string_delete(str);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dirmaster_sort ( dm, int first_tag, ... )
/*
@node afc_dirmaster_sort

		 NAME: afc_dirmaster_sort ( dm, ... ) - Sorts the DirMaster list

			 SYNOPSIS: FileInfo * afc_dirmaster_sort ( DirMaster * dm,  ... )

		DESCRIPTION: This function sorts the DirMaster list.

		INPUT: - dm       - Pointer to a valid DirMaster instance
		 - ...      - Tags to be parsed before the sort. Please refer to
								afc_dirmaster_set_tag() for a list of valid tags.

	RESULTS: - the (new) first element in the list.

		NOTES: - Remember to finish the tag list with AFC_TAG_END.

			 SEE ALSO: - afc_array_sort()

@endnode
*/
FileInfo *afc_dirmaster_sort(DirMaster *dm, int first_tag, ...)
{
	va_list tags;

	va_start(tags, first_tag);

	afc_dirmaster_internal_parse_tags(dm, first_tag, tags);

	va_end(tags);

	return (afc_array_sort(dm->am, afc_dirmaster_internal_sort_files));
}
// }}}
// {{{ afc_dirmaster_add_item ( dm, fullname, file_name, descr )
/*
@node afc_dirmaster_add_item

		 NAME: afc_dirmaster_add_item ( dm, fullname, file_name, descr ) - Adds a new item to the DirMaster

			 SYNOPSIS: FileInfo * afc_dirmaster_add_item ( DirMaster * dm, char * fullname, char * file_name, struct stat * descr )

		DESCRIPTION: This function adds a new file/dir to the current DirMaster list. It is a very low level function that should only
				   be used in special cases.

		INPUT: - dm        - Pointer to a valid DirMaster instance
		 - fullname  - The complite path and file name of the file/dir you intend to add.
		 - file_name - Just the file name of the file you intend to add.
		 - descr     - The stat structure filled with the info of the file you intend to add.
								 This structure must be filled before calling this function.

	RESULTS: - the new FileInfo just created or NULL in case of errors.

			 SEE ALSO:

@endnode
*/
FileInfo *afc_dirmaster_add_item(DirMaster *dm, char *fullname, char *fname, struct stat *descr)
{
	FileInfo *info;
	struct passwd *pass;
	struct group *grp;
	struct stat lnk;	   // This stat is used only if the stat passed is a symlink to know the kind of item linked
	char tmpbuf[NAME_MAX]; // Flawfinder: ignore
	int lnklen;

	info = afc_malloc(sizeof(FileInfo));

	if (info == NULL)
		return (NULL);

	if ((info->st = afc_malloc(sizeof(struct stat))) == NULL)
	{
		afc_free(info);
		return (NULL);
	}

	memcpy(info->st, descr, sizeof(struct stat));

	strncpy(info->name, fname, NAME_MAX);

	if (S_ISDIR(descr->st_mode))
	{
		info->kind = FINFO_KIND_DIR;
	}
	else
	{
		if (S_ISLNK(descr->st_mode))
		{
			stat(fullname, &lnk);
			if (S_ISDIR(lnk.st_mode))
				info->kind = FINFO_KIND_LINK | FINFO_KIND_DIR;
			else
				info->kind = FINFO_KIND_LINK | FINFO_KIND_FILE;

			lnklen = readlink(fullname, tmpbuf, NAME_MAX);
			tmpbuf[lnklen] = 0;

			strcat(info->name, " -> ");
			strcat(info->name, tmpbuf);
		}
		else
			info->kind = FINFO_KIND_FILE;
	}

	info->dm = dm;

	if (fname[0] == '.')
		info->hidden = TRUE;
	else
		info->hidden = FALSE;

	info->selected = FALSE;

	info->size = (unsigned long)descr->st_size;

	afc_dirmaster_internal_size2string(dm, info->csize, descr->st_size);

	if (dm->conv_date_access)
		afc_dirmaster_internal_date2string(info->caccess, descr->st_atime, dm->date_format);
	if (dm->conv_date_modify)
		afc_dirmaster_internal_date2string(info->cmodify, descr->st_mtime, dm->date_format);
	if (dm->conv_date_change)
		afc_dirmaster_internal_date2string(info->cchange, descr->st_ctime, dm->date_format);

	if (dm->conv_mode)
		afc_dirmaster_internal_mode2string(info->cmode, descr->st_mode);

	if (dm->conv_user)
	{
		pass = getpwuid(descr->st_uid);
		if (pass != NULL)
			strncpy(info->cuser, pass->pw_name, 10);
		else
			snprintf(info->cuser, 10, "%d", descr->st_uid);
	}

	if (dm->conv_group)
	{
		grp = getgrgid(descr->st_gid);
		if (grp)
			strncpy(info->cgroup, grp->gr_name, 10);
		else
			snprintf(info->cgroup, 10, "%d", descr->st_gid);
	}

	return (info);
}
// }}}
// {{{ afc_dirmaster_before_first ( dm ) ***************
/*
int afc_dirmaster_before_first ( DirMaster * dm )
{
	return ( afc_array_before_first ( dm->am ) );
}
*/
// }}}

/**
	 PRIVATE FUNCTIONS
*/
// {{{ afc_dirmaster_internal_date2string ( str, date, format )
static char *afc_dirmaster_internal_date2string(char *str, unsigned long date, int format)
{
	struct tm *time;

	time = localtime((const time_t *)&date);

	switch (format)
	{
	case (DATEFORMAT_DD_MM_YYYY):
		sprintf(str, "%2.2d-%2.2d-%4d", time->tm_mday, time->tm_mon + 1, 1900 + time->tm_year);
		break;
	case (DATEFORMAT_MM_DD_YYYY):
		sprintf(str, "%2.2d-%2.2d-%4d", time->tm_mon + 1, time->tm_mday, 1900 + time->tm_year);
		break;
	case (DATEFORMAT_HH_MM):
		sprintf(str, "%2.2d:%2.2d", time->tm_hour, time->tm_min);
		break;
	case (DATEFORMAT_HH_MM_SS):
		sprintf(str, "%2.2d:%2.2d.%2.2d", time->tm_hour, time->tm_min, time->tm_sec);
		break;

	case (DATEFORMAT_DD_MM_YYYY_HH_MM):
		sprintf(str, "%2.2d-%2.2d-%4d %2.2d:%2.2d", time->tm_mday, time->tm_mon + 1, 1900 + time->tm_year, time->tm_hour, time->tm_min);
		break;

	case (DATEFORMAT_MM_DD_YYYY_HH_MM):
		sprintf(str, "%2.2d-%2.2d-%4d %2.2d:%2.2d", time->tm_mon + 1, time->tm_mday, 1900 + time->tm_year, time->tm_hour, time->tm_min);
		break;

	default:
		strcpy(str, "#undefined");
		break;
	}

	return (str);
}
// }}}

/*
	 NOTE: this routine has been adapted by the one found in gentoo file manager
		   written by Emil Brink
*/
// {{{ afc_dirmaster_internal_mode2string ( buf, mode )
static char *afc_dirmaster_internal_mode2string(char *buf, unsigned long mode)
{
	char *grp[] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};
	int u, g, o;

	u = (mode & S_IRWXU) >> 6;
	g = (mode & S_IRWXG) >> 3;
	o = (mode & S_IRWXO);

	snprintf(buf, 16, "-%s%s%s", grp[u], grp[g], grp[o]);

	/* Set the left-most character according to the file's intrinsic type. */
	if (S_ISLNK(mode))
		buf[0] = 'l';
	else if (S_ISDIR(mode))
		buf[0] = 'd';
	else if (S_ISBLK(mode))
		buf[0] = 'b';
	else if (S_ISCHR(mode))
		buf[0] = 'c';
	else if (S_ISFIFO(mode))
		buf[0] = 'p';
	else if (S_ISSOCK(mode))
		buf[0] = 's'; /* This is just a guess... */

	if (mode & S_ISVTX)
		buf[9] = (buf[9] == '-') ? 'T' : 't'; /* Sticky bit set? This is not POSIX... */
	if (mode & S_ISGID)
		buf[6] = (buf[6] == '-') ? 'S' : 's'; /* Set GID bit set? */
	if (mode & S_ISUID)
		buf[3] = (buf[3] == '-') ? 'S' : 's'; /* Set UID bit set? */

	return (buf);
}
// }}}
// {{{ afc_dirmaster_internal_size2string ( dm, dest, size )
static void afc_dirmaster_internal_size2string(DirMaster *dm, char *dest, unsigned long size)
{
	int res = 0;
	int count = 0;
	int rsize = 0;
	unsigned long base = 0;
	unsigned long rbase = 0;
	int the_base = 1024;
	char buf[20];

	if (dm->size_format == SIZEFORMAT_BYTES)
	{
		snprintf(dest, 10, "%lu %s", size, afc_dirmaster_size_bases[0]);
		return;
	}

	if (dm->size_format == SIZEFORMAT_HUMAN_1000)
		the_base = 1000;

	count = 0;
	base = the_base;

	res = (size / base);

	while (res > 0)
	{
		rbase = base;
		rsize = res;
		base = base * the_base;
		res = (size / base);
		count++;
	}

	if (count == 0)
		snprintf(dest, 10, "%lu b", size);
	else
	{
		if (dm->size_decimals)
		{
			snprintf(buf, 20, "%lu", size - (rbase * rsize));
			buf[dm->size_decimals] = 0;
			snprintf(dest, 10, "%d.%s %s", rsize, buf, afc_dirmaster_size_bases[count]);
		}
		else
		{
			snprintf(dest, 10, "%d %s", rsize, afc_dirmaster_size_bases[count]);
		}
	}
}
// }}}
// {{{ afc_dirmaster_internal_readd ( dm, path, date_format )
static int afc_dirmaster_internal_readd(DirMaster *dm, const char *path, int date_format)
{
	DIR *dir;
	struct dirent *file;
	struct stat descr;
	char dirname[255];
	char fullname[255];
	FileInfo *info;

	if ((dir = opendir(path)) == NULL)
		return (errno);

	strncpy(dirname, path, 255);

	if (dirname[strlen(dirname) - 1] != '/')
		strcat(dirname, "/");

	afc_string_copy(dm->current_dir, dirname, ALL);

	while ((file = readdir(dir)) != NULL)
	{
		if ((strcmp(file->d_name, "..") == 0) || (strcmp(file->d_name, ".") == 0))
			continue;

		snprintf(fullname, 255, "%s%s", dirname, file->d_name);

		if (lstat(fullname, &descr) == 0)
		{
			if ((info = afc_dirmaster_add_item(dm, fullname, file->d_name, &descr)) == NULL)
				return (AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "dirmaster_add_item"));
			afc_array_add(dm->am, info, AFC_ARRAY_ADD_TAIL);
		}
		else
		{
			fprintf(stderr, "ERROR: stat() failed on: %s\n", fullname);
		}
	}

	closedir(dir);

	return (0);
}
// }}}
// {{{ afc_dirmaster_internal_sort_files ( a, b )
static int afc_dirmaster_internal_sort_files(const void *a, const void *b)
{
	FileInfo *fa = (FileInfo *)(*(FileInfo **)a);
	FileInfo *fb = (FileInfo *)(*(FileInfo **)b);
	struct _internalSortInfo *isi = &fa->dm->isi;

	char *ca = NULL;
	char *cb = NULL;
	char *aa = NULL;
	char *bb = NULL;
	unsigned long va = 0, vb = 0;
	short str = FALSE;
	signed long res;

	switch (isi->field)
	{
	case FINFO_NAME:
		ca = fa->name;
		cb = fb->name;
		str = TRUE;
		break;
	case FINFO_MODE:
		if (fa->dm->conv_mode)
		{
			ca = fa->cmode;
			cb = fb->cmode;
			str = TRUE;
		}
		else
		{
			va = fa->st->st_mode;
			vb = fb->st->st_mode;
		}
		break;
	case FINFO_USER:
		if (fa->dm->conv_user)
		{
			ca = fa->cuser;
			cb = fb->cuser;
			str = TRUE;
		}
		else
		{
			va = fa->st->st_uid;
			vb = fb->st->st_uid;
		}
		break;
	case FINFO_GROUP:
		if (fa->dm->conv_group)
		{
			ca = fa->cgroup;
			cb = fb->cgroup;
			str = TRUE;
		}
		else
		{
			va = fa->st->st_gid;
			vb = fb->st->st_gid;
		}
		break;
	case FINFO_DATE_ACCESS:
		va = fa->st->st_atime; // date_access;
		vb = fb->st->st_atime; // date_access;
		break;
	case FINFO_DATE_MODIFY:
		va = fa->st->st_mtime; // date_modify;
		vb = fb->st->st_mtime; // date_modify;
		break;
	case FINFO_DATE_CHANGE:
		va = fa->st->st_ctime; // date_change;
		vb = fb->st->st_ctime; // date_change;
		break;
	case FINFO_SIZE:
		va = fa->size;
		vb = fb->size;
		break;
	}

	if (str)
	{
		if (isi->case_insensitive)
		{
			aa = afc_string_new(strlen(ca));
			bb = afc_string_new(strlen(cb));

			afc_string_copy(aa, ca, ALL);
			afc_string_copy(bb, cb, ALL);
			afc_string_upper(aa);
			afc_string_upper(bb);
			res = -afc_string_comp(aa, bb, ALL);

			afc_string_delete(aa);
			afc_string_delete(bb);
		}
		else
			res = -afc_string_comp(ca, cb, ALL);
	}
	else
	{
		if (va > vb)
			res = 1;
		else
			res = -1;
	}

	if (isi->inverted)
		res = -res;

	return (res);
}
// }}}
// {{{ afc_dirmaster_internal_parse_tags ( dm, first_tag, tags )
static int afc_dirmaster_internal_parse_tags(DirMaster *dm, int first_tag, va_list tags)
{
	int tag;
	void *val;

	tag = first_tag;

	while (tag)
	{
		val = va_arg(tags, void *);

		afc_dirmaster_set_tag(dm, tag, val);

		tag = va_arg(tags, int);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
int main()
{
	struct afc_dirmaster *dirm = afc_dirmaster_new();
	FileInfo *f;

	dirm->size_format = SIZEFORMAT_HUMAN_1000;
	dirm->size_decimals = 3;

	afc_dirmaster_scan_dir(dirm, "/tmp");

	afc_dirmaster_sort(dirm, AFC_DIRMASTER_TAG_SORT_INVERTED, TRUE, AFC_TAG_END);

	f = afc_dirmaster_first(dirm);
	while (f)
	{
		printf("File Name: %s - Size: (%lu) %s - %s\n", f->name, f->size, f->csize, f->cmode);

		f = afc_dirmaster_next(dirm);
	}

	afc_dirmaster_delete(dirm);

	return (0);
}
// }}}
#endif
