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
/*
@config
	TITLE:     CGIManager
	VERSION:   1.10
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it

	COMMAND:   add_emphasis cookie Cookie cookies Cookies form Form forms Forms GET POST FORM CGI
@endnode

@history
	V1.10	- Many changes to accomodate the WIN32 version
@endhistory
*/

#include "cgi_manager.h"

// {{{ docs
/*
@node quote
*That aint nice you laughin, you see, my mule don't like people laughin, he gets the crazy idea you're laughin at him, so now*
*if you'll apologize, like i know you're going to, i might convince him that you really didn't mean it.*

	Fistfull of Dollars
@endnode

@node intro
CGIManager trasparently handles CGI Forms and cookies for your web applications.
Main features of this class are:

- It automatically reads all fields posted by a FORM, being them sent by a GET or POST one.
- It automatically reads any cookie sent by the browser
- It automatically writes back any cookie to browser 
@endnode
*/
// }}}
// {{{ statics
static const char class_name[] = "CGIManager";

static int afc_cgi_manager_internal_dump ( CGIManager * cgi, Dictionary * dict, char * message );
static int afc_cgi_manager_internal_get_headers ( CGIManager * cgi );
static int afc_cgi_manager_internal_method_get ( CGIManager * cgi );
static int afc_cgi_manager_internal_method_post ( CGIManager * cgi );
static int afc_cgi_manager_internal_parse_data ( CGIManager * cgi, char * data );
static int afc_cgi_manager_internal_add_key ( CGIManager * cgi, char * keyval, int mode );
static int afc_cgi_manager_internal_get_cookies ( CGIManager * cgi );
static char afc_cgi_manager_internal_decode ( CGIManager * cgi, char * str);
static int afc_cgi_manager_internal_unescape (  CGIManager * cgi, char * str );
static int afc_cgi_manager_internal_clear_dict ( CGIManager * cgi, Dictionary * dict );
static int _get_charset ( CGIManager * cgi );
// }}}

// {{{ afc_cgi_manager_new ()
/*
@node afc_cgi_manager_new

	         NAME: afc_cgi_manager_new ()    - Initializes a new afc_cgi_manager instance.

	     SYNOPSIS: CGIManager * afc_cgi_manager_new ()

	  DESCRIPTION: This function initializes a new afc_cgi_manager instance.

	        INPUT: NONE

	      RESULTS: a valid inizialized afc_cgi_manager structure. NULL in case of errors.

	     SEE ALSO: - afc_cgi_manager_delete()

@endnode
*/
CGIManager * _afc_cgi_manager_new ( const char * file, const char * func, const unsigned int line )
{
TRY ( CGIManager * )

	CGIManager * cgi_manager = _afc_malloc ( sizeof ( CGIManager ), file, func, line );

	if ( cgi_manager == NULL ) RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "cgi_manager", NULL );

	cgi_manager->magic = AFC_CGI_MANAGER_MAGIC;

	if ( ( cgi_manager->headers = afc_dictionary_new () ) == NULL ) 
		RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "headers", NULL );

	if ( ( cgi_manager->fields = afc_dictionary_new () ) == NULL ) 
		RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "fields", NULL );

	if ( ( cgi_manager->cookies = afc_dictionary_new () ) == NULL ) 
		RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "cookies", NULL );

	if ( ( cgi_manager->split = afc_stringnode_new () ) == NULL ) 
		RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "split", NULL );

	if ( ( cgi_manager->cookies_expire = afc_string_new ( 30 ) ) == NULL )
		RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "expire", NULL );

	if ( ( cgi_manager->cookies_domain = afc_string_new ( 50 ) ) == NULL )
		RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "cookies_domain", NULL );

	if ( ( cgi_manager->cookies_path = afc_string_new ( 50 ) ) == NULL )
		RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "cookies_path", NULL );

	if ( ( cgi_manager->content_type = afc_string_new ( 1024 ) ) == NULL )
		RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "content_type", NULL );

	if ( ( cgi_manager->charset = afc_string_new ( 255 ) ) == NULL )
		RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "charset", NULL );

	afc_string_copy ( cgi_manager->content_type, "text/html", ALL );

	RETURN ( cgi_manager );

EXCEPT
	afc_cgi_manager_delete ( cgi_manager );

FINALLY

ENDTRY
}
// }}}
// {{{ afc_cgi_manager_delete ( cgi )
/*
@node afc_cgi_manager_delete

	         NAME: afc_cgi_manager_delete ( cgi_manager )  - Disposes a valid cgi_manager instance.

	     SYNOPSIS: int afc_cgi_manager_delete ( CGIManager * cgi_manager)

	  DESCRIPTION: This function frees an already alloc'd afc_cgi_manager structure.

	        INPUT: - cgi_manager  - Pointer to a valid afc_cgi_manager class.

	      RESULTS: should be AFC_ERR_NO_ERROR

	        NOTES: - this method calls: afc_cgi_manager_clear()

	     SEE ALSO: - afc_cgi_manager_new()
	               - afc_cgi_manager_clear()
@endnode
*/
int _afc_cgi_manager_delete ( CGIManager * cgi_manager ) 
{
	int afc_res; 

	if ( ( afc_res = afc_cgi_manager_clear ( cgi_manager ) ) != AFC_ERR_NO_ERROR ) return ( afc_res );

	afc_dictionary_delete ( cgi_manager->headers  );
	afc_dictionary_delete ( cgi_manager->fields   );
	afc_dictionary_delete ( cgi_manager->cookies  );

	afc_stringnode_delete ( cgi_manager->split );

	afc_string_delete ( cgi_manager->content_type   );
	afc_string_delete ( cgi_manager->cookies_expire );
	afc_string_delete ( cgi_manager->cookies_domain );
	afc_string_delete ( cgi_manager->cookies_path   );
	afc_string_delete ( cgi_manager->charset   );

	afc_free ( cgi_manager );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_cgi_manager_clear ( cgi )
/*
@node afc_cgi_manager_clear

	         NAME: afc_cgi_manager_clear ( cgi_manager )  - Clears all stored data

	     SYNOPSIS: int afc_cgi_manager_clear ( CGIManager * cgi_manager)

	  DESCRIPTION: Use this function to clear all stored data in the current cgi_manager instance.

	        INPUT: - cgi_manager    - Pointer to a valid afc_cgi_manager instance.

	      RESULTS: should be AFC_ERR_NO_ERROR
	              
	     SEE ALSO: - afc_cgi_manager_delete()
	               
@endnode

*/
int afc_cgi_manager_clear ( CGIManager * cgi ) 
{
	if ( cgi == NULL ) return ( AFC_LOG_FAST ( AFC_ERR_NULL_POINTER ) );
	if ( cgi->magic != AFC_CGI_MANAGER_MAGIC ) return ( AFC_LOG_FAST ( AFC_ERR_INVALID_POINTER ) );

	/* Custom Clean-up code should go here */

	if ( cgi->headers ) afc_cgi_manager_internal_clear_dict ( cgi, cgi->headers );
	if ( cgi->fields  ) afc_cgi_manager_internal_clear_dict ( cgi, cgi->fields );
	if ( cgi->split   ) afc_stringnode_clear ( cgi->split );

	cgi->is_post_read = FALSE;

	cgi->are_headers_set = FALSE;

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_cgi_manager_get_data ( cgi )
/*
@node afc_cgi_manager_get_data

	         NAME: afc_cgi_manager_get_data ( cgi_manager )  - Gets data from the previous Form

	     SYNOPSIS: int afc_cgi_manager_get_data ( CGIManager * cgi_manager)

	  DESCRIPTION: This function gets all data from the previous FORM. If properely set, (see
	               afc_cgi_manager_set_tag() ), it will handle also cookies.
			Please, remember that, from version 1.10, you should set the headers data
			using afc_cgi_manager_set_default_headers() or afc_cgi_manager_set_header_value()
			calls before calling this function. 
			If you do not call any of the above, afc_cgi_manager_set_default_headers() will be
			called by default.
	               
	        INPUT: - cgi_manager    - Pointer to a valid afc_cgi_manager instance.

	      RESULTS: should be AFC_ERR_NO_ERROR
	              
	     SEE ALSO: - afc_cgi_manager_get_val()
	               
@endnode
*/
int afc_cgi_manager_get_data ( CGIManager * cgi )
{
	AFC_DEBUG_FUNC ();

	if ( cgi->are_headers_set == FALSE ) afc_cgi_manager_set_default_headers ( cgi );

	afc_dictionary_clear ( cgi->fields );

	afc_cgi_manager_internal_get_headers ( cgi );

	if ( cgi->method == AFC_CGI_MANAGER_METHOD_GET )       afc_cgi_manager_internal_method_get  ( cgi );
	else if ( cgi->method == AFC_CGI_MANAGER_METHOD_POST ) afc_cgi_manager_internal_method_post ( cgi );

	if ( cgi->handle_cookies ) afc_cgi_manager_internal_get_cookies ( cgi );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_cgi_manager_get_val ( cgi, name )
/*
@node afc_cgi_manager_get_val

	         NAME: afc_cgi_manager_get_val ( cgi_manager, name )  - Gets a specific form field data

	     SYNOPSIS: int afc_cgi_manager_get_val ( CGIManager * cgi_manager, char * name )

	  DESCRIPTION: this function retrieve a specific form field value. All you have to provide
	               is the form field name, and then (if the field really exists) it will be returned.
	               
	        INPUT: 	- cgi_manager    - Pointer to a valid afc_cgi_manager instance.
		 	- name           - Form Field name.

	      RESULTS: - a pointer to a valid string containing the requested field's value.
	               - If an error occurs, or the field cannot be found, then NULL is returned.
	              
	     SEE ALSO: - afc_cgi_manager_get_cookie()
@endnode
*/
char * afc_cgi_manager_get_val ( CGIManager * cgi, char * name )
{
	char * str = afc_string_dup ( name );
	char * s;

	afc_debug_adv ( __internal_afc_base, AFC_DEBUG_VERBOSE, class_name, "%s for: %s", __FUNCTION__, name );

	if ( str == NULL ) 
	{
		AFC_LOG_FAST_INFO ( AFC_ERR_NO_MEMORY, "afc_string_dup" );
		return ( NULL );		// If no memory for afc_string_dup, we return NULL 
	}

	afc_string_upper ( str );		// Let's make the key uppercase           

	s = ( char * )	afc_dictionary_get ( cgi->fields, str );	          // We look for the value in the Form Fields first 

	if ( s == NULL ) s = ( char * ) afc_dictionary_get ( cgi->headers, str ); // If it is missing, then we look in the headers 

	afc_string_delete ( str );						// Free the afc_string_dup() string

	return ( s );								// Return the  (found ?) value
}
// }}}
// {{{ afc_cgi_manager_write_header ( cgi )
/*
@node afc_cgi_manager_write_header

	         NAME: afc_cgi_manager_write_header ( cgi_manager )  - Writes a valid HTTP header with all cookies set

	     SYNOPSIS: int afc_cgi_manager_write_header ( CGIManager * cgi_manager )

	  DESCRIPTION: This functions writes a valid HTTP header on the stdout. If the class is set to handle cookies
	               (see afc_cgi_manager_set_tag() ), then also all needed cookies are set and returned.
	               
	        INPUT: - cgi_manager    - Pointer to a valid afc_cgi_manager instance.

	      RESULTS: should return AFC_ERR_NO_ERROR
	              
	     SEE ALSO: - afc_cgi_manager_set_cookie() 
	               - afc_cgi_manager_set_content_type()
		       - afc_cgi_manager_get_header_str ()
@endnode
*/
int afc_cgi_manager_write_header ( CGIManager * cgi )
{
	char * head;

	if ( cgi->headers_sent ) return ( AFC_ERR_NO_ERROR );

	if ( ( head = afc_string_new ( 16384 ) ) == NULL ) return ( AFC_LOG_FAST ( AFC_ERR_NO_MEMORY ) );

	cgi->headers_sent = TRUE;

	afc_cgi_manager_get_header_str ( cgi, head );

	printf ( "%s", head );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_cgi_manager_set_content_type ( cgi, type )
/*
@node afc_cgi_manager_set_content_type

	         NAME: afc_cgi_manager_set_content_type ( cgi_manager, type )  - Set the returning content type

	     SYNOPSIS: int afc_cgi_manager_set_content_type ( CGIManager * cgi_manager, char * type )

	  DESCRIPTION: 	This function sets the content type that will be sent back to the browser by the afc_cgi_manager_write_header()
		 	call. 
	               
	        INPUT: 	- cgi_manager    - Pointer to a valid afc_cgi_manager instance.
		 	- type           - The content type you want to set. Some examples are:
						+ "text/html" - HTML page
						+ "image/gif" - A GIF Image
	                                  	+ "text/ascii" - A text file

	      RESULTS: should return AFC_ERR_NO_ERROR
	              
	     SEE ALSO: - afc_cgi_manager_set_cookie() 
		 - afc_cgi_manager_write_header()
@endnode
*/
int afc_cgi_manager_set_content_type ( CGIManager * cgi, char * type )
{
	afc_string_copy ( cgi->content_type, type, ALL );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_cgi_manager_set_cookie ( cgi, key, val )
/*
@node afc_cgi_manager_set_cookie

	         NAME: afc_cgi_manager_set_cookie ( cgi_manager, key, value )  - Set a new cookie

	     SYNOPSIS: int afc_cgi_manager_set_cookie ( CGIManager * cgi_manager, char * key, char * value )

	  DESCRIPTION: This function sets (or resets) a new cookie.
	               
	        INPUT: 	- cgi_manager    - Pointer to a valid afc_cgi_manager instance.
		 	- key            - Cookie name
	               	- value          - Cookie value

	      RESULTS: should return AFC_ERR_NO_ERROR

		NOTES: - if you set a cookie, then the CGIManager will be automatically set to appropriately handle cookies,
	                 even if you didn't set it explicitely with afc_cgi_manager_set_tag().
	              
	     SEE ALSO: 	- afc_cgi_manager_get_cookie() 
		 	- afc_cgi_manager_set_cookie_domain()
		 	- afc_cgi_manager_set_cookie_expire()
		 	- afc_cgi_manager_set_cookie_path()
		
@endnode
*/
int afc_cgi_manager_set_cookie ( CGIManager * cgi, char * key, char * value )
{
TRY ( int )

	char * k = afc_string_dup ( key );
	char * v = afc_string_dup ( value );
	int    res;

	cgi->handle_cookies = TRUE;	// If a cookie is set, then we want to handle it.
	afc_string_upper ( k );

	if ( ( res = afc_dictionary_set ( cgi->cookies, k, v ) ) != AFC_ERR_NO_ERROR )
		RAISE_RC ( AFC_LOG_ERROR, res, "set_cookie", key, res );

	RETURN ( AFC_ERR_NO_ERROR );

EXCEPT

FINALLY
	afc_string_delete ( k );

ENDTRY
}
// }}}
// {{{ afc_cgi_manager_get_cookie ( cgi, key )
/*
@node afc_cgi_manager_get_cookie

	         NAME: afc_cgi_manager_get_cookie ( cgi_manager, key )  - Retrieve a cookie value

	     SYNOPSIS: char * afc_cgi_manager_get_cookie ( CGIManager * cgi_manager, char * key )

	  DESCRIPTION: This function retrieves a cookie value. If the cookie does not exists, then
	               a NULL pointer will be returned.
	               
	        INPUT: 	- cgi_manager    - Pointer to a valid afc_cgi_manager instance.
		 	- key            - Cookie name
	               	- value          - Cookie value

	      RESULTS: 	- the value of the cookie you requested
		 	- NULL in case of errors (cookie not found)

		NOTES: - if you set a cookie, then the CGIManager will be automatically set to appropriately handle cookies,
	                 even if you didn't set it explicitely with afc_cgi_manager_set_tag().
	              
	     SEE ALSO: - afc_cgi_manager_set_cookie() 
		
@endnode
*/
char * afc_cgi_manager_get_cookie ( CGIManager * cgi, char * key )
{
	char * k = afc_string_dup ( key );
	char * s;

	afc_string_upper ( k );

	s = ( char * ) afc_dictionary_get ( cgi->cookies, k );

	afc_string_delete ( k );

	return ( s );	
}
// }}}
// {{{ afc_cgi_manager_set_cookie_domain ( cgi, domain )
/*
@node afc_cgi_manager_set_cookie_domain

	         NAME: afc_cgi_manager_set_cookie_domain ( cgi_manager, domain )  - Set the cookies domain

	     SYNOPSIS: int afc_cgi_manager_set_cookie_domain ( CGIManager * cgi_manager, char * domain )

	  DESCRIPTION: 	This function sets the new cookie domain. By default, a cookie can be read back only by the
		 	same site, but this sometimes is limitating. Suppose you have a bunch of sites, like:
		 	site1.somedomain.com, site2.somedomain.com, site3.somedomain.com and you set a cookie when
	               	the user is on site1. If it moves to site2, you will not be able to read back the cookie you set.
	               	That's where this function comes handy: you can set a /sub-domain/ that will match all your sites, 
	               	passing a string like ".somedomain.com", all sites belonging to /somedomain.com/ will be able to
	               	read and set the same cookies.
	               
	        INPUT: - cgi_manager    - Pointer to a valid afc_cgi_manager instance.
	               - domain         - Domain you want to set

	      RESULTS: should return AFC_ERR_NO_ERROR

	     SEE ALSO: - afc_cgi_manager_set_cookie() 
		 - afc_cgi_manager_set_cookie_expire()
		 - afc_cgi_manager_set_cookie_path()
		
@endnode
*/
int afc_cgi_manager_set_cookie_domain ( CGIManager * cgi, char * dom )
{
	afc_string_copy ( cgi->cookies_domain, dom, ALL );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_cgi_manager_set_cookie_path ( cgi, path )
/*
@node afc_cgi_manager_set_cookie_path

	         NAME: afc_cgi_manager_set_cookie_path ( cgi_manager, path )  - Set the cookies path

	     SYNOPSIS: int afc_cgi_manager_set_cookie_path ( CGIManager * cgi_manager, char * path )

	  DESCRIPTION: This function sets the new cookie path. Please, read the discussion in afc_cgi_manager_set_cookie_domain()
		 for more info. If you want a cookie to be seen in all parts of your site, set the path to "/"
	               
	        INPUT: - cgi_manager    - Pointer to a valid afc_cgi_manager instance.
	               - path           - path you want to set

	      RESULTS: should return AFC_ERR_NO_ERROR

	     SEE ALSO: 	- afc_cgi_manager_set_cookie() 
		 	- afc_cgi_manager_set_cookie_expire()
		 	- afc_cgi_manager_set_cookie_domain()
		
@endnode
*/
int afc_cgi_manager_set_cookie_path ( CGIManager * cgi, char * path )
{
	afc_string_copy ( cgi->cookies_path, path, ALL );
	
	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_cgi_manager_set_cookie_expire ( cgi, days )
/*
@node afc_cgi_manager_set_cookie_expire

	         NAME: afc_cgi_manager_set_cookie_expire ( cgi_manager, days )  - Set the cookies path

	     SYNOPSIS: int afc_cgi_manager_set_cookie_expire ( CGIManager * cgi_manager, int days )

	  DESCRIPTION: This function sets the lifecycle of a cookie. By default, cookies are meant to be binded to
	               the current session and expire as soon as the user closes its browser. You can make cookies
	               last longer by setting the expire date (specified in days).
	               
	        INPUT: - cgi_manager    - Pointer to a valid afc_cgi_manager instance.
	               - days           - How many days should the cookie last.

	      RESULTS: should return AFC_ERR_NO_ERROR

	     SEE ALSO: 	- afc_cgi_manager_set_cookie() 
		 	- afc_cgi_manager_set_cookie_path()
		 	- afc_cgi_manager_set_cookie_domain()
		
@endnode
*/
int afc_cgi_manager_set_cookie_expire ( CGIManager * cgi, int days )
{
	time_t curr_time;
	struct tm * tim;

	time ( &curr_time );

	curr_time += ( 86400 * days );

	tim = localtime ( &curr_time );

	strftime ( cgi->cookies_expire, afc_string_max ( cgi->cookies_expire ), "%a, %d %b %Y %H:%M:%S", tim );

	afc_string_reset_len ( cgi->cookies_expire );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_cgi_manager_set_tag ( cgi, tag, value )
/*
@node afc_cgi_manager_set_tag

	         NAME: afc_cgi_manager_set_tag ( cgi_manager, tag, value )  - Set a tag

	     SYNOPSIS: int afc_cgi_manager_set_tag ( CGIManager * cgi_manager, int tag, void * value )

	  DESCRIPTION: This function sets a tag in the class.
	               
	        INPUT: - cgi_manager    - Pointer to a valid afc_cgi_manager instance.
	               - tag		  - Tag to be set. Valid values are:
						+ AFC_CGI_MANAGER_TAG_HANDLE_COOKIES - Defines whether the instance should 
										 handles cookies or not. Valid values are:

							* TRUE - The CGIManager will handle cookies
							* FALSE - The CGIManager will not handle cookies (default)

	      RESULTS: should return AFC_ERR_NO_ERROR

	     SEE ALSO: 	- afc_cgi_manager_set_cookie() 
		 	- afc_cgi_manager_set_cookie_path()
		 	- afc_cgi_manager_set_cookie_domain()
		
@endnode
*/
int afc_cgi_manager_set_tag ( CGIManager * cgi, int tag, void * val )
{
	switch ( tag )
	{
		case AFC_CGI_MANAGER_TAG_HANDLE_COOKIES:
			cgi->handle_cookies = ( short ) ( int ) ( long ) val;
			break;
	}

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_cgi_manager_debug_dump ( cgi )
/*
@node afc_cgi_manager_debug_dump 

	         NAME: afc_cgi_manager_debug_dump  ( cgi_manager )  - Dumps all data for debug

	     SYNOPSIS: int afc_cgi_manager_debug_dump  ( CGIManager * cgi_manager )

	  DESCRIPTION: This function dumps all data for debug. The dump is done inside an HTML table.
	               
	        INPUT: - cgi_manager    - Pointer to a valid afc_cgi_manager instance.

	      RESULTS: should return AFC_ERR_NO_ERROR

	     SEE ALSO: 
		
@endnode
*/
int afc_cgi_manager_debug_dump ( CGIManager * cgi )
{
	afc_cgi_manager_internal_dump ( cgi, cgi->headers, "Header's Values" );
	afc_cgi_manager_internal_dump ( cgi, cgi->fields,  "Form's Values" );
	afc_cgi_manager_internal_dump ( cgi, cgi->cookies, "Cookies" );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_cgi_manager_set_header_value ( cgi, key, val )
/*
@node afc_cgi_manager_set_header_value 

	         NAME: afc_cgi_manager_set_header_value  ( cgi_manager, key, val )  - Sets an Header Value

	     SYNOPSIS: int afc_cgi_manager_set_header_value  ( CGIManager * cgi_manager, const char * key, const char * val )

	  DESCRIPTION: This function sets an header variable to the given value. This function is mainly used in Win32 ISAPI
		       when there afc_cgi_manager_set_default_headers() cannot work.
	               
	        INPUT: - cgi_manager    - Pointer to a valid afc_cgi_manager instance.
		       - key		- Variable name
		       - val		- Variable value

	      RESULTS: should return AFC_ERR_NO_ERROR

	     SEE ALSO: - afc_cgi_manager_set_default_headers ()
		
@endnode
*/
int afc_cgi_manager_set_header_value ( CGIManager * cgi, const char * key, const char * val )
{
	cgi->are_headers_set = TRUE;

	afc_dictionary_set  ( cgi->headers, key, afc_string_dup ( val ) );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_cgi_manager_set_default_headers ( cgi )
/*
@node afc_cgi_manager_set_default_headers 

	         NAME: afc_cgi_manager_set_default_headers  ( cgi_manager )  - Gets all headers values from env

	     SYNOPSIS: int afc_cgi_manager_set_default_headers  ( CGIManager * cgi_manager )

	  DESCRIPTION: This function reads from the environment the CGI headers and sets them inside the /fields/
			Dictionary.
	               
	        INPUT: - cgi_manager    - Pointer to a valid afc_cgi_manager instance.

	      RESULTS: should return AFC_ERR_NO_ERROR

	     SEE ALSO: - afc_cgi_manager_set_header_value ()
		       - afc_cgi_manager_get_data ()
		
@endnode
*/
int afc_cgi_manager_set_default_headers ( CGIManager * cgi )
{
	static char * fields[] = {
					"GATEWAY_INTERFACE", "SERVER_PROTOCOL", "PATH_INFO", "PATH_TRANSLATED",
					"QUERY_STRING", "CONTENT_TYPE", "CONTENT_LENGTH", "REQUEST_METHOD",
					"SERVER_SOFTWARE", "SERVER_NAME", "SERVER_ADMIN", "SERVER_PORT", "SCRIPT_NAME",
					"DOCUMENT_ROOT", "REMOTE_HOST", "REMOTE_ADDR", "REMOTE_USER", "REMOTE_GROUP",
					"AUTH_TYPE", "REMOTE_IDENT", "HTTP_ACCEPT", "HTTP_COOKIE", "HTTP_ACCEPT_LANGUAGE",
					"HTTP_REFERER", "HTTP_USER_AGENT", 
					NULL 
				 };

	char * s;
	int t=0;

	cgi->are_headers_set = TRUE;

	while ( fields[t] )
	{
		if ( ( s = getenv ( fields[t] ) ) ) 
			if ( strlen ( s ) ) afc_dictionary_set  ( cgi->headers, fields[t], afc_string_dup ( s ) );

		t++;
	}

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_cgi_manager_get_header_str ( cgi, dest )
/*
@node afc_cgi_manager_get_header_str

	         NAME: afc_cgi_manager_get_header_str  ( cgi_manager, dest )  - Gets the Header String

	     SYNOPSIS: int afc_cgi_manager_get_header_str  ( CGIManager * cgi_manager, char * dest )

	  DESCRIPTION: This function creates an header string to be used with afc_cgi_manager_write_header()
		       or the corresponding Win32 ISAPI function.
	               
	        INPUT: - cgi_manager    - Pointer to a valid afc_cgi_manager instance.
		       - dest		- An AFC String where the data will be copied to.

	      RESULTS: should return AFC_ERR_NO_ERROR

		NOTES: - Remember: /dest/ is an AFC string created with afc_string_new()

	     SEE ALSO: - afc_cgi_manager_set_header_value ()
		       - afc_cgi_manager_get_data ()
@endnode
*/
int afc_cgi_manager_get_header_str ( CGIManager * cgi, char * dest )
{
	char * tmp;
	char * val;

	if ( ( tmp = afc_string_new ( 8192 ) ) == NULL ) return ( AFC_LOG_FAST ( AFC_ERR_NO_MEMORY ) );

	afc_string_make ( dest, "Content-type: %s;\r\n", cgi->content_type );

	if ( cgi->handle_cookies )
	{
		val = afc_dictionary_first ( cgi->cookies );
		while ( val )
		{
			afc_string_make ( tmp, "Set-Cookie: %s=%s; expires=%s GMT;", afc_dictionary_get_key ( cgi->cookies ), val, cgi->cookies_expire );
			afc_string_add ( dest, tmp, ALL );

			if ( afc_string_len ( cgi->cookies_path ) )
			{
				afc_string_make ( tmp, " path=%s;", cgi->cookies_path );
				afc_string_add ( dest, tmp, ALL );
			}

			if ( afc_string_len ( cgi->cookies_domain ) )
			{
				afc_string_make ( tmp,  " domain=%s;",  cgi->cookies_domain );
				afc_string_add  ( dest, tmp, ALL );
			}
	
			afc_string_add ( dest, "\r\n", ALL );
			
			val = afc_dictionary_next ( cgi->cookies );
		} 
	}

	afc_string_add ( dest, "\r\n", ALL );

	afc_string_delete ( tmp );

	return ( AFC_ERR_NO_ERROR );
}
// }}}

/*
	INTERNAL FUNCTIONS
*/
static int afc_cgi_manager_internal_dump ( CGIManager * cgi, Dictionary * dict, char * message )
{
	char * key;

	printf ( "<table border=\"0\">\n" );
	printf ( "<tr><td colspan=\"2\" align=\"center\">%s</td></tr>\n", message );
	key = ( char * ) afc_dictionary_first ( dict );
	while ( key )
	{
		printf ( "<tr><td align=\"right\">%s:</td><td>%s</td></tr>\n", afc_dictionary_get_key ( dict ), key );
		key = ( char * ) afc_dictionary_next ( dict );
	} 
	printf ( "</table>\n");

	return ( AFC_ERR_NO_ERROR );
}

static int afc_cgi_manager_internal_get_headers ( CGIManager * cgi )
{
	char * s;

	if ( ( s = afc_dictionary_get  ( cgi->headers, "REQUEST_METHOD" ) ) == NULL )
	{
		AFC_DEBUG ( AFC_DEBUG_VERBOSE, "REQUEST_METHOD *NOT* set" );
		return ( AFC_ERR_NO_ERROR );
	}

	_get_charset ( cgi );
	
	afc_debug_adv ( __internal_afc_base, AFC_DEBUG_VERBOSE, class_name, "REQUEST_METHOD: %s", s );

	if ( strcasecmp ( s, "GET" ) == 0 )       cgi->method = AFC_CGI_MANAGER_METHOD_GET;
	else if ( strcasecmp ( s, "POST" ) == 0 ) cgi->method = AFC_CGI_MANAGER_METHOD_POST;
	
	return ( AFC_ERR_NO_ERROR );
}

static int _get_charset ( CGIManager * cgi )
{
	char * content = afc_dictionary_get ( cgi->headers, "CONTENT_TYPE" );
	char * x, * s, * c;

	if ( ! content ) return AFC_NO_ERR;

	s = afc_string_dup ( content );

	x = strstr ( s, "charset=" );
	if ( ! x ) 
	{
		afc_string_delete ( s );
		return AFC_NO_ERR;
	}

	x = x + 8;

	c = strstr ( x, ";" );
	if ( c ) *c = '\0';
	
	afc_string_copy ( cgi->charset, x, ALL );
	afc_string_delete ( s );

	return AFC_NO_ERR;
}

static int afc_cgi_manager_internal_method_get ( CGIManager * cgi )
{
	char * s;

	AFC_DEBUG_FUNC ();

	if ( ( s = afc_cgi_manager_get_val ( cgi, "QUERY_STRING" ) ) == NULL ) return ( AFC_ERR_NO_ERROR );

	return ( afc_cgi_manager_internal_parse_data ( cgi, s ) );
}

/*
	This function tries to get data from a POST request.
	It can work in two ways: the first is the 'standard' way, by getting data from the standard input, the second
	(explicitly added for Win32 ISAPI) is to get data from an already set POST_DATA variable (the POST_DATA should
	be set by using afc_cgi_manager_set_header_value() function call.
*/
static int afc_cgi_manager_internal_method_post ( CGIManager * cgi )
{
	int content_length;
	char * s;
	char * data;
	unsigned int bytes;

	AFC_DEBUG_FUNC ();

	if ( cgi->is_post_read ) return AFC_NO_ERR;


	if ( ( data = afc_cgi_manager_get_val ( cgi, "POST_DATA" ) ) == NULL )
	{
		if ( ( s = afc_cgi_manager_get_val ( cgi, "CONTENT_LENGTH" ) ) == NULL ) return ( AFC_ERR_NO_ERROR );
	
		content_length = atoi ( s ); /* convert the string length to integer */

		if ( ( data = afc_malloc ( sizeof ( char ) * content_length + 1 ) ) == NULL ) return ( AFC_LOG_FAST ( AFC_ERR_NO_MEMORY ) );

		bytes = fread ( data, sizeof ( char ), content_length, stdin );
		if ( bytes != ( unsigned int ) content_length) 
		{
			_afc_dprintf ( "%s::%s - bytes: %d - expected: %d\n", __FILE__, __FUNCTION__, bytes, content_length );
			afc_free ( data );
			return ( AFC_LOG ( AFC_LOG_ERROR, AFC_CGI_MANAGER_ERR_POST_READ, "fread() returns a different size of bytes", NULL ) );
		}

		data [ content_length ] = '\0'; 

		afc_cgi_manager_internal_parse_data ( cgi, data );

		afc_free ( data );
	} else {
		afc_cgi_manager_internal_parse_data ( cgi, data );
	}

	cgi->is_post_read = TRUE;

	return ( AFC_ERR_NO_ERROR );
}

static int afc_cgi_manager_internal_parse_data ( CGIManager * cgi, char * data )
{
	char * s;

	AFC_DEBUG_FUNC ( );

	/* Translate "+" chars in " " */
	s = data;
	while ( s[0] )
	{
		if ( s[0]=='+'  ) s[0]=' ';
		s++;
	}

	/* Split into tokens */
	afc_stringnode_split ( cgi->split, data, "&" );

	s = afc_stringnode_first ( cgi->split );
	while ( s )
	{
		afc_string_trim ( s );
		if ( afc_string_len ( s ) ) afc_cgi_manager_internal_add_key ( cgi, s, AFC_CGI_MANAGER_MODE_FORM );

		s = afc_stringnode_next ( cgi->split );
	}

	return ( AFC_ERR_NO_ERROR );
}

static int afc_cgi_manager_internal_add_key ( CGIManager * cgi, char * keyval, int mode )
{
	char * str = strstr ( keyval, "=" );
	char * key, * value;
	Dictionary * dict = NULL;

	AFC_DEBUG_FUNC ();

	afc_debug_adv ( __internal_afc_base, AFC_DEBUG_VERBOSE, class_name, "Add key: %s", keyval );

	// FIXME: if str is null, the key is not valid (MS Internet Explorer Only)
	if ( str == NULL ) return ( AFC_ERR_NO_ERROR );

	if ( strlen ( keyval ) == 0 ) return ( AFC_ERR_NO_ERROR );
	if ( strlen ( str ) == 1 )    return ( AFC_ERR_NO_ERROR );

	if ( mode == AFC_CGI_MANAGER_MODE_FORM )        dict = cgi->fields;
	else if ( mode == AFC_CGI_MANAGER_MODE_COOKIE ) dict = cgi->cookies;

	// FIXME: maybe I should return an error if no dictionary has been selected
	if ( dict == NULL ) return ( AFC_ERR_NO_ERROR );

	if ( str != NULL ) str[0]='\0';
	
	key = afc_string_dup ( keyval );	// Duplicate the Key to make it uppercase
	afc_string_upper ( key );
	afc_cgi_manager_internal_unescape ( cgi, key );

	value = afc_string_dup ( str+1 );

	if ( strcmp ( key, "HTTP_COOKIE" ) != 0 ) 		// We do not unescape cookies... yet
	  afc_cgi_manager_internal_unescape ( cgi, value );

	afc_dictionary_set ( dict, key, value );

	afc_string_delete ( key ); // Since Dictionary creates a copy of the key itself, we can free this one
	
	return ( AFC_ERR_NO_ERROR );
}

/* Cookie Section */
static int afc_cgi_manager_internal_get_cookies ( CGIManager * cgi )
{
	char * cookie_string = afc_cgi_manager_get_val ( cgi, "HTTP_COOKIE" );
	char * s;

	AFC_DEBUG_FUNC ();

	if ( cookie_string == NULL ) return ( AFC_ERR_NO_ERROR );

	afc_stringnode_split ( cgi->split, cookie_string, "; " );

	if ( ( s = afc_stringnode_first ( cgi->split ) ) == NULL ) return ( AFC_ERR_NO_ERROR );

	while ( s )
	{
		if ( afc_string_len ( s ) ) afc_cgi_manager_internal_add_key ( cgi, s, AFC_CGI_MANAGER_MODE_COOKIE );
		s = afc_stringnode_next ( cgi->split );
	}

	return ( AFC_ERR_NO_ERROR );
}

/* Common section */
static char afc_cgi_manager_internal_decode ( CGIManager * cgi, char * str)
{
	return ( ( ( (str[0] >= 'A' ? ( ( str[0] & 0xdf ) -'A' ) + 10 : ( str[0] - '0' ) ) ) * 16 ) + ( str[1] >= 'A' ? ( ( str[1] & 0xdf) - 'A' ) +10 : ( str[1] - '0' ) ) );
}

static int afc_cgi_manager_internal_unescape (  CGIManager * cgi, char * str )
{
	register int x,y;

	AFC_DEBUG_FUNC ();

	for ( x=0, y=0; str[y]; ++x, ++y )
	{
	  	if ( ( str[x] = str[y] ) == '%' )
	  	{
			str[x] = afc_cgi_manager_internal_decode ( cgi, &str[y+1] );
			y+=2;
		}
	}
	str[x] = '\0';

	return ( AFC_ERR_NO_ERROR );
}

static int afc_cgi_manager_internal_clear_dict ( CGIManager * cgi, Dictionary * dict )
{
	char * val;

	afc_dictionary_before_first ( dict );

	while ( ( val = afc_dictionary_next ( dict ) ) ) afc_string_delete ( val );

	afc_dictionary_clear ( dict );

	return ( AFC_ERR_NO_ERROR );
}

#ifdef TEST_CGI_MANAGER
int main ( int argc, char * argv[] )
{
	CGIManager * cgi_manager = afc_cgi_manager_new();

	if ( cgi_manager == NULL ) 
	{
	  fprintf ( stderr, "Init of class CGIManager failed.\n" );
	  return ( 1 );
	}

	cgi_manager->handle_cookies = TRUE;

	afc_cgi_manager_get_data ( cgi_manager );
	afc_cgi_manager_set_cookie_expire ( cgi_manager, 1 );
	afc_cgi_manager_set_cookie_path ( cgi_manager, "/" );
	afc_cgi_manager_set_cookie ( cgi_manager, "clark", "superman" );

	afc_cgi_manager_set_content_type ( cgi_manager, "text/plain" );

	afc_cgi_manager_write_header ( cgi_manager );

	afc_cgi_manager_dump_all ( cgi_manager );

	afc_cgi_manager_delete ( cgi_manager );

	return ( 0 ); 
}
#endif
