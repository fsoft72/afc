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
#ifndef AFC_DYNAMIC_CLASS_MASTER_H
#define AFC_DYNAMIC_CLASS_MASTER_H
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <dlfcn.h>

#include "base.h"
#include "hash_master.h"
#include "dictionary.h"
#include "dynamic_class.h"

/* DynamicClassMaster'Magic' value: 'DYNA' */
#define AFC_DYNAMIC_CLASS_MASTER_MAGIC ('D' << 24 | 'Y' << 16 | 'C' << 8 | 'M')

/* DynamicClassMaster Base  */
#define AFC_DYNAMIC_CLASS_MASTER_BASE 0xD000

enum
{
	AFC_DYNAMIC_CLASS_MASTER_ERR_DLSYM = AFC_DYNAMIC_CLASS_MASTER_BASE + 1,
	AFC_DYNAMIC_CLASS_MASTER_ERR_DLOPEN,
	AFC_DYNAMIC_CLASS_MASTER_ERR_INSTANCE,
	AFC_DYNAMIC_CLASS_MASTER_ERR_INVALID_INSTANCE,
	AFC_DYNAMIC_CLASS_MASTER_ERR_DUPLICATE_NAME,
	AFC_DYNAMIC_CLASS_MASTER_ERR_CLASS_NOT_FOUND
};

// TAGS
enum
{
	AFC_DYNAMIC_CLASS_MASTER_TAG_INFO = AFC_DYNAMIC_CLASS_MASTER_BASE + 1,
	AFC_DYNAMIC_CLASS_MASTER_TAG_CHECK_PARAMS
};

/* INFO Definitions */
enum
{
	AFC_DYNAMIC_CLASS_MASTER_INFO_NAME = AFC_DYNAMIC_CLASS_MASTER_BASE + 1, // Name of the Dynamic Class
	AFC_DYNAMIC_CLASS_MASTER_INFO_VERSION,									// DClass version, eg. 1.00
	AFC_DYNAMIC_CLASS_MASTER_INFO_AUTHOR,									// Author of the DClass
	AFC_DYNAMIC_CLASS_MASTER_INFO_EMAIL,									// Author's e-mail
	AFC_DYNAMIC_CLASS_MASTER_INFO_URL,										// DClass reference URL
	AFC_DYNAMIC_CLASS_MASTER_INFO_DESCR,									// DClass description
	AFC_DYNAMIC_CLASS_MASTER_INFO_DESCR_SHORT								// Short description
};

struct afc_dynamic_class_master
{
	unsigned long magic; /* DynamicClassMaster Magic Value */
	Dictionary *classes;
	HashMaster *instances;

	BOOL check_params; // Flag T/F. If T, plugins will perform params checking before execute()

	void *info; // Generic Info (it will be assigned to any new instance)
};

typedef struct afc_dynamic_class_master DynamicClassMaster;

struct afc_dynamic_class_master_data
{
	void *dl_handler; // dlopen() handler

	DynamicClass *(*new_instance)(void); // New Instance function
	int (*del_instance)(DynamicClass *); // Delete Instance function

	// Optional functions

	char *(*get_info)(int info_id); // Gets some info from the class. See AFC_DYNAMIC_CLASS_MASTER_INFO_*
};

typedef struct afc_dynamic_class_master_data DCMData;

struct afc_dynamic_class_master_instance_data
{
	DCMData *data;
	DynamicClass *instance;
	char *class_name;
};

typedef struct afc_dynamic_class_master_instance_data DCMIData;

/* Function Prototypes */
#define afc_dynamic_class_master_delete(dcm)   \
	if (dcm)                                   \
	{                                          \
		_afc_dynamic_class_master_delete(dcm); \
		dcm = NULL;                            \
	}
DynamicClassMaster *afc_dynamic_class_master_new(void);
int _afc_dynamic_class_master_delete(DynamicClassMaster *dcm);
int afc_dynamic_class_master_clear(DynamicClassMaster *dcm);
char *afc_dynamic_class_master_get_info(DynamicClassMaster *dcm, char *class_name, int info_id);

int afc_dynamic_class_master_load(DynamicClassMaster *dcm, const char *class_name, const char *file_name);
int afc_dynamic_class_master_add(DynamicClassMaster *dcm, const char *class_name, void *handler, DynamicClass *(*new_inst)(void), int (*del_inst)(DynamicClass *), char *(*info)(int));
DynamicClass *afc_dynamic_class_master_new_instance(DynamicClassMaster *dcm, const char *class_name);
int afc_dynamic_class_master_delete_instance(DynamicClassMaster *dcm, DynamicClass *instance);
int afc_dynamic_class_master_set_tag(DynamicClassMaster *dcm, int tag, void *val);
#define afc_dynamic_class_master_set_tags(dcm, first_tag, ...) _afc_dynamic_class_master_set_tags(dcm, first_tag, ##__VA_ARGS__, AFC_TAG_END)
int _afc_dynamic_class_master_set_tags(DynamicClassMaster *dcm, int first_tag, ...);
int afc_dynamic_class_master_has_class(DynamicClassMaster *dcm, const char *class_name);
#endif
