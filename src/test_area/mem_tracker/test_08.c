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
#include "../../base.h"
#include "../../mem_tracker.h"
#include "../../hash_master.h"
#include "../../array.h"
#include "../../string.h"
#include "../../dictionary.h"

int _clean_func(void *mem)
{
	char *s = (char *)mem;
	afc_string_delete(s);

	return 0;
}

int main()
{
	AFC *afc = afc_new();
	ArrayMaster *d;
	int t;
	char buf[1024];

	afc_track_mallocs(afc);
	afc_set_tags(afc, AFC_TAG_LOG_LEVEL, AFC_LOG_WARNING,
				 AFC_TAG_SHOW_MALLOCS, TRUE,
				 /* AFC_TAG_SHOW_FREES,	TRUE, */
				 AFC_TAG_END);

	d = afc_array_new();
	afc_array_set_clear_func(d, _clean_func);

	for (t = 0; t < 100; t++)
	{
		if (!(t % 300))
			printf("Pos: %d\n", t);

		sprintf(buf, "key%d", t);
		afc_array_add_tail(d, afc_string_dup(buf));
	}

	// afc_array_clear ( d );

	afc_array_delete(d);
	afc_delete(afc);

	return (0);
}
