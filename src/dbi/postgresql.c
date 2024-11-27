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
#include <pgsql/libpq-fe.h>
#include <afc/dbi_manager.h>

// {{{ INTERNAL STUFF
struct db_pgsql
{
	PGconn *connection;

	int num_rows;
	int num_cols;

	int curr_row; // Current Row in the Recordset

	PGresult *result;

	Dictionary *fields;
};

typedef struct db_pgsql DBpgsql;

DynamicClass *dynamic_class_new_instance(void);
void dynamic_class_del_instance(DynamicClass *dc);

static const char class_name[] = "DBI: pgsql";

static int pgsql_method_init(DynamicClass *dc);
static int pgsql_method_connect(DynamicClass *dc);
static int pgsql_method_close(DynamicClass *dc);
static int pgsql_method_query(DynamicClass *dc);
static int pgsql_method_num_cols(DynamicClass *dc);
static int pgsql_method_num_rows(DynamicClass *dc);
static int pgsql_method_fetch_row(DynamicClass *dc);
static int pgsql_method_free(DynamicClass *dc);

static int pgsql_internal_free(DynamicClass *dc, DBpgsql *db);
// }}}

static void *__pgsql_client;

#ifndef TEST_CLASS
void _init(void)
{
	__pgsql_client = dlopen("libpq.so", RTLD_LAZY | RTLD_GLOBAL);
}

void _fini(void)
{
	if (__pgsql_client)
		dlclose(__pgsql_client);
}
#endif

// {{{ new_instance ()
DynamicClass *dynamic_class_new_instance(void)
{
	DynamicClass *dc = afc_dynamic_class_new();

	if (dc == NULL)
		return (NULL);

	DB_SETV_N(dc, "_dbi_max_reconn", 5);

	afc_dynamic_class_add_method(dc, "init", NULL, pgsql_method_init);
	afc_dynamic_class_add_method(dc, "connect", "SSSS", pgsql_method_connect);
	afc_dynamic_class_add_method(dc, "close", NULL, pgsql_method_close);
	afc_dynamic_class_add_method(dc, "query", "S", pgsql_method_query);
	afc_dynamic_class_add_method(dc, "num_cols", NULL, pgsql_method_num_cols);
	afc_dynamic_class_add_method(dc, "num_rows", NULL, pgsql_method_num_rows);
	afc_dynamic_class_add_method(dc, "fetch_row", NULL, pgsql_method_fetch_row);
	afc_dynamic_class_add_method(dc, "free", NULL, pgsql_method_free);

	return (dc);
}
// }}}
// {{{ del_instance ()
void dynamic_class_del_instance(DynamicClass *dc)
{
	DBpgsql *db;

	if (dc == NULL)
		return;

	pgsql_method_close(dc);

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
static int pgsql_method_init(DynamicClass *dc)
{
	DBpgsql *db;

	if ((db = afc_malloc(sizeof(DBpgsql))) == NULL)
		return (AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "db"));

	DB_SET_DATA(dc, db);

	if ((db->fields = afc_dictionary_new()) == NULL)
		return (AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "fields"));

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ connect ( host, db, login, pwd )
static int pgsql_method_connect(DynamicClass *dc)
{
	char *host, *dbname, *login, *pwd;
	char *conninfo;
	DBpgsql *db;

	db = DB_GET_DATA(dc);

	if (db->connection != NULL)
		return (AFC_DBI_MANAGER_ERR_ALREADY_CONNECTED);

	host = afc_array_first(dc->args);
	dbname = afc_array_next(dc->args);
	login = afc_array_next(dc->args);
	pwd = afc_array_next(dc->args);

	conninfo = afc_string_new(1024);

	afc_string_make(conninfo, "host = '%s' dbname = '%s' user = '%s' password = '%s'", host, dbname, login, pwd);

	db->connection = PQconnectdb(""); // conninfo );

	if (PQstatus(db->connection) == CONNECTION_BAD)
		printf("Error connecting\n");

	afc_string_delete(conninfo);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ close ()
static int pgsql_method_close(DynamicClass *dc)
{
	DBpgsql *db;

	db = DB_GET_DATA(dc);

	pgsql_internal_free(dc, db);

	if (db->connection)
		PQfinish(db->connection);
	db->connection = NULL;

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ query ( sql )
static int pgsql_method_query(DynamicClass *dc)
{
	DBpgsql *db;
	char *sql;

	db = DB_GET_DATA(dc);

	if (db->connection == NULL)
		return (AFC_DBI_MANAGER_ERR_NOT_CONNECTED);

	// Get the query from method call args
	sql = afc_array_first(dc->args);

	// Free data
	pgsql_internal_free(dc, db);

	// Performs the query
	db->result = PQexec(db->connection, sql);

	switch (PQresultStatus(db->result))
	{
	case PGRES_COMMAND_OK:
	case PGRES_TUPLES_OK:
	case PGRES_COPY_OUT:
	case PGRES_COPY_IN:
		break;

	case PGRES_BAD_RESPONSE:
	case PGRES_NONFATAL_ERROR:
	case PGRES_FATAL_ERROR:
	case PGRES_EMPTY_QUERY:
		printf("ERROR: %s\n", PQerrorMessage(db->connection));
		printf("ERROR: %s\n", PQresStatus(PQresultStatus(db->result)));

		return (AFC_LOG(AFC_LOG_ERROR, AFC_DBI_MANAGER_ERR_QUERY_FAILED, "The query failed", PQresultErrorMessage(db->result)));
	}

	db->num_rows = PQntuples(db->result);
	db->num_cols = PQnfields(db->result);

	DB_SETV_N(dc, "num_rows", db->num_rows);
	DB_SETV_N(dc, "num_cols", db->num_cols);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ num_rows ()
int pgsql_method_num_rows(DynamicClass *dc)
{
	dc->result = DB_GETV_P(dc, "num_rows");
	dc->result_type = AFC_DYNAMIC_CLASS_RESULT_TYPE_INTEGER;

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ num_cols ()
int pgsql_method_num_cols(DynamicClass *dc)
{
	dc->result = DB_GETV_P(dc, "num_cols");
	dc->result_type = AFC_DYNAMIC_CLASS_RESULT_TYPE_INTEGER;

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ fetch_row ( names )
static int pgsql_method_fetch_row(DynamicClass *dc)
{
	DBpgsql *db;
	char *name, *v;
	int t, i;

	db = DB_GET_DATA(dc);

	// Clear the result Dictionary
	afc_dictionary_clear(db->fields);
	dc->result = NULL;

	if (db->curr_row >= db->num_rows)
		return (AFC_DBI_MANAGER_ERR_END_OF_RESULT_SET);

	dc->result_type = AFC_DYNAMIC_CLASS_RESULT_TYPE_DICTIONARY;

	i = db->num_cols;

	for (t = 0; t < i; t++)
	{
		name = PQfname(db->result, t);

		v = PQgetvalue(db->result, db->curr_row, t);
		if (v == NULL)
			v = "";

		afc_dictionary_set(db->fields, name, v);
	}

	db->curr_row += 1;

	dc->result = db->fields;

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ free ()
static int pgsql_method_free(DynamicClass *dc)
{
	DBpgsql *db;

	db = DB_GET_DATA(dc);

	return (pgsql_internal_free(dc, db));
}
// }}}

// ------------------------------------------------------------------------------------------------------
// INTERNAL FUNCTIONS
// ------------------------------------------------------------------------------------------------------
// {{{ pgsql_internal_free ( dc, db )
static int pgsql_internal_free(DynamicClass *dc, DBpgsql *db)
{
	// Clear the result Dictionary
	afc_dictionary_clear(db->fields);

	// Clear the PGresult
	if (db->result)
		PQclear(db->result);

	db->result = NULL;

	db->curr_row = 0;
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
	int rows, cols;

	__pgsql_client = dlopen("libpq.so", RTLD_LAZY | RTLD_GLOBAL);

	afc_track_mallocs(afc);

	afc_set_tags(afc, AFC_TAG_SHOW_MALLOCS, FALSE,
				 AFC_TAG_SHOW_FREES, FALSE,
				 AFC_TAG_LOG_LEVEL, AFC_LOG_WARNING,
				 AFC_TAG_END, AFC_TAG_END);

	dc = dynamic_class_new_instance();

	afc_dynamic_class_execute(dc, "init", AFC_DYNAMIC_CLASS_ARG_END);
	afc_dynamic_class_execute(dc, "connect", "", "", "", "", AFC_DYNAMIC_CLASS_ARG_END);
	afc_dynamic_class_execute(dc, "query", "SELECT * FROM prova", AFC_DYNAMIC_CLASS_ARG_END);

	afc_dynamic_class_execute(dc, "num_rows", AFC_DYNAMIC_CLASS_ARG_END);
	rows = (int)dc->result;
	afc_dynamic_class_execute(dc, "num_cols", AFC_DYNAMIC_CLASS_ARG_END);
	cols = (int)dc->result;

	printf("Rows: %d - Cols: %d\n", rows, cols);

	afc_dynamic_class_execute(dc, "fetch_row", TRUE, AFC_DYNAMIC_CLASS_ARG_END);

	while ((dict = dc->result))
	{
		dump_dict(dict);
		afc_dynamic_class_execute(dc, "fetch_row", true, AFC_DYNAMIC_CLASS_ARG_END);
	}

	dynamic_class_del_instance(dc);
	afc_delete(afc);

	if (__pgsql_client)
		dlclose(__pgsql_client);

	return (0);
}
#endif
