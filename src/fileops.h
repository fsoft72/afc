/* 
 * Advanced Foundation Classes
 * Copyright (C) 2000/2004  Fabio Rotondo 
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
#ifndef AFC_FILEOPS_H
#define AFC_FILEOPS_H

#include <stdio.h>
#include <errno.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <malloc.h>

#ifdef MINGW
#include <io.h>
#include <sys/utime.h>
#else
#include <utime.h>
#endif // MINGW



#include "base.h"
#include "strings.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* AFC File Operations 'Magic' value: 'FILE' */
#define AFC_FILEOPS_MAGIC ( 'F' << 24 | 'I' << 16 | 'L' << 8 | 'E' )

#define AFC_FILEOPS_BASE 0xC000

/* Standard Errors */
enum { 	AFC_FILEOPS_ERR_STAT  = 2,
	AFC_FILEOPS_ERR_OPEN_DIR,
	AFC_FILEOPS_ERR_CHOWN,
	AFC_FILEOPS_ERR_CHMOD,
	AFC_FILEOPS_ERR_UTIME,
	AFC_FILEOPS_ERR_CANNOT_READ,
	AFC_FILEOPS_ERR_CANNOT_WRITE,
	AFC_FILEOPS_ERR_UNSUPPORTED_ATTR,
	AFC_FILEOPS_ERR_UNLINK,
	AFC_FILEOPS_ERR_RMDIR,
	AFC_FILEOPS_ERR_MKDIR,
	AFC_FILEOPS_ERR_LINK,
	AFC_FILEOPS_ERR_RENAME,
	AFC_FILEOPS_ERR_NOT_FOUND /* File/Dir not found */
};

/* Standard Attributes */
enum { 	AFC_FILEOPS_TAG_DEBUG = 1,				/* Flag T/F. If TRUE, debug info will be sent to stderr */
	AFC_FILEOPS_TAG_ERROR,					/* The last errno received.	*/
	AFC_FILEOPS_TAG_BLOCK_CHOWN,				/* Flag T/F. If TRUE, when chown() fails, the function returns an error */
	AFC_FILEOPS_TAG_BLOCK_CHMOD,				/* Flag T/F. If TRUE, when chmod() fails, the function returns an error */
	AFC_FILEOPS_TAG_BLOCK_UTIME,				/* Flag T/F. Ilf TRUE, when utime() fails, the function returns an error */
	AFC_FILEOPS_TAG_BLOCK_MKDIR_EXISTS,
	AFC_FILEOPS_TAG_STAT,
	AFC_FILEOPS_TAG_OWNER,
	AFC_FILEOPS_TAG_GROUP,
	AFC_FILEOPS_TAG_MODE,
	AFC_FILEOPS_TAG_INFO,
	AFC_FILEOPS_TAG_BUFFER,
	AFC_FILEOPS_TAG_UPDATE_FUNCT,
	AFC_FILEOPS_TAG_UPDATE_INFO 
};


struct fileop_internal_copy
{
	char * source;		// Source File (including path) to copy
	char * dest;            // Dest File name (including path) where to copy 
  	char * buffer;		// Copy Buffer
  	int    bufsize;		// Size of the Copy Buffer ( in bytes )	
};

typedef struct fileop_internal_copy FOCopy;

struct fileops 
{
	unsigned long magic;

	int   last_error;		   	// Last errno encountered 

	short block_chown;		    	// Flag T/F. If TRUE, the function fails if it cannot chown() a file ( default: FALSE ) 
	short block_chmod;        		// Flag T/F. If TRUE, the function fails if it cannot chmod() a file ( default: TRUE  )
	short block_utime;		    	// Flag T/F. If TRUE, the function fails if it cannot utime() a file ( default: TRUE  )
	short block_mkdir_exists; 		// Flag T/F. If TRUE the mkdir() command will fail if the desired dir already exists ( default: FALSE ) 

  	int   uid;               		// User ID to assign to the file
  	int   gid;				// Group ID to assign to the file
  	int   mode;				// Mode to assign to the file

	FOCopy * foc;				// Additional values for Copy operation

	int (*update_funct) (int, void *, void *);

	void * update_info;			// Additional info passed to update_function 
  	void * info;				// Additional info passed to the update function
};

typedef struct fileops FileOperations;

#define AFC_FILEOPS_MAX_DIR_LEN			2048
#define AFC_FILEOPS_MAX_FILE_LEN		 255

#define AFC_FILEOPS_MODE_FILE			1
#define AFC_FILEOPS_MODE_DIR			2

/* Update function modes */
#define AFC_FILEOPS_UPDATE_MODE_FILENAME	0	// Triggered when a new file is opened
#define AFC_FILEOPS_UPDATE_MODE_POSITION  	1 	// Triggered when anoter piece of file has been read
#define AFC_FILEOPS_UPDATE_MODE_SIZE      	2 	// Triggered when a new file is opened and we now the file size

/* Copy Errors */
#define AFC_FILEOPS_COPY_DEFAULT_BUFFER 	4096 

#define AFC_FILEOPS_COPY_ERR_NO_BUFFER		1
#define AFC_FILEOPS_COPY_ERR_COPYING		2

#define	AFC_FILEOPS_COPY_TAG_INFO		1
#define	AFC_FILEOPS_COPY_TAG_BUFFER		2

/* FileOperations Commands */
#define afc_fileops_delete(fo)	if ( fo ) { _afc_fileops_delete ( fo ); fo = NULL; }

FileOperations * afc_fileops_new    (void );
int _afc_fileops_delete      ( FileOperations * );
int afc_fileops_clear ( FileOperations * fo );

#ifndef MINGW
int afc_fileops_set_tags ( FileOperations * fo, int first_tag, ... );
int  afc_fileops_set_tag    ( FileOperations *, int, void * );
int  afc_fileops_exists_full ( FileOperations *, char *, char * );
int  afc_fileops_chmod       ( FileOperations *, char *, int );
int  afc_fileops_chown       ( FileOperations *, char *, int, int );
int  afc_fileops_utime       ( FileOperations *, char *, struct utimbuf * );
#endif // MINGW

int  afc_fileops_exists      ( FileOperations *, char * );
int  afc_fileops_del         ( FileOperations *, char * );

#ifndef MINGW
int  afc_fileops_mkdir       ( FileOperations *, char * );
int  afc_fileops_copy        ( FileOperations *, char *, char * );
int  afc_fileops_move        ( FileOperations *, char *, char * );
int  afc_fileops_link        ( FileOperations *, char *, char * );
#endif // MINGW

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
