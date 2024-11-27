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

int my_sort(const void *v1, const void *v2)
{
	char *c1 = AFC_ARRAY_SORT_ELEMENT(char *, v1);
	char *c2 = AFC_ARRAY_SORT_ELEMENT(char *, v2);

	return (strcmp(c1, c2));
}

#define ITEMS 2

int dump_all(ArrayMaster *am)
{
	int i = afc_array_len(am);
	int t;

	printf("Dumping: %d items \n\n", i);
	for (t = 0; t < i; t++)
		printf("Item: %d - %s\n", t, (char *)afc_array_item(am, t));

	printf("--------------------------------------\n");

	return (AFC_ERR_NO_ERROR);
}

int dump_list(ArrayMaster *am)
{
	char *s;
	int i = 0;

	printf("List dumping ... \n");
	s = afc_array_first(am);
	while (s)
	{
		printf("Item: %d - %s\n", i++, s);
		s = afc_array_next(am);
	}

	printf("--------------------------------------\n");
	return (AFC_ERR_NO_ERROR);
}

int dump_list_reverse(ArrayMaster *am)
{
	char *s;
	int i = 0;

	printf("REVERSE List dumping...\n");
	s = afc_array_last(am);
	while (s)
	{
		printf("Item: %d - %s\n", i++, s);
		s = afc_array_prev(am);
	}
	printf("--------------------------------------\n");
	return (AFC_ERR_NO_ERROR);
}

int main()
{
	AFC *afc = afc_new();
	ArrayMaster *am = afc_array_new();
	int t = 0;
	char *buf;

	afc_array_init(am, ITEMS + 1);

	printf("Creating %d elements...\n", ITEMS);
	for (t = ITEMS; t > 0; t--)
	{
		buf = (char *)afc_malloc(15);
		sprintf(buf, "%4.4d", t);
		afc_array_add(am, buf, AFC_ARRAY_ADD_TAIL);
	}
	printf("     DONE!\n");

	printf("Sorting... \n");
	afc_array_sort(am, my_sort);
	printf("     DONE!\n");

	dump_all(am);

	for (t = 0; t < ITEMS; t++)
	{
		buf = afc_array_item(am, t);
		afc_free(buf);
	}

	afc_array_clear(am);

	dump_all(am);

	afc_array_add(am, afc_string_dup("first"), AFC_ARRAY_ADD_TAIL);
	afc_array_add(am, afc_string_dup("second"), AFC_ARRAY_ADD_TAIL);
	afc_array_add(am, afc_string_dup("third"), AFC_ARRAY_ADD_TAIL);
	afc_array_add(am, afc_string_dup("last"), AFC_ARRAY_ADD_TAIL);

	dump_list(am);

	printf("Delete last item...\n");
	afc_array_del(am); // Delete the last element

	dump_list(am);

	printf("Delete first item...\n");
	afc_array_first(am);
	afc_array_del(am); // Delete the first element
	dump_list(am);

	printf("Delete succ element...\n");
	afc_array_next(am);
	afc_array_del(am);
	dump_list(am);

	printf("Clearing all the Array...\n");
	afc_array_clear(am);
	dump_list(am);

	printf("Adding all 4 elements again...\n");
	afc_array_add(am, afc_string_dup("first"), AFC_ARRAY_ADD_TAIL);
	afc_array_add(am, afc_string_dup("second"), AFC_ARRAY_ADD_TAIL);
	afc_array_add(am, afc_string_dup("third"), AFC_ARRAY_ADD_TAIL);
	afc_array_add(am, afc_string_dup("last"), AFC_ARRAY_ADD_TAIL);
	dump_list(am);

	dump_list_reverse(am);

	printf("Deleting item: \"first\"...\n");
	afc_array_del(am);
	dump_list_reverse(am);

	afc_array_delete(am);

	afc_delete(afc);

	return (0);
}
