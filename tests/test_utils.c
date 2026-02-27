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
#include "test_utils.h"

static int total_test = 0;
static int total_ok = 0;
static int total_ko = 0;

void test_header(void)
{
	printf("| ACTION                         | EXPECTED         | RESULT\n");
	printf("+--------------------------------+------------------+------------------------------------------\n");
}

void print_row(void)
{
	printf("+--------------------------------+------------------+------------------------------------------\n");
}

void print_summary(void)
{
	print_row();
	printf("| Total Tests:                   | %5.5d            |\n", total_test);
	printf("| Total OK:                      | %5.5d            |\n", total_ok);
	printf("| Total Failed:                  | %5.5d            |\n", total_ko);
	print_row();
}

int get_test_failures(void)
{
	return total_ko;
}

void print_res(char *action, void *expected, void *result, int mode)
{
	total_test++;

	if (mode == 0)
	{
		printf("| %30.30s | %16d | %d", action, (int)(long)expected, (int)(long)result);

		if ((int)(long)expected == (int)(long)result)
		{
			printf("%10.10s\n", "OK");
			total_ok++;
		}
		else
		{
			printf("%10.10s\n", "FAILED");
			total_ko++;
		}
	}
	else
	{
		const char *exp_str = expected ? (const char *)expected : "(null)";
		const char *res_str = result ? (const char *)result : "(null)";

		printf("| %30.30s | %16.16s | %s", action, exp_str, res_str);

		if (strcmp(exp_str, res_str) == 0)
		{
			printf("%10.10s\n", "OK");
			total_ok++;
		}
		else
		{
			printf("%10.10s (%s)\n", "FAILED", res_str);
			total_ko++;
		}
	}
}
