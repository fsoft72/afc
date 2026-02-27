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
 * test_mem_tracker.c - Comprehensive tests for the AFC memory tracker module.
 *
 * Tests cover:
 *   - afc_track_mallocs() to enable tracking
 *   - afc_malloc() / afc_free() tracking
 *   - Verification that tracker is created and operational
 *   - Basic allocation/deallocation cycles
 *   - Tracker statistics (allocs, frees, alloc_bytes)
 *   - Multiple allocations and selective freeing
 */

#include "test_utils.h"
#include "../src/mem_tracker.h"

int main(void)
{
	AFC *afc = afc_new();
	MemTracker *tracker;
	void *mem1, *mem2, *mem3;
	unsigned int initial_allocs, initial_frees, initial_bytes;
	test_header();

	/* ===================================================================
	 * SECTION 1: afc_track_mallocs() - enabling the tracker
	 * =================================================================== */

	/* Before enabling, tracker should be NULL. */
	print_res("tracker initially NULL",
		(void *)(long)1,
		(void *)(long)(afc->tracker == NULL),
		0);

	/* Enable tracking. */
	tracker = afc_track_mallocs(afc);

	/* Tracker should now be non-NULL. */
	print_res("tracker enabled",
		(void *)(long)1,
		(void *)(long)(tracker != NULL),
		0);

	/* Tracker should be the same as afc->tracker. */
	print_res("tracker == afc->tracker",
		(void *)(long)1,
		(void *)(long)(tracker == afc->tracker),
		0);

	/* Calling afc_track_mallocs again should return the same tracker (idempotent). */
	print_res("track_mallocs idempotent",
		(void *)(long)1,
		(void *)(long)(afc_track_mallocs(afc) == tracker),
		0);

	print_row();

	/* ===================================================================
	 * SECTION 2: Tracker initial state
	 * =================================================================== */

	/* Note: afc_track_mallocs may have been called after some internal
	 * allocations, so allocs/frees may not be zero. We just verify
	 * the tracker struct is properly initialized in terms of its
	 * data structures. */

	/* show_mallocs defaults to FALSE. */
	print_res("show_mallocs default",
		(void *)(long)FALSE,
		(void *)(long)tracker->show_mallocs,
		0);

	/* show_frees defaults to FALSE. */
	print_res("show_frees default",
		(void *)(long)FALSE,
		(void *)(long)tracker->show_frees,
		0);

	/* data array should be allocated (non-NULL). */
	print_res("data array allocated",
		(void *)(long)1,
		(void *)(long)(tracker->data != NULL),
		0);

	/* free list should be allocated (non-NULL). */
	print_res("free list allocated",
		(void *)(long)1,
		(void *)(long)(tracker->free != NULL),
		0);

	/* data_max should be 200 (initial capacity). */
	print_res("data_max == 200",
		(void *)(long)200,
		(void *)(long)tracker->data_max,
		0);

	/* free_max should be 100 (initial capacity). */
	print_res("free_max == 100",
		(void *)(long)100,
		(void *)(long)tracker->free_max,
		0);

	print_row();

	/* ===================================================================
	 * SECTION 3: Basic allocation tracking with afc_malloc() / afc_free()
	 * =================================================================== */

	/* Record baseline stats before our test allocations. */
	initial_allocs = tracker->allocs;
	initial_frees = tracker->frees;
	initial_bytes = tracker->alloc_bytes;

	/* Allocate a 64-byte block. */
	mem1 = afc_malloc(64);
	print_res("malloc(64) not NULL",
		(void *)(long)1,
		(void *)(long)(mem1 != NULL),
		0);

	/* After one allocation, allocs counter should increase by 1. */
	print_res("allocs +1",
		(void *)(long)(initial_allocs + 1),
		(void *)(long)tracker->allocs,
		0);

	/* Allocated bytes should increase by 64. */
	print_res("alloc_bytes +64",
		(void *)(long)(initial_bytes + 64),
		(void *)(long)tracker->alloc_bytes,
		0);

	/* Free the block. */
	afc_free(mem1);

	/* Frees counter should increase by 1. */
	print_res("frees +1",
		(void *)(long)(initial_frees + 1),
		(void *)(long)tracker->frees,
		0);

	/* Allocated bytes should return to baseline. */
	print_res("alloc_bytes restored",
		(void *)(long)initial_bytes,
		(void *)(long)tracker->alloc_bytes,
		0);

	print_row();

	/* ===================================================================
	 * SECTION 4: Multiple allocations and selective freeing
	 * =================================================================== */

	/* Record baseline again. */
	initial_allocs = tracker->allocs;
	initial_frees = tracker->frees;
	initial_bytes = tracker->alloc_bytes;

	/* Allocate three blocks. */
	mem1 = afc_malloc(32);
	mem2 = afc_malloc(64);
	mem3 = afc_malloc(128);

	/* All should be non-NULL. */
	print_res("multi alloc mem1",
		(void *)(long)1,
		(void *)(long)(mem1 != NULL),
		0);

	print_res("multi alloc mem2",
		(void *)(long)1,
		(void *)(long)(mem2 != NULL),
		0);

	print_res("multi alloc mem3",
		(void *)(long)1,
		(void *)(long)(mem3 != NULL),
		0);

	/* Allocs counter should have increased by 3. */
	print_res("allocs +3",
		(void *)(long)(initial_allocs + 3),
		(void *)(long)tracker->allocs,
		0);

	/* Total allocated bytes: +32 + 64 + 128 = +224. */
	print_res("alloc_bytes +224",
		(void *)(long)(initial_bytes + 224),
		(void *)(long)tracker->alloc_bytes,
		0);

	/* Free the middle block. */
	afc_free(mem2);

	/* Frees counter should increase by 1. */
	print_res("free middle frees +1",
		(void *)(long)(initial_frees + 1),
		(void *)(long)tracker->frees,
		0);

	/* Allocated bytes should decrease by 64. */
	print_res("alloc_bytes after mid free",
		(void *)(long)(initial_bytes + 160),
		(void *)(long)tracker->alloc_bytes,
		0);

	/* Free the remaining blocks. */
	afc_free(mem1);
	afc_free(mem3);

	/* Frees counter should have increased by 3 total. */
	print_res("frees +3 total",
		(void *)(long)(initial_frees + 3),
		(void *)(long)tracker->frees,
		0);

	/* Allocated bytes should be back to baseline. */
	print_res("alloc_bytes baseline",
		(void *)(long)initial_bytes,
		(void *)(long)tracker->alloc_bytes,
		0);

	print_row();

	/* ===================================================================
	 * SECTION 5: afc_malloc returns zeroed memory
	 * =================================================================== */

	/* afc_malloc should return zeroed memory. */
	mem1 = afc_malloc(16);

	/* Check that first byte is zero. */
	print_res("malloc zeroed [0]",
		(void *)(long)0,
		(void *)(long)(int)((unsigned char *)mem1)[0],
		0);

	/* Check that last byte is zero. */
	print_res("malloc zeroed [15]",
		(void *)(long)0,
		(void *)(long)(int)((unsigned char *)mem1)[15],
		0);

	afc_free(mem1);

	print_row();

	/* ===================================================================
	 * SECTION 6: show_mallocs and show_frees via afc_set_tag
	 * =================================================================== */

	/* Enable show_mallocs via afc_set_tag. */
	afc_set_tag(afc, AFC_TAG_SHOW_MALLOCS, (void *)(long)TRUE);
	print_res("show_mallocs set TRUE",
		(void *)(long)1,
		(void *)(long)(tracker->show_mallocs != 0),
		0);

	/* Disable show_mallocs. */
	afc_set_tag(afc, AFC_TAG_SHOW_MALLOCS, (void *)(long)FALSE);
	print_res("show_mallocs set FALSE",
		(void *)(long)0,
		(void *)(long)tracker->show_mallocs,
		0);

	/* Enable show_frees via afc_set_tag. */
	afc_set_tag(afc, AFC_TAG_SHOW_FREES, (void *)(long)TRUE);
	print_res("show_frees set TRUE",
		(void *)(long)1,
		(void *)(long)(tracker->show_frees != 0),
		0);

	/* Disable show_frees. */
	afc_set_tag(afc, AFC_TAG_SHOW_FREES, (void *)(long)FALSE);
	print_res("show_frees set FALSE",
		(void *)(long)0,
		(void *)(long)tracker->show_frees,
		0);

	print_row();

	/* ===================================================================
	 * SECTION 7: Allocation-then-reallocation tracking
	 * =================================================================== */

	initial_bytes = tracker->alloc_bytes;

	/* Allocate a small block. */
	mem1 = afc_malloc(32);
	print_res("realloc: initial alloc",
		(void *)(long)1,
		(void *)(long)(mem1 != NULL),
		0);

	print_res("realloc: bytes +32",
		(void *)(long)(initial_bytes + 32),
		(void *)(long)tracker->alloc_bytes,
		0);

	/* Realloc to a larger size. The tracker should update the tracked size. */
	mem1 = afc_realloc(mem1, 128);
	print_res("realloc: grew not NULL",
		(void *)(long)1,
		(void *)(long)(mem1 != NULL),
		0);

	/* After realloc, bytes should reflect the new size (128) instead of old (32). */
	print_res("realloc: bytes updated",
		(void *)(long)(initial_bytes + 128),
		(void *)(long)tracker->alloc_bytes,
		0);

	/* Free the reallocated block. */
	afc_free(mem1);

	/* After free, bytes should return to baseline. */
	print_res("realloc: bytes restored",
		(void *)(long)initial_bytes,
		(void *)(long)tracker->alloc_bytes,
		0);

	print_row();

	/* ===================================================================
	 * SECTION 8: String allocations are also tracked
	 * =================================================================== */

	initial_allocs = tracker->allocs;

	/* AFC strings use afc_malloc internally, so they should be tracked. */
	{
		char *str = afc_string_new(50);

		/* String allocation should increase alloc count. */
		print_res("str alloc tracked",
			(void *)(long)1,
			(void *)(long)(tracker->allocs > initial_allocs),
			0);

		/* Verify string works correctly while tracked. */
		afc_string_copy(str, "tracked", ALL);
		print_res("tracked str content",
			"tracked",
			str,
			1);

		/* Free the string. */
		initial_frees = tracker->frees;
		afc_string_delete(str);

		/* String deallocation should increase free count. */
		print_res("str free tracked",
			(void *)(long)1,
			(void *)(long)(tracker->frees > initial_frees),
			0);
	}

	print_summary();

	/* Cleanup */
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
