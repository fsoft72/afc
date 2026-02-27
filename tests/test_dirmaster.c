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

/*
 * test_dirmaster.c - Comprehensive tests for the AFC DirMaster module.
 *
 * Tests cover creation/deletion, scanning a real directory (/tmp),
 * verifying non-empty results, traversal with first/next macros,
 * tag-based configuration of date and size formats, clearing,
 * scanning a non-existent directory, and the search function.
 */

#include "test_utils.h"
#include "../src/dirmaster.h"

/* Temporary test directory path - /tmp should always exist and have entries */
#define TEST_DIR "/tmp"

/* A path that should never exist */
#define NONEXISTENT_DIR "/tmp/__afc_dirmaster_nonexistent_dir__"

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ----------------------------------------------------------------
	 * 1. Creation and initial state
	 * ---------------------------------------------------------------- */
	DirMaster *dm = afc_dirmaster_new();
	print_res("dirmaster_new != NULL",
		(void *)(long)1, (void *)(long)(dm != NULL), 0);

	/* Should be empty right after creation (afc_array_is_empty returns 1 for true, not TRUE) */
	print_res("is_empty after new",
		(void *)(long)1, (void *)(long)afc_dirmaster_is_empty(dm), 0);

	print_res("len after new == 0",
		(void *)(long)0, (void *)(long)afc_dirmaster_len(dm), 0);

	/* Check default format values */
	print_res("default date_format",
		(void *)(long)DATEFORMAT_MM_DD_YYYY,
		(void *)(long)dm->date_format, 0);

	print_res("default size_format",
		(void *)(long)SIZEFORMAT_BYTES,
		(void *)(long)dm->size_format, 0);

	print_res("default size_decimals",
		(void *)(long)2, (void *)(long)dm->size_decimals, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 2. afc_dirmaster_set_tag() - date format
	 * ---------------------------------------------------------------- */
	int res = afc_dirmaster_set_tag(dm,
		AFC_DIRMASTER_TAG_DATE_FORMAT, (void *)(long)DATEFORMAT_DD_MM_YYYY);
	print_res("set_tag date_format OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);
	print_res("date_format changed",
		(void *)(long)DATEFORMAT_DD_MM_YYYY,
		(void *)(long)dm->date_format, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 3. afc_dirmaster_set_tag() - size format
	 * ---------------------------------------------------------------- */
	res = afc_dirmaster_set_tag(dm,
		AFC_DIRMASTER_TAG_SIZE_FORMAT, (void *)(long)SIZEFORMAT_HUMAN);
	print_res("set_tag size_format OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);
	print_res("size_format == HUMAN",
		(void *)(long)SIZEFORMAT_HUMAN,
		(void *)(long)dm->size_format, 0);

	/* Set size decimals */
	afc_dirmaster_set_tag(dm,
		AFC_DIRMASTER_TAG_SIZE_DECIMALS, (void *)(long)3);
	print_res("size_decimals == 3",
		(void *)(long)3, (void *)(long)dm->size_decimals, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 4. afc_dirmaster_set_tag() - conversion flags
	 * ---------------------------------------------------------------- */
	afc_dirmaster_set_tag(dm,
		AFC_DIRMASTER_TAG_CONV_DATE_ACCESS, (void *)(long)TRUE);
	print_res("conv_date_access = TRUE",
		(void *)(long)TRUE, (void *)(long)dm->conv_date_access, 0);

	afc_dirmaster_set_tag(dm,
		AFC_DIRMASTER_TAG_CONV_USER, (void *)(long)TRUE);
	print_res("conv_user = TRUE",
		(void *)(long)TRUE, (void *)(long)dm->conv_user, 0);

	afc_dirmaster_set_tag(dm,
		AFC_DIRMASTER_TAG_CONV_MODE, (void *)(long)TRUE);
	print_res("conv_mode = TRUE",
		(void *)(long)TRUE, (void *)(long)dm->conv_mode, 0);

	afc_dirmaster_set_tag(dm,
		AFC_DIRMASTER_TAG_CONV_GROUP, (void *)(long)TRUE);
	print_res("conv_group = TRUE",
		(void *)(long)TRUE, (void *)(long)dm->conv_group, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 5. afc_dirmaster_scan_dir() on /tmp
	 * ---------------------------------------------------------------- */
	res = afc_dirmaster_scan_dir(dm, TEST_DIR);
	print_res("scan_dir /tmp returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* /tmp should contain at least some files */
	unsigned long num_items = afc_dirmaster_len(dm);
	print_res("scanned dir not empty",
		(void *)(long)1, (void *)(long)(num_items > 0), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 6. Traverse results with first/next macros
	 * ---------------------------------------------------------------- */
	FileInfo *fi = afc_dirmaster_first(dm);
	print_res("first() != NULL",
		(void *)(long)1, (void *)(long)(fi != NULL), 0);

	/* Verify the first entry has a valid name (non-empty) */
	int has_name = (fi != NULL && strlen(fi->name) > 0);
	print_res("first entry has name",
		(void *)(long)1, (void *)(long)has_name, 0);

	/* The first entry should have a valid kind */
	int valid_kind = (fi != NULL && (fi->kind == FINFO_KIND_FILE
		|| fi->kind == FINFO_KIND_DIR
		|| (fi->kind & FINFO_KIND_LINK)));
	print_res("first entry valid kind",
		(void *)(long)1, (void *)(long)valid_kind, 0);

	/* Count all entries by traversing the list */
	unsigned long count = 0;
	fi = afc_dirmaster_first(dm);
	while (fi != NULL)
	{
		count++;
		fi = afc_dirmaster_next(dm);
	}
	print_res("traversal count matches len",
		(void *)(long)num_items, (void *)(long)count, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 7. afc_dirmaster_item() - indexed access
	 * ---------------------------------------------------------------- */
	if (num_items > 0)
	{
		fi = afc_dirmaster_item(dm, 0);
		print_res("item(0) != NULL",
			(void *)(long)1, (void *)(long)(fi != NULL), 0);
	}

	/* Last item access */
	fi = afc_dirmaster_last(dm);
	print_res("last() != NULL",
		(void *)(long)1, (void *)(long)(fi != NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 8. Verify conversion fields are populated
	 * ---------------------------------------------------------------- */
	fi = afc_dirmaster_first(dm);
	if (fi != NULL)
	{
		/* cmodify should be non-empty since conv_date_modify is TRUE by default */
		int has_modify = (strlen(fi->cmodify) > 0);
		print_res("cmodify populated",
			(void *)(long)1, (void *)(long)has_modify, 0);

		/* cmode should be non-empty since we set conv_mode = TRUE */
		int has_mode = (strlen(fi->cmode) > 0);
		print_res("cmode populated",
			(void *)(long)1, (void *)(long)has_mode, 0);

		/* cuser should be non-empty since we set conv_user = TRUE */
		int has_user = (strlen(fi->cuser) > 0);
		print_res("cuser populated",
			(void *)(long)1, (void *)(long)has_user, 0);
	}

	print_row();

	/* ----------------------------------------------------------------
	 * 9. afc_dirmaster_clear()
	 * ---------------------------------------------------------------- */
	res = afc_dirmaster_clear(dm);
	print_res("clear returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);
	print_res("len after clear == 0",
		(void *)(long)0, (void *)(long)afc_dirmaster_len(dm), 0);
	print_res("is_empty after clear",
		(void *)(long)1, (void *)(long)afc_dirmaster_is_empty(dm), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 10. Scan again to test re-use after clear
	 * ---------------------------------------------------------------- */
	/* Reset to bytes format for predictable csize values */
	afc_dirmaster_set_tag(dm,
		AFC_DIRMASTER_TAG_SIZE_FORMAT, (void *)(long)SIZEFORMAT_BYTES);

	res = afc_dirmaster_scan_dir(dm, TEST_DIR);
	print_res("re-scan after clear OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);
	print_res("re-scan not empty",
		(void *)(long)1, (void *)(long)(afc_dirmaster_len(dm) > 0), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 11. afc_dirmaster_set_tags() macro - multiple tags at once
	 * ---------------------------------------------------------------- */
	afc_dirmaster_set_tags(dm,
		AFC_DIRMASTER_TAG_DATE_FORMAT, (void *)(long)DATEFORMAT_HH_MM_SS,
		AFC_DIRMASTER_TAG_SIZE_FORMAT, (void *)(long)SIZEFORMAT_HUMAN_1000);
	print_res("set_tags date HH_MM_SS",
		(void *)(long)DATEFORMAT_HH_MM_SS,
		(void *)(long)dm->date_format, 0);
	print_res("set_tags size HUMAN_1000",
		(void *)(long)SIZEFORMAT_HUMAN_1000,
		(void *)(long)dm->size_format, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 12. Edge case: scan non-existent directory
	 * ---------------------------------------------------------------- */
	res = afc_dirmaster_scan_dir(dm, NONEXISTENT_DIR);
	print_res("scan nonexistent != NO_ERR",
		(void *)(long)1, (void *)(long)(res != AFC_ERR_NO_ERROR), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 13. Edge case: clear with NULL pointer
	 * ---------------------------------------------------------------- */
	res = afc_dirmaster_clear(NULL);
	print_res("clear(NULL) returns err",
		(void *)(long)1, (void *)(long)(res != AFC_ERR_NO_ERROR), 0);

	/* ----------------------------------------------------------------
	 * Cleanup and summary
	 * ---------------------------------------------------------------- */
	print_summary();

	afc_dirmaster_delete(dm);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
