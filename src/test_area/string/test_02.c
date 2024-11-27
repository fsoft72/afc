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
	char *s, *t;

	s = afc_string_new(15);

	afc_string_copy(s, "ciao", ALL);

	afc_string_delete(s);

	s = afc_string_dup("hello world");

	t = afc_string_dup(s);

	afc_string_delete(s);
	afc_string_delete(t);
	afc_delete(afc);

	return (0);
}
