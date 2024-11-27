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
#include "../test_utils.h"

void dump_dir(FSTree *fst, char *path)
{
	ArrayMaster *am = afc_fstree_list(fst, path);
	FSTData *data;

	data = afc_array_first(am);
	while (data)
	{
		if (data->kind == AFC_FSTREE_KIND_FOLDER)
			printf("(dir) ");
		else
			printf("      ");

		printf("%s\n", data->name);

		data = afc_array_next(am);
	}
}

int main()
{
	AFC *afc = afc_new();
	FSTree *fst = afc_fstree_new();

	afc_fstree_set_tag(fst, AFC_FSTREE_TAG_ADD_DIRS, (void *)TRUE);

	afc_fstree_add(fst, "first", NULL);
	afc_fstree_add(fst, "second", NULL);
	afc_fstree_add(fst, "third", NULL);
	afc_fstree_add(fst, "/this/is/strange", NULL);

	afc_fstree_find(fst, "second");

	dump_dir(fst, "/");

	afc_fstree_delete(fst);
	afc_delete(afc);

	return (0);
}
