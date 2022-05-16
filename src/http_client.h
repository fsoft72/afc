#ifndef AFC_HTTP_CLIENT_H
#define AFC_HTTP_CLIENT_H
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "afc.h"

/* HttpClient'Magic' value: 'HTTP' */
#define AFC_HTTP_CLIENT_MAGIC ( 'H' << 24 | 'T' << 16 | 'T' << 8 | 'P' )

/* HttpClient Base  */
#define AFC_HTTP_CLIENT_BASE 0x1000

enum {
	AFC_HTTP_CLIENT_ERR_REQUEST = AFC_HTTP_CLIENT_BASE,
	AFC_HTTP_CLIENT_ERR_GETRESP

};

struct afc_http_client
{
  unsigned long magic;     /* HttpClient Magic Value */
  InetClient * inet;
  char * host;
  int port;
  BOOL isconnected;
};

typedef struct afc_http_client HttpClient;

/* Function Prototypes */
HttpClient * afc_http_client_new ( char * host, int port );
int afc_http_client_delete ( HttpClient * hc );
int afc_http_client_clear ( HttpClient * hc );
#endif
