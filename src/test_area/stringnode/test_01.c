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
	StringNode *sn = afc_stringnode_new();
	char *str = afc_string_new(50);

	test_header();

	afc_stringnode_add(sn, "hello", AFC_STRINGNODE_ADD_TAIL);
	afc_stringnode_add(sn, "little", AFC_STRINGNODE_ADD_TAIL);
	afc_stringnode_add(sn, "world", AFC_STRINGNODE_ADD_TAIL);

	print_res("afc_stringnode_obj()", "world", afc_stringnode_obj(sn), 1);
	print_res("afc_stringnode_first()", "hello", afc_stringnode_first(sn), 1);
	print_res("afc_stringnode_last()", "world", afc_stringnode_last(sn), 1);
	print_res("afc_stringnode_next()", NULL, afc_stringnode_next(sn), 1);

	print_row();

	afc_string_copy(str, "first/second/third/last", ALL);
	afc_stringnode_split(sn, str, "/");

	print_res("afc_stringnode_first()", "first", afc_stringnode_first(sn), 1);
	print_res("afc_stringnode_last()", "last", afc_stringnode_last(sn), 1);

	print_row();

	afc_string_copy(str, "first/second/third\\/item/last", ALL);
	afc_stringnode_set_tags(sn, AFC_STRINGNODE_TAG_ESCAPE_CHAR, '\\', AFC_TAG_END);
	afc_stringnode_split(sn, str, "/");

	print_res("afc_stringnode_first()", "first", afc_stringnode_first(sn), 1);
	print_res("afc_stringnode_last()", "last", afc_stringnode_last(sn), 1);
	print_res("afc_stringnode_prev()", "third\\/item", afc_stringnode_prev(sn), 1);

	print_row();

	afc_string_copy(str, "/first/second/third/last", ALL);
	afc_stringnode_split(sn, str, "/");

	print_res("afc_stringnode_first()", "", afc_stringnode_first(sn), 1);
	print_res("afc_stringnode_next()", "first", afc_stringnode_next(sn), 1);

	print_row();

	afc_string_copy(str, "\\/first/second/third/last\\/", ALL);
	afc_stringnode_split(sn, str, "/");

	print_res("afc_stringnode_first()", "\\/first", afc_stringnode_first(sn), 1);
	print_res("afc_stringnode_next()", "second", afc_stringnode_next(sn), 1);
	print_res("afc_stringnode_last()", "last\\/", afc_stringnode_last(sn), 1);

	print_row();

	afc_string_copy(str, "\\/first\\/second\\/third/last\\/", ALL);
	afc_stringnode_split(sn, str, "/ ");

	print_res("afc_stringnode_first()", "\\/first\\/second\\/third", afc_stringnode_first(sn), 1);
	print_res("afc_stringnode_next()", "last\\/", afc_stringnode_next(sn), 1);
	print_res("afc_stringnode_last()", "last\\/", afc_stringnode_last(sn), 1);

	afc_stringnode_split(sn, ":1;80.1: :2;93.3:", ":");

	print_res("afc_stringnode_first()", "1;80.1", afc_stringnode_item(sn, 1), 1);
	print_res("afc_stringnode_next()", "2;93.3", afc_stringnode_item(sn, 3), 1);

	print_summary();

	afc_string_delete(str);
	afc_stringnode_delete(sn);

	afc_delete(afc);

	return (0);
}
