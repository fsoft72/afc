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

int elements = 0;

Hash *hm;

int main()
{
	AFC *afc = afc_new();

	if (afc == NULL)
		return (1);

	test_header();

	hm = afc_hash_new();

	afc_hash_add(hm, 1, (void *)1);
	afc_hash_add(hm, 2, (void *)2);
	afc_hash_add(hm, 3, (void *)3);
	afc_hash_add(hm, 4, (void *)4);

	afc_hash_find(hm, 1);
	afc_hash_del(hm);

	afc_hash_delete(hm);

	afc_delete(afc);

	return (0);
}
