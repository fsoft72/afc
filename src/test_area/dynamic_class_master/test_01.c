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
#include <stdlib.h>

#include "../test_utils.h"

// Prototypes
DynamicClass *class1_new_instance();
int class1_del_instance(DynamicClass *dc);
int class1_test(DynamicClass *dc);

DynamicClass *class2_new_instance();
int class2_del_instance(DynamicClass *dc);
int class2_test(DynamicClass *dc);

int main()
{
	AFC *afc = afc_new();
	DynamicClassMaster *dcm;
	DynamicClass *dc1, *dc2;

	if (afc == NULL)
		return (1);

	test_header();

	dcm = afc_dynamic_class_master_new();

	afc_dynamic_class_master_add(dcm, "class1", NULL, class1_new_instance, class1_del_instance, NULL);
	afc_dynamic_class_master_add(dcm, "class2", NULL, class2_new_instance, class2_del_instance, NULL);

	dc1 = afc_dynamic_class_master_new_instance(dcm, "class1");
	afc_dynamic_class_execute(dc1, "test", AFC_DYNAMIC_CLASS_ARG_END);

	afc_dynamic_class_set_var(dc1, AFC_DYNAMIC_CLASS_VAR_KIND_STRING, "ciao", "Hello");

	dc2 = afc_dynamic_class_master_new_instance(dcm, "class2");
	afc_dynamic_class_execute(dc2, "test", AFC_DYNAMIC_CLASS_ARG_END);

	afc_dynamic_class_master_delete_instance(dcm, dc2);
	afc_dynamic_class_master_delete_instance(dcm, dc1);

	afc_dynamic_class_master_delete(dcm);
	afc_delete(afc);

	return (0);
}

/* INTERNAL DYNAMIC CLASSES */

// CLASS1
DynamicClass *class1_new_instance()
{
	DynamicClass *dc = afc_dynamic_class_new();

	afc_dynamic_class_add_method(dc, "test", class1_test);

	return (dc);
}

int class1_del_instance(DynamicClass *dc)
{
	afc_dynamic_class_delete(dc);

	return (AFC_ERR_NO_ERROR);
}

int class1_test(DynamicClass *dc)
{
	printf("Class1 Test!\n");

	return (AFC_ERR_NO_ERROR);
}

// CLASS2
DynamicClass *class2_new_instance()
{
	DynamicClass *dc = afc_dynamic_class_new();

	afc_dynamic_class_add_method(dc, "test", class2_test);

	return (dc);
}

int class2_del_instance(DynamicClass *dc)
{
	afc_dynamic_class_delete(dc);

	return (AFC_ERR_NO_ERROR);
}

int class2_test(DynamicClass *dc)
{
	printf("class2 Test!\n");

	return (AFC_ERR_NO_ERROR);
}
