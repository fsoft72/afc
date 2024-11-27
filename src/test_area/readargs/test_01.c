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

void simple_01(ReadArgs *rdarg)
{
	afc_readargs_parse(rdarg, "NAME/A	SURNAME/A AGE/N", "John Smith 35");
	print_res("afc_readargs_get_by_pos[0]", "John", afc_readargs_get_by_pos(rdarg, 0), 1);
	print_res("afc_readargs_get_by_pos[1]", "Smith", afc_readargs_get_by_pos(rdarg, 1), 1);
	print_res("afc_readargs_get_by_pos[2]", (void *)35, (void *)(int)afc_readargs_get_by_pos(rdarg, 2), 0);
}

void simple_02(ReadArgs *rdarg)
{
	print_row();

	afc_readargs_parse(rdarg, "NAME/A,SURNAME/A,AGE/N", "SURNAME Smith AGE=35 John");

	print_res("afc_readargs_get_by_name[name]", "John", afc_readargs_get_by_name(rdarg, "NAME"), 1);
	print_res("afc_readargs_get_by_name[surname]", "Smith", afc_readargs_get_by_name(rdarg, "SURNAME"), 1);
	print_res("afc_readargs_get_by_name[age]", (void *)35, (void *)(int)afc_readargs_get_by_name(rdarg, "AGE"), 0);
}

void simple_03(ReadArgs *rdarg)
{
	print_row();

	afc_readargs_parse(rdarg, "NAME/A,AGE/N,MALE/S,FEMALE/S", "John MALE 35");

	print_res("afc_readargs_get_by_pos[0]", "John", afc_readargs_get_by_pos(rdarg, 0), 1);
	print_res("afc_readargs_get_by_pos[1]", (void *)35, (void *)(int)afc_readargs_get_by_pos(rdarg, 1), 0);
	print_res("afc_readargs_get_by_pos[2]", (void *)TRUE, (void *)(int)afc_readargs_get_by_pos(rdarg, 2), 0);
	print_res("afc_readargs_get_by_pos[3]", (void *)FALSE, (void *)(int)afc_readargs_get_by_pos(rdarg, 3), 0);
}

void quote_01(ReadArgs *rdarg)
{
	print_row();

	afc_readargs_parse(rdarg, "NAME/A,SURNAME/A,AGE/N", "John \"K B Smith\" 35");

	print_res("afc_readargs_get_by_pos[0]", "John", afc_readargs_get_by_pos(rdarg, 0), 1);
	print_res("afc_readargs_get_by_pos[1]", "K B Smith", afc_readargs_get_by_pos(rdarg, 1), 1);
	print_res("afc_readargs_get_by_pos[2]", (void *)35, (void *)(int)afc_readargs_get_by_pos(rdarg, 2), 0);
}

void quote_02(ReadArgs *rdarg)
{
	// Same test as quote_01, but using tags here and there
	print_row();

	afc_readargs_parse(rdarg, "NAME/A,SURNAME/A,AGE/N", "John	\"K	B	Smith\"	35");

	print_res("afc_readargs_get_by_pos[0]", "John", afc_readargs_get_by_pos(rdarg, 0), 1);
	print_res("afc_readargs_get_by_pos[1]", "K	B	Smith", afc_readargs_get_by_pos(rdarg, 1), 1);
	print_res("afc_readargs_get_by_pos[2]", (void *)35, (void *)(int)afc_readargs_get_by_pos(rdarg, 2), 0);
}

void multi_01(ReadArgs *rdarg)
{
	NodeMaster *nm;
	char *s;

	print_row();
	afc_readargs_parse(rdarg, "NAME/A,AGE/N,VALS/M", "John 35 val01 val02 val03");

	print_res("afc_readargs_get_by_pos[0]", "John", afc_readargs_get_by_pos(rdarg, 0), 1);
	print_res("afc_readargs_get_by_pos[1]", (void *)35, (void *)(int)afc_readargs_get_by_pos(rdarg, 1), 0);

	nm = afc_readargs_get_by_pos(rdarg, 2);

	s = afc_nodemaster_first(nm);
	print_res("val: 01", "val01", s, 1);

	s = afc_nodemaster_next(nm);
	print_res("val: 02", "val02", s, 1);

	s = afc_nodemaster_next(nm);
	print_res("val: 03", "val03", s, 1);
}

void multi_02(ReadArgs *rdarg)
{
	NodeMaster *nm;
	char *s;

	print_row();
	afc_readargs_parse(rdarg, "NAME/A,AGE/N,VALS/M", "John val01 AGE=35 val02 val03");

	print_res("afc_readargs_get_by_pos[0]", "John", afc_readargs_get_by_pos(rdarg, 0), 1);
	print_res("afc_readargs_get_by_pos[1]", (void *)35, (void *)(int)afc_readargs_get_by_pos(rdarg, 1), 0);

	nm = afc_readargs_get_by_pos(rdarg, 2);

	s = afc_nodemaster_first(nm);
	print_res("val: 01", "val01", s, 1);

	s = afc_nodemaster_next(nm);
	print_res("val: 02", "val02", s, 1);

	s = afc_nodemaster_next(nm);
	print_res("val: 03", "val03", s, 1);
}

void multi_03(ReadArgs *rdarg)
{
	NodeMaster *nm;
	char *s;

	print_row();
	afc_readargs_parse(rdarg, "NAME/A,AGE/N,VALS/M", "AGE=35 John val01 val02 val03");

	print_res("afc_readargs_get_by_pos[0]", "John", afc_readargs_get_by_pos(rdarg, 0), 1);
	print_res("afc_readargs_get_by_pos[1]", (void *)35, (void *)(int)afc_readargs_get_by_pos(rdarg, 1), 0);

	nm = afc_readargs_get_by_pos(rdarg, 2);

	s = afc_nodemaster_first(nm);
	print_res("val: 01", "val01", s, 1);

	s = afc_nodemaster_next(nm);
	print_res("val: 02", "val02", s, 1);

	s = afc_nodemaster_next(nm);
	print_res("val: 03", "val03", s, 1);
}

void complex_01(ReadArgs *rdarg)
{
	NodeMaster *nm;
	char *s;

	print_row();
	afc_readargs_parse(rdarg, "EXT/S,INT/S,COMMAND/A,ARGS/M", "EXT cmd val01 val02 val03");

	print_res("afc_readargs_get_by_pos[0]", (void *)TRUE, (void *)(int)afc_readargs_get_by_pos(rdarg, 0), 0);
	print_res("afc_readargs_get_by_pos[1]", (void *)FALSE, (void *)(int)afc_readargs_get_by_pos(rdarg, 1), 0);
	print_res("afc_readargs_get_by_pos[2]", "cmd", afc_readargs_get_by_pos(rdarg, 2), 1);

	nm = afc_readargs_get_by_pos(rdarg, 3);

	s = afc_nodemaster_first(nm);
	print_res("val: 01", "val01", s, 1);

	s = afc_nodemaster_next(nm);
	print_res("val: 02", "val02", s, 1);

	s = afc_nodemaster_next(nm);
	print_res("val: 03", "val03", s, 1);
}

void complex_02(ReadArgs *rdarg)
{
	NodeMaster *nm;
	char *s;

	print_row();
	afc_readargs_parse(rdarg, "EXT/S,INT/S,COMMAND/A,AGE/N,MALE/S,ARGS/M,FEMALE/S", "EXT val01 COMMAND=cmd val02 val03 MALE AGE=35");

	print_res("afc_readargs_get_by_pos[0]", (void *)TRUE, (void *)(int)afc_readargs_get_by_pos(rdarg, 0), 0);
	print_res("afc_readargs_get_by_pos[1]", (void *)FALSE, (void *)(int)afc_readargs_get_by_pos(rdarg, 1), 0);
	print_res("afc_readargs_get_by_pos[2]", "cmd", afc_readargs_get_by_pos(rdarg, 2), 1);
	print_res("afc_readargs_get_by_pos[3]", (void *)35, (void *)(int)afc_readargs_get_by_pos(rdarg, 3), 0);
	print_res("afc_readargs_get_by_pos[4]", (void *)TRUE, (void *)(int)afc_readargs_get_by_pos(rdarg, 4), 0);
	print_res("afc_readargs_get_by_pos[6]", (void *)FALSE, (void *)(int)afc_readargs_get_by_pos(rdarg, 6), 0);

	nm = afc_readargs_get_by_pos(rdarg, 5);

	s = afc_nodemaster_first(nm);
	print_res("val: 01", "val01", s, 1);

	s = afc_nodemaster_next(nm);
	print_res("val: 02", "val02", s, 1);

	s = afc_nodemaster_next(nm);
	print_res("val: 03", "val03", s, 1);
}

void complex_03(ReadArgs *rdarg)
{
	NodeMaster *nm;
	char *s;

	print_row();
	afc_readargs_parse(rdarg, "EXT/S,INT/S,COMMAND/A,AGE/N,MALE/S,ARGS/M,FEMALE/S", "EXT COMMAND=cmd MALE AGE=35");

	print_res("afc_readargs_get_by_pos[0]", (void *)TRUE, (void *)(int)afc_readargs_get_by_pos(rdarg, 0), 0);
	print_res("afc_readargs_get_by_pos[1]", (void *)FALSE, (void *)(int)afc_readargs_get_by_pos(rdarg, 1), 0);
	print_res("afc_readargs_get_by_pos[2]", "cmd", afc_readargs_get_by_pos(rdarg, 2), 1);
	print_res("afc_readargs_get_by_pos[3]", (void *)35, (void *)(int)afc_readargs_get_by_pos(rdarg, 3), 0);
	print_res("afc_readargs_get_by_pos[4]", (void *)TRUE, (void *)(int)afc_readargs_get_by_pos(rdarg, 4), 0);
	print_res("afc_readargs_get_by_pos[6]", (void *)FALSE, (void *)(int)afc_readargs_get_by_pos(rdarg, 6), 0);

	if ((nm = afc_readargs_get_by_pos(rdarg, 5)))
	{
		s = afc_nodemaster_first(nm);
		print_res("val: 01", NULL, s, 1);

		s = afc_nodemaster_next(nm);
		print_res("val: 02", NULL, s, 1);

		s = afc_nodemaster_next(nm);
		print_res("val: 03", NULL, s, 1);
	}
}

void complex_04(ReadArgs *rdarg)
{
	NodeMaster *nm;
	char *s;

	print_row();
	afc_readargs_parse(rdarg, "EXT/S,INT/S,COMMAND/A,AGE/N,MALE/S,ARGS/M,FEMALE/S", "EXT {fab} MALE AGE=35\n\n\t\n");

	print_res("afc_readargs_get_by_pos[0]", (void *)TRUE, (void *)(int)afc_readargs_get_by_pos(rdarg, 0), 0);
	print_res("afc_readargs_get_by_pos[1]", (void *)FALSE, (void *)(int)afc_readargs_get_by_pos(rdarg, 1), 0);
	print_res("afc_readargs_get_by_pos[2]", "{fab}", afc_readargs_get_by_pos(rdarg, 2), 1);
	print_res("afc_readargs_get_by_pos[3]", (void *)35, (void *)(int)afc_readargs_get_by_pos(rdarg, 3), 0);
	print_res("afc_readargs_get_by_pos[4]", (void *)TRUE, (void *)(int)afc_readargs_get_by_pos(rdarg, 4), 0);
	print_res("afc_readargs_get_by_pos[6]", (void *)FALSE, (void *)(int)afc_readargs_get_by_pos(rdarg, 6), 0);

	if ((nm = afc_readargs_get_by_pos(rdarg, 5)))
	{
		s = afc_nodemaster_first(nm);
		print_res("val: 01", NULL, s, 1);

		s = afc_nodemaster_next(nm);
		print_res("val: 02", NULL, s, 1);

		s = afc_nodemaster_next(nm);
		print_res("val: 03", NULL, s, 1);
	}
}

void wrong_01(ReadArgs *rdarg)
{
	print_row();

	afc_readargs_parse(rdarg, "ARG1,ARG2,ARG3", "");

	print_res("afc_readargs_get_by_pos[0]", NULL, afc_readargs_get_by_pos(rdarg, 0), 1);
}

void wrong_02(ReadArgs *rdarg)
{
	print_row();

	afc_readargs_parse(rdarg, "ARG1,ARG2,ARG3", NULL);

	print_res("afc_readargs_get_by_pos[0]", NULL, afc_readargs_get_by_pos(rdarg, 0), 1);
}

void wrong_03(ReadArgs *rdarg)
{
	print_row();

	afc_readargs_parse(rdarg, "", "a b c");

	print_res("afc_readargs_get_by_pos[0]", NULL, afc_readargs_get_by_pos(rdarg, 0), 1);
}

void keyword_01(ReadArgs *rdarg)
{
	print_row();

	afc_readargs_parse(rdarg, "NAME/A SURNAME/K", "John Smith");

	print_res("afc_readargs_get_by_pos[0]", "John", afc_readargs_get_by_pos(rdarg, 0), 1);
	print_res("afc_readargs_get_by_pos[1]", NULL, afc_readargs_get_by_pos(rdarg, 1), 1);
}

void keyword_02(ReadArgs *rdarg)
{
	print_row();

	afc_readargs_parse(rdarg, "NAME/A SURNAME/K", "John SURNAME Smith");

	print_res("afc_readargs_get_by_pos[0]", "John", afc_readargs_get_by_pos(rdarg, 0), 1);
	print_res("afc_readargs_get_by_pos[1]", "Smith", afc_readargs_get_by_pos(rdarg, 1), 1);
}

int main()
{
	AFC *afc = afc_new();
	ReadArgs *rdarg = afc_readargs_new();

	test_header();

	simple_01(rdarg);
	simple_02(rdarg);
	simple_03(rdarg);

	quote_01(rdarg);
	quote_02(rdarg);

	multi_01(rdarg);
	multi_02(rdarg);
	multi_03(rdarg);

	complex_01(rdarg);
	complex_02(rdarg);
	complex_03(rdarg);
	complex_04(rdarg);

	wrong_01(rdarg);
	wrong_02(rdarg);
	wrong_03(rdarg);

	keyword_01(rdarg);
	keyword_02(rdarg);

	// print_res ( "afc_string_copy[3]", "123", s, 1 );

	print_summary();

	afc_readargs_delete(rdarg);
	afc_delete(afc);

	exit(0);
}
