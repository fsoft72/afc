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
#ifndef AFC_CGI_MANAGER_H
#define AFC_CGI_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>

#include "base.h"
#include "dictionary.h"
#include "array_master.h"
#include "stringnode.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* AFC afc_cgi_manager Magic Number */
#define AFC_CGI_MANAGER_MAGIC ( 'C' << 24 | 'G' << 16 | 'I' << 8 | 'M' )

/* AFC afc_cgi_manager Base value for constants */
#define AFC_CGI_MANAGER_BASE 0xb000



/* Errors for afc_cgi_manager */
enum {	AFC_CGI_MANAGER_ERR_NO_REQUEST = AFC_CGI_MANAGER_BASE + 1,	/* Pointer passed is not of the right kind */
	AFC_CGI_MANAGER_ERR_POST_READ 					/* Error reading the POST */
};

enum {	AFC_CGI_MANAGER_METHOD_UNDEF = 0,
	AFC_CGI_MANAGER_METHOD_GET,
	AFC_CGI_MANAGER_METHOD_POST };

enum {	AFC_CGI_MANAGER_DEBUG_NONE = 0,
	AFC_CGI_MANAGER_DEBUG_ACTIONS, 
	AFC_CGI_MANAGER_DEBUG_RESULTS,
	AFC_CGI_MANAGER_DEBUG_INTERNALS };

enum {	AFC_CGI_MANAGER_MODE_FORM = 0,
	AFC_CGI_MANAGER_MODE_COOKIE  };

enum { 	AFC_CGI_MANAGER_TAG_DEBUG = 0,
	AFC_CGI_MANAGER_TAG_HANDLE_COOKIES  };

struct afc_cgi_manager 
{
	unsigned long magic;

	struct afc_dictionary      * headers;   /* Used to store HEADERS fields */
  	struct afc_dictionary      * fields;	/* Used to store FORM fields    */
	struct afc_dictionary      * cookies;   /* Used to store Cookies        */

  	StringNode * split;

	int method;			/* Can be AFC_CGI_MANAGER_METHOD /GET/, /POST/ or /UNDEF/ */

	char * content_type;

	char * cookies_expire;
	char * cookies_domain;
	char * cookies_path;

	time_t curr_time;
	struct tm the_right_time;

	short handle_cookies;		/* Flag T/F. If True, Cookie handling will be performed */
	short headers_sent;		/* Flag T/F. If TRUE the header has been already sent */
	short are_headers_set;		/* Flag T/F. If FALSE, headers have not been set (yet) */

	BOOL  is_post_read;		/* Flag T/F. If TRUE, POST arguments have already been read */

	char * charset;			/* Incoming request charset */
};

typedef struct afc_cgi_manager CGIManager;

#define afc_cgi_manager_new()		_afc_cgi_manager_new ( __FILE__, __FUNCTION__, __LINE__ )
#define afc_cgi_manager_delete(cgi)	if ( cgi ) { _afc_cgi_manager_delete ( cgi ); cgi = NULL; }

CGIManager * _afc_cgi_manager_new ( const char * file, const char * func, const unsigned int line );
int    _afc_cgi_manager_delete ( CGIManager * ) ;
int    afc_cgi_manager_clear ( CGIManager * ) ;

int    afc_cgi_manager_get_data          ( CGIManager * );
char * afc_cgi_manager_get_val           ( CGIManager *, char * );
int    afc_cgi_manager_write_header      ( CGIManager * );
int    afc_cgi_manager_set_content_type  ( CGIManager *, char * );
int    afc_cgi_manager_set_cookie        ( CGIManager *, char *, char * );
char * afc_cgi_manager_get_cookie        ( CGIManager *, char * );
int    afc_cgi_manager_set_cookie_domain ( CGIManager *, char * );
int    afc_cgi_manager_set_cookie_expire ( CGIManager *, int );
int    afc_cgi_manager_set_cookie_path   ( CGIManager *, char * );
int    afc_cgi_manager_set_tag ( CGIManager *, int, void * );

int    afc_cgi_manager_debug_dump ( CGIManager * cgi );
int afc_cgi_manager_set_default_headers ( CGIManager * cgi );
int afc_cgi_manager_get_header_str ( CGIManager * cgi, char * dest );

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
