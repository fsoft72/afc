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
#include <afc/afc.h>
#include <mysql/mysql.h>
#include <afc/dbi_manager.h>

// {{{ INTERNAL STUFF
struct db_mysql
{
	MYSQL mysql;
	MYSQL *connection;

	MYSQL_RES *result;
	MYSQL_ROW row;

	int num_rows;
	int num_cols;

	Dictionary *fields;
};

typedef struct db_mysql DBMysql;

DynamicClass *dynamic_class_new_instance(void);
void dynamic_class_del_instance(DynamicClass *dc);

static const char class_name[] = "DBI: MySQL";

static int mysql_method_init(DynamicClass *dc);
static int mysql_method_connect(DynamicClass *dc);
static int mysql_method_close(DynamicClass *dc);
static int mysql_method_query(DynamicClass *dc);
static int mysql_method_num_cols(DynamicClass *dc);
static int mysql_method_num_rows(DynamicClass *dc);
static int mysql_method_fetch_row(DynamicClass *dc);
static int mysql_method_free(DynamicClass *dc);

static int mysql_int_next_row(DynamicClass *dc, DBMysql *db);
static int mysql_int_free(DynamicClass *dc, DBMysql *db);
// }}}

static void *__mysql_client;

#ifndef TEST_CLASS
void _init(void)
{
	__mysql_client = dlopen("libmysqlclient.so", RTLD_LAZY | RTLD_GLOBAL);
}

void _fini(void)
{
	if (__mysql_client)
		dlclose(__mysql_client);
}
#endif

// {{{ new_instance ()
DynamicClass *dynamic_class_new_instance(void)
{
	DynamicClass *dc = afc_dynamic_class_new();

	if (dc == NULL)
		return (NULL);

	DB_SETV_N(dc, "_dbi_max_reconn", 5);

	afc_dynamic_class_add_method(dc, "init", NULL, mysql_method_init);
	afc_dynamic_class_add_method(dc, "connect", "SSSS", mysql_method_connect);
	afc_dynamic_class_add_method(dc, "close", NULL, mysql_method_close);
	afc_dynamic_class_add_method(dc, "query", "S", mysql_method_query);
	afc_dynamic_class_add_method(dc, "num_cols", NULL, mysql_method_num_cols);
	afc_dynamic_class_add_method(dc, "num_rows", NULL, mysql_method_num_rows);
	afc_dynamic_class_add_method(dc, "fetch_row", NULL, mysql_method_fetch_row);
	afc_dynamic_class_add_method(dc, "free", NULL, mysql_method_free);

	return (dc);
}
// }}}
// {{{ del_instance ()
void dynamic_class_del_instance(DynamicClass *dc)
{
	DBMysql *db;

	if (dc == NULL)
		return;

	mysql_method_close(dc);

	db = DB_GET_DATA(dc);

	if (db)
	{
		afc_dictionary_delete(db->fields);
		afc_free(db);
	}

	afc_dynamic_class_delete(dc);
}
// }}}

// {{{ init ()
static int mysql_method_init(DynamicClass *dc)
{
	DBMysql *db;

	if ((db = afc_malloc(sizeof(DBMysql))) == NULL)
		return (AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "db"));

	DB_SET_DATA(dc, db);

	if ((db->fields = afc_dictionary_new()) == NULL)
		return (AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "fields"));

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ connect ( host, db, login, pwd )
static int mysql_method_connect(DynamicClass *dc)
{
	char *host, *dbname, *login, *pwd;
	int c = 0, max_reconn;
	DBMysql *db;

	db = DB_GET_DATA(dc);

	if (db->connection != NULL)
		return (AFC_DBI_MANAGER_ERR_ALREADY_CONNECTED);

	host = afc_array_first(dc->args);
	dbname = afc_array_next(dc->args);
	login = afc_array_next(dc->args);
	pwd = afc_array_next(dc->args);

	max_reconn = DB_GETV_N(dc, "_dbi_max_reconn");

	mysql_init(&db->mysql);

	do
	{
		if ((db->connection = mysql_real_connect(&db->mysql, host, login, pwd, dbname, 0, 0, 0)) == NULL)
		{
			c++;
			if (c == max_reconn)
				return (AFC_LOG(AFC_LOG_ERROR, AFC_DBI_MANAGER_ERR_CONNECT_FAILED, "Connection to database failed", mysql_error(&db->mysql)));
		}
		else
			break;
	} while (c < max_reconn);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ close ()
static int mysql_method_close(DynamicClass *dc)
{
	DBMysql *db;

	db = DB_GET_DATA(dc);

	mysql_int_free(dc, db);

	if (db->connection)
		mysql_close(db->connection);

	db->connection = (MYSQL *)NULL;

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ query ( sql )
static int mysql_method_query(DynamicClass *dc)
{
	DBMysql *db;
	char *sql;

#define RETRIES 5
#define ER_LOCK_DEADLOCK 1213

	db = DB_GET_DATA(dc);

	if (db->connection == NULL)
		return (AFC_DBI_MANAGER_ERR_NOT_CONNECTED);

	// Get the query from method call args
	sql = afc_array_first(dc->args);

	mysql_int_free(dc, db);

	// Performs the query
	int count = 0;

	while (count < RETRIES)
	{
		if (mysql_query(db->connection, sql) != 0)
		{
			if ((mysql_errno(db->connection) == ER_LOCK_DEADLOCK) && (count++ < RETRIES))
				continue;
			else
				return (AFC_LOG(AFC_LOG_ERROR, AFC_DBI_MANAGER_ERR_QUERY_FAILED, "The query failed", mysql_error(db->connection)));
		}
		else
			break;
	}

	// Save the query result
	db->result = mysql_store_result(db->connection);

	// And if it has failed, report an error
	if (mysql_errno(db->connection))
		return (AFC_LOG(AFC_LOG_ERROR, AFC_DBI_MANAGER_ERR_QUERY_STORAGE_FAILED, "Query Storage Failed", mysql_error(db->connection)));

	db->num_rows = mysql_affected_rows(db->connection);

	DB_SETV_N(dc, "num_rows", db->num_rows);

	if (db->result != NULL)
	{
		db->num_cols = mysql_num_fields(db->result);
		DB_SETV_N(dc, "num_cols", db->num_cols);
	}
	else
	{
		db->num_cols = 0;
		DB_SETV_N(dc, "num_cols", 0);
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ num_rows ()
int mysql_method_num_rows(DynamicClass *dc)
{
	dc->result = DB_GETV_P(dc, "num_rows");
	dc->result_type = AFC_DYNAMIC_CLASS_RESULT_TYPE_INTEGER;

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ num_cols ()
int mysql_method_num_cols(DynamicClass *dc)
{
	dc->result = DB_GETV_P(dc, "num_cols");
	dc->result_type = AFC_DYNAMIC_CLASS_RESULT_TYPE_INTEGER;

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ fetch_row ( names )
static int mysql_method_fetch_row(DynamicClass *dc)
{
	DBMysql *db;
	char *name, *v;
	int res, t, i;
	MYSQL_FIELD *f;

	db = DB_GET_DATA(dc);

	// Clear the result Dictionary
	afc_dictionary_clear(db->fields);
	dc->result = NULL;

	if (mysql_int_next_row(dc, db) != AFC_ERR_NO_ERROR)
		return (res);

	dc->result_type = AFC_DYNAMIC_CLASS_RESULT_TYPE_DICTIONARY;

	if (db->row != NULL)
	{
		i = db->num_cols;

		for (t = 0; t < i; t++)
		{
			f = mysql_fetch_field_direct(db->result, t);
			name = f->name;

			v = db->row[t];
			if (v == NULL)
				v = "";

			afc_dictionary_set(db->fields, name, v);
		}

		dc->result = db->fields;
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ free ( names )
static int mysql_method_free(DynamicClass *dc)
{
	DBMysql *db;

	db = DB_GET_DATA(dc);

	mysql_int_free(dc, db);

	return (AFC_ERR_NO_ERROR);
}
// }}}

// ------------------------------------------------------------------------------------------------------
// INTERNAL FUNCTIONS
// ------------------------------------------------------------------------------------------------------

// {{{ int_next_row ( dc, db )
static int mysql_int_next_row(DynamicClass *dc, DBMysql *db)
{
	if (db->result == NULL)
		return (AFC_LOG(AFC_LOG_ERROR, AFC_DBI_MANAGER_ERR_NO_RESULT_SET, "No result set defined.", NULL));

	db->row = mysql_fetch_row(db->result);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ int_free ( dc, db )
static int mysql_int_free(DynamicClass *dc, DBMysql *db)
{
	// Clear the result Dictionary
	afc_dictionary_clear(db->fields);

	// If a result already exists, free it
	if (db->result)
		mysql_free_result(db->result);

	db->result = NULL;

	db->num_rows = 0;
	db->num_cols = 0;

	return (AFC_ERR_NO_ERROR);
}
// }}}

#ifdef TEST_CLASS
void dump_dict(Dictionary *dict)
{
	char *s;

	printf("=== DUMP DICT\n");
	s = afc_dictionary_first(dict);
	while (s)
	{
		printf("Key: %s - %s\n", afc_dictionary_get_key(dict), s);
		s = afc_dictionary_next(dict);
	}
	printf("=== END DICT\n");
}

int main(int argc, char *argv[])
{
	AFC *afc = afc_new();
	DynamicClass *dc;
	Dictionary *dict;

	__mysql_client = dlopen("libmysqlclient.so", RTLD_LAZY | RTLD_GLOBAL);

	afc_track_mallocs(afc);

	afc_set_tags(afc, AFC_TAG_SHOW_MALLOCS, FALSE,
				 AFC_TAG_SHOW_FREES, FALSE,
				 AFC_TAG_LOG_LEVEL, AFC_LOG_WARNING,
				 AFC_TAG_END, AFC_TAG_END);

	dc = dynamic_class_new_instance();

	afc_dynamic_class_execute(dc, "init", AFC_DYNAMIC_CLASS_ARG_END);
	afc_dynamic_class_execute(dc, "connect", "127.0.0.1", "omen", "root", "", AFC_DYNAMIC_CLASS_ARG_END);
	afc_dynamic_class_execute(dc, "query", "SELECT * FROM wb_user", AFC_DYNAMIC_CLASS_ARG_END);

	afc_dynamic_class_execute(dc, "fetch_row", TRUE, AFC_DYNAMIC_CLASS_ARG_END);

	while ((dict = dc->result))
	{
		dump_dict(dict);
		afc_dynamic_class_execute(dc, "fetch_row", false, AFC_DYNAMIC_CLASS_ARG_END);
	}

	dynamic_class_del_instance(dc);
	afc_delete(afc);

	if (__mysql_client)
		dlclose(__mysql_client);

	return (0);
}
#endif
