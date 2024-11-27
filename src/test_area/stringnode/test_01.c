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
	StringList *sn = afc_string_list_new();
	char *str = afc_string_new(50);

	test_header();

	afc_string_list_add(sn, "hello", AFC_STRING_LIST_ADD_TAIL);
	afc_string_list_add(sn, "little", AFC_STRING_LIST_ADD_TAIL);
	afc_string_list_add(sn, "world", AFC_STRING_LIST_ADD_TAIL);

	print_res("afc_string_list_obj()", "world", afc_string_list_obj(sn), 1);
	print_res("afc_string_list_first()", "hello", afc_string_list_first(sn), 1);
	print_res("afc_string_list_last()", "world", afc_string_list_last(sn), 1);
	print_res("afc_string_list_next()", NULL, afc_string_list_next(sn), 1);

	print_row();

	afc_string_copy(str, "first/second/third/last", ALL);
	afc_string_list_split(sn, str, "/");

	print_res("afc_string_list_first()", "first", afc_string_list_first(sn), 1);
	print_res("afc_string_list_last()", "last", afc_string_list_last(sn), 1);

	print_row();

	afc_string_copy(str, "first/second/third\\/item/last", ALL);
	afc_string_list_set_tags(sn, AFC_STRING_LIST_TAG_ESCAPE_CHAR, '\\', AFC_TAG_END);
	afc_string_list_split(sn, str, "/");

	print_res("afc_string_list_first()", "first", afc_string_list_first(sn), 1);
	print_res("afc_string_list_last()", "last", afc_string_list_last(sn), 1);
	print_res("afc_string_list_prev()", "third\\/item", afc_string_list_prev(sn), 1);

	print_row();

	afc_string_copy(str, "/first/second/third/last", ALL);
	afc_string_list_split(sn, str, "/");

	print_res("afc_string_list_first()", "", afc_string_list_first(sn), 1);
	print_res("afc_string_list_next()", "first", afc_string_list_next(sn), 1);

	print_row();

	afc_string_copy(str, "\\/first/second/third/last\\/", ALL);
	afc_string_list_split(sn, str, "/");

	print_res("afc_string_list_first()", "\\/first", afc_string_list_first(sn), 1);
	print_res("afc_string_list_next()", "second", afc_string_list_next(sn), 1);
	print_res("afc_string_list_last()", "last\\/", afc_string_list_last(sn), 1);

	print_row();

	afc_string_copy(str, "\\/first\\/second\\/third/last\\/", ALL);
	afc_string_list_split(sn, str, "/ ");

	print_res("afc_string_list_first()", "\\/first\\/second\\/third", afc_string_list_first(sn), 1);
	print_res("afc_string_list_next()", "last\\/", afc_string_list_next(sn), 1);
	print_res("afc_string_list_last()", "last\\/", afc_string_list_last(sn), 1);

	afc_string_list_split(sn, ":1;80.1: :2;93.3:", ":");

	print_res("afc_string_list_first()", "1;80.1", afc_string_list_item(sn, 1), 1);
	print_res("afc_string_list_next()", "2;93.3", afc_string_list_item(sn, 3), 1);

	print_summary();

	afc_string_delete(str);
	afc_string_list_delete(sn);

	afc_delete(afc);

	return (0);
}
