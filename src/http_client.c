#include "http_client.h"

static const char class_name [] = "HttpClient";

/*
@node afc_http_client_new

           NAME: afc_http_client_new () - Initializes a new HttpClient instance.

       SYNOPSIS: HttpClient * afc_http_client_new ()

    DESCRIPTION: This function initializes a new HttpClient instance.

          INPUT: NONE

        RESULTS: a valid inizialized HttpClient structure. NULL in case of errors.

       SEE ALSO: - afc_http_client_delete()

@endnode
*/
HttpClient * afc_http_client_new ( char * host, int port )
{
  	HttpClient * hc = ( HttpClient * ) afc_malloc ( sizeof ( HttpClient ) );

  	if ( hc == NULL )
   	{
    		AFC_LOG_FAST ( AFC_ERR_NO_MEMORY );
    		return ( NULL );
  	}

  	hc->magic = AFC_HTTP_CLIENT_MAGIC;
  	if ( ( hc->inet = afc_inet_client_new () ) == NULL ) 
  	{
    		AFC_LOG_FAST_INFO ( AFC_ERR_NO_MEMORY, "inet" );
    		afc_http_client_delete ( hc );
    		return ( NULL );
  	}
	
	hc->host = afc_string_dup ( host );
	hc->port = port;
	hc->isconnected = FALSE;

  	return ( hc );
}


/*
@node afc_http_client_delete

           NAME: afc_http_client_delete ( hc )  - Disposes a valid HttpClient instance.

       SYNOPSIS: int afc_http_client_delete ( HttpClient * hc)

    DESCRIPTION: This function frees an already alloc'd HttpClient structure.

          INPUT: - hc  - Pointer to a valid afc_http_client class.

        RESULTS: should be AFC_ERR_NO_ERROR

          NOTES: - this method calls: afc_http_client_clear()

       SEE ALSO: - afc_http_client_new()
                 - afc_http_client_clear()
@endnode

*/
int afc_http_client_delete ( HttpClient * hc ) 
{
  	int afc_res; 

  	if ( ( afc_res = afc_http_client_clear ( hc ) ) != AFC_ERR_NO_ERROR ) return ( afc_res );

  	if ( hc->inet ) afc_inet_client_delete ( hc->inet );

  	/* NOTE: any class contained in afc_http_client should be deleted here */

  	afc_free ( hc );

  	return ( AFC_ERR_NO_ERROR );
}

/*
@node afc_http_client_clear

           NAME: afc_http_client_clear ( hc )  - Clears all stored data

       SYNOPSIS: int afc_http_client_clear ( HttpClient * hc)

    DESCRIPTION: Use this function to clear all stored data in the current hc instance.

          INPUT: - hc    - Pointer to a valid afc_http_client instance.

        RESULTS: should be AFC_ERR_NO_ERROR
                
       SEE ALSO: - afc_http_client_delete()
                 
@endnode

*/
int afc_http_client_clear ( HttpClient * hc ) 
{
  	if ( hc == NULL ) return ( AFC_ERR_NULL_POINTER );
 
  	if ( hc->magic != AFC_HTTP_CLIENT_MAGIC ) return ( AFC_ERR_INVALID_POINTER );



  	/* Custom Clean-up code should go here */

	if ( hc->isconnected )
	{ 
		afc_inet_client_close ( hc->inet );
		hc->isconnected = FALSE;
	}

  	if ( hc->inet ) afc_inet_client_clear ( hc->inet );
	
	afc_string_delete ( hc->host );
	hc->host = afc_string_new ( 1024 );
	hc->port = 80;
	
  	return ( AFC_ERR_NO_ERROR );
}

int afc_http_client_close ( HttpClient * hc )
{
	return ( hc->isconnected ? afc_inet_client_close ( hc->inet ) : AFC_ERR_NO_ERROR );
}

int afc_http_client_request ( HttpClient * hc, char * command, char * url, char * data, int datalen, Dictionary * headers )
{
TRY ( int )
	int res;
	char * cmd, * val, * key;
	
	if ( !hc->isconnected ) 
	{
		res = afc_inet_client_open ( hc->inet, hc->host, hc->port ); 
		if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_REQUEST, "Cannot open connection", hc->host, res );
		hc->isconnected = TRUE;
	}

	cmd = afc_string_new ( afc_string_len ( command ) + afc_string_len ( url ) + 13 );
	afc_string_make ( cmd, "%s %s HTTP/1.1\r\n", command, url );
	
	res = afc_inet_client_send ( hc->inet, cmd, afc_string_len ( cmd ) ); 
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_REQUEST, "Cannot send command", cmd, res );
	
	if ( headers && ( val = ( char * ) afc_dictionary_first ( headers ) ) ) 
	{
		do 
		{
			
			key = afc_dictionary_get_key ( headers );
			
			afc_string_delete ( cmd );
			cmd = afc_string_new ( afc_string_len ( key ) + afc_string_len ( val ) + 5 );

			afc_string_make ( cmd, "%s: %s\r\n", key, val );

			res = afc_inet_client_send ( hc->inet, cmd, afc_string_len ( cmd ) ); 
			if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_REQUEST, "Cannot send header", cmd, res );
			
		} while ( ( val = ( char * ) afc_dictionary_succ ( headers ) ) );
	}
	res = afc_inet_client_send ( hc->inet, "\r\n", 2 ); 
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_REQUEST, "Cannot end line", "", res );
	
	if ( data )
	{	
		res = afc_inet_client_send ( hc->inet, data, datalen ); 
		if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_REQUEST, "Cannot send data", data, res );
	}		
	
	RETURN ( AFC_ERR_NO_ERROR );
	
EXCEPT 

FINALLY

ENDTRY	
}


/*Internal Functions*/

int afc_http_client_internal_getresponse ( HttpClient * hc )
{
TRY ( int )
	FILE * fd;
	char * buf = afc_string_new ( 1024 );
	char * pver, * pcode, * preason;
	int res;

	//get the status line
	fd = afc_inet_client_get_file ( hc->inet );

	afc_string_fget ( buf, fd );
	afc_string_trim ( buf );

	pver = buf;
	pcode = strchr ( buf, ' ' );
	if ( !pcode ) RAISE_RC ( AFC_LOG_ERROR, AFC_HTTP_CLIENT_ERR_GETRESP, "Cannot get status line", buf, res );

	RETURN ( AFC_ERR_NO_ERROR );

EXCEPT

FINALLY
	afc_string_delete ( buf );

ENDTRY
}


#ifdef TEST_CLASS
int main ( int argc, char * argv[] )
{
	int res; 
	
  	AFC * afc = afc_new ();
  	HttpClient * hc = afc_http_client_new( "10.0.20.3" , 80 );

  	if ( hc == NULL ) 
  	{
    		fprintf ( stderr, "Init of class HttpClient failed.\n" );
    		return ( 1 );
  	}
	
	res = afc_http_client_request ( hc, "GET", "/", NULL, 0, NULL );

  	afc_http_client_delete ( hc );
  	afc_delete ( afc );

  	return ( 0 ); 
}
#endif
