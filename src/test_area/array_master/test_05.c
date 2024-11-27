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
	int t;

	afc_array_before_first(am);

	while ((t = (int)afc_array_next(am)))
		printf("Val: %d\n", t);

	return (AFC_ERR_NO_ERROR);
}

int main()
{
	AFC *afc = afc_new();
	ArrayMaster *am = afc_array_new();
	int t;

	for (t = 1; t <= 10; t++)
		afc_array_add(am, (void *)t, AFC_ARRAY_ADD_TAIL);

	dump_all(am, t);

	afc_delete(afc);

	return (0);
}
