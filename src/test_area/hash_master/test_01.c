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

#include "../../base.h"
#include "../../array.h"

#include "../test_utils.h"

int elements = 0;

Hash *hm;

int main()
{
	AFC *afc = afc_new();
	char *str = afc_string_new(10);
	char *s;
	int t;

	if (afc == NULL)
		return (1);

	test_header();

	hm = afc_hash_new();

	// Set 20 keys
	for (t = 0; t < 20; t++)
	{
		afc_string_make(str, "%d", t);
		afc_hash_add(hm, t, afc_string_dup(str));
	}

	// Retrieve those 20 keys
	for (t = 0; t < 20; t++)
	{
		afc_string_make(str, "%d", t);
		print_res("find()", str, afc_hash_find(hm, t), 1);
	}
	print_row();

	// Retrieve and free 10 keys
	for (t = 10; t < 20; t++)
	{
		afc_string_make(str, "%d", t);
		print_res("find()", str, s = afc_hash_find(hm, t), 1);

		if (s)
			afc_string_delete(s);

		afc_hash_del(hm);
	}

	// Retrieve all 20 keys (10-20 should be NULL)
	for (t = 10; t < 20; t++)
	{
		if (t < 10)
		{
			afc_string_make(str, "%d", t);
			print_res("find()", str, afc_hash_find(hm, t), 1);
		}
		else
			print_res("find()", NULL, afc_hash_find(hm, t), 1);
	}

	// Free all resources
	for (t = 0; t < 20; t++)
	{
		s = afc_hash_find(hm, t);
		if (s)
			afc_string_delete(s);
	}

	print_row();
	print_summary();

	afc_hash_delete(hm);
	afc_string_delete(str);

	afc_delete(afc);

	return (0);
}
