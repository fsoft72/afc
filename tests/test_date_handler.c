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

/**
 * test_date_handler.c - Comprehensive tests for the DateHandler class.
 *
 * Tests cover:
 *   - Object creation and deletion
 *   - Setting a known date and verifying fields
 *   - Date validation (valid and invalid dates, leap year edge cases)
 *   - Day of week calculation
 *   - Julian date get/set round-trip
 *   - Adding and subtracting days
 *   - String conversion in various modes
 *   - set_today() basic operation
 */

#include "test_utils.h"
#include "../src/date_handler.h"

/* Error code for invalid date from the source */
#define AFC_DH_ERR_INVALID_DATE (AFC_DATE_HANDLER_BASE + 1)

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	char buf[256];

	/* ---- Test 1: Object creation ---- */
	DateHandler *dh = afc_date_handler_new();
	print_res("date_handler_new() not NULL",
		(void *)(long)1,
		(void *)(long)(dh != NULL),
		0);

	print_row();

	/* ---- Test 2: Set a known date (2000-01-01) and verify return code ---- */
	{
		int res = afc_date_handler_set(dh, 2000, 1, 1);
		print_res("set(2000,1,1) -> NO_ERROR",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
	}

	/* ---- Test 3: Verify internal fields after set ---- */
	print_res("year == 2000",
		(void *)(long)2000,
		(void *)(long)dh->year,
		0);
	print_res("month == 1",
		(void *)(long)1,
		(void *)(long)dh->month,
		0);
	print_res("day == 1",
		(void *)(long)1,
		(void *)(long)dh->day,
		0);

	print_row();

	/* ---- Test 4: Date validation - valid dates ---- */
	{
		int res = afc_date_handler_is_valid(dh, 2024, 2, 29);
		print_res("valid: 2024-02-29 (leap)",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
	}
	{
		int res = afc_date_handler_is_valid(dh, 2000, 2, 29);
		print_res("valid: 2000-02-29 (leap/400)",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
	}
	{
		int res = afc_date_handler_is_valid(dh, 2023, 12, 31);
		print_res("valid: 2023-12-31",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
	}

	/* ---- Test 5: Date validation - invalid dates ---- */
	{
		int res = afc_date_handler_is_valid(dh, 2023, 2, 29);
		print_res("invalid: 2023-02-29 (no leap)",
			(void *)(long)AFC_DH_ERR_INVALID_DATE,
			(void *)(long)res,
			0);
	}
	{
		int res = afc_date_handler_is_valid(dh, 1900, 2, 29);
		print_res("invalid: 1900-02-29 (no leap)",
			(void *)(long)AFC_DH_ERR_INVALID_DATE,
			(void *)(long)res,
			0);
	}
	{
		int res = afc_date_handler_is_valid(dh, 2023, 13, 1);
		print_res("invalid: month=13",
			(void *)(long)AFC_DH_ERR_INVALID_DATE,
			(void *)(long)res,
			0);
	}
	{
		int res = afc_date_handler_is_valid(dh, 2023, 4, 31);
		print_res("invalid: Apr 31",
			(void *)(long)AFC_DH_ERR_INVALID_DATE,
			(void *)(long)res,
			0);
	}

	print_row();

	/* ---- Test 6: Day of week ---- */
	/* 2000-01-01 was a Saturday (day_of_week = 6) */
	afc_date_handler_set(dh, 2000, 1, 1);
	{
		int dow = afc_date_handler_get_day_of_week(dh);
		print_res("2000-01-01 dow=6 (Sat)",
			(void *)(long)6,
			(void *)(long)dow,
			0);
	}

	/* 2024-01-01 was a Monday (day_of_week = 1) */
	afc_date_handler_set(dh, 2024, 1, 1);
	{
		int dow = afc_date_handler_get_day_of_week(dh);
		print_res("2024-01-01 dow=1 (Mon)",
			(void *)(long)1,
			(void *)(long)dow,
			0);
	}

	/* 1972-01-10 was a Monday (day_of_week = 1) */
	afc_date_handler_set(dh, 1972, 1, 10);
	{
		int dow = afc_date_handler_get_day_of_week(dh);
		print_res("1972-01-10 dow=1 (Mon)",
			(void *)(long)1,
			(void *)(long)dow,
			0);
	}

	print_row();

	/* ---- Test 7: Julian date round-trip ---- */
	afc_date_handler_set(dh, 2000, 6, 15);
	{
		long jd = afc_date_handler_get_julian(dh);
		/* Save the original date fields */
		int orig_year = dh->year;
		int orig_month = dh->month;
		int orig_day = dh->day;

		/* Reset and set from Julian */
		afc_date_handler_set_julian(dh, jd);

		print_res("julian round-trip year",
			(void *)(long)orig_year,
			(void *)(long)dh->year,
			0);
		print_res("julian round-trip month",
			(void *)(long)orig_month,
			(void *)(long)dh->month,
			0);
		print_res("julian round-trip day",
			(void *)(long)orig_day,
			(void *)(long)dh->day,
			0);
	}

	print_row();

	/* ---- Test 8: Add days ---- */
	afc_date_handler_set(dh, 2023, 12, 30);
	afc_date_handler_add_days(dh, 3);
	print_res("add 3 days: year=2024",
		(void *)(long)2024,
		(void *)(long)dh->year,
		0);
	print_res("add 3 days: month=1",
		(void *)(long)1,
		(void *)(long)dh->month,
		0);
	print_res("add 3 days: day=2",
		(void *)(long)2,
		(void *)(long)dh->day,
		0);

	/* ---- Test 9: Subtract days (add negative) ---- */
	afc_date_handler_set(dh, 2024, 3, 1);
	afc_date_handler_add_days(dh, -1);
	/* 2024 is a leap year, so March 1 minus 1 = Feb 29 */
	print_res("sub 1 day: year=2024",
		(void *)(long)2024,
		(void *)(long)dh->year,
		0);
	print_res("sub 1 day: month=2",
		(void *)(long)2,
		(void *)(long)dh->month,
		0);
	print_res("sub 1 day: day=29 (leap)",
		(void *)(long)29,
		(void *)(long)dh->day,
		0);

	print_row();

	/* ---- Test 10: to_string in YYYYMMDD mode ---- */
	afc_date_handler_set(dh, 2023, 7, 4);
	afc_date_handler_to_string(dh, buf, sizeof(buf), AFC_DATE_HANDLER_MODE_YYYYMMDD);
	print_res("to_string YYYYMMDD",
		(void *)"2023/07/04",
		(void *)buf,
		1);

	/* ---- Test 11: to_string in DDMMYYYY mode ---- */
	afc_date_handler_to_string(dh, buf, sizeof(buf), AFC_DATE_HANDLER_MODE_DDMMYYYY);
	print_res("to_string DDMMYYYY",
		(void *)"04/07/2023",
		(void *)buf,
		1);

	/* ---- Test 12: to_string in MMDDYYYY mode ---- */
	afc_date_handler_to_string(dh, buf, sizeof(buf), AFC_DATE_HANDLER_MODE_MMDDYYYY);
	print_res("to_string MMDDYYYY",
		(void *)"07/04/2023",
		(void *)buf,
		1);

	/* ---- Test 13: to_string in TEXT mode ---- */
	/* 2023-07-04 is a Tuesday */
	afc_date_handler_to_string(dh, buf, sizeof(buf), AFC_DATE_HANDLER_MODE_TEXT);
	print_res("to_string TEXT",
		(void *)"Tue 04 Jul 2023",
		(void *)buf,
		1);

	print_row();

	/* ---- Test 14: set_today() should not fail ---- */
	{
		int res = afc_date_handler_set_today(dh);
		print_res("set_today() -> NO_ERROR",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
	}

	/* ---- Test 15: After set_today, year should be reasonable (>= 2025) ---- */
	print_res("set_today year >= 2025",
		(void *)(long)1,
		(void *)(long)(dh->year >= 2025),
		0);

	print_summary();

	/* Cleanup */
	afc_date_handler_delete(dh);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
