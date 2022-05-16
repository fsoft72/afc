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
#include "dynamic_class.h"

static const char class_name [] = "DynamicClass";

static int afc_dynamic_class_internal_parse_args ( DynamicClass * dc, va_list args );
static int afc_dynamic_class_internal_clear_vars ( DynamicClass * dc, Dictionary * vars );
static DynamicClassVar *  afc_dynamic_class_internal_alloc ( DynamicClass * dc, int kind, char * name, void * val );
// static int afc_dynamic_class_internal_clear_methods ( DynamicClass * dc );

static int afc_dynamic_class_internal_clear_method ( void * m );

// {{{ docs
/*
@config
	TITLE:     DynamicClass
	VERSION:   1.00
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
@endnode

@node quote
	*President Merkin Muffley: Gentlemen, you can't fight in here, this is the war room!*

	Dr. Strangelove
@endnode

@node intro
DynamicClass is the abstraction layer of AFC "plugins". A DynamicClass rappresents a dynamically loaded
class in memory, that can be used inside your applications, once you have them loaded from disk.
Load, instance creation and instance disposion are handled by DynamicClassMaster, so you should have a look
at that class to know how to load new "plugins" in memory.

Once you have a DynamicClass instance, you can work with it in several ways. The standard way to execute a DynamicClass
*method* is to call the afc_dynamic_class_execute() call, that will resolve method pointer and will call it.
If you want really fast and responsive feedback, you may wish to save the function pointer inside a DynamicClassMethod variable.
to do so, simply call afc_dynamic_class_find_method(), and you'll be able to call that method by your own.

:Note:
	Please, keep in mind that this is stuff for advanced users only. There are some things you have to manually
	set if you want to call a method by your own. 


DynamicClass also supports storing of *variables*. You can set and get variable values by using
afc_dynamic_class_set_var() and afc_dynamic_class_get_var() respectively.
Variables can be of three kind: 

- AFC_DYNAMIC_CLASS_VAR_KIND_NUM: Numeric variables
- AFC_DYNAMIC_CLASS_VAR_KIND_POINTER: Pointer to something.
- AFC_DYNAMIC_CLASS_VAR_KIND_STRING: A string.

@endnode
*/
// }}}

// {{{ afc_dynamic_class_new ()
/*
@node afc_dynamic_class_new

	         NAME: afc_dynamic_class_new ( afc ) - Initializes a new DynamicClass instance.

	     SYNOPSIS: DynamicClass * afc_dynamic_class_new ()

	  DESCRIPTION: This function initializes a new DynamicClass instance.

	        INPUT: - NONE

	      RESULTS: a valid inizialized DynamicClass structure. NULL in case of errors.

	     SEE ALSO: - afc_dynamic_class_delete()

@endnode
*/
DynamicClass * afc_dynamic_class_new ()
{
TRY ( DynamicClass * )

	DynamicClass * dc = ( DynamicClass * ) afc_malloc ( sizeof ( DynamicClass ) );

	if ( dc == NULL ) RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "Dynamic Class", NULL );

	dc->magic = AFC_DYNAMIC_CLASS_MAGIC;

	if ( ( dc->methods = afc_dictionary_new () ) == NULL ) RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "methods", NULL );

	afc_dictionary_set_clear_func ( dc->methods, afc_dynamic_class_internal_clear_method );

	if ( ( dc->args = afc_array_master_new () ) == NULL ) RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "args", NULL );

	dc->add_arg_end = TRUE;

	RETURN ( dc );

EXCEPT
	afc_dynamic_class_delete ( dc );

FINALLY

ENDTRY
}
// }}}
// {{{ afc_dynamic_class_delete ( dc )
/*
@node afc_dynamic_class_delete

	         NAME: afc_dynamic_class_delete ( dc )  - Disposes a valid DynamicClass instance.

	     SYNOPSIS: int afc_dynamic_class_delete ( DynamicClass * dc)

	  DESCRIPTION: This function frees an already alloc'd DynamicClass structure.

	        INPUT: - dc  - Pointer to a valid afc_dynamic_class class.

	      RESULTS: should be AFC_ERR_NO_ERROR

	        NOTES: - this method calls: afc_dynamic_class_clear()

	     SEE ALSO: - afc_dynamic_class_new()
	               - afc_dynamic_class_clear()
@endnode

*/
int _afc_dynamic_class_delete ( DynamicClass * dc ) 
{
	int afc_res; 

	AFC_DEBUG_FUNC ();

	if ( ( afc_res = afc_dynamic_class_clear ( dc ) ) != AFC_ERR_NO_ERROR ) return ( afc_res );

	afc_dictionary_delete 	( dc->vars );
	afc_dictionary_delete 	( dc->methods );
	afc_dictionary_delete 	( dc->private_vars );
	afc_array_master_delete	( dc->args );

	afc_free ( dc );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_dynamic_class_clear ( dc )
/*
@node afc_dynamic_class_clear

	         NAME: afc_dynamic_class_clear ( dc )  - Clears all stored data

	     SYNOPSIS: int afc_dynamic_class_clear ( DynamicClass * dc)

	  DESCRIPTION: Use this function to clear all stored data in the current dc instance.

	        INPUT: - dc    - Pointer to a valid afc_dynamic_class instance.

	      RESULTS: should be AFC_ERR_NO_ERROR
	              
	     SEE ALSO: - afc_dynamic_class_delete()
	               
@endnode
*/
int afc_dynamic_class_clear ( DynamicClass * dc ) 
{
	AFC_DEBUG_FUNC ();

	if ( dc == NULL ) return ( AFC_ERR_NULL_POINTER );
 
	if ( dc->magic != AFC_DYNAMIC_CLASS_MAGIC ) return ( AFC_ERR_INVALID_POINTER );

	/* Custom Clean-up code should go here */
	if ( dc->vars ) afc_dynamic_class_internal_clear_vars ( dc, dc->vars );

	/* ----------------------------------  */

	if ( dc->vars ) afc_dictionary_clear ( dc->vars );

	// afc_dynamic_class_internal_clear_methods ( dc );
	if ( dc->methods ) afc_dictionary_clear ( dc->methods );

	if ( dc->private_vars ) afc_dictionary_clear ( dc->private_vars );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_dynamic_class_add_method ( dc, name, params, func )
/*
@node afc_dynamic_class_add_method

	         NAME: afc_dynamic_class_add_method ( dc, name, params, func )  - Adds a new method to the class

	     SYNOPSIS: int afc_dynamic_class_clear ( DynamicClass * dc, char * name, char * params, DynamicClassMethod func )

	  DESCRIPTION: 	This function adds a new method to the current Dynamic Class. The method will have the name
		  	you specify with the *name* parameter and will point to the *func* function.

	        INPUT: 	- dc    	- Pointer to a valid DynamicClass instance.
		       	- name  	- Name of the method
			- params	- Params list defined in this way:
						+ S - The param is a String
						+ N - The param is a Number
						+ P - The param is a Pointer

			- func		- Function to bind to this method name	

	      RESULTS: should be AFC_ERR_NO_ERROR
	              
	     SEE ALSO: - afc_dynamic_class_execute()
	               - afc_dynamic_class_find_method()
	               
@endnode
*/
int afc_dynamic_class_add_method ( DynamicClass * dc, char * name, char * params, DynamicClassMethod func )
{
	DynamicClassMethodData * dcmd;

	if ( ( dcmd = afc_malloc ( sizeof ( DynamicClassMethodData ) ) ) == NULL )
		return ( AFC_LOG_FAST ( AFC_ERR_NO_MEMORY ) );

	dcmd->name = afc_string_dup ( name );
	dcmd->func = func;
	dcmd->params = afc_string_dup ( params );

	return ( afc_dictionary_set ( dc->methods, name, dcmd ) );	
}
// }}}
// {{{ afc_dynamic_class_execute ( dc, name, ... )
/*
@node afc_dynamic_class_execute

	         NAME: afc_dynamic_class_execute ( dc, name, ... )  - Executes a public method in DynamicClass

	     SYNOPSIS: int afc_dynamic_class_execute ( DynamicClass * dc, const char * name, ... )

	  DESCRIPTION: 	This function executes the specified method of a DynamicClass. The method you want to
			call is specified in the *name* arg. 
			This function accepts a variable length of parameters that will be passed directly to the
			method being called. Remember to finish the list with AFC_DYNAMIC_CLASS_ARG_END.
			

	        INPUT: 	- dc    	- Pointer to a valid DynamicClass instance.
		       	- name  	- Name of the method to be found.
			- ...		- Any number of params you want to pass to the method being called.
					  Remember to finish the list with AFC_DYNAMIC_CLASS_ARG_END.

	      RESULTS: 	- The DynamicClassMethod pointer if the method exists.
			- NULL if the method cannot be found.

		NOTES:	- *ALWAYS* finish the list with AFC_DYNAMIC_CLASS_ARG_END, even if the list is empty, remember to
			  finish it with it!
	              
	     SEE ALSO: 	- afc_dynamic_class_execute_vars()
			- afc_dynamic_class_find_method()
	               	- afc_dynamic_class_add_method()
	               
@endnode
*/
int _afc_dynamic_class_execute ( DynamicClass * dc, const char * name, ... )
{
	va_list args;
	int res;

	// Gets the pointer for the varargs attributes
	va_start ( args, name );

	res = afc_dynamic_class_execute_vars ( dc, name, args );

	// Free the stack from the given args
	va_end ( args );

	return ( res );
}
// }}}
// {{{ afc_dynamic_class_execute_vars ( dc, name, args )
/*
@node afc_dynamic_class_execute_vars

	         NAME: afc_dynamic_class_execute_vars ( dc, name, args )  - Executes a public method in DynamicClass

	     SYNOPSIS: int afc_dynamic_class_execute_vars ( DynamicClass * dc, const char * name, va_list args )

	  DESCRIPTION: 	This function executes the specified method of a DynamicClass. The method you want to
			call is specified in the *name* arg. 
			This function accepts a variable length of parameters that will be passed directly to the
			method being called. Remember to finish the list with AFC_DYNAMIC_CLASS_ARG_END.
			

	        INPUT: 	- dc    	- Pointer to a valid DynamicClass instance.
		       	- name  	- Name of the method to be found.
			- args		- A va_list containing all the args for the method
					 

	      RESULTS: 	- The DynamicClassMethod pointer if the method exists.
			- NULL if the method cannot be found.

		NOTES:	- *ALWAYS* finish the list with AFC_DYNAMIC_CLASS_ARG_END, even if the list is empty, remember to
			  finish it with it!
	              
	     SEE ALSO: 	- afc_dynamic_class_execute()
			- afc_dynamic_class_find_method()
	               	- afc_dynamic_class_add_method()
	               
@endnode
*/
int afc_dynamic_class_execute_vars ( DynamicClass * dc, const char * name, va_list args )
{
	DynamicClassMethod func;	
	int res;
	DynamicClassMethodData * dcmd;

	dc->result_type = AFC_DYNAMIC_CLASS_RESULT_TYPE_UNKNOWN;

	if ( ( dcmd = afc_dictionary_get ( dc->methods, name ) ) == NULL ) 
		return ( AFC_LOG ( AFC_LOG_WARNING, AFC_DYNAMIC_CLASS_ERR_METHOD_NOT_FOUND, "Requested method not found", name ) );
	
	func =   dcmd->func;

	// Parses the args and dispose them inside the ArrayMaster dc->args
	if ( args != NULL ) afc_dynamic_class_internal_parse_args ( dc, args );

	// Calls the plugin method selected
	res = func ( dc );

	return ( res );
}
// }}}
// {{{ afc_dynamic_class_find_method ( dc, name )
/*
@node afc_dynamic_class_find_method

	         NAME: afc_dynamic_class_find_method ( dc, name )  - Finds a public method inside a DynamicClass

	     SYNOPSIS: DynamicClassMethodData * afc_dynamic_class_find_method ( DynamicClass * dc, char * name )

	  DESCRIPTION: 	This function searches the DynamicClass for a specific method. If it can found it, then
			the function pointer (DynamicClassMethod) is returned.

	        INPUT: 	- dc    	- Pointer to a valid DynamicClass instance.
		       	- name  	- Name of the method to be found.

	      RESULTS: 	- The DynamicClassMethod pointer if the method exists.
			- NULL if the method cannot be found.
	              
	     SEE ALSO: - afc_dynamic_class_execute()
	               - afc_dynamic_class_add_method()
	               
@endnode
*/
/*
DynamicClassMethodData * afc_dynamic_class_find_method ( DynamicClass * dc, char * name )
{
	return ( ( DynamicClassMethodData * ) afc_dictionary_get ( dc->methods, name ) );
}
*/
// }}}
// {{{ afc_dynamic_class_set_var ( dc, kind, name, val ) *****
int afc_dynamic_class_set_var ( DynamicClass * dc, int kind, char * name, void * val )
{
	DynamicClassVar * var;

	if ( dc->vars == NULL )
		if ( ( dc->vars = afc_dictionary_new () ) == NULL ) 
			return ( AFC_LOG_FAST_INFO ( AFC_ERR_NO_MEMORY, "vars" ) );

	var = afc_dynamic_class_internal_alloc ( dc, kind, name, val );
	
	// If internal_alloc() returns a NULL var, it means that we have to
	// do nothing.
	if ( var == NULL ) return ( AFC_ERR_NO_ERROR );

	// Set the right kind of it
	var->kind = kind;


	// Copy the value
	switch ( kind )
	{
		case AFC_DYNAMIC_CLASS_VAR_KIND_NUM:
		case AFC_DYNAMIC_CLASS_VAR_KIND_POINTER:
			var->value = val;
			break;
		case AFC_DYNAMIC_CLASS_VAR_KIND_STRING:
			var->value = afc_string_dup ( ( char * ) val );
			break;
	}

	return ( afc_dictionary_set ( dc->vars, name, var ) );
}
// }}}
// {{{ afc_dynamic_class_get_var ( dc, name ) *****
void * afc_dynamic_class_get_var ( DynamicClass * dc, char * name )
{
	DynamicClassVar * var;

	if ( ( dc == NULL ) || ( dc->vars == NULL ) ) return ( NULL );

	if ( ( var = afc_dictionary_get_default ( dc->vars, name, NULL ) ) == NULL ) return ( NULL );

	return ( var->value );
}
// }}}

// -------------------------------------------------------------------------------------------------------------
// INTERNAL FUNCTIONS
// -------------------------------------------------------------------------------------------------------------
// {{{ afc_dynamic_class_internal_parse_args ( dc, args )
static int afc_dynamic_class_internal_parse_args ( DynamicClass * dc, va_list args )
{
	void * v;

	afc_array_master_clear ( dc->args );

	while ( ( v = va_arg ( args, void * ) ) != AFC_DYNAMIC_CLASS_ARG_END )
	{
		afc_array_master_add ( dc->args, v, AFC_ARRAY_MASTER_ADD_TAIL );
	}

	if ( dc->add_arg_end ) afc_array_master_add ( dc->args, AFC_DYNAMIC_CLASS_ARG_END, AFC_ARRAY_MASTER_ADD_TAIL );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_dynamic_class_internal_clear_vars ( dc, vars )
static int afc_dynamic_class_internal_clear_vars ( DynamicClass * dc, Dictionary * vars )
{
	DynamicClassVar * var;

	var = afc_dictionary_first ( vars );

	while ( var )
	{
		if ( var->kind == AFC_DYNAMIC_CLASS_VAR_KIND_STRING ) 
			afc_string_delete ( var->value );

		afc_free ( var );

		var = afc_dictionary_next ( vars );
	}

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_dynamic_class_internal_alloc ( dc, kind, name, val )
static DynamicClassVar *  afc_dynamic_class_internal_alloc ( DynamicClass * dc, int kind, char * name, void * val )
{
	DynamicClassVar * var;

	// Try to see if the var is already present inside the dictionary
	var = afc_dictionary_get ( dc->vars, name );

	// If it is not present, we alloc a new instance and return
	if ( var == NULL )
	{
		// Alloc a new DynamicClassVar structure
		if  ( ( var = afc_malloc ( sizeof ( DynamicClassVar ) ) ) == NULL )		
		{	
			AFC_LOG_FAST ( AFC_ERR_NO_MEMORY );

			return ( NULL );
		} 

		return ( var );
	}

	// If we get this far, it's because we already have that variable in memory.

	// First of all, it the "old" var was a string, we have to delete the string linked to it
	if ( var->kind == AFC_DYNAMIC_CLASS_VAR_KIND_STRING )	afc_string_delete ( var->value );

	// If the VALUE is NULL, then we simply delete it from the list of variables set
	if ( val == NULL ) 
	{
		afc_free ( var );
		afc_dictionary_set ( dc->vars, name, NULL );

		return ( NULL );
	}

	return ( var );
}
// }}}
// {{{ afc_dynamic_class_internal_clear_method ( m )
static int afc_dynamic_class_internal_clear_method ( void * m )
{
	DynamicClassMethodData * dcmd = m;

	if ( dcmd == NULL ) return ( AFC_ERR_NO_ERROR );

	afc_string_delete ( dcmd->name );
	afc_string_delete ( dcmd->params );
		
	afc_free ( dcmd );

	return ( AFC_ERR_NO_ERROR );
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
int test_func ( DynamicClass * dc )
{
	int t = 0 , v;

	printf ( "Test function :-)\n" );

	v = ( int ) afc_array_master_first ( dc->args );

	while ( v != ( int ) AFC_DYNAMIC_CLASS_ARG_END )
	{
		printf ( "Arg: %d - %d\n", t++, v );

		v = ( int ) afc_array_master_next ( dc->args );
	}

	printf ( "Pippo: %d\n", ( int ) afc_dynamic_class_get_var ( dc, "pippo" ) );
	printf ( "Pluto: %s\n", ( char * ) afc_dynamic_class_get_var ( dc, "pluto" ) );


	return ( AFC_ERR_NO_ERROR );
}


int main ( int argc, char * argv[] )
{
	AFC * afc = afc_new ();
	DynamicClass * dc = afc_dynamic_class_new();

	if ( dc == NULL ) 
	{
	  fprintf ( stderr, "Init of class DynamicClass failed.\n" );
	  return ( 1 );
	}

	afc_dynamic_class_add_method ( dc, "test", test_func );

	afc_dynamic_class_set_var ( dc, AFC_DYNAMIC_CLASS_VAR_KIND_NUM, "pippo",  ( void * ) 123 );
	afc_dynamic_class_set_var ( dc, AFC_DYNAMIC_CLASS_VAR_KIND_STRING, "pluto", "ciao" );

	afc_dynamic_class_execute ( dc, "test", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, AFC_DYNAMIC_CLASS_ARG_END );
	afc_dynamic_class_execute ( dc, "test", 3, 2, 1, 0, AFC_DYNAMIC_CLASS_ARG_END );
	afc_dynamic_class_execute ( dc, "test" , 111, AFC_DYNAMIC_CLASS_ARG_END );


	afc_dynamic_class_delete ( dc );
	afc_delete ( afc );

	return ( 0 ); 
}
// }}}
#endif
