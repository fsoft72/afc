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

int main()
{
	AFC *afc = afc_new();
	char buf[1024];
	char *res;
	unsigned char *x;

	sprintf(buf, "cos%c%cXXXX", 0xc3, 0xac);
	res = afc_string_utf8_to_latin1(buf);

	x = res;
	while (*x)
	{
		printf("%x ", *x++);
	}

	printf(res);

	afc_string_delete(res);

	afc_delete(afc);

	return (0);
}
