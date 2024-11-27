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

struct iinfo
{
	int size;
	char *perm;
	char *uname;
	char *gname;
};

typedef struct iinfo IInfo;

int import_list(FSTree *fst, FILE *fh, char *buf)
{
	StringNode *sn = afc_stringnode_new();
	char *name;
	char *x;
	IInfo *info;
	char b[255];

	if (sn == NULL)
		return (AFC_ERR_NO_MEMORY);

	afc_stringnode_set_tags(sn, AFC_STRINGNODE_TAG_DISCARD_ZERO_LEN, TRUE, AFC_TAG_END);

	while (afc_string_fget(buf, fh))
	{
		afc_string_trim(buf);

		afc_stringnode_split(sn, buf, " ");

		info = afc_malloc(sizeof(IInfo));

		name = afc_stringnode_item(sn, 8);
		info->size = atoi(afc_stringnode_item(sn, 3));
		info->perm = afc_string_dup(afc_stringnode_item(sn, 0));

		// afc_fstree_cd ( fst, "/" );

		sprintf(b, "/%s", name);

		if (info->perm[0] == 'd')
			afc_fstree_mkdir(fst, b);
		else
			afc_fstree_add(fst, b, info);
	}

	afc_stringnode_delete(sn);

	return (AFC_ERR_NO_ERROR);
}

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
	char *buf = afc_string_new(1024);
	FILE *fh = fopen("./input_list.3", "r");

	afc_fstree_set_tag(fst, AFC_FSTREE_TAG_ADD_DIRS, (void *)TRUE);

	import_list(fst, fh, buf);

	if (fh)
		fclose(fh);

	afc_fstree_internal_recursive_dump(fst, fst->head, 0);

	dump_dir(fst, "/");

	afc_string_delete(buf);
	afc_fstree_delete(fst);
	afc_delete(afc);

	return (0);
}
