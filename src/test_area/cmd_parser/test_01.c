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
#include "../../afc.h"

struct somedata
{
	CommandParser * cmdp;
};

//window class
int window_cb_open ( DynamicClass * dyn )
{
	struct somedata * mydata = ( struct somedata * ) afc_array_master_first ( dyn->args );
	char * w = ( char * ) afc_cmd_parser_arg_get_by_name ( mydata->cmdp, "W" );
	char * h = ( char * ) afc_cmd_parser_arg_get_by_name ( mydata->cmdp, "H" );
	char * title = ( char * ) afc_cmd_parser_arg_get_by_name ( mydata->cmdp, "TITLE" );

	printf ( "window open - title: <%s>, width: <%s>, height: <%s>\n", title, w, h );

	return ( AFC_ERR_NO_ERROR );
}

int window_cb_close ( DynamicClass * dyn )
{
	printf ( "window close\n" );

	return ( AFC_ERR_NO_ERROR );
}

int window_get_template ( DynamicClass * dyn )
{
	dyn->result = "TITLE W H";
	return ( AFC_ERR_NO_ERROR );
}

DynamicClass * window_new_instance ()
{
	DynamicClass * dyn = afc_dynamic_class_new ();

	if ( dyn == NULL ) return NULL;

	afc_dynamic_class_add_method ( dyn, "open_callback", window_cb_open );
	afc_dynamic_class_add_method ( dyn, "close_callback", window_cb_close );
	afc_dynamic_class_add_method ( dyn, "get_template", window_get_template );

	return ( dyn );
}

int window_del_instance ( DynamicClass * dyn )
{
	if ( dyn != NULL ) afc_dynamic_class_delete ( dyn );

	return ( AFC_ERR_NO_ERROR );
}

//button class
int button_cb_open ( DynamicClass * dyn )
{
	struct somedata * mydata = ( struct somedata * ) afc_array_master_first ( dyn->args );
	char * name = ( char * ) afc_cmd_parser_arg_get_by_name ( mydata->cmdp, "NAME" );
	char * label = ( char * ) afc_cmd_parser_arg_get_by_name ( mydata->cmdp, "LABEL" );
	char * callback = ( char * ) afc_cmd_parser_arg_get_by_name ( mydata->cmdp, "CB" );

	printf ( "button open - name: <%s>, label: <%s>, cb: <%s>\n", name, label, callback );

	return ( AFC_ERR_NO_ERROR );
}

int button_cb_close ( DynamicClass * dyn )
{
	printf ( "button close\n" );

	return ( AFC_ERR_NO_ERROR );
}

int button_get_template ( DynamicClass * dyn )
{
	dyn->result = "NAME LABEL CB";
	return ( AFC_ERR_NO_ERROR );
}

DynamicClass * button_new_instance ()
{
	DynamicClass * dyn = afc_dynamic_class_new ();

	if ( dyn == NULL ) return NULL;

	afc_dynamic_class_add_method ( dyn, "open_callback", button_cb_open );
	afc_dynamic_class_add_method ( dyn, "close_callback", button_cb_close );
	afc_dynamic_class_add_method ( dyn, "get_template", button_get_template );

	return ( dyn );
}

int button_del_instance ( DynamicClass * dyn )
{
	if ( dyn != NULL ) afc_dynamic_class_delete ( dyn );

	return ( AFC_ERR_NO_ERROR );
}

//eqrows class
int eqrows_cb_open ( DynamicClass * dyn )
{
	printf ( "eqrows open\n" );

	return ( AFC_ERR_NO_ERROR );
}

int eqrows_cb_close ( DynamicClass * dyn )
{
	printf ( "eqrows close\n" );

	return ( AFC_ERR_NO_ERROR );
}

int eqrows_get_template ( DynamicClass * dyn )
{
	dyn->result = "";
	return ( AFC_ERR_NO_ERROR );
}

DynamicClass * eqrows_new_instance ()
{
	DynamicClass * dyn = afc_dynamic_class_new ();

	if ( dyn == NULL ) return NULL;

	afc_dynamic_class_add_method ( dyn, "open_callback", eqrows_cb_open );
	afc_dynamic_class_add_method ( dyn, "close_callback", eqrows_cb_close );
	afc_dynamic_class_add_method ( dyn, "get_template", eqrows_get_template );

	return ( dyn );
}

int eqrows_del_instance ( DynamicClass * dyn )
{
	if ( dyn != NULL ) afc_dynamic_class_delete ( dyn );

	return ( AFC_ERR_NO_ERROR );
}

//separator class
int separator_get_template ( DynamicClass * dyn )
{
	dyn->result = "NAME";
	return ( AFC_ERR_NO_ERROR );
}

int separator_cb_open ( DynamicClass * dyn )
{
	printf ( "separator!\n" );

	return ( AFC_ERR_NO_ERROR );
}

DynamicClass * separator_new_instance ()
{
	DynamicClass * dyn = afc_dynamic_class_new ();

	if ( dyn == NULL ) return NULL;

	afc_dynamic_class_add_method ( dyn, "open_callback", separator_cb_open );
	//afc_dynamic_class_add_method ( dyn, "close_callback", separator_cb_close );
	afc_dynamic_class_add_method ( dyn, "get_template", separator_get_template );

	return ( dyn );
}

int separator_del_instance ( DynamicClass * dyn )
{
	if ( dyn != NULL ) afc_dynamic_class_delete ( dyn );

	return ( AFC_ERR_NO_ERROR );
}


/*****************
  MAIN
******************/

int main ()
{
	CommandParser * cmdp = afc_cmd_parser_new ();
	DynamicClassMaster * dcm = afc_dynamic_class_master_new ();
	AFC * afc = afc_new ();
	char * stuff;
	int afc_res = AFC_ERR_NO_ERROR;
	struct somedata * mydata = NULL;

	mydata = ( struct somedata * ) afc_malloc ( sizeof ( struct somedata ) );
	mydata->cmdp = cmdp;

	stuff = afc_string_dup ("
(window \"This is a title\" 640 480
  (button b hello! clicked)
  (eqrows
    (button c Wow NULL)
    (separator)
  )
  (if expr 1
    (eqrows
      (button a foo NULL)
      (button d ddd NULL)
    )
    (eqrows
      (separator)
      (separator)
    )
  )
)
");

	afc_dynamic_class_master_add ( dcm, "window", NULL, window_new_instance, window_del_instance, NULL );
	afc_dynamic_class_master_add ( dcm, "button", NULL, button_new_instance, button_del_instance, NULL );
	afc_dynamic_class_master_add ( dcm, "eqrows", NULL, eqrows_new_instance, eqrows_del_instance, NULL );
	afc_dynamic_class_master_add ( dcm, "separator", NULL, separator_new_instance, separator_del_instance, NULL );

	if ( ( afc_res = afc_cmd_parser_add_commands ( cmdp, dcm ) ) == AFC_ERR_NO_ERROR ) {
		afc_res = afc_cmd_parser_parse_string ( cmdp, stuff, mydata );
	}

	printf ( "res: %x\n", afc_res );

	/* The order is: first cmd_parser deletes the plugin' instances, then class_master deletes itself.
	   If class_master is deleted first, then it will dispose all the plugins still used by cmd_parser,
  	 generating some errors. */
	afc_cmd_parser_delete ( cmdp );
	afc_dynamic_class_master_delete ( dcm );

	afc_free ( mydata );
	afc_string_delete ( stuff );

	afc_delete ( afc );

	exit ( 0 );
}


