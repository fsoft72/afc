#include "ftp_client.h"

static const char class_name [] = "FtpClient";

static int afc_ftp_client_internal_get_answer ( FtpClient * fc, char * answer );
static int afc_ftp_client_internal_retr_active ( FtpClient * fc, char * command, afc_ftp_client_retr_callback callback, void * param, int block_size, int rest, BOOL lines );
static int afc_ftp_client_internal_retr_passive ( FtpClient * fc, char * command, afc_ftp_client_retr_callback callback, void * param, int block_size, int rest, BOOL lines );
static int afc_ftp_client_internal_store_active ( FtpClient * fc, char * command, afc_ftp_client_store_callback callback, void * param, int block_size, BOOL lines );
static int afc_ftp_client_internal_store_passive ( FtpClient * fc, char * command, afc_ftp_client_store_callback callback, void * param, int block_size, BOOL lines );

/*
@node afc_ftp_client_new

           NAME: afc_ftp_client_new () - Initializes a new FtpClient instance.

       SYNOPSIS: FtpClient * afc_ftp_client_new ()

    DESCRIPTION: This function initializes a new FtpClient instance.

          INPUT: NONE

        RESULTS: a valid inizialized FtpClient structure. NULL in case of errors.

       SEE ALSO: - afc_ftp_client_delete()

@endnode
*/
FtpClient * afc_ftp_client_new ( void )
{
  	FtpClient * fc = ( FtpClient * ) afc_malloc ( sizeof ( FtpClient ) );

  	if ( fc == NULL )
   	{
    		AFC_LOG_FAST ( AFC_ERR_NO_MEMORY );
    		return ( NULL );
  	}

  	fc->magic = AFC_FTP_CLIENT_MAGIC;
  	if ( ( fc->inet = afc_inet_client_new () ) == NULL ) 
  	{
    		AFC_LOG_FAST_INFO ( AFC_ERR_NO_MEMORY, "inet" );
    		afc_ftp_client_delete ( fc);
    		return ( NULL );
  	}

	if ( ( fc->last_answer = afc_string_new ( 1024 ) ) == NULL )
        {
                AFC_LOG_FAST_INFO ( AFC_ERR_NO_MEMORY, "last_answer" );
		afc_inet_client_delete ( fc->inet );
                afc_ftp_client_delete ( fc);
		
                return ( NULL );
        }

	fc->pasv = FALSE;


  	return ( fc );
}


/*
@node afc_ftp_client_delete

           NAME: afc_ftp_client_delete ( fc )  - Disposes a valid FtpClient instance.

       SYNOPSIS: int afc_ftp_client_delete ( FtpClient * fc)

    DESCRIPTION: This function frees an already alloc'd FtpClient structure.

          INPUT: - fc  - Pointer to a valid afc_ftp_client class.

        RESULTS: should be AFC_ERR_NO_ERROR

          NOTES: - this method calls: afc_ftp_client_clear()

       SEE ALSO: - afc_ftp_client_new()
                 - afc_ftp_client_clear()
@endnode

*/
int afc_ftp_client_delete ( FtpClient * fc ) 
{
  	int afc_res; 

  	if ( ( afc_res = afc_ftp_client_clear ( fc ) ) != AFC_ERR_NO_ERROR ) return ( afc_res );

  	if ( fc->inet ) afc_inet_client_delete ( fc->inet );

	if ( fc->last_answer ) afc_string_delete ( fc->last_answer );

  /* NOTE: any class contained in afc_ftp_client should be deleted here */

  afc_free ( fc );

  return ( AFC_ERR_NO_ERROR );
}

/*
@node afc_ftp_client_clear

           NAME: afc_ftp_client_clear ( fc )  - Clears all stored data

       SYNOPSIS: int afc_ftp_client_clear ( FtpClient * fc)

    DESCRIPTION: Use this function to clear all stored data in the current fc instance.

          INPUT: - fc    - Pointer to a valid afc_ftp_client instance.

        RESULTS: should be AFC_ERR_NO_ERROR
                
       SEE ALSO: - afc_ftp_client_delete()
                 
@endnode

*/
int afc_ftp_client_clear ( FtpClient * fc ) 
{
  	if ( fc == NULL ) return ( AFC_ERR_NULL_POINTER );
 
  	if ( fc->magic != AFC_FTP_CLIENT_MAGIC ) return ( AFC_ERR_INVALID_POINTER );

	if ( fc->last_answer ) afc_string_clear ( fc->last_answer );

	fc->last_code = 0;

	fc->pasv = FALSE;

  /* Custom Clean-up code should go here */

  if ( fc->inet ) afc_inet_client_clear ( fc->inet );

  return ( AFC_ERR_NO_ERROR );
}

int afc_ftp_client_connect ( FtpClient * fc, char * host, int port )
{
TRY ( int )

  	int res = AFC_ERR_NULL_POINTER;
	int code;
	char * answer = afc_string_new ( 1024 );

	afc_dprintf ( "%s: Connecting to %s on port %d\n", __FUNCTION__, host, port );

  	res = afc_inet_client_open ( fc->inet, host, port );

  	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_CONNECTION, "Connection Error", host, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );


  	if ( code != 220 ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_CONNECTION, "Connection Error", host, res );

	RETURN ( AFC_ERR_NO_ERROR );

EXCEPT

FINALLY
	afc_string_delete ( answer );

ENDTRY
}

int afc_ftp_client_cwd ( FtpClient * fc, char * pathname )
{ 
TRY ( int )
	char * cmd;
	int res;
	char * answer;
	int code;

	cmd = afc_string_new ( 1024 );
	answer = afc_string_new ( 1024 );

	afc_dprintf ( "%s: changing remote directory to %s\n", __FUNCTION__, pathname ); 

	afc_string_make ( cmd, "CWD %s\r\n", pathname );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) );
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_CWD, "Cannot change directory", pathname, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 250 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_CWD, "Cannot change directory", pathname, AFC_FTP_CLIENT_ERR_CWD );
	}	

	RETURN ( AFC_ERR_NO_ERROR );	

EXCEPT 

FINALLY 

	afc_string_delete ( cmd );
	afc_string_delete ( answer );

ENDTRY
	
}

int afc_ftp_client_delete_file ( FtpClient * fc, char * filename )
{ 
TRY ( int )
	char * cmd;
	int res;
	char * answer;
	int code;

	cmd = afc_string_new ( 1024 );
	answer = afc_string_new ( 1024 );

	afc_dprintf ( "%s: deleting file %s\n", __FUNCTION__, filename ); 

	afc_string_make ( cmd, "DELE %s\r\n", filename );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) );
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_DELE, "Cannot delete file", filename, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 250 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_DELE, "Cannot delete file", filename, AFC_FTP_CLIENT_ERR_DELE );
	}	

	RETURN ( AFC_ERR_NO_ERROR );	

EXCEPT 

FINALLY 

	afc_string_delete ( cmd );
	afc_string_delete ( answer );

ENDTRY
	
}

int afc_ftp_client_mkd ( FtpClient * fc, char * pathname )
{ 
TRY ( int )
	char * cmd;
	int res;
	char * answer;
	int code;

	cmd = afc_string_new ( 1024 );
	answer = afc_string_new ( 1024 );

	afc_dprintf ( "%s: Creating directory %s\n", __FUNCTION__, pathname ); 

	afc_string_make ( cmd, "MKD %s\r\n", pathname );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) );
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_MKD, "Cannot create directory", pathname, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 257 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_MKD, "Cannot create directory", pathname, AFC_FTP_CLIENT_ERR_MKD );
	}	

	RETURN ( AFC_ERR_NO_ERROR );	

EXCEPT 

FINALLY 

	afc_string_delete ( cmd );
	afc_string_delete ( answer );

ENDTRY
	
}

int afc_ftp_client_rmd ( FtpClient * fc, char * pathname )
{ 
TRY ( int )
	char * cmd;
	int res;
	char * answer;
	int code;

	cmd = afc_string_new ( 1024 );
	answer = afc_string_new ( 1024 );

	afc_dprintf ( "%s: Deleting directory %s\n", __FUNCTION__, pathname ); 

	afc_string_make ( cmd, "RMD %s\r\n", pathname );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) );
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RMD, "Cannot delete directory", pathname, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 250 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RMD, "Cannot delete directory", pathname, AFC_FTP_CLIENT_ERR_RMD );
	}	

	RETURN ( AFC_ERR_NO_ERROR );	

EXCEPT 

FINALLY 

	afc_string_delete ( cmd );
	afc_string_delete ( answer );

ENDTRY
	
}

int afc_ftp_client_pwd ( FtpClient * fc, char * cdir )
{ 
TRY ( int )
	char * cmd;
	int res;
	char * answer, *p1, *p2;
	int code;

	cmd = afc_string_new ( 1024 );
	answer = afc_string_new ( 1024 );

	afc_dprintf ( "%s: retreiving current directory \n", __FUNCTION__ ); 

	afc_string_make ( cmd, "PWD\r\n" );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) );
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_PWD, "Cannot retreive current directory", "", res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 257 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_PWD, "Cannot retreive current directory", "", AFC_FTP_CLIENT_ERR_PWD );
	}
	
	p1 = strchr ( answer, '"' );
	if ( !p1 ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_PWD, "Cannot retreive current directory", "", AFC_FTP_CLIENT_ERR_PWD );

	p1++;

	p2 = strchr ( p1, '"');
	if ( !p2 ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_PWD, "Cannot retreive current directory", "", AFC_FTP_CLIENT_ERR_PWD );

        afc_string_mid ( cdir, answer, ( u_long ) ( p1 - answer ), ( u_long ) ( p2 - p1 ) );	

	RETURN ( AFC_ERR_NO_ERROR );

EXCEPT 

FINALLY 

	afc_string_delete ( cmd );
	afc_string_delete ( answer );

ENDTRY
	
}


int afc_ftp_client_quit ( FtpClient * fc )

{ 
TRY ( int )
	char * cmd;
	int res;
	char * answer;

	cmd = afc_string_new ( 1024 );
	answer = afc_string_new ( 1024 );

	afc_dprintf ( "%s: closing FTP connection\n", __FUNCTION__ ); 

	afc_string_make ( cmd, "QUIT\r\n" );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) );
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_QUIT, "Cannot quit FTP connection",  "", res );

	afc_inet_client_close ( fc->inet );

	RETURN ( AFC_ERR_NO_ERROR );	

EXCEPT 

FINALLY 

	afc_string_delete ( cmd );
	afc_string_delete ( answer );

ENDTRY
	
}


int afc_ftp_client_rename ( FtpClient * fc, char * oldname, char * newname )
{ 
TRY ( int )
	char * cmd;
	int res;
	char * answer;
	int code;

	cmd = afc_string_new ( 1024 );
	answer = afc_string_new ( 1024 );

	afc_dprintf ( "%s: Changing name for %s\n", __FUNCTION__, oldname ); 

	afc_string_make ( cmd, "RNFR %s\r\n", oldname );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) );
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RENAME, "Cannot Rename file", oldname, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 350 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RENAME, "Cannot rename file", oldname, AFC_FTP_CLIENT_ERR_RENAME );
	}
		
	afc_dprintf ( "%s: Renaming file to %s\n", __FUNCTION__, newname ); 

	afc_string_make ( cmd, "RNTO %s\r\n", newname );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) );
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RENAME, "Cannot Rename file", oldname, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 250 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RENAME, "Cannot Rename file", oldname, AFC_FTP_CLIENT_ERR_RENAME );
	}	
	RETURN ( AFC_ERR_NO_ERROR );	

EXCEPT 

FINALLY 

	afc_string_delete ( cmd );
	afc_string_delete ( answer );

ENDTRY
	
}


int afc_ftp_client_sendcmd ( FtpClient * fc, char * command )

{ 
TRY ( int )
	char * cmd;
	int res;
	char * answer;
	int code;

	cmd = afc_string_new ( 1024 );
	answer = afc_string_new ( 1024 );

	afc_dprintf ( "%s: Sending command %s\n", __FUNCTION__, command ); 

	afc_string_make ( cmd, "%s\r\n", command );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) );
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_SENDCMD, "Cannot send command", command, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	fc->last_code = code;
	if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
	fc->last_answer = afc_string_dup ( answer );

	RETURN ( AFC_ERR_NO_ERROR );	

EXCEPT 

FINALLY 

	afc_string_delete ( cmd );
	afc_string_delete ( answer );

ENDTRY
	
}


int afc_ftp_client_size ( FtpClient * fc, char * filename, int * file_size )
{ 
TRY ( int )
	char * cmd;
	int res;
	char * answer;
	int code;

	cmd = afc_string_new ( 1024 );
	answer = afc_string_new ( 1024 );

	afc_dprintf ( "%s: Getting file size for %s\n", __FUNCTION__, filename ); 

	afc_string_make ( cmd, "SIZE %s\r\n", filename );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) );
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_SIZE, "Cannot get file size", filename, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 213 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_SIZE, "Cannot get file size", filename, AFC_FTP_CLIENT_ERR_SIZE );
	}	

	afc_string_trim ( answer );
	* file_size = atoi ( answer );

	RETURN ( AFC_ERR_NO_ERROR );	

EXCEPT 

FINALLY 

	afc_string_delete ( cmd );
	afc_string_delete ( answer );

ENDTRY
	
}


int afc_ftp_client_retrbinary ( FtpClient * fc, char * command, afc_ftp_client_retr_callback callback, void * param, int block_size, int rest )
{
	if ( fc->pasv ) return afc_ftp_client_internal_retr_passive ( fc, command, callback, param, block_size, rest, FALSE );
	else return  afc_ftp_client_internal_retr_active ( fc, command, callback, param, block_size, rest, FALSE );
}


int afc_ftp_client_retrlines ( FtpClient * fc, char * command, afc_ftp_client_retr_callback callback, void * param )
{
	if ( fc->pasv ) return afc_ftp_client_internal_retr_passive ( fc, command, callback, param, 0, 0, TRUE );
	else return  afc_ftp_client_internal_retr_active ( fc, command, callback, param, 0, 0, TRUE );
}


int afc_ftp_client_storbinary ( FtpClient * fc, char * command, afc_ftp_client_store_callback callback, void * param, int block_size )
{
	if ( fc->pasv ) return afc_ftp_client_internal_store_passive ( fc, command, callback, param, block_size, FALSE );
	else return  afc_ftp_client_internal_store_active ( fc, command, callback, param, block_size, FALSE );
}


int afc_ftp_client_storlines ( FtpClient * fc, char * command, afc_ftp_client_store_callback callback, void * param )
{
	if ( fc->pasv ) return afc_ftp_client_internal_store_passive ( fc, command, callback, param, 1024, TRUE );
	else return  afc_ftp_client_internal_store_active ( fc, command, callback, param, 1024, TRUE );
}


int afc_ftp_client_set_pasv ( FtpClient * fc, BOOL pasv )
{ 
	fc->pasv = pasv;	
	return ( AFC_ERR_NO_ERROR );	
}



int afc_ftp_client_login ( FtpClient * fc, char * username, char * password )
{

TRY ( int )
	char * cmd;
	int res;
	char * answer;
	int code;

	cmd = afc_string_new ( 1024 );
	answer = afc_string_new ( 1024 );

	afc_dprintf ( "%s: sending username %s\n", __FUNCTION__, username ); 

	afc_string_make ( cmd, "USER %s\r\n", username );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) );
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_LOGIN, "Login Error", username, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 331 && code != 230 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_LOGIN, "Bad login or password", username, AFC_FTP_CLIENT_ERR_LOGIN );
	}	

	if ( code == 230 ) RETURN ( AFC_ERR_NO_ERROR );

	afc_dprintf ( "%s: sending password %s\n", __FUNCTION__, password );
	
	afc_string_make ( cmd, "PASS %s\r\n", password );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) );
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_LOGIN, "Login Error", username, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 230 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_LOGIN, "Bad login or password", username, AFC_FTP_CLIENT_ERR_LOGIN );
	}	

	RETURN ( AFC_ERR_NO_ERROR );	

EXCEPT 

FINALLY 

	afc_string_delete ( cmd );
	afc_string_delete ( answer );

ENDTRY
	

}


//Internal Functions
static int afc_ftp_client_internal_pasv ( FtpClient * fc, char * ip, int * port )
{
TRY ( int )
	char * cmd, * answer;
	u_char ip1, ip2, ip3, ip4, p1, p2;
	char * pc1, * pc2;
	int res, code;

	cmd = afc_string_new ( 1024 );
	answer = afc_string_new ( 1024 );

	afc_string_make ( cmd, "PASV\r\n" );

	afc_dprintf ( "%s: setting transfer mode to passive\n", __FUNCTION__ );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) ); 
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_PASV, "Cannot set passive mode", cmd, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 227 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_PASV, "Cannot send command", cmd, AFC_FTP_CLIENT_ERR_PASV );
	}	

	pc1 = strchr ( answer, '(' );
	if ( !pc1 ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_PASV, "Bad passive response", answer, AFC_FTP_CLIENT_ERR_PASV );

	pc1++;
	pc2 = strchr ( pc1, ',' );
	if ( !pc2 ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_PASV, "Bad passive response", answer, AFC_FTP_CLIENT_ERR_PASV );
		
	*pc2 = 0;
	ip1 = ( u_char ) atoi ( pc1 );
	*pc2 = ',';

	pc1 = pc2 + 1;

	pc2 = strchr ( pc1, ',' );
	if ( !pc2 ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_PASV, "Bad passive response", answer, AFC_FTP_CLIENT_ERR_PASV );
		
	*pc2 = 0;
	ip2 = ( u_char ) atoi ( pc1 );
	*pc2 = ',';

	pc1 = pc2 + 1;

	pc2 = strchr ( pc1, ',' );
	if ( !pc2 ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_PASV, "Bad passive response", answer, AFC_FTP_CLIENT_ERR_PASV );
		
	*pc2 = 0;
	ip3 = ( u_char ) atoi ( pc1 );
	*pc2 = ',';

	pc1 = pc2 + 1;
	pc2 = strchr ( pc1, ',' );
	if ( !pc2 ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_PASV, "Bad passive response", answer, AFC_FTP_CLIENT_ERR_PASV );
		
	*pc2 = 0;
	ip4 = ( u_char ) atoi ( pc1 );
	*pc2 = ',';

	pc1 = pc2 + 1;

	pc2 = strchr ( pc1, ',' );
	if ( !pc2 ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_PASV, "Bad passive response", answer, AFC_FTP_CLIENT_ERR_PASV );
		
	*pc2 = 0;
	p1 = ( u_char ) atoi ( pc1 );
	*pc2 = ',';

	pc1 = pc2 + 1;

	pc2 = strchr ( pc1, ')' );
	if ( !pc2 ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_PASV, "Bad passive response", answer, AFC_FTP_CLIENT_ERR_PASV );
		
	*pc2 = 0;
	p2 = ( u_char ) atoi ( pc1 );
	*pc2 = ',';

	sprintf ( ip, "%d.%d.%d.%d", ( int ) ip1, ( int ) ip2, ( int ) ip3, ( int ) ip4 );
	*port =  ( ( int ) p1 << 8 ) |  p2;

	RETURN ( AFC_ERR_NO_ERROR );

EXCEPT

FINALLY
	afc_string_delete ( cmd );
	afc_string_delete ( answer );

ENDTRY
}

static int afc_ftp_client_internal_store_passive ( FtpClient * fc, char * command, afc_ftp_client_store_callback callback, void * param, int block_size, BOOL lines )
{
TRY ( int )	
	InetClient * ic;
	int res, code, port, dlen;
	char * cmd, * answer;
	char ip[32];
	u_char * pbuffer;

	
	pbuffer = afc_malloc ( block_size );
	ic = afc_inet_client_new ( );
	cmd = afc_string_new ( 1024 );
	answer = afc_string_new ( 1024 );

	afc_string_make ( cmd, "TYPE %c\r\n", lines ? 'A' : 'I' );

	afc_dprintf ( "%s: setting %s transfer type\n", __FUNCTION__, lines ? "ASCII" : "binary" );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) ); 
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_STORE, "Cannot set transfer type", cmd, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 200 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_STORE, "Cannot send command", cmd, AFC_FTP_CLIENT_ERR_STORE );
	}

	res = afc_ftp_client_internal_pasv ( fc, ip, &port );
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_STORE, "Cannot set passive mode", "", res );

	afc_dprintf ( "%s: Opening data connection to %s on port %d\n", __FUNCTION__, ip, port );

  	res = afc_inet_client_open ( ic, ip, port );

  	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_STORE, "Cannot open data connection", ip, res );
	
	afc_string_make ( cmd, "%s\r\n", command );

	afc_dprintf ( "%s: sending command %s\n", __FUNCTION__, cmd );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) ); 
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_STORE, "Error sending command", cmd, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 125 && code != 150 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_STORE, "Cannot send command", cmd, AFC_FTP_CLIENT_ERR_STORE );
	}

	dlen = block_size;
	while ( callback ( pbuffer, &dlen, param) == AFC_ERR_NO_ERROR )
	{
		res = afc_inet_client_send ( ic, pbuffer, dlen );
		if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_STORE, "Error sending data", "", res );
	
		dlen = block_size;
	}

	afc_inet_client_close ( ic );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 226 && code != 250 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_STORE, "Error sending file", command, AFC_FTP_CLIENT_ERR_STORE );
	}



	RETURN ( AFC_ERR_NO_ERROR );	

EXCEPT 

FINALLY 

	afc_string_delete ( cmd );
	afc_string_delete ( answer );
	afc_inet_client_delete ( ic );
	afc_free ( pbuffer );

ENDTRY

}

static int afc_ftp_client_internal_store_active ( FtpClient * fc, char * command, afc_ftp_client_store_callback callback, void * param, int block_size, BOOL lines )
{
	return ( AFC_ERR_NO_ERROR );
}

	
static int afc_ftp_client_internal_retr_passive ( FtpClient * fc, char * command, afc_ftp_client_retr_callback callback, void * param, int block_size, int rest, BOOL lines )
{
TRY ( int )	
	InetClient * ic;
	int res, code, port, dlen;
	char * cmd, * answer;
	char ip[32];
	u_char * pbuffer;

	
	pbuffer = afc_malloc ( block_size );
	ic = afc_inet_client_new ( );
	cmd = afc_string_new ( 1024 );
	answer = afc_string_new ( 1024 );

	afc_string_make ( cmd, "TYPE %c\r\n", lines ? 'A' : 'I' );

	afc_dprintf ( "%s: setting %s transfer type\n", __FUNCTION__, lines ? "ASCII" : "binary" );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) ); 
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot set transfer type", cmd, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 200 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot send command", cmd, AFC_FTP_CLIENT_ERR_RETR );
	}

	res = afc_ftp_client_internal_pasv ( fc, ip, &port );
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot set passive mode", "", res );

	afc_dprintf ( "%s: Opening data connection to %s on port %d\n", __FUNCTION__, ip, port );

  	res = afc_inet_client_open ( ic, ip, port );

  	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot open data connection", ip, res );
	
	if ( rest )
	{
		afc_string_make ( cmd, "REST %d\r\n", rest );

		afc_dprintf ( "%s: sending command %s\n", __FUNCTION__, cmd );

		res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) ); 
		if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot set transfer type", cmd, res );

		code = afc_ftp_client_internal_get_answer ( fc, answer );
		if ( code != 350 ) 
		{
			fc->last_code = code;
			if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
			fc->last_answer = afc_string_dup ( answer );
			RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot send command", cmd, AFC_FTP_CLIENT_ERR_RETR );
		}
	}

	afc_string_make ( cmd, "%s\r\n", command );

	afc_dprintf ( "%s: sending command %s\n", __FUNCTION__, cmd );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) ); 
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Error sending command", cmd, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 125 && code != 150 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot send command", cmd, AFC_FTP_CLIENT_ERR_RETR );
	}

	if ( lines )
	{
		while ( afc_inet_client_get ( ic ) == AFC_ERR_NO_ERROR )
		{
			if ( callback )
			{
				res = callback ( ic->buf, afc_string_len ( ic->buf ), param );
				if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot recieve file", command, res );
			} else
				printf ( "%s", ic->buf );
		}

	} else {

		dlen = block_size;
		while ( afc_inet_client_get_binary ( ic, pbuffer, &dlen ) == AFC_ERR_NO_ERROR )
		{
			res = callback ( pbuffer, dlen, param );	
			if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot recieve file", command, res );
			dlen = block_size;
		}

	}

	afc_inet_client_close ( ic );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 226 && code != 250 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Error receiving file", command, AFC_FTP_CLIENT_ERR_RETR );
	}



	RETURN ( AFC_ERR_NO_ERROR );	

EXCEPT 

FINALLY 

	afc_string_delete ( cmd );
	afc_string_delete ( answer );
	afc_inet_client_delete ( ic );
	afc_free ( pbuffer );

ENDTRY

}


static int afc_ftp_client_internal_retr_active ( FtpClient * fc, char * file_or_cmd, afc_ftp_client_retr_callback callback, void * param, int block_size, int rest, BOOL lines )
{
TRY ( int )
	InetServer * is;
	struct sockaddr_in laddr, loc_ip;
	socklen_t addrlen = sizeof ( laddr );
	int res, addr, code;
	u_short port;
	u_char ip1, ip2, ip3, ip4, p1, p2;
	char * cmd, * answer;

	is = afc_inet_server_new ( );
	cmd = afc_string_new ( 1024 );
	answer = afc_string_new ( 1024 );

	afc_string_make ( cmd, "TYPE I\r\n" );

	afc_dprintf ( "%s: setting binary transfer type \n", __FUNCTION__ );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) ); 
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot set transfer type", cmd, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 200 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot send command", cmd, AFC_FTP_CLIENT_ERR_RETR );
	}
	
	res = afc_inet_server_create ( is, 0 );
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot bind port", file_or_cmd, res );

	res = getsockname ( is->listener, ( struct sockaddr * ) &laddr, &addrlen );
	if ( res != 0 ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot get local port number", file_or_cmd, res );

	addrlen = sizeof ( loc_ip );
	res = getsockname ( fc->inet->sockfd, ( struct sockaddr * ) &loc_ip, &addrlen );
	if ( res != 0 ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot get local IP address", file_or_cmd, res );

	addr = ntohl ( loc_ip.sin_addr.s_addr );
	port = ntohs ( laddr.sin_port );
	
	ip1 = ( u_char ) ( addr >> 24 );
	ip2 = ( u_char ) ( addr >> 16 );
	ip3 = ( u_char ) ( addr >>  8 );
	ip4 = ( u_char ) ( addr );
	p1  = ( u_char ) ( port >> 8 );
	p2  = ( u_char ) ( port );

	afc_string_make ( cmd, "PORT %d,%d,%d,%d,%d,%d\r\n", ( int ) ip1, ( int ) ip2, ( int ) ip3, ( int ) ip4, ( int ) p1, ( int ) p2 );

	afc_dprintf ( "%s: sending command %s\n", __FUNCTION__, cmd );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) ); 
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot send command", cmd, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 200 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot send command", cmd, AFC_FTP_CLIENT_ERR_RETR );
	}

	afc_string_make ( cmd, "RETR %s\r\n", file_or_cmd );

	afc_dprintf ( "%s: sending command %s\n", __FUNCTION__, cmd );

	res = afc_inet_client_send ( fc->inet, cmd, afc_string_len ( cmd ) ); 
	if ( res != AFC_ERR_NO_ERROR ) RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot send command", cmd, res );

	code = afc_ftp_client_internal_get_answer ( fc, answer );
	if ( code != 125 && code != 150 ) 
	{
		fc->last_code = code;
		if ( fc->last_answer ) afc_string_delete ( fc->last_answer );
		fc->last_answer = afc_string_dup ( answer );
		RAISE_RC ( AFC_LOG_ERROR, AFC_FTP_CLIENT_ERR_RETR, "Cannot send command", cmd, AFC_FTP_CLIENT_ERR_RETR );
	}

	while ( ( res = afc_inet_server_process ( is ) ) == AFC_ERR_NO_ERROR )
	{
		
	}

	RETURN ( AFC_ERR_NO_ERROR );	

EXCEPT 

FINALLY 

	afc_string_delete ( cmd );
	afc_string_delete ( answer );
	afc_inet_server_delete ( is );

ENDTRY
	
}


static int afc_ftp_client_internal_get_answer ( FtpClient * fc, char * answer )
{
	FILE * fd;
  	char * buf = afc_string_new ( 1024 );
  	BOOL firstloop = TRUE;
	char code[4];

  	while ( TRUE ) 
	{ 

		fd = afc_inet_client_get_file ( fc->inet );
		//TODO:controllo errori
    		afc_string_fget ( buf, fd );

    		if ( firstloop )
		{ 
			strncpy ( code, buf, 3 );
			code[ 3 ] = '\0';
			firstloop = FALSE;
		}
		if ( buf [ 3 ] == ' ' && !afc_string_comp ( buf, code, 3 ) ) break;
	
  	}  
	
	afc_string_copy ( answer, buf + 4, ALL );

	afc_dprintf ( "%s: Got answer %s", __FUNCTION__, buf );

	afc_string_delete ( buf );

	return atoi ( code );
}

#ifdef TEST_CLASS

int retr_binary_callback ( u_char * pblock, int blocklen, void * param )
{
	FILE * fp = ( FILE * ) param;
	fwrite ( pblock, blocklen, 1, fp );

	return ( AFC_ERR_NO_ERROR );
}


int store_binary_callback ( u_char * pdata, int * plen, void * param )
{
	FILE * fp = ( FILE * ) param;
	int count = fread ( pdata, sizeof ( u_char ), *plen, fp );
	if (count <= 0) return AFC_FTP_CLIENT_ERR_EOF;
	*plen = count;
	return ( AFC_ERR_NO_ERROR );
}



int main ( int argc, char * argv[] )
{
 	int res; 
//	FILE * fp = NULL;
	char * cdir;
	int file_size = 0;

	AFC * afc = afc_new ();
  	FtpClient * fc = afc_ftp_client_new();

  	if ( fc == NULL ) 
  	{
    		fprintf ( stderr, "Init of class FtpClient failed.\n" );
    		return ( 1 );
  	}

	res = afc_ftp_client_connect ( fc, "10.0.20.123", 21 );
	printf ( "%d\n", res );	

	res = afc_ftp_client_login ( fc, "fabio", "STATfs72" );
	printf ( "%d\n", res );

//	res = afc_ftp_client_cwd ( fc, "johnnolo" );
//	printf ( "%d\n", res );

//	res = afc_ftp_client_delete_file ( fc, "ntuser.ini" );
//	printf ( "%d\n", res );

//	res = afc_ftp_client_mkd ( fc, "mammolo" );
//	printf ( "%d\n", res );

//	res = afc_ftp_client_rmd ( fc, "mammolo" );
//	printf ( "%d\n", res );
	
//	afc_ftp_client_set_pasv ( fc, TRUE );


//	fp = fopen ( "plutino", "wb" );
//	res = afc_ftp_client_retrbinary ( fc, "RETR paolo", retr_binary_callback, fp, 1024, 0 ); 
//	fclose ( fp );


//	res = afc_ftp_client_retrlines ( fc, "RETR paolo", NULL, NULL );

//	fp = fopen ( "plutino", "rb" );
//	res = afc_ftp_client_storlines ( fc, "STOR cucucu2", store_binary_callback, fp );
//	fclose ( fp );

	cdir = afc_string_new ( 1024 );
	afc_ftp_client_pwd ( fc, cdir );
	printf ( "%s\n", cdir ); 
	afc_string_delete ( cdir );

//	afc_ftp_client_rename ( fc, "io", "ciaobelli" );

//	afc_ftp_client_sendcmd ( fc, "SYST\r\n" );
//	printf ( "%d %s\n", fc->last_code, fc->last_answer );

	afc_ftp_client_size ( fc, "ciaobelli", &file_size );
	printf ( "%d\n", file_size );
	 

	afc_ftp_client_quit ( fc );

  	afc_ftp_client_delete ( fc );
  	afc_delete ( afc );
	

  	return ( 0 ); 
}
#endif


