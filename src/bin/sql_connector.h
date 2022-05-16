#ifndef AFC_SQL_CONNECTOR_H
#define AFC_SQL_CONNECTOR_H

#include "base.h"
#include "exceptions.h"


#define AFC_SQL_CONNECTOR_MAGIC	( 'S' << 24 | 'Q' << 16 | 'L' << 8 | '_' )
#define AFC_SQL_CONNECTOR_BASE	0x1000

struct afc_sql_connector
{
	unsigned long magic;


};

typedef struct afc_sql_connector SqlConnector;

SqlConnector * afc_sql_connector_new ();
int _afc_sql_connector_delete ( SqlConnector * sc );
int afc_sql_connector_clear   ( SqlConnector * sc );
#define afc_sql_connector_delete(sc)	if ( sc ) { _afc_sql_connector_delete ( sc ); sc = NULL; }

#endif
