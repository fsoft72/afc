#include "sql_connector.h"

/*
@config
	TITLE:   SqlConnector
	VERSION: 1.0
@endnode
*/

// {{{ docs
/*
@node quote
	*Put the quote here*

	Quote Author
@endnode

@node intro
	SqlConnector documentation introduction should go here.
	Use the reST syntax to create docs.
@endnode
*/
// }}}

static const char class_name[] = "SqlConnector";

// {{{ afc_sql_connector_new ()   
/*
@node afc_sql_connector_new

                 NAME: afc_sql_connector_new ()    - Initializes a new SqlConnector instance.

             SYNOPSIS: SqlConnector * afc_sql_connector_new ()

          DESCRIPTION: This function initializes a new SqlConnector instance.

                INPUT: NONE

              RESULTS: a valid inizialized SqlConnector instance. NULL in case of errors.

             SEE ALSO: - afc_sql_connector_delete()
                       - afc_sql_connector_clear()
@endnode
*/
SqlConnector * afc_sql_connector_new ()
{
TRY ( SqlConnector * )

        SqlConnector * sc = afc_malloc ( sizeof ( SqlConnector ) );

        if ( sc == NULL ) RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "sc", NULL );
        sc->magic = AFC_SQL_CONNECTOR_MAGIC;


        RETURN ( sc );

EXCEPT
        afc_sql_connector_delete ( sc );

FINALLY

ENDTRY
}
// }}}
// {{{ afc_sql_connector_delete ( sc )
/*
@node afc_sql_connector_delete 

                 NAME: afc_sql_connector_delete ( sc ) - Dispose a SqlConnector instance.

             SYNOPSIS: int afc_sql_connector_delete ( SqlConnector * sc )

          DESCRIPTION: Use this method to delete an object's instance.

                INPUT: - sc - Pointer to a *valid* SqlConnector instance.

              RESULTS: - AFC_ERR_NO_ERROR on success.

             SEE ALSO: afc_sql_connector_new()
@endnode
*/
int _afc_sql_connector_delete ( SqlConnector * sc )
{
        int res;

        if ( ( res = afc_sql_connector_clear ( sc ) ) != AFC_ERR_NO_ERROR ) return ( res );

        afc_free ( sc );

        return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_sql_connector_clear ( sc )
/*
@node afc_sql_connector_clear 

                 NAME: afc_sql_connector_clear ( sc ) - Frees all unused memory

             SYNOPSIS: int afc_sql_connector_clear ( SqlConnector * sc )

          DESCRIPTION: This method clears all data inside the SqlConnector instance, 
                       except the main classes. 

                INPUT: - sc   - Pointer to a *valid* SqlConnector instance.

              RESULTS: - AFC_ERR_NO_ERROR on success.

             SEE ALSO: - afc_sql_connector_new()
		       - afc_sql_connector_delete()
@endnode
*/
int afc_sql_connector_clear ( SqlConnector * sc )
{
        if ( sc == NULL ) return ( AFC_LOG_FAST ( AFC_ERR_NULL_POINTER ) );
        if ( sc->magic != AFC_SQL_CONNECTOR_MAGIC ) return ( AFC_LOG_FAST ( AFC_ERR_INVALID_POINTER ) );


        return ( AFC_ERR_NO_ERROR );
}
// }}}

// {{{ TEST_CLASS
#ifdef TEST_CLASS
int main ( int argc, char * argv [] )
{
        AFC * afc;
        SqlConnector * sc;

        afc = afc_new ();
        afc_track_mallocs ( afc );
        afc_set_tags ( afc, AFC_TAG_LOG_LEVEL, AFC_LOG_WARNING, AFC_TAG_END );

        sc = afc_sql_connector_new ();

        afc_sql_connector_delete ( sc );
        afc_delete ( afc );
        return  ( 0 );
}
#endif
// }}}

