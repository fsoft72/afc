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
#ifndef AFC_DYNAMIC_CLASS_H
#define AFC_DYNAMIC_CLASS_H
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "base.h"
#include "array.h"
#include "dictionary.h"

/* DynamicClass'Magic' value: 'DYNC' */
#define AFC_DYNAMIC_CLASS_MAGIC ('D' << 24 | 'Y' << 16 | 'N' << 8 | 'C')

/* DynamicClass Base  */
#define AFC_DYNAMIC_CLASS_BASE 0xC000

enum
{
	AFC_DYNAMIC_CLASS_ERR_METHOD_NOT_FOUND = AFC_DYNAMIC_CLASS_BASE + 1
};

enum
{
	AFC_DYNAMIC_CLASS_RESULT_TYPE_UNKNOWN = 0,
	AFC_DYNAMIC_CLASS_RESULT_TYPE_INTEGER,
	AFC_DYNAMIC_CLASS_RESULT_TYPE_STRING,
	AFC_DYNAMIC_CLASS_RESULT_TYPE_POINTER,
	AFC_DYNAMIC_CLASS_RESULT_TYPE_DICTIONARY
};

struct afc_dynamic_class
{
	unsigned long magic; /* DynamicClass Magic Value */

	ArrayMaster *args; // Store the args from a method call

	Dictionary *vars;
	Dictionary *methods;
	Dictionary *private_vars;

	short check_params; // Perform param checking (default: false)

	void *result;	 // This can be used to store a method result
	int result_type; // Result type returned by the method

	void *info; // Use and abuse it as you like

	short add_arg_end; // Flag. If TRUE, inside dyn->args will be added an AFC_DYNAMIC_CLASS_ARG_END after the last element
					   // Default is TRUE, for backward compatibility.
};

typedef struct afc_dynamic_class DynamicClass;

typedef int (*DynamicClassMethod)(DynamicClass *);

#define AFC_DYNAMIC_CLASS_VAR_KIND_NUM 0
#define AFC_DYNAMIC_CLASS_VAR_KIND_STRING 1
#define AFC_DYNAMIC_CLASS_VAR_KIND_POINTER 2
#define AFC_DYNAMIC_CLASS_VAR_KIND_DICTIONARY 3

struct afc_dynamic_class_var
{
	char kind;	 // See: AFC_DYNAMIC_CLASS_VAR_KIND_*
	void *value; // Variable value
};

typedef struct afc_dynamic_class_var DynamicClassVar;

struct afc_dynamic_class_method_data
{
	char *name;
	DynamicClassMethod func;
	char *params;
};

typedef struct afc_dynamic_class_method_data DynamicClassMethodData;

#define AFC_DYNAMIC_CLASS_ARG_END (void *)0xdeadbeef

/* Function Prototypes */
#define afc_dynamic_class_delete(dc)   \
	if (dc)                            \
	{                                  \
		_afc_dynamic_class_delete(dc); \
		dc = NULL;                     \
	}
DynamicClass *afc_dynamic_class_new(void);
int _afc_dynamic_class_delete(DynamicClass *dc);
int afc_dynamic_class_clear(DynamicClass *dc);

int afc_dynamic_class_add_method(DynamicClass *dc, char *name, char *params, DynamicClassMethod func);
#define afc_dynamic_class_execute(dc, name, ...) _afc_dynamic_class_execute(dc, name, ##__VA_ARGS__, AFC_DYNAMIC_CLASS_ARG_END)
int _afc_dynamic_class_execute(DynamicClass *dc, const char *name, ...);
int afc_dynamic_class_execute_vars(DynamicClass *dc, const char *name, va_list args);
// DynamicClassMethodData * afc_dynamic_class_find_method ( DynamicClass * dc, char * name );
#define afc_dynamic_class_find_method(dc, name) (DynamicClassMethodData *)(dc ? afc_dictionary_get(dc->methods, name) : NULL)
int afc_dynamic_class_set_var(DynamicClass *dc, int kind, char *name, void *val);
void *afc_dynamic_class_get_var(DynamicClass *dc, char *name);
#endif
