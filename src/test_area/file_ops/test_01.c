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

int create_file(char *name)
{
	FILE *fh;

	fh = fopen(name, "w");

	if (fh == NULL)
		return (-1);

	fclose(fh);

	return (0);
}

int main()
{
	AFC *afc = afc_new();
	FileOperations *fo;

	if (afc == NULL)
		return (1);

	fo = afc_fileops_new();

	test_header();

	print_res("mkdir(\"/tmp/fo_tst\")", AFC_ERR_NO_ERROR, (void *)afc_fileops_mkdir(fo, "/tmp/fo_tst"), 0);
	print_res("creating empty file", 0, (void *)create_file("/tmp/fo_tst/empty"), 0);
	print_res("mkdir(\"/tmp/fo_tst/dir1\")", AFC_ERR_NO_ERROR, (void *)afc_fileops_mkdir(fo, "/tmp/fo_tst/dir1"), 0);
	print_res("creating base file", 0, (void *)create_file("/tmp/fo_tst/dir1/base"), 0);
	print_res("link(\"base\", \"link1\")", 0, (void *)afc_fileops_link(fo, "/tmp/fo_tst/dir1/base", "/tmp/fo_tst/dir1/link1"), 0);
	print_res("copy(\"link1\", \"link2\")", 0, (void *)afc_fileops_copy(fo, "/tmp/fo_tst/dir1/link1", "/tmp/fo_tst/dir1/link2"), 0);
	print_res("copy(\"dir1\", \"dir2\")", 0, (void *)afc_fileops_copy(fo, "/tmp/fo_tst/dir1", "/tmp/fo_tst/dir2"), 0);
	print_res("move(\"link1\", \"link0\")", 0, (void *)afc_fileops_move(fo, "/tmp/fo_tst/dir1/link1", "/tmp/fo_tst/dir1/link0"), 0);
	print_res("del(\"/tmp/fo_tst\")", 0, (void *)afc_fileops_del(fo, "/tmp/fo_tst"), 0);

	// print_res ( "find()", str, s = afc_hash_find ( hm, t ), 1 );

	print_row();
	print_summary();

	afc_fileops_delete(fo);

	afc_delete(afc);

	return (0);
}
