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
/*
	DBI Manager 1.00 - Written by Fabio Rotondo ( fabio@rotondo.it )
*/

#include "dbi_manager.h"

static const char class_name[] = "DBIManager";

// {{{ docs
/*
@node quote
*Dr. Frederick Frankenstein:  	What a filthy job.*

*Igor: 	Could be worse.*

*Dr. Frederick Frankenstein: 	How?*

*Igor: 	Could be raining.*


	*Young Frankenstein*
@endnode

@node intro

DBI is an abstraction layer to access databases. At the moment it supports MySQL and PostgreSQL.

@endnode

@node history
	- 1.00:		Initial Release
@endnode
*/
// }}}

// {{{ afc_dbi_manager_new ()
/*
@node afc_dbi_manager_new

		   NAME: afc_dbi_manager_new () - Initializes a new DBIManager instance.

	   SYNOPSIS: DBIManager * afc_dbi_manager_new ()

	DESCRIPTION: This function initializes a new DBIManager instance.

		  INPUT: NONE

		RESULTS: a valid inizialized DBIManager structure. NULL in case of errors.

	   SEE ALSO: - afc_dbi_manager_delete()

@endnode
*/
DBIManager *afc_dbi_manager_new(void)
{
	TRY(DBIManager *)

	DBIManager *dbi = (DBIManager *)afc_malloc(sizeof(DBIManager));

	if (dbi == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "dbi", NULL);

	dbi->magic = AFC_DBI_MANAGER_MAGIC;

	if ((dbi->dcm = afc_dynamic_class_master_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "Dynamic Class Master", NULL);

	if ((dbi->modules_path = afc_string_new(1024)) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "modules_path", NULL);

	afc_string_copy(dbi->modules_path, "/usr/local/lib/afc/dbi", ALL);

	RETURN(dbi);

	EXCEPT
	afc_dbi_manager_delete(dbi);

	FINALLY

	ENDTRY
}
// }}}
// {{{ afc_dbi_manager_delete ( dbi )
/*
@node afc_dbi_manager_delete

		   NAME: afc_dbi_manager_delete ( dbi_manager )  - Disposes a valid DBIManager instance.

	   SYNOPSIS: int afc_dbi_manager_delete ( DBIManager * dbi_manager)

	DESCRIPTION: This function frees an already alloc'd DBIManager structure.

		  INPUT: - dbi_manager  - Pointer to a valid afc_dbi_manager class.

		RESULTS: should be AFC_ERR_NO_ERROR

		  NOTES: - this method calls: afc_dbi_manager_clear()

	   SEE ALSO: - afc_dbi_manager_new()
				 - afc_dbi_manager_clear()
@endnode
*/
int _afc_dbi_manager_delete(DBIManager *dbi)
{
	int afc_res;

	if ((afc_res = afc_dbi_manager_clear(dbi)) != AFC_ERR_NO_ERROR)
		return (afc_res);

	afc_dynamic_class_master_delete(dbi->dcm);
	afc_string_delete(dbi->modules_path);
	afc_free(dbi);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dbi_manager_clear ( dbi )
/*
@node afc_dbi_manager_clear

		   NAME: afc_dbi_manager_clear ( dbi_manager )  - Clears all stored data

	   SYNOPSIS: int afc_dbi_manager_clear ( DBIManager * dbi_manager)

	DESCRIPTION: Use this function to clear all stored data in the current dbi_manager instance.

		  INPUT: - dbi_manager    - Pointer to a valid afc_dbi_manager instance.

		RESULTS: should be AFC_ERR_NO_ERROR

	   SEE ALSO: - afc_dbi_manager_delete()

@endnode
*/
int afc_dbi_manager_clear(DBIManager *dbi_manager)
{
	if (dbi_manager == NULL)
		return (AFC_ERR_NULL_POINTER);

	if (dbi_manager->magic != AFC_DBI_MANAGER_MAGIC)
		return (AFC_ERR_INVALID_POINTER);

	/* Custom Clean-up code should go here */
	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dbi_manager_new_instance ( dbi, class, library_name )
/*
@node afc_dbi_manager_new_instance

		   NAME: afc_dbi_manager_new_instance ( dbi, class, library_name )  - Creates a new DBI Instance to a driver

	   SYNOPSIS: int afc_dbi_manager_new_instance ( DBIManager * dbi, const char * class_name, const char * library_name )

	DESCRIPTION: This function attemps to init a new instance of a specific Dynamic Class handling the desired Database.

		  INPUT: - dbi    	- Pointer to a valid afc_dbi_manager instance.
		 - class_name	- Name of the Dynamic Class devoted to the interface with the specific Database. (eg: "mysql")
		 - library_name - Name of the Dynamic Class file containing the shared library for the database. You can specify
				  both an absolute path (starting with a "/" or "." char) or a relative path (containing just the
				  file name of the shared object to be loaded). In the second case the shared library will be
				  loaded from the "default" DBI path (by default: /usr/local/lib/afc/dbi)

		RESULTS: should be AFC_ERR_NO_ERROR

	   SEE ALSO: - afc_dbi_manager_delete()

@endnode
*/
DynamicClass *afc_dbi_manager_new_instance(DBIManager *dbi, const char *class_name, const char *library_name)
{
	char buf[1024];
	const char *s;

	// If the class is missing, we can try to load it right now
	if (afc_dynamic_class_master_has_class(dbi->dcm, class_name) != AFC_ERR_NO_ERROR)
	{
		if (library_name == NULL)
		{
			AFC_LOG(AFC_LOG_ERROR, AFC_DBI_MANAGER_ERR_PLUGIN_NOT_FOUND, "Plugin Not Found", class_name);
			return (NULL);
		}

		// If the library name starts with "/" it is an absolute path name
		if ((library_name[0] == '/') || (library_name[0] == '.'))
			s = library_name;
		else
		{
			sprintf(buf, "%s/%s", dbi->modules_path, library_name);
			s = buf;
		}

		if (afc_dynamic_class_master_load(dbi->dcm, class_name, s) != AFC_ERR_NO_ERROR)
			return (NULL);
	}

	return (afc_dynamic_class_master_new_instance(dbi->dcm, class_name));
}
// }}}
// {{{ afc_dbi_manager_delete_instance ( dbi, dc )
int afc_dbi_manager_delete_instance(DBIManager *dbi, DynamicClass *dc)
{
	return (afc_dynamic_class_master_delete_instance(dbi->dcm, dc));
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
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

int dbi_test(DBIManager *dbi, const char *dbi_driver, const char *libname, const char *host, const char *dbname, const char *user, const char *pwd, const char *sql)
{
	DynamicClass *dc;

	dc = afc_dbi_manager_new_instance(dbi, dbi_driver, libname);

	DBI_INIT(dc);

	DBI_CONNECT(dc, host, dbname, user, pwd);
	DBI_QUERY(dc, sql);

	printf("*************** RESULT ****************\n");
	printf("Rows: %d - Cols: %d\n\n", DBI_NUM_ROWS(dc), DBI_NUM_COLS(dc));

	DBI_FETCH(dc);

	while (dc->result)
	{
		dump_dict(dc->result);
		DBI_FETCH(dc);
	}

	DBI_CLOSE(dc);

	afc_dbi_manager_delete_instance(dbi, dc);
}

int main(int argc, char *argv[])
{
	AFC *afc = afc_new();
	DBIManager *dbi;

	afc_track_mallocs(afc);

	afc_set_tags(afc, AFC_TAG_SHOW_MALLOCS, FALSE,
				 AFC_TAG_SHOW_FREES, FALSE,
				 AFC_TAG_LOG_LEVEL, AFC_LOG_WARNING,
				 AFC_TAG_END, AFC_TAG_END);

	if ((dbi = afc_dbi_manager_new()) == NULL)
	{
		fprintf(stderr, "Init of class DBIManager failed.\n");
		return (1);
	}

	dbi_test(dbi, "mysql", "./dbi/modules/mysql.so", "127.0.0.1", "omen", "root", "", "SELECT * FROM wb_user");
	dbi_test(dbi, "postgresql", "./dbi/modules/postgresql.so", "127.0.0.1", "omen", "root", "", "SELECT * FROM prova");

	afc_dbi_manager_delete(dbi);
	afc_delete(afc);

	return (0);
}
// }}}
#endif
