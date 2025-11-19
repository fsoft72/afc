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

// {{{ docs
/*
@config
	TITLE:     FileOperations
	VERSION:   1.00
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
@endnode

@node quote
*Yesterday I was a dog. Today I'm a dog. Tomorrow I'll probably still be a dog. Sigh! There's so little hope for advancement*

	Snoopy
@endnode

@node intro
FileOperations is a very special class that ease you the heavy task of system file handling.

With this class, you can perform the most common file operations tasks with just a single call very
close to the standard shell's commands.

Main features are:

- Ease of use: all functions closely resemble shell's functions.
- Configurability: using afc_fileops_set_tags() and afc_fileops_set_tag() you can change class behaviour in special situations.
- Power: function like afc_fileops_copy() or afc_fileops_del() are recursive, moving whole dir trees with a single function call.

To create a FileOperations instance, simply call afc_fileops_new(), and delete it when you have finished using afc_fileops_delete().
Since the task of creating and disposing this class is somewhat demaninding, please consider the opportunity of keeping an already
allocated instance around if you think you could need it later.

Once you have a FileOperations instance working, you can do a lot of things. For example you can:

- Check whether a file/dir already exists with afc_fileops_exists() or afc_fileops_exists_full().
- Delete a file/dir with afc_fileops_del().
- Copy a file/dir with afc_fileops_copy().
- Move or rename a file/dir with afc_fileops_move().
- Create symlinks with afc_fileops_link().
- Alter file/dir permissions with afc_fileops_chmod().
- Alter file/dir user and group ids with afc_fileops_chown().
- Alter file/dir times with afc_fileops_utime().

There are some things you can configure, like whether to return or not an error code in case an afc_fileops_chmod() fails: see
afc_fileops_set_tags() and afc_fileops_set_tag() for more info.

@endnode
*/
// }}}

#include "fileops.h"

static const char class_name[] = "FileOperations";

static int afc_fileops_internal_scan_dir(FileOperations *fo, char *path, int action_file(FileOperations *, struct stat *, char *, char *, char *, void *), int action_dir(FileOperations *, struct stat *, char *, char *, char *, void *), int action_end_dir(FileOperations *, char *, void *), void *info);
static int afc_fileops_internal_move_dir(FileOperations *fo, struct stat *descr, char *fulldir, char *path, char *dirname, void *info);
static int afc_fileops_internal_move_file(FileOperations *fo, struct stat *descr, char *fullname, char *path, char *filename, void *info);
static int afc_fileops_internal_del_dir(FileOperations *fo, struct stat *descr, char *fulldir, char *path, char *dirname, void *info);
static int afc_fileops_internal_del_file(FileOperations *fo, struct stat *descr, char *fullname, char *path, char *filename, void *info);
static int afc_fileops_internal_copy_new_dir(FileOperations *fo, struct stat *descr, char *fulldir, char *path, char *dirname, void *info);
static int afc_fileops_internal_copy_new_file(FileOperations *fo, struct stat *descr, char *fullname, char *path, char *filename, void *info);
static FOCopy *afc_fileops_internal_copy_new(void);
static void afc_fileops_internal_copy_delete(FOCopy *foc);
static int afc_fileops_copy_internal_set_file_stat(FileOperations *fo, struct stat *st);
static int afc_fileops_copy_internal_copy(FileOperations *fo);
#ifndef MINGW
static int afc_fileops_internal_physical_move(FileOperations *fo, char *source, char *dest);
static int afc_fileops_internal_parse_tags(FileOperations *fo, int first_tag, va_list tags);
#endif // MINGW

// {{{ afc_fileops_new ()
/*
@node afc_fileops_new

		 NAME: afc_fileops_new () - Initializes a new FileOperations object.

			 SYNOPSIS: FileOperations * afc_fileops_new ()

		DESCRIPTION: Use this command to inizialize a FileOperations object.

		INPUT: NONE

	RESULTS: an initialized FileOperations structure.

			 SEE ALSO: afc_fileops_delete()
@endnode
*/
FileOperations *afc_fileops_new()
{
	FileOperations *fo = (FileOperations *)afc_malloc(sizeof(FileOperations));

	if (fo == NULL)
		return (NULL);

	// Try to create an instance of the "inner class" "Fileops Internal Copy"
	if ((fo->foc = afc_fileops_internal_copy_new()) == NULL)
	{
		afc_fileops_delete(fo);
		return (NULL);
	}

	// Set default blocking errors

	fo->block_chown = FALSE;		// Chown is not blocking
	fo->block_chmod = TRUE;			// Chmod is blocking
	fo->block_utime = TRUE;			// setutime() is blocking
	fo->block_mkdir_exists = FALSE; // mkdir is not blocking

	// Set no default value for the following vars

	fo->uid = -1; /* No default value is set for these flags */
	fo->gid = -1; /* This means that the same values of the file being copied will be used */
	fo->mode = -1;

	return (fo);
}
// }}}
// {{{ afc_fileops_delete ( fo )
/*
@node afc_fileops_delete

		 NAME: afc_fileops_delete ( fo ) - Dispose a FileOperations object

			 SYNOPSIS: int afc_fileops_delete ( FileOperations * fo )

		DESCRIPTION: This function frees an already alloc'd FileOperations.

		INPUT: - fo	- Pointer to a valid FileOperations class.

	RESULTS: the return code should be AFC_ERR_NO_ERROR

			 SEE ALSO: - afc_fileops_new()
@endnode
*/
int _afc_fileops_delete(FileOperations *fo)
{
	if (fo == NULL)
		return (AFC_ERR_NO_ERROR);
	if (fo->foc)
		afc_fileops_internal_copy_delete(fo->foc);

	afc_free(fo);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_clear ( fo )
int afc_fileops_clear(FileOperations *fo)
{
	return AFC_ERR_NO_ERROR;
}
// }}}

#ifndef MINGW
// {{{ afc_fileops_set_tags ( fo, first_tag, ... )
/*
@node afc_fileops_set_tags

		 NAME: afc_fileops_set_tags ( fo, first_tag, ... ) - Sets a group of tags

			 SYNOPSIS: int afc_fileops_set_tags ( FileOperations * fo, int first_tag, ... )

		DESCRIPTION: This function sets a group of tags. To get a list of valid tags, please
				   consult afc_fileops_set_tag().

		INPUT: - fo	 - Pointer to a valid FileOperations class.
				   - first_tag - First tag to be set
		 - ... - List of tags and values

	RESULTS: the return code should be AFC_ERR_NO_ERROR

		NOTES: - remember to end the tag list with AFC_TAG_END

	NOT IN: Win32

			 SEE ALSO: - afc_fileops_set_tag()
@endnode
*/
int afc_fileops_set_tags(FileOperations *fo, int first_tag, ...)
{
	va_list tags;

	va_start(tags, first_tag);

	afc_fileops_internal_parse_tags(fo, first_tag, tags);

	va_end(tags);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_set_tag ( fo, tag, val )
/*
@node afc_fileops_set_tag

		 NAME: afc_fileops_set_tag ( fo, tag, value ) - Sets a tag

			 SYNOPSIS: int afc_fileops_set_tag ( FileOperations * fo, int tag, void * value )

		DESCRIPTION: This function set a tag.

		INPUT: - fo	 - Pointer to a valid FileOperations class.
		 - tag - Tag to set. Valid tags are:
				+ AFC_FILEOPS_TAG_ERROR -  set last_error to the passed value.
				+ AFC_FILEOPS_TAG_BLOCK_CHOWN -  Flag TRUE/FALSE. If TRUE, afc_fileops_chown() will fail if it cannot chown the file.
				+ AFC_FILEOPS_TAG_BLOCK_CHMOD -  Flag TRUE/FALSE. If TRUE, afc_fileops_chmod() will fail if it cannot chmod the file.
				+ AFC_FILEOPS_TAG_BLOCK_UTIME -  Flag TRUE/FALSE. If TRUE, afc_fileops_utime() will fail if it cannot setutime the file.
				+ AFC_FILEOPS_TAG_BLOCK_MKDIR_EXISTS -  Flag TRUE/FALSE. If TRUE, afc_fileops_mkdir() will fail if it cannot create a dir.
				+ AFC_FILEOPS_TAG_STAT -  You can pass a valid stat structure as argument and owner, group and mode will be taken from this stat file.
				+ AFC_FILEOPS_TAG_OWNER -  The numerical value for the owner of any new/copied file. Default value is -1 which means "current user"
				+ AFC_FILEOPS_TAG_GROUP -  The numerical value of the groupof any new/copied file. Default value is -1 which means "current group"
				+ AFC_FILEOPS_TAG_MODE -   The numerical value of the mode of any new/copied file. Default value is -1 which means "current mode"
				+ AFC_FILEOPS_TAG_BUFFER -  Buffr size for copy operations. Default value is 4096 bytes (AFC_FILEOPS_COPY_DEFAULT_BUFFER)

		 - value - Value to set to the tag.

	RESULTS: the return code should be AFC_ERR_NO_ERROR

	NOT IN: Win32

			 SEE ALSO: - afc_fileops_set_tags()
@endnode
*/
int afc_fileops_set_tag(FileOperations *fo, int attr, void *val)
{
	struct stat *st;

	switch (attr)
	{
	case AFC_FILEOPS_TAG_ERROR:
		fo->last_error = (int)(long)val;
		break;

	case AFC_FILEOPS_TAG_BLOCK_CHOWN:
		fo->block_chown = (short)(int)(long)val;
		break;

	case AFC_FILEOPS_TAG_BLOCK_CHMOD:
		fo->block_chmod = (short)(int)(long)val;
		break;

	case AFC_FILEOPS_TAG_BLOCK_UTIME:
		fo->block_utime = (short)(int)(long)val;
		break;

	case AFC_FILEOPS_TAG_BLOCK_MKDIR_EXISTS:
		fo->block_mkdir_exists = (short)(int)(long)val;
		break;

	case AFC_FILEOPS_TAG_STAT:
		st = (struct stat *)val;
		fo->uid = st->st_uid;
		fo->gid = st->st_gid;
		fo->mode = st->st_mode;
		break;

	case AFC_FILEOPS_TAG_OWNER:
		fo->uid = (int)(long)val;
		break;

	case AFC_FILEOPS_TAG_GROUP:
		fo->gid = (int)(long)val;
		break;

	case AFC_FILEOPS_TAG_MODE:
		fo->mode = (int)(long)val;
		break;

	case AFC_FILEOPS_TAG_BUFFER:
		if (fo->foc->buffer)
			afc_free(fo->foc->buffer);

		if ((fo->foc->buffer = (char *)afc_malloc((int)(long)val)) == NULL)
			return (AFC_LOG_FAST(AFC_ERR_NO_MEMORY));

		fo->foc->bufsize = (int)(long)val;
		break;

	case AFC_FILEOPS_TAG_UPDATE_FUNCT:
		fo->update_funct = val;
		break;

	case AFC_FILEOPS_TAG_UPDATE_INFO:
		fo->update_info = val;
		break;

	default:
		afc_string_make(__internal_afc_base->tmp_string, "%x", attr);
		return (AFC_LOG_FAST_INFO(AFC_ERR_UNSUPPORTED_TAG, __internal_afc_base->tmp_string));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_exists_full ( fo, dir, fname )
/*
@node afc_fileops_exists_full

	NAME: afc_fileops_exists_full ( fo, dir, file_name ) - Check whether a file/dir exists in file system

	SYNOPSIS: int afc_fileops_exists_full ( FileOperations * fo, char * dir, char * file_name )

 DESCRIPTION: This function is the same of afc_fileops_exists(), but you can provide the /file_name/ and
				   /dir/ separately.

	   INPUT: - fo	  - Pointer to a valid FileOperations class.
			  - dir       - The full path to get to the file
			  - file_name - The file/dir name to check

	 RESULTS: - AFC_ERR_NO_ERROR - The file/dir exists
			  - AFC_FILEOPS_ERR_STAT - The file does not exists in the file system, or not enough permissions to stat the file.

	  NOT IN: Win32

	SEE ALSO: - afc_fileops_exists()
@endnode
*/
int afc_fileops_exists_full(FileOperations *fo, char *dir, char *fname)
{
	int res;
	char *path;

	if ((dir == NULL) || (fname == NULL))
		return (AFC_ERR_NULL_POINTER);

	if ((path = afc_string_new(strlen(dir) + strlen(fname) + 10)) == NULL)
		return (AFC_ERR_NO_MEMORY);

	afc_string_make(path, "%s/%s", dir, fname);

	res = afc_fileops_exists(fo, path);

	afc_string_delete(path);

	return (res);
}
// }}}
// {{{ afc_fileops_chown ( fo, file_name, uid, gid )
/*
@node afc_fileops_chown

		 NAME: afc_fileops_chown ( fo, file_name, uid, gid ) - Change the file permissions

			 SYNOPSIS: int afc_fileops_chown ( FileOperations * fo, char * file_name, int uid, int gid )

		DESCRIPTION: This function changes the user and group owner of a file. It is the same of the shell
				   command /chown/.

		INPUT: - fo	       - Pointer to a valid FileOperations class.
		 - file_name - The file/dir name to work on
		 - uid	     - New user id for the file
		 - gid       - New group id for the file

	RESULTS: - should return AFC_ERR_NO_ERROR
				   - If /chown/ fails, you may get AFC_FILEOPS_ERR_CHOWN, but only if you set that
					 /chown/ errors are blocking with afc_fileops_set_tag()

	  NOTES: - This function may or may not return an error value in case of real errors, depending
					 on the settings you have made using afc_fileops_set_tag() or afc_fileops_set_tags().

NOT IN:	Win32
			 SEE ALSO: - afc_fileops_chmod()
		 - afc_fileops_utime()
		 - afc_fileops_set_tag()
		 - afc_fileops_set_tags()
@endnode
*/
int afc_fileops_chown(FileOperations *fo, char *fname, int uid, int gid)
{
	if (fo->update_funct)
		fo->update_funct(AFC_FILEOPS_UPDATE_MODE_FILENAME, fname, fo->update_info);

	if ((chown(fname, uid, gid)) == -1)
	{
		fo->last_error = errno;

		AFC_LOG(AFC_LOG_NOTICE, AFC_FILEOPS_ERR_CHOWN, strerror(errno), fname);

		if (fo->block_chown)
			return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_CHOWN, strerror(errno), fname));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_move ( fo, source, dest )
/*
@node afc_fileops_move

		 NAME: afc_fileops_move ( fo, source, dest ) - Move a file/dir

			 SYNOPSIS: int afc_fileops_move ( FileOperations * fo, char * source, char * dest )

		DESCRIPTION: This function moves a file/dir from one path to another. It can also be used to rename files.
				   It corresponds to the shell command /mv/.

		INPUT: - fo	        - Pointer to a valid FileOperations class.
		 - source     - Source file/dir name (with full path) to move or rename
		 - dest       - Destination file/dir name (with full path) where to move or rename the /source/.

	RESULTS: - AFC_ERR_NO_ERROR: no error occurred
		 - if /move/ fails for some reason, you'll get the AFC_FILEOPS_ERR_RENAME error.

	  NOTES: - This function is able to move files and dirs across different file systems on different devices.
					 It first tries to use the system call /rename/, but if it fails with /EXDEV/ error, then it
					 performs a physical move all all files/dirs selected.
				   - This function is recursive.

NOT IN: Win32

			 SEE ALSO: - afc_fileops_copy()
		 - afc_fileops_link()
		 - afc_fileops_del()
		 - afc_fileops_mkdir()
@endnode
*/
int afc_fileops_move(FileOperations *fo, char *source, char *dest)
{
	int res;

	res = rename(source, dest);

	if (errno == EXDEV)
	{
		afc_fileops_internal_physical_move(fo, source, dest);
	}
	else
	{

		if (fo->update_funct)
			fo->update_funct(AFC_FILEOPS_UPDATE_MODE_FILENAME, source, fo->update_info);

		if (res != 0)
		{
			fo->last_error = errno;

			return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_RENAME, strerror(errno), source));
		}
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_copy ( fo, source, dest )
/*
@node afc_fileops_copy

		 NAME: afc_fileops_copy ( fo, source, dest ) - Copy a file/dir

			 SYNOPSIS: int afc_fileops_copy ( FileOperations * fo, char * source, char * dest )

		DESCRIPTION: This function copies a file/dir from one path to another.
				   It corresponds to the shell command /cp/.

		INPUT: - fo	        - Pointer to a valid FileOperations class.
		 - source     - Source file/dir name (with full path) to copy
		 - dest       - Destination file/dir name (with full path) where to copy the /source/.

	RESULTS: - AFC_ERR_NO_ERROR: no error occurred
		 - if /copy/ fails for some reason, you'll get different return codes depending on where /copy/ failed.

	  NOTES: - This function is able to move files and dirs across different file systems on different devices.
					 It first tries to use the system call /rename/, but if it fails with /EXDEV/ error, then it
					 performs a physical move all all files/dirs selected.
				   - This function is recursive.

NOT IN: Win32

			 SEE ALSO: - afc_fileops_move()
		 - afc_fileops_link()
		 - afc_fileops_del()
		 - afc_fileops_mkdir()
@endnode
*/
int afc_fileops_copy(FileOperations *fo, char *source, char *dest)
{
	struct stat st;
	int err;

	if (fo->foc->buffer == NULL)
		return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_COPY_ERR_NO_BUFFER, "No buffer avaible", NULL));

	fo->foc->source = source;
	fo->foc->dest = dest;

	if (stat(fo->foc->source, &st) == -1)
	{
		fo->last_error = errno;

		return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_STAT, strerror(errno), fo->foc->source));
	}

	if (S_ISDIR(st.st_mode))
	{
		if ((err = afc_fileops_mkdir(fo, fo->foc->dest)) != AFC_ERR_NO_ERROR)
			return (err);

		if ((err = afc_fileops_internal_scan_dir(fo, fo->foc->source, afc_fileops_internal_copy_new_file, afc_fileops_internal_copy_new_dir, NULL, fo->foc->dest)) != AFC_ERR_NO_ERROR)
			return (err);
	}
	else
	{
		if ((err = afc_fileops_copy_internal_copy(fo)) != AFC_ERR_NO_ERROR)
			return (AFC_FILEOPS_COPY_ERR_COPYING);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_link ( fo, source, dest )
/*
@node afc_fileops_link

		 NAME: afc_fileops_link ( fo, source, dest ) - Create a symbolic link to a file/dir

			 SYNOPSIS: int afc_fileops_link ( FileOperations * fo, char * source, char * dest )

		DESCRIPTION: This function creates a symbolic link to a file or dir.
				   It corresponds to the shell command /ln -s/.

		INPUT: - fo	        - Pointer to a valid FileOperations class.
		 - source     - Source file/dir name (with full path) to link
		 - dest       - Destination file/dir name (with full path) where to link /source/.

	RESULTS: - AFC_ERR_NO_ERROR: no error occurred
		 - if /copy/ fails for some reason, you'll get an AFC_FILEOPS_ERR_LINK error.

NOT IN: Win32
			 SEE ALSO: - afc_fileops_move()
		 - afc_fileops_copy()
		 - afc_fileops_del()
		 - afc_fileops_mkdir()
@endnode
*/
int afc_fileops_link(FileOperations *fo, char *src, char *dest)
{
	if (symlink(src, dest) == -1)
	{
		fo->last_error = errno;

		return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_LINK, strerror(errno), src));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
#endif // MINGW

// {{{ afc_fileops_exists ( fo, file_name )
/*
@node afc_fileops_exists

		 NAME: afc_fileops_exists ( fo, file_name ) - Check whether a file/dir exists in file system

			 SYNOPSIS: int afc_fileops_exists ( FileOperations * fo, char * file_name )

		DESCRIPTION: This function checks whether the provided /file_name/ is present in the file system or not.

		INPUT: - fo	 - Pointer to a valid FileOperations class.
		 - file_name - The file/dir name to check

	RESULTS: - AFC_ERR_NO_ERROR - The file/dir exists
				   - AFC_FILEOPS_ERR_STAT - The file does not exists in the file system, or not enough permissions
											to stat the file.

			 SEE ALSO: - afc_fileops_exists_full()
@endnode
*/
int afc_fileops_exists(FileOperations *fo, char *fname)
{
	struct stat f;

	if (fo->update_funct)
		fo->update_funct(AFC_FILEOPS_UPDATE_MODE_FILENAME, fname, fo->update_info);

	if ((stat(fname, &f)) != 0)
	{
		fo->last_error = errno;
		if (errno != 2)
			return (AFC_LOG(AFC_LOG_WARNING, AFC_FILEOPS_ERR_STAT, "Cannot stat() file/dir", fname));

		return (AFC_FILEOPS_ERR_NOT_FOUND);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_chmod ( fo, fname, mode )
/*
@node afc_fileops_chmod

		 NAME: afc_fileops_chmod ( fo, file_name, mode ) - Change the file permissions

			 SYNOPSIS: int afc_fileops_chmod ( FileOperations * fo, char * file_name, int mode )

		DESCRIPTION: This function changes the file permissions. It has the same function of the shell
				   command /chmod/.

		INPUT: - fo	       - Pointer to a valid FileOperations class.
		 - file_name - The file/dir name to work on
		 - mode      - The new file permissions

	RESULTS: - should return AFC_ERR_NO_ERROR
				   - If /chmod/ fails, you may get AFC_FILEOPS_ERR_CHMOD, but only if you set that
					 /chmod/ errors are blocking with afc_fileops_set_tag()

	  NOTES: - This function may or may not return an error value in case of real errors, depending
					 on the settings you have made using afc_fileops_set_tag() or afc_fileops_set_tags().

			 SEE ALSO: - afc_fileops_chown()
		 - afc_fileops_utime()
		 - afc_fileops_set_tag()
		 - afc_fileops_set_tags()
@endnode
*/
int afc_fileops_chmod(FileOperations *fo, char *fname, int mode)
{
	if (fo->update_funct)
		fo->update_funct(AFC_FILEOPS_UPDATE_MODE_FILENAME, fname, fo->update_info);

	if ((chmod(fname, mode)) == -1)
	{
		fo->last_error = errno;

		AFC_LOG(AFC_LOG_NOTICE, AFC_FILEOPS_ERR_CHMOD, strerror(errno), fname);

		if (fo->block_chmod)
			return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_CHMOD, strerror(errno), fname));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_utime ( fo, file_name, times )
/*
@node afc_fileops_utime

		 NAME: afc_fileops_utime ( fo, file_name, times ) - Change the file times

			 SYNOPSIS: int afc_fileops_utime ( FileOperations * fo, char * file_name, struct utimbuf * times )

		DESCRIPTION: This function changes the time of the file /file_name/.

		INPUT: - fo	       - Pointer to a valid FileOperations class.
		 - file_name - The file/dir name to work on
		 - times     - An already set /utimbuf/ structure with the new values.

	RESULTS: - should return AFC_ERR_NO_ERROR
				   - If /utime/ fails, you may get AFC_FILEOPS_ERR_UTIME, but only if you set that
					 /utime/ errors are blocking with afc_fileops_set_tag()

	  NOTES: - This function may or may not return an error value in case of real errors, depending
					 on the settings you have made using afc_fileops_set_tag() or afc_fileops_set_tags().

			 SEE ALSO: - afc_fileops_chmod()
		 - afc_fileops_chown()
		 - afc_fileops_set_tag()
		 - afc_fileops_set_tags()
@endnode
*/
int afc_fileops_utime(FileOperations *fo, char *fname, struct utimbuf *times)
{
	if (fo->update_funct)
		fo->update_funct(AFC_FILEOPS_UPDATE_MODE_FILENAME, fname, fo->update_info);

	if (utime(fname, times) == -1)
	{
		fo->last_error = errno;

		AFC_LOG(AFC_LOG_NOTICE, AFC_FILEOPS_ERR_UTIME, strerror(errno), fname);

		if (fo->block_utime)
			return (AFC_LOG(AFC_LOG_NOTICE, AFC_FILEOPS_ERR_UTIME, strerror(errno), fname));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_del ( fo, file_name )
/*
@node afc_fileops_del

		 NAME: afc_fileops_del ( fo, file_name ) - Delete a file/dir

			 SYNOPSIS: int afc_fileops_del ( FileOperations * fo, char * file_name )

		DESCRIPTION: This function deletes the given file or dir provided with /file_name/.
				   If you provide a directory, then this function recurively deletes /all/ dir contents.

		INPUT: - fo	       - Pointer to a valid FileOperations class.
		 - file_name - The file/dir name to work on

	RESULTS: - AFC_ERR_NO_ERROR - No error occurred
				   - AFC_FILEOPS_ERR_RMDIR - Errors removing a directory
				   - AFC_FILEOPS_ERR_UNLINK - Errors removing a file


			 SEE ALSO: - afc_fileops_copy()
		 - afc_fileops_link()
		 - afc_fileops_mkdir()
		 - afc_fileops_move()
@endnode
*/
int afc_fileops_del(FileOperations *fo, char *fname)
{
	struct stat st;
	int err;

#ifdef MINGW
	if (stat(fname, &st) == -1)
#else
	if (lstat(fname, &st) == -1)
#endif // MINGW
	{
		fo->last_error = errno;

		return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_STAT, strerror(errno), fname));
	}

	if (fo->update_funct)
		fo->update_funct(AFC_FILEOPS_UPDATE_MODE_FILENAME, fname, fo->update_info);

	if (S_ISDIR(st.st_mode))
	{
		if ((err = afc_fileops_internal_scan_dir(fo, fname, afc_fileops_internal_del_file, afc_fileops_internal_del_dir, NULL, fname)) == AFC_ERR_NO_ERROR)
		{
			if (rmdir(fname) == -1)
			{
				fo->last_error = errno;

				return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_RMDIR, strerror(errno), fname));
			}
		}
		else
			return (err);
	}
	else
	{
		if (unlink(fname) == -1)
		{
			fo->last_error = errno;

			return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_UNLINK, strerror(errno), fname));
		}
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_mkdir ( fo, name )
/*
@node afc_fileops_mkdir

		 NAME: afc_fileops_mkdir ( fo, dir_name ) - Create a new dir

			 SYNOPSIS: int afc_fileops_mkdir ( FileOperations * fo, char * dir_name )

		DESCRIPTION: This function create a new dir in the file system. The provided /dir_name/ must be
				   a full path to the place where you want it to be created.

		INPUT: - fo	       - Pointer to a valid FileOperations class.
		 - dir_name  - Dir containing full path

	RESULTS: - AFC_ERR_NO_ERROR: no error occurred.
				   - If /mkdir/ fails because the directory you attemped to create already exists, you may get AFC_FILEOPS_ERR_MKDIR, but only if you set that
					 /mkdir/ errors are blocking with afc_fileops_set_tag().
		 - In case /mkdir/ fails for some other reason, you'll get the AFC_FILEOPS_ERR_MKDIR error.

	  NOTES: - This function may or may not return an error value in case of real errors, depending
					 on the settings you have made using afc_fileops_set_tag() or afc_fileops_set_tags().

			 SEE ALSO: - afc_fileops_copy()
		 - afc_fileops_link()
		 - afc_fileops_del()
		 - afc_fileops_move()
@endnode
*/
int afc_fileops_mkdir(FileOperations *fo, char *name)
{
	if (fo->update_funct)
		fo->update_funct(AFC_FILEOPS_UPDATE_MODE_FILENAME, name, fo->update_info);

#ifdef MINGW
	if (mkdir(name) != 0)
#else
	if (mkdir(name, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
#endif // MINGW
	{
		if ((errno == EEXIST) && (fo->block_mkdir_exists == FALSE))
			return (AFC_ERR_NO_ERROR);

		fo->last_error = errno;

		return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_MKDIR, strerror(errno), name));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}

/* =============================================================================================================
	 INTERNAL FUNCTIONS
============================================================================================================= */
// {{{ afc_fileops_internal_scan_dir ( fo, path, action_file, action_dir, action_end_dir, info )
static int afc_fileops_internal_scan_dir(FileOperations *fo, char *path, int action_file(FileOperations *, struct stat *, char *, char *, char *, void *), int action_dir(FileOperations *, struct stat *, char *, char *, char *, void *), int action_end_dir(FileOperations *, char *, void *), void *info)
{
	DIR *dir;
	struct dirent *file;
	struct stat descr;
	char dirname[AFC_FILEOPS_MAX_DIR_LEN];
	char fullname[AFC_FILEOPS_MAX_DIR_LEN];
	int err;

	if ((dir = opendir(path)) == NULL)
	{
		fo->last_error = errno;

		return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_OPEN_DIR, strerror(errno), path));
	}

	strcpy(dirname, path);

	if (dirname[strlen(dirname) - 1] != '/')
		strcat(dirname, "/");

	while ((file = readdir(dir)) != NULL)
	{
		if ((strcmp(file->d_name, "..") == 0) || (strcmp(file->d_name, ".") == 0))
			continue;

		snprintf(fullname, sizeof(fullname), "%s%s", dirname, file->d_name);

#ifdef MINGW
		if (stat(fullname, &descr) == -1)
#else
		if (lstat(fullname, &descr) == -1)
#endif
		{
			fo->last_error = errno;

			return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_STAT, strerror(errno), fullname));
		}

		if (S_ISDIR(descr.st_mode))
		{
			if (action_dir)
				if ((err = action_dir(fo, &descr, fullname, dirname, file->d_name, info)) != 0)
					return (err);
		}
		else
		{
			if (action_file)
				if ((err = action_file(fo, &descr, fullname, dirname, file->d_name, info)) != 0)
					return (err);
		}
	}

	closedir(dir);

	if (action_end_dir)
		if ((err = action_end_dir(fo, path, info)) != 0)
			return (err);

	return (0);
}
// }}}
// {{{ afc_fileops_internal_move_dir ( fo, descr, fulldir, path, dirname, info )
static int afc_fileops_internal_move_dir(FileOperations *fo, struct stat *descr, char *fulldir, char *path, char *dirname, void *info)
{
	int err;
	char *dest_path = (char *)info;
	char buf[4096];

	sprintf(buf, "%s/%s", dest_path, dirname);

	if ((err = afc_fileops_mkdir(fo, buf)) != AFC_ERR_NO_ERROR)
		return (err);

	if ((err = afc_fileops_internal_scan_dir(fo, fulldir, afc_fileops_internal_move_file, afc_fileops_internal_move_dir, NULL, buf)) != AFC_ERR_NO_ERROR)
		return (err);

	if (rmdir(fulldir) == -1)
	{
		fo->last_error = errno;

		return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_RMDIR, strerror(errno), buf));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_internal_move_file ( fo, descr, fullname, path, filename, info )
static int afc_fileops_internal_move_file(FileOperations *fo, struct stat *descr, char *fullname, char *path, char *filename, void *info)
{
	char *dest_path = (char *)info;
	char buf[4096];

	sprintf(buf, "%s/%s", dest_path, filename);

	fo->foc->source = fullname;
	fo->foc->dest = buf;

	if (afc_fileops_copy_internal_copy(fo) != AFC_ERR_NO_ERROR)
		return (AFC_FILEOPS_COPY_ERR_COPYING);

	if (unlink(fullname) == -1)
	{
		fo->last_error = errno;

		return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_UNLINK, strerror(errno), buf));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_internal_del_dir ( fo, descr, fulldir, path, dirname, info )
static int afc_fileops_internal_del_dir(FileOperations *fo, struct stat *descr, char *fulldir, char *path, char *dirname, void *info)
{
	int err;
	char *dest_path = (char *)info;
	char buf[4096];

	sprintf(buf, "%s/%s", dest_path, dirname);

	if ((err = afc_fileops_internal_scan_dir(fo, buf, afc_fileops_internal_del_file, afc_fileops_internal_del_dir, NULL, buf)) != AFC_ERR_NO_ERROR)
		return (err);

	if (rmdir(buf) == -1)
	{
		fo->last_error = errno;

		return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_RMDIR, strerror(errno), buf));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_internal_del_file ( fo, descr, fullname, path, filename, info )
static int afc_fileops_internal_del_file(FileOperations *fo, struct stat *descr, char *fullname, char *path, char *filename, void *info)
{
	char *dest_path = (char *)info;
	char buf[4096];

	sprintf(buf, "%s/%s", dest_path, filename);

	if (fo->update_funct)
		fo->update_funct(AFC_FILEOPS_UPDATE_MODE_FILENAME, buf, fo->update_info);

	if (unlink(buf) == -1)
	{
		fo->last_error = errno;

		return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_UNLINK, strerror(errno), buf));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_internal_copy_new_dir ( fo, descr, fulldir, path, dirname, info )
static int afc_fileops_internal_copy_new_dir(FileOperations *fo, struct stat *descr, char *fulldir, char *path, char *dirname, void *info)
{
	char *dest_path = (char *)info;
	char buf[4096];
	int err;

	sprintf(buf, "%s/%s", dest_path, dirname);

	if ((err = afc_fileops_mkdir(fo, buf)) != AFC_ERR_NO_ERROR)
		return (err);

	return (afc_fileops_internal_scan_dir(fo, fulldir, afc_fileops_internal_copy_new_file, afc_fileops_internal_copy_new_dir, NULL, buf));
}
// }}}
// {{{ afc_fileops_internal_copy_new_file ( fo, descr, fullname, path, filename, info )
static int afc_fileops_internal_copy_new_file(FileOperations *fo, struct stat *descr, char *fullname, char *path, char *filename, void *info)
{
	char *dest_path = (char *)info;
	char buf[4096];

	sprintf(buf, "%s/%s", dest_path, filename);

	fo->foc->source = fullname;
	fo->foc->dest = buf;

	afc_fileops_copy_internal_copy(fo);

	return (0);
}
// }}}
// {{{ afc_fileops_internal_copy_new ()
static FOCopy *afc_fileops_internal_copy_new()
{
	FOCopy *foc = (FOCopy *)afc_malloc(sizeof(FOCopy));

	if (foc == NULL)
		return (NULL);

	if ((foc->buffer = (char *)afc_malloc(AFC_FILEOPS_COPY_DEFAULT_BUFFER)) == NULL)
	{
		afc_fileops_internal_copy_delete(foc);
		return (NULL);
	}

	foc->bufsize = AFC_FILEOPS_COPY_DEFAULT_BUFFER;

	return (foc);
}
// }}}
// {{{ afc_fileops_internal_copy_delete ( foc )
static void afc_fileops_internal_copy_delete(FOCopy *foc)
{
	if (foc == NULL)
		return;

	if (foc->buffer)
		afc_free(foc->buffer);

	afc_free(foc);
}
// }}}
// {{{ afc_fileops_copy_internal_set_file_stat ( fo, st )
static int afc_fileops_copy_internal_set_file_stat(FileOperations *fo, struct stat *st)
{
	struct utimbuf times;
	int mode, uid, gid;
	int err;

	if (fo->mode == -1)
		mode = st->st_mode;
	else
		mode = fo->mode;

	if (fo->uid == -1)
		uid = st->st_uid;
	else
		uid = fo->uid;

	if (fo->gid == -1)
		gid = st->st_gid;
	else
		gid = fo->gid;

#ifndef MINGW
	if ((err = afc_fileops_chown(fo, fo->foc->dest, uid, gid)) != AFC_ERR_NO_ERROR)
		return (err);
#endif // MINGW

	if ((err = afc_fileops_chmod(fo, fo->foc->dest, mode)) != AFC_ERR_NO_ERROR)
		return (err);

	times.actime = st->st_atime;
	times.modtime = st->st_mtime;

	if ((err = afc_fileops_utime(fo, fo->foc->dest, &times)) != AFC_ERR_NO_ERROR)
		return (err);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_copy_internal_copy ( fo )
static int afc_fileops_copy_internal_copy(FileOperations *fo)
{
	int ret = 0;
	int src, dest, qty;
	char *ptr;
	int size = 0;
	struct stat st;
	int quit = 0;

	if ((src = open(fo->foc->source, O_RDONLY)) == -1)
	{
		fo->last_error = errno;

		return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_CANNOT_READ, strerror(errno), fo->foc->source));
	}

	fstat(src, &st);

	if (fo->update_funct)
		quit = fo->update_funct(AFC_FILEOPS_UPDATE_MODE_FILENAME, fo->foc->source, fo->update_info);
	if (fo->update_funct)
		quit = fo->update_funct(AFC_FILEOPS_UPDATE_MODE_SIZE, (void *)st.st_size, fo->update_info);

	if ((dest = open(fo->foc->dest, O_WRONLY | O_CREAT | O_EXCL, 0)) == -1)
	{
		fo->last_error = errno;

		return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_CANNOT_WRITE, strerror(errno), fo->foc->dest));
	}

	while ((qty = read(src, fo->foc->buffer, fo->foc->bufsize)) && (quit == 0))
	{
		ptr = fo->foc->buffer;
		while ((qty > 0) && (quit == 0))
		{
			if ((ret = write(dest, ptr, qty)) == -1)
			{
				close(src);
				close(dest);

				return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_COPY_ERR_COPYING, strerror(errno), fo->foc->dest));
			}
			qty -= ret;
			ptr += ret;

			size += ret;

			if (fo->update_funct)
				quit = fo->update_funct(AFC_FILEOPS_UPDATE_MODE_POSITION, (void *)(long)size, fo->update_info);
		}
	}

	close(src);
	close(dest);

	if (afc_fileops_copy_internal_set_file_stat(fo, &st) != AFC_ERR_NO_ERROR)
		return (AFC_FILEOPS_COPY_ERR_COPYING);

	if (qty == -1)
	{
		return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_CANNOT_READ, strerror(errno), fo->foc->source));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}

#ifndef MINGW
// {{{ afc_fileops_internal_parse_tags ( fo, first_tag, tags )
static int afc_fileops_internal_parse_tags(FileOperations *fo, int first_tag, va_list tags)
{
	int tag;
	void *val;

	tag = first_tag;

	while (tag)
	{
		val = va_arg(tags, void *);
		afc_fileops_set_tag(fo, tag, val);
		tag = va_arg(tags, int);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_fileops_internal_physical_move ( fo, source, dest )
static int afc_fileops_internal_physical_move(FileOperations *fo, char *source, char *dest)
{
	struct stat st;
	int err;

	if (stat(source, &st) == -1)
	{
		fo->last_error = errno;

		return (AFC_LOG(AFC_LOG_ERROR, AFC_FILEOPS_ERR_STAT, strerror(errno), source));
	}

	if (S_ISDIR(st.st_mode))
	{
		if ((err = afc_fileops_mkdir(fo, dest)) != AFC_ERR_NO_ERROR)
			return (err);

		if ((err = afc_fileops_internal_scan_dir(fo, source, afc_fileops_internal_move_file, afc_fileops_internal_move_dir, NULL, dest)) != AFC_ERR_NO_ERROR)
			return (err);

		if ((err = afc_fileops_del(fo, source)) != AFC_ERR_NO_ERROR)
			return (err);
	}
	else
	{
		// Set the source and dest info
		fo->foc->source = source;
		fo->foc->dest = dest;

		// Perform the copy
		if ((err = afc_fileops_copy_internal_copy(fo)) != AFC_ERR_NO_ERROR)
			return (AFC_FILEOPS_COPY_ERR_COPYING);

		// Try to unlink (delete) the source file
		if ((err = afc_fileops_del(fo, source)) != AFC_ERR_NO_ERROR)
			return (err);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
#endif // MINGW

#ifdef TEST_FILEOPS
// {{{ TEST_CLASS
int update(int v, void *val, void *info)
{
	if (v == AFC_FILEOPS_UPDATE_MODE_POSITION)
		printf("Status: %d\n", (int)val);
	else
		printf("UPDATE: FileName: %s\n", (char *)val);

	return (FALSE);
}

int main()
{
	FileOperations *fo = afc_fileops_new();
	FILE *fh;

	printf("Setting the new buffer at 8000 bytes ...\n");
	if (afc_fileops_set_tag(fo, AFC_FILEOPS_TAG_BUFFER, (void *)8000) != AFC_ERR_NO_ERROR)
	{
		printf("ERROR: cannot set the new buffer.\n");
		exit(1);
	}

	afc_fileops_set_tag(fo, AFC_FILEOPS_TAG_UPDATE_FUNCT, update);

	printf("Creating a dir called \"fo_test_dir\" in the current dir\n");
	if (afc_fileops_mkdir(fo, "./fo_test_dir") != AFC_ERR_NO_ERROR)
	{
		printf("ERROR: cannot created dir.\n");
		exit(1);
	}

	printf("Creating an empty file in \"fo_test_dir\" called \"file1\"\n");
	if ((fh = fopen("./fo_test_dir/file1", "w")) == NULL)
	{
		printf("ERROR: cannot create empty file.\n");
		exit(1);
	}

	printf("Copying \"file1\" as \"file2\" and \"file3\" in the same dir\n");
	if (afc_fileops_copy(fo, "./fo_test_dir/file1", "./fo_test_dir/file2") != AFC_ERR_NO_ERROR)
	{
		printf("ERROR: cannot copy file1 in file2\n");
		exit(1);
	}

	printf("Copying \"file1\" as \"file2\" and \"file3\" in the same dir\n");
	if (afc_fileops_copy(fo, "./fo_test_dir/file1", "./fo_test_dir/file3") != AFC_ERR_NO_ERROR)
	{
		printf("ERROR: cannot copy file1 in file3\n");
		exit(1);
	}

	printf("Duplicating dir \"fo_test_dir\" in \"fo_test_dir2\"\n");
	if (afc_fileops_copy(fo, "./fo_test_dir", "./fo_test_dir2") != AFC_ERR_NO_ERROR)
	{
		printf("ERROR: cannot copy \"fo_test_dir\" in \"fo_test_dir2\"\n");
		exit(1);
	}

	printf("Deleting file \"fo_test_dir/file1\"\n");
	if (afc_fileops_del(fo, "./fo_test_dir/file1") != AFC_ERR_NO_ERROR)
	{
		printf("ERROR: cannot delete file \"fo_test_dir/file1\"\n");
		exit(1);
	}

	printf("Creating symlink \"fo_test_dir2/file1\" to \"fo_test_dir/file1\"\n");
	if (afc_fileops_link(fo, "./fo_test_dir2/file1", "./fo_test_dir/file1") != AFC_ERR_NO_ERROR)
	{
		printf("ERROR: cannot symlink \"fo_test_dir2/file1\" to \"fo_test_dir/file1\"\n");
		exit(1);
	}

	printf("Deleting symlink \"fo_test_dir/file1\"\n");
	if (afc_fileops_del(fo, "./fo_test_dir/file1") != AFC_ERR_NO_ERROR)
	{
		printf("ERROR: cannot delete symlink \"fo_test_dir/file1\"\n");
		exit(1);
	}

	printf("Deleting dirs \"fo_test_dir\" and \"fo_test_dir2\"\n");
	if (afc_fileops_del(fo, "./fo_test_dir") != AFC_ERR_NO_ERROR)
	{
		printf("ERROR: cannot delete dir \"fo_test_dir\"\n");
		exit(1);
	}

	if (afc_fileops_del(fo, "./fo_test_dir2") != AFC_ERR_NO_ERROR)
	{
		printf("ERROR: cannot delete dir \"fo_test_dir2\"\n");
		exit(1);
	}

	return (0);
}
// }}}
#endif
