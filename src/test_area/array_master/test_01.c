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
#include <time.h>

#include "../test_utils.h"

int elements = 0;

Array *am;

int count = 0;

int clear_func(void *data)
{

	count++;

	if (!(count % 10))
		printf("Count: %d\n", count);

	// fprintf ( stderr, "Clear Func: %x\n", ( int ) data );
	afc_string_delete(data);

	return (AFC_ERR_NO_ERROR);
}

void add_element()
{
	char *s;

	// printf ( "Adding a new Element...\n ");

	s = afc_string_dup("1");
	// printf ( "MEM: %x\n", ( int ) s );

	afc_array_add(am, s, AFC_ARRAY_ADD_TAIL);
	elements++;

	// printf ( "Elements: %d - AM Elements: %lu\n", elements, afc_array_len ( am ) );
}

void del_element()
{
	int num;

	if (elements == 0)
		return;

	// num = ( random() % elements ) + 1;
	num = 2;

	printf("*** Deleting an Element...%d\n ", num);

	if (afc_array_item(am, num))
	{
		afc_array_del(am);
		elements--;

		printf("*** Elements: %d - AM Elements: %lu\n", elements, afc_array_len(am));
	}
}

int main()
{
	AFC *afc = afc_new();
	int t;
	int n;
	int i;

	srandom(time(NULL));

	if (afc == NULL)
		return (1);

	afc_track_mallocs(afc);
	afc_set_tags(afc, AFC_TAG_LOG_LEVEL, AFC_LOG_NOTICE,
				 AFC_TAG_SHOW_MALLOCS, FALSE,
				 AFC_TAG_SHOW_FREES, FALSE,
				 AFC_TAG_END);

	am = afc_array_new();
	afc_array_set_clear_func(am, clear_func);

	for (t = 0; t < 3; t++)
	{
		// n = ( int ) random ();
		n = 10;
		i = n % 2;

		if (i == 0)
			add_element();
		else
			del_element();
	}

	printf("************** EXIT\n");

	afc_malloc(100);

	afc_mem_tracker_dump_stats(afc->tracker, TRUE);

	afc_array_delete(am);

	afc_mem_tracker_dump_stats(afc->tracker, TRUE);
	afc_delete(afc);

	return (0);
}
