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
#include "dynamic_class_master.h"

static const char class_name[] = "DynamicClassMaster";

// static int afc_dynamic_class_master_internal_clear_classes ( DynamicClassMaster * dcm );
// static int afc_dynamic_class_master_internal_clear_instances ( DynamicClassMaster * dcm );

static int afc_dynamic_class_master_internal_clear_class(void *c);
static int afc_dynamic_class_master_internal_clear_instance(Hash *hm, void *d);

// {{{ docs
/*
@config
	TITLE:     DynamicClassMaster
	VERSION:   1.00
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
@endnode

@node quote
*What's big and green, and if it falls out of a tree it'll kill you?*

*A pool table.*

	Gary Larson
@endnode

@node intro
DynamicClassMaster is a class that handles loading and managing of "plugin" (rappresented in AFC with the DynamicClass).

DynamicClasses cannot be used by their own: you need a DynamicClassMaster to load them and make them avaible to your applications.

To load a DynamicClass inside your DynamicClassMaster, simply call the afc_dynamic_class_master_load() method.
This will load the "plugin" in memory.

Once a "plugin" definition is in memory, you can create an instance of it by calling the
afc_dynamic_class_master_new_instance() method. Remember to free all instances by calling the
afc_dynamic_class_master_delete_instance() method once you have done with them. Anyway, they'll all
be freed automatically by DynamicClassMaster before exiting.

@endnode
*/
// }}}

// {{{ afc_dynamic_class_master_new ()
/*
@node afc_dynamic_class_master_new

			 NAME: afc_dynamic_class_master_new () - Initializes a new DynamicClassMaster instance.

		 SYNOPSIS: DynamicClassMaster * afc_dynamic_class_master_new ()

	  DESCRIPTION: This function initializes a new DynamicClassMaster instance.

			INPUT: NONE

		  RESULTS: a valid inizialized DynamicClassMaster structure. NULL in case of errors.

		 SEE ALSO: - afc_dynamic_class_master_delete()

@endnode
*/
DynamicClassMaster *afc_dynamic_class_master_new()
{
	TRY(DynamicClassMaster *)

	DynamicClassMaster *dcm = (DynamicClassMaster *)afc_malloc(sizeof(DynamicClassMaster));

	if (dcm == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "dcm", NULL);

	dcm->magic = AFC_DYNAMIC_CLASS_MASTER_MAGIC;

	if ((dcm->classes = afc_dictionary_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "classes", NULL);
	afc_dictionary_set_clear_func(dcm->classes, afc_dynamic_class_master_internal_clear_class);

	if ((dcm->instances = afc_hash_new()) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "instances", NULL);

	afc_hash_set_clear_func(dcm->instances, afc_dynamic_class_master_internal_clear_instance);

	RETURN(dcm);

	EXCEPT
	afc_dynamic_class_master_delete(dcm);

	FINALLY

	ENDTRY
}
// }}}
// {{{ afc_dynamic_class_master_delete ( dcm )
/*
@node afc_dynamic_class_master_delete

			 NAME: afc_dynamic_class_master_delete ( dcm )  - Disposes a valid DynamicClassMaster instance.

		 SYNOPSIS: int afc_dynamic_class_master_delete ( DynamicClassMaster * dcm)

	  DESCRIPTION: This function frees an already alloc'd DynamicClassMaster structure.

			INPUT: - dcm  - Pointer to a valid afc_dynamic_class_master class.

		  RESULTS: should be AFC_ERR_NO_ERROR

			NOTES: - this method calls: afc_dynamic_class_master_clear()

		 SEE ALSO: - afc_dynamic_class_master_new()
				   - afc_dynamic_class_master_clear()
@endnode

*/
int _afc_dynamic_class_master_delete(DynamicClassMaster *dcm)
{
	int afc_res;

	AFC_DEBUG_FUNC();

	if ((afc_res = afc_dynamic_class_master_clear(dcm)) != AFC_ERR_NO_ERROR)
		return (afc_res);

	afc_dictionary_delete(dcm->classes);
	afc_hash_delete(dcm->instances);

	afc_free(dcm);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dynamic_class_master_clear ( dcm )
/*
@node afc_dynamic_class_master_clear

			 NAME: afc_dynamic_class_master_clear ( dcm )  - Clears all stored data

		 SYNOPSIS: int afc_dynamic_class_master_clear ( DynamicClassMaster * dcm)

	  DESCRIPTION: 	Use this function to clear all stored data in the current dcm instance.
			Operations performed are the following:

			1. All instances not freed using afc_dynamic_class_master_delete_instance()
			   are freed by the manager.
			2. All classes definitions loaded using afc_dynamic_class_master_load() are
			   removed from memory.
			3. Internal lists are cleared.

			INPUT: - dcm    - Pointer to a valid afc_dynamic_class_master instance.

		  RESULTS: should be AFC_ERR_NO_ERROR

		 SEE ALSO: 	- afc_dynamic_class_master_delete()
			- afc_dynamic_class_master_new_instance()
			- afc_dynamic_class_master_delete_instance()
			- afc_dynamic_class_master_load()

@endnode
*/
int afc_dynamic_class_master_clear(DynamicClassMaster *dcm)
{
	AFC_DEBUG_FUNC();

	if (dcm == NULL)
		return (AFC_ERR_NULL_POINTER);

	if (dcm->magic != AFC_DYNAMIC_CLASS_MASTER_MAGIC)
		return (AFC_ERR_INVALID_POINTER);

	// delete any plugin still in memory
	// afc_dynamic_class_master_internal_clear_instances ( dcm );
	afc_hash_clear(dcm->instances);

	// delete all the classes loaded (plugins)
	// afc_dynamic_class_master_internal_clear_classes ( dcm );
	afc_dictionary_clear(dcm->classes);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dynamic_class_master_load ( dcm, class_name, file_name )
/*
@node afc_dynamic_class_master_load

			 NAME: afc_dynamic_class_master_load ( dcm, class_name, file_name )  - Loads a new class in memory

		 SYNOPSIS: int afc_dynamic_class_master_load ( DynamicClassMaster * dcm, const char * class_name, const char * file_name )

	  DESCRIPTION: 	This function loads a DynamicClass in memory. When loading, you have to specify a valid class name,
			using the *class_name* arg, since the file loaded can be any valid dynamic class and the loader is
			unable to know what class it actually is. So, for example, you can have a dynamic class stored in a
			file called "foo.so", and internally instantiated like "List".

			INPUT: 	- dcm    	- Pointer to a valid afc_dynamic_class_master instance.
			- class_name	- Name to assign to the class loaded
			- file_name	- Fully qualified file name to load the class from.

		  RESULTS: should be AFC_ERR_NO_ERROR

		NOTES:	- If you will try to load two different classes with the same /class_name/, you'll get an error message
			  and the new class will not be loaded in memory.

		 SEE ALSO: 	- afc_dynamic_class_master_add()
			- afc_dynamic_class_master_delete()
			- afc_dynamic_class_master_new_instance()
			- afc_dynamic_class_master_delete_instance()
@endnode
*/
int afc_dynamic_class_master_load(DynamicClassMaster *dcm, const char *class_name, const char *file_name)
{
	void *handler;
	DynamicClass *(*new_inst)(void);
	int (*del_inst)(DynamicClass *);
	char *(*info)(int);

	if (afc_dictionary_get(dcm->classes, class_name) != NULL)
		return (AFC_LOG(AFC_LOG_ERROR, AFC_DYNAMIC_CLASS_MASTER_ERR_DUPLICATE_NAME, "A class with the same internal name already exists", class_name));

	if ((handler = dlopen(file_name, RTLD_LAZY | RTLD_GLOBAL)) == NULL)
		return (AFC_LOG(AFC_LOG_ERROR, AFC_DYNAMIC_CLASS_MASTER_ERR_DLOPEN, "Could not load class", dlerror()));

	if ((new_inst = dlsym(handler, "dynamic_class_new_instance")) == NULL)
	{
		dlclose(handler);
		return (AFC_LOG(AFC_LOG_ERROR, AFC_DYNAMIC_CLASS_MASTER_ERR_DLSYM, "Could not find symbol", dlerror()));
	}

	if ((del_inst = dlsym(handler, "dynamic_class_del_instance")) == NULL)
	{
		dlclose(handler);
		return (AFC_LOG(AFC_LOG_ERROR, AFC_DYNAMIC_CLASS_MASTER_ERR_DLSYM, "Could not find symbol", dlerror()));
	}

	// Optional functions.
	// These functions can be omitted in the plugin declaration
	info = dlsym(handler, "dynamic_class_get_info");

	afc_dynamic_class_master_add(dcm, class_name, handler, new_inst, del_inst, info);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dynamic_class_master_add ( dcm, class_name, handler, new_inst, del_inst, info )
/*
@node afc_dynamic_class_master_add

			 NAME: afc_dynamic_class_master_add ( dcm, class_name, handler, new_inst, del_inst, info )  - Adds a new Dynamic Class in memory

		 SYNOPSIS: int afc_dynamic_class_master_add ( DynamicClassMaster * dcm, const char * class_name, void * handler, DynamicClass * ( *new_inst ) (), int ( *del_inst ) (DynamicClass *), chat * (*info) (int) );

	  DESCRIPTION: 	This function adds a DynamicClass in memory. This is very usefull if you want to add "static" DynamicClasses that
			are not loaded directly from a file, but already present in memory. This is a "low level" function, because it is not
			so easy to use.
			Since this function is used directly by afc_dynamic_class_master_load(), you have to specify a dllib handler as third param.
			This can be set to NULL if you are adding a class already in memory.

			INPUT: 	- dcm    	- Pointer to a valid afc_dynamic_class_master instance.
			- class_name	- Name to assign to the class loaded
			- handler	- The dllib handler. This can be set to NULL if you are adding a "static" class.
			- new_inst	- Pointer to the function to be used when creating a new DynamicClass instance.
			- del_inst	- Pointer to the function to be used when deleting a DynamicClass instance.
			- info		- Pointer to the function to be used to get info about a DynamicClass. This can be set to NULL.

		  RESULTS: should be AFC_ERR_NO_ERROR

		NOTES:	- If you will try to load two different classes with the same /class_name/, you'll get an error message
			  and the new class will not be loaded in memory.

		 SEE ALSO: 	- afc_dynamic_class_master_load()
			- afc_dynamic_class_master_delete()
			- afc_dynamic_class_master_new_instance()
			- afc_dynamic_class_master_delete_instance()
@endnode
*/
int afc_dynamic_class_master_add(DynamicClassMaster *dcm, const char *class_name, void *handler, DynamicClass *(*new_inst)(void), int (*del_inst)(DynamicClass *), char *(*info)(int))
{
	DCMData *data;

	if (afc_dictionary_get(dcm->classes, class_name) != NULL)
		return (AFC_LOG(AFC_LOG_ERROR, AFC_DYNAMIC_CLASS_MASTER_ERR_DUPLICATE_NAME, "A class with the same internal name already exists", class_name));

	if ((data = afc_malloc(sizeof(DCMData))) == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NO_MEMORY));

	data->dl_handler = handler;
	data->new_instance = new_inst;
	data->del_instance = del_inst;
	data->get_info = info;

	// Adding new class to the classes repository
	afc_dictionary_set(dcm->classes, class_name, data);

	// fprintf ( stderr, ">>> CLASS: %s (%x)\n", class_name, ( int ) data );

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dynamic_class_master_new_instance ( dcm, class_name )
/*
@node afc_dynamic_class_master_new_instance

			 NAME: afc_dynamic_class_master_new_instance ( dcm, class_name )  - Creates a new instance of a DynamicClass in memory

		 SYNOPSIS: DynamicClass * afc_dynamic_class_master_new_instance ( DynamicClassMaster * dcm, const char * class_name )

	  DESCRIPTION: 	This function creates a new instance of the specified *class_name*. The *class_name* must be a valid name
			of a class currently in memory, assigned bu the afc_dynamic_class_master_load() function call.

			INPUT: 	- dcm    	- Pointer to a valid afc_dynamic_class_master instance.
			- class_name	- Name of the class to create the new instance from.

		  RESULTS: 	- a pointer to a ready to use DynamicClass
			- NULL in case of any error

		 SEE ALSO: 	- afc_dynamic_class_master_delete_instance()
			- afc_dynamic_class_master_load()
			- afc_dynamic_class_master_add()
@endnode
*/
DynamicClass *afc_dynamic_class_master_new_instance(DynamicClassMaster *dcm, const char *class_name)
{
	DCMData *data = afc_dictionary_get(dcm->classes, class_name);
	DCMIData *idata;
	DynamicClass *dc;

	if (data == NULL)
	{
		AFC_LOG(AFC_LOG_ERROR, AFC_DYNAMIC_CLASS_MASTER_ERR_INSTANCE, "Could not get instance", class_name);

		return (NULL);
	}

	dc = data->new_instance();

	if (dc)
	{
		dc->info = dcm->info; // Propagates the DCM info into the DC

		idata = afc_malloc(sizeof(DCMIData));

		// if (  strcmp ( class_name, "separator" ) == 0 )
		// fprintf ( stderr, "INSTANCE: %x %s (%x)\n", (int ) idata, class_name, ( int ) data );

		// Save the Instance data inside the idata structure
		idata->class_name = afc_string_dup(class_name); // Name of the class instantiated
		idata->data = data;								// Class containing all methods (base class)
		idata->instance = dc;							// Address of the new instance (DynamicClass)

		// Add this idata to the Hash managing instances
		// The key is the new DynamicClass instance, while the data is the idata itself
		afc_hash_add(dcm->instances, (unsigned long int)dc, idata);

		// fprintf ( stderr, ">>> New Instance: %s (%x)\n", class_name, ( int )idata );
	}

	return (dc);
}
// }}}
// {{{ afc_dynamic_class_master_delete_instance ( dcm, instance )
/*
@node afc_dynamic_class_master_delete_instance

			 NAME: afc_dynamic_class_master_delete_instance ( dcm, instance )  - Deletes a DynamicClass instance from memory

		 SYNOPSIS: int afc_dynamic_class_delete_instance ( DynamicClassMaster * dcm, DynamicClass * instance )

	  DESCRIPTION: 	This function removes a DynamicClass instance from memory. Please, remember that this function just deletes
			an /instance/ of a specified class and not the class itself, so any other instance of the same class will continue
			to work with no problems.

			INPUT: 	- dcm    	- Pointer to a valid afc_dynamic_class_master instance.
			- instance	- Instance to free from memory

		  RESULTS: 	- should be AFC_ERR_NO_ERROR

		 SEE ALSO: 	- afc_dynamic_class_master_new_instance()
@endnode
*/
int afc_dynamic_class_master_delete_instance(DynamicClassMaster *dcm, DynamicClass *instance)
{
	DCMIData *idata = afc_hash_find(dcm->instances, (unsigned long int)instance);

	AFC_DEBUG_FUNC();

	if (idata == NULL)
		return (AFC_LOG(AFC_LOG_ERROR, AFC_DYNAMIC_CLASS_MASTER_ERR_INVALID_INSTANCE, "Invalid address for this instance", NULL));

	afc_hash_del(dcm->instances);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dynamic_class_master_get_info ( dcm, class_name, info_id )
/*
@node afc_dynamic_class_master_get_info

			 NAME: afc_dynamic_class_master_get_info ( dcm, class_name, info_id )  - Gets various info about a Class in memory

		 SYNOPSIS: char * afc_dynamic_class_get_info ( DynamicClassMaster * dcm, char * class_name, int info_id )

	  DESCRIPTION: 	This function can get some info about a certain class currently in memory. This function relies upon
			a specific optional Class function. If the Dynamic Class has not defined it, you will simply get NULL
			return values when this function is called.

			INPUT: 	- dcm    	- Pointer to a valid afc_dynamic_class_master instance.
			- class_name	- Name of the class to query
			- info_id	- Value of the info you want. Valid values are:
						+ AFC_DYNAMIC_CLASS_MASTER_INFO_NAME - Name of the Dynamic Class
						+ AFC_DYNAMIC_CLASS_MASTER_INFO_VERSION - Version in the format 1.2.3
						+ AFC_DYNAMIC_CLASS_MASTER_INFO_AUTHOR - Name of the class'author
						+ AFC_DYNAMIC_CLASS_MASTER_INFO_EMAIL - E-mail of the class'author
						+ AFC_DYNAMIC_CLASS_MASTER_INFO_URL - URL of reference for the class
						+ AFC_DYNAMIC_CLASS_MASTER_INFO_DESCR - Description of the class
						+ AFC_DYNAMIC_CLASS_MASTER_INFO_DESCR_SHORT - Short description of the class

		NOTES:	- Please, remember that the implementation of this function is *optional* inside Dynamic Classes.

			- Since the /info_id/ is just a numerical value, a Dynamic Class could define others values as well not
			  appearing in the list above. Please, refer to the Dynamic Class documentation for it.

		  RESULTS: 	a string containing the desired text, or NULL if the method is not supported or the /info_id/ out of range.

		 SEE ALSO: 	- afc_dynamic_class_master_new_instance()
@endnode
*/
char *afc_dynamic_class_master_get_info(DynamicClassMaster *dcm, char *class_name, int info_id)
{
	DCMData *data = afc_dictionary_get(dcm->classes, class_name);

	if (data == NULL)
	{
		AFC_LOG(AFC_LOG_ERROR, AFC_DYNAMIC_CLASS_MASTER_ERR_INSTANCE, "Could not get instance", class_name);

		return (NULL);
	}

	// The get_info function has not been defined inside the plugin
	if (data->get_info == NULL)
		return (NULL);

	// The method exists, so we simply call it with the info_id required and return its value
	return (data->get_info(info_id));
}
// }}}
// {{{ afc_dynamic_class_master_set_tag ( dcm, tag, val ) ***********
int afc_dynamic_class_master_set_tag(DynamicClassMaster *dcm, int tag, void *val)
{
	switch (tag)
	{
	case AFC_DYNAMIC_CLASS_MASTER_TAG_INFO:
		dcm->info = val;
		break;

	case AFC_DYNAMIC_CLASS_MASTER_TAG_CHECK_PARAMS:
		dcm->check_params = (BOOL)(int)(long)val;
		break;

	default:
		return (AFC_LOG_FAST(AFC_ERR_UNSUPPORTED_TAG));
	}

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dynamic_class_master_set_tags ( dcm, first_tag, ... ) ***********
int _afc_dynamic_class_master_set_tags(DynamicClassMaster *dcm, int first_tag, ...)
{
	va_list tags;
	unsigned int tag;
	void *val;

	va_start(tags, first_tag);

	tag = first_tag;

	while (tag != AFC_TAG_END)
	{
		val = va_arg(tags, void *);

		afc_dynamic_class_master_set_tag(dcm, tag, val);

		tag = va_arg(tags, int);
	}

	va_end(tags);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dynamic_class_master_has_class ( dcm, class_name )
/*
@node afc_dynamic_class_master_has_class

			 NAME: afc_dynamic_class_master_has_class ( dcm, class_name )  - Checkes whether a certain class is already loaded

		 SYNOPSIS: int afc_dynamic_class_has_class ( DynamicClassMaster * dcm, const char * class_name )

	  DESCRIPTION: 	This function can check whether a certatin class specified by /class_name/ has already been loaded or not.
			This is useful, for example, if you want to load plugins "on demand" rather than loading them all at program
			startup.

			INPUT: 	- dcm    	- Pointer to a valid afc_dynamic_class_master instance.
			- class_name	- Name of the class to query

		  RESULTS: 	- AFC_ERR_NO_ERROR	- The DynamicClassMaster already has this class in memory
			- AFC_DYNAMIC_CLASS_MASTER_ERR_CLASS_NOT_FOUND	- The Class is not in memory

		 SEE ALSO: 	- afc_dynamic_class_master_load()
@endnode
*/
int afc_dynamic_class_master_has_class(DynamicClassMaster *dcm, const char *class_name)
{
	if (afc_dictionary_get(dcm->classes, class_name) != NULL)
		return (AFC_ERR_NO_ERROR);
	else
		return (AFC_DYNAMIC_CLASS_MASTER_ERR_CLASS_NOT_FOUND);
}
// }}}

// -----------------------------------------------------------------------------------------------------
// INTERNAL FUNCTIONS
// -----------------------------------------------------------------------------------------------------
// {{{ afc_dynamic_class_master_internal_clear_class ( c )
static int afc_dynamic_class_master_internal_clear_class(void *c)
{
	DCMData *data = c;

	if (data == NULL)
		return (AFC_ERR_NO_ERROR);

	if (data->dl_handler)
		dlclose(data->dl_handler);

	afc_free(data);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_dynamic_class_master_internal_clear_instance ( hm, d )
static int afc_dynamic_class_master_internal_clear_instance(Hash *hm, void *d)
{
	DCMIData *idata = d;

	if (idata == NULL)
		return (AFC_ERR_NO_ERROR);

	idata->data->del_instance(idata->instance);
	afc_string_delete(idata->class_name);
	afc_free(idata);

	return (AFC_ERR_NO_ERROR);
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
int main(int argc, char *argv[])
{
	AFC *afc = afc_new();
	DynamicClassMaster *dcm = afc_dynamic_class_master_new();
	DynamicClass *dc;
	DynamicClass *dc2;

	if (dcm == NULL)
	{
		fprintf(stderr, "Init of class DynamicClassMaster failed.\n");
		return (1);
	}

	afc_dynamic_class_master_load(dcm, "test1", "plugins/plug1.so");
	dc = afc_dynamic_class_master_new_instance(dcm, "test1");

	afc_dynamic_class_master_load(dcm, "test2", "plugins/plug2.so");
	dc2 = afc_dynamic_class_master_new_instance(dcm, "test2");

	afc_dynamic_class_execute(dc, "test", AFC_DYNAMIC_CLASS_ARG_END);
	afc_dynamic_class_execute(dc2, "test", AFC_DYNAMIC_CLASS_ARG_END);

	// afc_dynamic_class_master_delete_instance ( dcm, dc );
	afc_dynamic_class_master_delete_instance(dcm, dc2);

	afc_dynamic_class_master_delete(dcm);
	afc_delete(afc);

	return (0);
}
// }}}
#endif
