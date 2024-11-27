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

int dump_all(ArrayMaster *am, int c)
{
	int i = afc_array_len(am);
	int t;

	printf("--------------------------------------\n");
	printf("%d - Dumping: %d items\n\n", c, i);
	for (t = 0; t < i; t++)
		printf("Item: %d - %d\n", t, (int)afc_array_item(am, t));
	printf("--------------------------------------\n");

	return (AFC_ERR_NO_ERROR);
}

int main()
{
	AFC *afc = afc_new();
	ArrayMaster *am = afc_array_new();
	int t;

	for (t = 0; t < 10; t++)
	{
		if (!(t % 2))
			afc_array_add(am, (void *)t, AFC_ARRAY_ADD_TAIL);
		else
		{
			afc_array_item(am, 0);
			afc_array_del(am);
		}

		dump_all(am, t);
	}

	return (0);
}
