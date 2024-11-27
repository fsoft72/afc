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
#include "../../array.h"

int clear_func(void *data)
{
	if (!data)
		return AFC_NO_ERR;

	printf("FREE: %p\n", data);
	afc_free(data);

	return AFC_NO_ERR;
}

int main()
{
	AFC *afc = afc_new();
	Array *am;
	int t;

	afc_track_mallocs(afc);
	afc_set_tags(afc, AFC_TAG_LOG_LEVEL, AFC_LOG_WARNING, AFC_TAG_END);

	am = afc_array_new();
	afc_array_set_clear_func(am, clear_func);

	for (t = 0; t < 10; t++)
		afc_array_add(am, afc_malloc(10), AFC_ARRAY_ADD_TAIL);

	for (t = 5; t < 10; t++)
	{
		afc_array_item(am, t);
		afc_array_del(am);
	}

	afc_array_delete(am);

	afc_delete(afc);

	return (0);
}
