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
	char *test_str = "1234567890";

	test_header();

	s = afc_string_new(5);

	afc_string_copy(s, test_str, 3);
	print_res("afc_string_copy[3]", "123", s, 1);

	afc_string_copy(s, test_str, 5);
	print_res("afc_string_copy[5]", "12345", s, 1);

	afc_string_copy(s, test_str, 10);
	print_res("afc_string_copy[10]", "12345", s, 1);

	afc_string_copy(s, test_str, 0);
	print_res("afc_string_copy[0]", "", s, 1);
	print_res("afc_string_len()", (void *)0, (void *)afc_string_len(s), 0);

	afc_string_copy(s, "123\t\n", ALL);
	afc_string_trim(s);
	print_res("afc_string_trim", "123", s, 1);

	afc_string_make(s, "%c 1%c%c", '\t', 13, 10);
	afc_string_trim(s);
	print_res("afc_string_trim", "1", s, 1);

	afc_string_add(s, "2345", ALL);
	print_res("afc_string_add", "12345", s, 1);

	afc_string_add(s, "67890", ALL);
	print_res("afc_string_add", "12345", s, 1);

	afc_string_copy(s, "12", ALL);
	afc_string_add(s, "3456", 2);
	print_res("afc_string_add", "1234", s, 1);

	afc_string_clear(s);
	print_res("afc_string_clear", "", s, 1);

	afc_string_add(s, "123", ALL);
	print_res("afc_string_add", "123", s, 1);

	afc_string_make(s, "%s", test_str);
	print_res("afc_string_make", "12345", s, 1);

	print_res("afc_string_len()", (void *)5, (void *)afc_string_len(s), 0);
	print_res("afc_string_max()", (void *)5, (void *)afc_string_max(s), 0);

	t = afc_string_dup(s);
	print_res("afc_string_dup[s]", s, t, 1);
	afc_string_delete(t);

	afc_string_copy(s, "hello", ALL);
	print_res("afc_string_upper", "HELLO", afc_string_upper(s), 1);
	print_res("afc_string_lower", "hello", afc_string_lower(s), 1);

	afc_string_delete(s);

	s = afc_string_new(50);

	afc_string_copy(s, "/tmp/pizza/planet", ALL);
	print_res("afc_string_dirname", "/tmp/pizza", t = afc_string_dirname(s), 1);
	afc_string_delete(t);

	afc_string_copy(s, "/tmp/pizza/planet", ALL);
	print_res("afc_string_basename", "planet", t = afc_string_basename(s), 1);
	afc_string_delete(t);

	s = afc_string_temp("/tmp/test");
	print_res("afc_string_temp", "abc", s, 1);

	t = afc_string_new(50);

	afc_string_copy(s, "test_this", ALL);
	afc_string_copy(t, "test_this out", ALL);
	print_res("afc_string_comp", (void *)0, (void *)afc_string_comp(t, s, 5), 0);
	print_res("afc_string_comp", (void *)-32, (void *)afc_string_comp(t, s, ALL), 0);
	/*

		print_res ( "afc_string_delete[t]", ( void * ) 0, ( void * ) t, 0 );
		print_res ( "afc_string_max[t]", ( void * ) 0, ( void * ) afc_string_max ( t ),0 );
		print_res ( "afc_string_len[t]", ( void * ) 0, ( void * ) afc_string_len ( t ),0 );

		afc_string_copy ( t, test_str, 3 );
		print_res ( "afc_string_copy[t,3]", NULL, t, 1);

		afc_string_delete ( s );

	*/
	afc_string_delete(s);

	print_summary();

	afc_delete(afc);

	return (0);
}
