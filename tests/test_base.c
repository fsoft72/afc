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
 * test_base.c - Comprehensive tests for the AFC base module.
 *
 * Tests cover:
 *   - afc_new() / afc_delete() lifecycle
 *   - AFC magic number verification
 *   - afc_set_tag() with various tags and log levels
 *   - Error code constants existence and distinctness
 */

#include "test_utils.h"
#include "../src/base.h"

/* Expected magic number computed from the 'BASE' string. */
#define EXPECTED_MAGIC ('B' << 24 | 'A' << 16 | 'S' << 8 | 'E')

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ===== afc_new() lifecycle tests ===== */

	/* Verify that afc_new() returns a non-NULL pointer. */
	print_res("afc_new() not NULL",
		(void *)(long)1,
		(void *)(long)(afc != NULL),
		0);

	/* Verify the global __internal_afc_base is set. */
	print_res("__internal_afc_base set",
		(void *)(long)1,
		(void *)(long)(__internal_afc_base == afc),
		0);

	print_row();

	/* ===== Magic number verification ===== */

	/* The magic number should match the expected value derived from 'BASE'. */
	print_res("AFC_MAGIC value",
		(void *)(long)EXPECTED_MAGIC,
		(void *)(long)afc->magic,
		0);

	/* Verify AFC_CLASS_MAGIC macro returns the same value. */
	print_res("AFC_CLASS_MAGIC macro",
		(void *)(long)EXPECTED_MAGIC,
		(void *)(long)AFC_CLASS_MAGIC(afc),
		0);

	print_row();

	/* ===== Default field values after afc_new() ===== */

	/* start_log_level defaults to 0 (AFC_LOG_MESSAGE). */
	print_res("default log level",
		(void *)(long)0,
		(void *)(long)afc->start_log_level,
		0);

	/* log_exit_critical defaults to FALSE (0). */
	print_res("default exit_critical",
		(void *)(long)0,
		(void *)(long)afc->log_exit_critical,
		0);

	/* debug_level defaults to 0 (AFC_DEBUG_NONE). */
	print_res("default debug_level",
		(void *)(long)0,
		(void *)(long)afc->debug_level,
		0);

	/* tmp_string should be allocated (not NULL). */
	print_res("tmp_string allocated",
		(void *)(long)1,
		(void *)(long)(afc->tmp_string != NULL),
		0);

	/* last_error should be allocated (not NULL). */
	print_res("last_error allocated",
		(void *)(long)1,
		(void *)(long)(afc->last_error != NULL),
		0);

	/* tracker should be NULL by default (no tracking enabled). */
	print_res("tracker is NULL",
		(void *)(long)1,
		(void *)(long)(afc->tracker == NULL),
		0);

	/* Output file defaults to stderr. */
	print_res("fout defaults to stderr",
		(void *)(long)1,
		(void *)(long)(afc->fout == stderr),
		0);

	print_row();

	/* ===== afc_set_tag() tests ===== */

	/* Set log level to AFC_LOG_WARNING and verify. */
	afc_set_tag(afc, AFC_TAG_LOG_LEVEL, (void *)(long)AFC_LOG_WARNING);
	print_res("set LOG_LEVEL WARNING",
		(void *)(long)AFC_LOG_WARNING,
		(void *)(long)afc->start_log_level,
		0);

	/* Set log level to AFC_LOG_ERROR and verify. */
	afc_set_tag(afc, AFC_TAG_LOG_LEVEL, (void *)(long)AFC_LOG_ERROR);
	print_res("set LOG_LEVEL ERROR",
		(void *)(long)AFC_LOG_ERROR,
		(void *)(long)afc->start_log_level,
		0);

	/* Set log level to AFC_LOG_CRITICAL and verify. */
	afc_set_tag(afc, AFC_TAG_LOG_LEVEL, (void *)(long)AFC_LOG_CRITICAL);
	print_res("set LOG_LEVEL CRITICAL",
		(void *)(long)AFC_LOG_CRITICAL,
		(void *)(long)afc->start_log_level,
		0);

	/* Set log level back to AFC_LOG_MESSAGE (0) and verify. */
	afc_set_tag(afc, AFC_TAG_LOG_LEVEL, (void *)(long)AFC_LOG_MESSAGE);
	print_res("set LOG_LEVEL MESSAGE",
		(void *)(long)AFC_LOG_MESSAGE,
		(void *)(long)afc->start_log_level,
		0);

	/* Set debug level to AFC_DEBUG_VERBOSE. */
	afc_set_tag(afc, AFC_TAG_DEBUG_LEVEL, (void *)(long)AFC_DEBUG_VERBOSE);
	print_res("set DEBUG_LEVEL VERBOSE",
		(void *)(long)AFC_DEBUG_VERBOSE,
		(void *)(long)afc->debug_level,
		0);

	/* Set debug level to AFC_DEBUG_EVERYTHING. */
	afc_set_tag(afc, AFC_TAG_DEBUG_LEVEL, (void *)(long)AFC_DEBUG_EVERYTHING);
	print_res("set DEBUG_EVERYTHING",
		(void *)(long)AFC_DEBUG_EVERYTHING,
		(void *)(long)afc->debug_level,
		0);

	/* Set log_exit_critical to TRUE. */
	afc_set_tag(afc, AFC_TAG_LOG_EXIT_CRITICAL, (void *)(long)TRUE);
	print_res("set EXIT_CRITICAL TRUE",
		(void *)(long)1,
		(void *)(long)(afc->log_exit_critical != 0),
		0);

	/* Set log_exit_critical back to FALSE. */
	afc_set_tag(afc, AFC_TAG_LOG_EXIT_CRITICAL, (void *)(long)FALSE);
	print_res("set EXIT_CRITICAL FALSE",
		(void *)(long)0,
		(void *)(long)afc->log_exit_critical,
		0);

	print_row();

	/* ===== afc_set_tags() (multiple tags at once) ===== */

	/* Set multiple tags in a single call. */
	afc_set_tags(afc,
		AFC_TAG_LOG_LEVEL, (void *)(long)AFC_LOG_NOTICE,
		AFC_TAG_DEBUG_LEVEL, (void *)(long)AFC_DEBUG_STANDARD);

	print_res("set_tags LOG_LEVEL",
		(void *)(long)AFC_LOG_NOTICE,
		(void *)(long)afc->start_log_level,
		0);

	print_res("set_tags DEBUG_LEVEL",
		(void *)(long)AFC_DEBUG_STANDARD,
		(void *)(long)afc->debug_level,
		0);

	print_row();

	/* ===== Error code constants: existence and distinctness ===== */

	/* Verify that AFC_ERR_NO_ERROR equals 0. */
	print_res("ERR_NO_ERROR == 0",
		(void *)(long)0,
		(void *)(long)AFC_ERR_NO_ERROR,
		0);

	/* Verify AFC_NO_ERR is the same as AFC_ERR_NO_ERROR. */
	print_res("NO_ERR == ERR_NO_ERROR",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)AFC_NO_ERR,
		0);

	/* Verify that all error codes are distinct from each other. */
	print_res("NO_MEMORY != NO_ERROR",
		(void *)(long)1,
		(void *)(long)(AFC_ERR_NO_MEMORY != AFC_ERR_NO_ERROR),
		0);

	print_res("NULL_PTR != NO_MEMORY",
		(void *)(long)1,
		(void *)(long)(AFC_ERR_NULL_POINTER != AFC_ERR_NO_MEMORY),
		0);

	print_res("INVALID_PTR != NULL_PTR",
		(void *)(long)1,
		(void *)(long)(AFC_ERR_INVALID_POINTER != AFC_ERR_NULL_POINTER),
		0);

	print_res("INVALID_LOG != INVALID_PTR",
		(void *)(long)1,
		(void *)(long)(AFC_ERR_INVALID_LOG_LEVEL != AFC_ERR_INVALID_POINTER),
		0);

	print_res("UNSUPPORTED_TAG distinct",
		(void *)(long)1,
		(void *)(long)(AFC_ERR_UNSUPPORTED_TAG != AFC_ERR_INVALID_LOG_LEVEL),
		0);

	print_row();

	/* ===== afc_clear() test ===== */

	/* afc_clear should return AFC_ERR_NO_ERROR for a valid AFC. */
	print_res("afc_clear() OK",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)afc_clear(afc),
		0);

	/* afc_clear with NULL should return AFC_ERR_NULL_POINTER. */
	print_res("afc_clear(NULL)",
		(void *)(long)AFC_ERR_NULL_POINTER,
		(void *)(long)afc_clear(NULL),
		0);

	print_row();

	/* ===== Tag constant values ===== */

	/* Verify AFC_TAG_END has the expected sentinel value. */
	print_res("AFC_TAG_END value",
		(void *)(long)0xDEADBEEF,
		(void *)(long)AFC_TAG_END,
		0);

	/* Verify AFC_BASE constant. */
	print_res("AFC_BASE value",
		(void *)(long)0xFF00,
		(void *)(long)AFC_BASE,
		0);

	/* Verify AFC_TAG_LOG_LEVEL is AFC_BASE + 1. */
	print_res("TAG_LOG_LEVEL value",
		(void *)(long)(AFC_BASE + 1),
		(void *)(long)AFC_TAG_LOG_LEVEL,
		0);

	print_summary();

	/* Cleanup: afc_delete should return AFC_ERR_NO_ERROR. */
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
