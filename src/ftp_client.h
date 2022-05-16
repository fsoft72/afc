#ifndef AFC_FTP_CLIENT_H
#define AFC_FTP_CLIENT_H
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "afc.h"

/* FtpClient'Magic' value: 'FTP_' */
#define AFC_FTP_CLIENT_MAGIC ( 'F' << 24 | 'T' << 16 | 'P' << 8 | '_' )

/* FtpClient Base  */
#define AFC_FTP_CLIENT_BASE 0x1000

enum {
	AFC_FTP_CLIENT_ERR_CONNECTION = AFC_FTP_CLIENT_BASE,
	AFC_FTP_CLIENT_ERR_LOGIN,
	AFC_FTP_CLIENT_ERR_CWD,
	AFC_FTP_CLIENT_ERR_DELE,
	AFC_FTP_CLIENT_ERR_MKD,
	AFC_FTP_CLIENT_ERR_RMD,
	AFC_FTP_CLIENT_ERR_RETR,
	AFC_FTP_CLIENT_ERR_PASV,
	AFC_FTP_CLIENT_ERR_STORE,
	AFC_FTP_CLIENT_ERR_EOF,
	AFC_FTP_CLIENT_ERR_PWD,
	AFC_FTP_CLIENT_ERR_QUIT,
	AFC_FTP_CLIENT_ERR_RENAME,
	AFC_FTP_CLIENT_ERR_SENDCMD,
	AFC_FTP_CLIENT_ERR_SIZE,
};

struct afc_ftp_client
{
	unsigned long magic;     /* FtpClient Magic Value */
	InetClient * inet;
	int last_code;           // Last answer code
	char * last_answer;	 // Last answer returned
	BOOL pasv;		 // shows passive or active mode
};

typedef struct afc_ftp_client FtpClient;

/* Function Prototypes */
FtpClient * afc_ftp_client_new ( void );
int afc_ftp_client_delete ( FtpClient * fc );
int afc_ftp_client_clear ( FtpClient * fc );

typedef int ( * afc_ftp_client_retr_callback ) ( u_char * pdata, int dlen, void * param ); 
typedef int ( * afc_ftp_client_store_callback ) ( u_char * pdata, int * plen, void * param ); 
#endif
