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
#ifndef AFC_DBI_MANAGER_H
#define AFC_DBI_MANAGER_H
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "afc.h"

/* DBIManager'Magic' value: 'DATA' */
#define AFC_DBI_MANAGER_MAGIC ( 'D' << 24 | 'B' << 16 | 'I' << 8 | 'M' )

/* DBIManager Base  */
#define AFC_DBI_MANAGER_BASE 0x20000

enum {	AFC_DBI_MANAGER_ERR_ALREADY_CONNECTED = 1,
	AFC_DBI_MANAGER_ERR_CONNECT_FAILED,
	AFC_DBI_MANAGER_ERR_QUERY_FAILED,
	AFC_DBI_MANAGER_ERR_QUERY_STORAGE_FAILED,
	AFC_DBI_MANAGER_ERR_NOT_CONNECTED,
	AFC_DBI_MANAGER_ERR_NO_RESULT_SET,
	AFC_DBI_MANAGER_ERR_END_OF_RESULT_SET,
	AFC_DBI_MANAGER_ERR_PLUGIN_NOT_FOUND
};

#define DB_SETV_P(dc,name,v)		afc_dynamic_class_set_var ( dc, AFC_DYNAMIC_CLASS_VAR_KIND_POINTER, name, v )
#define DB_GETV_P(dc,name)		afc_dynamic_class_get_var ( dc, name )

#define DB_SETV_N(dc,name,v)		afc_dynamic_class_set_var ( dc, AFC_DYNAMIC_CLASS_VAR_KIND_NUM, name, ( void * ) v )
#define DB_GETV_N(dc,name)		( unsigned long ) afc_dynamic_class_get_var ( dc, name )

#define DB_SET_DATA(dc,v)		afc_dynamic_class_set_var ( dc, AFC_DYNAMIC_CLASS_VAR_KIND_POINTER, "_dbi_manager_data",v )
#define DB_GET_DATA(dc)			afc_dynamic_class_get_var ( dc, "_dbi_manager_data" )

#define DBI_INIT(dc)				afc_dynamic_class_execute ( dc, "init", 				AFC_DYNAMIC_CLASS_ARG_END )
#define DBI_CONNECT(dc,host,db,login,pwd)	afc_dynamic_class_execute ( dc, "connect", host, db, login, pwd, 	AFC_DYNAMIC_CLASS_ARG_END )
#define DBI_CLOSE(dc)				afc_dynamic_class_execute ( dc, "close",				AFC_DYNAMIC_CLASS_ARG_END )
#define DBI_QUERY(dc,sql)			afc_dynamic_class_execute ( dc, "query", sql, 				AFC_DYNAMIC_CLASS_ARG_END )
#define DBI_NUM_COLS(dc)			( unsigned long ) afc_dynamic_class_get_var ( dc, "num_cols" )
#define DBI_NUM_ROWS(dc)			( unsigned long ) afc_dynamic_class_get_var ( dc, "num_rows" )
#define DBI_FETCH(dc)				afc_dynamic_class_execute ( dc, "fetch_row", AFC_DYNAMIC_CLASS_ARG_END )
#define DBI_FREE(dc)				afc_dynamic_class_execute ( dc, "free", AFC_DYNAMIC_CLASS_ARG_END )

struct afc_dbi_manager
{
	unsigned long magic;     /* DBIManager Magic Value */
	DynamicClassMaster * dcm;
	char * modules_path;		// Path where DBI modules reside
};

typedef struct afc_dbi_manager DBIManager;

/* Function Prototypes */
#define afc_dbi_manager_delete(dbi)	if ( dbi ) { _afc_dbi_manager_delete ( dbi ); dbi = NULL; }
DBIManager * afc_dbi_manager_new ( void );
int _afc_dbi_manager_delete ( DBIManager * dbi_manager );
int afc_dbi_manager_clear ( DBIManager * dbi_manager );
DynamicClass * afc_dbi_manager_new_instance ( DBIManager * dbi, const char * class_name, const char * library_name );
int afc_dbi_manager_delete_instance ( DBIManager * dbi, DynamicClass * dc );
#endif
