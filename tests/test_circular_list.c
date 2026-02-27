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
 * test_circular_list.c - Comprehensive tests for the CircularList class.
 *
 * Tests cover:
 *   - Creation and deletion
 *   - Initialization with max element count
 *   - Adding items with afc_circular_list_add()
 *   - Navigating with afc_circular_list_next() / prev()
 *   - Getting the current item with afc_circular_list_obj() macro
 *   - Verifying circular wrapping behavior
 *   - Deleting individual items with afc_circular_list_del()
 *   - Clearing all items with afc_circular_list_clear()
 *   - Max element limit enforcement
 *   - Edge cases: empty list operations
 */

#include "test_utils.h"
#include "../src/circular_list.h"

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ================================================================
	 * GROUP 1: Creation
	 * ================================================================ */

	CircularList *cl = afc_circular_list_new();

	/* Verify list was created (non-NULL) */
	print_res("new() returns non-NULL",
		(void *)(long)1,
		(void *)(long)(cl != NULL),
		0);

	/* Initial count should be 0 */
	print_res("initial count == 0",
		(void *)(long)0,
		(void *)(long)cl->count,
		0);

	/* The pointer should be NULL initially */
	print_res("initial pointer is NULL",
		(void *)(long)1,
		(void *)(long)(cl->pointer == NULL),
		0);

	/* afc_circular_list_obj should return NULL for empty list */
	void *obj = afc_circular_list_obj(cl);
	print_res("obj() on empty list",
		(void *)(long)0,
		(void *)(long)obj,
		0);

	print_row();

	/* ================================================================
	 * GROUP 2: Init with no max limit (0 = unlimited) and add items.
	 * We use integer values cast to void* as data pointers.
	 * ================================================================ */

	afc_circular_list_init(cl, 0);

	/* Add first item - value 10 */
	int res = afc_circular_list_add(cl, (void *)(long)10);
	print_res("add(10) result",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("count after add(10)",
		(void *)(long)1,
		(void *)(long)cl->count,
		0);

	/* Current object should be 10 */
	obj = afc_circular_list_obj(cl);
	print_res("obj() == 10 after first add",
		(void *)(long)10,
		(void *)(long)obj,
		0);

	/* Add second item - value 20 */
	res = afc_circular_list_add(cl, (void *)(long)20);
	print_res("add(20) result",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* Current object should now be 20 (last added) */
	obj = afc_circular_list_obj(cl);
	print_res("obj() == 20 after second add",
		(void *)(long)20,
		(void *)(long)obj,
		0);

	/* Add third item - value 30 */
	afc_circular_list_add(cl, (void *)(long)30);

	print_res("count after 3 adds",
		(void *)(long)3,
		(void *)(long)cl->count,
		0);

	/* Current object should be 30 */
	obj = afc_circular_list_obj(cl);
	print_res("obj() == 30 after third add",
		(void *)(long)30,
		(void *)(long)obj,
		0);

	print_row();

	/* ================================================================
	 * GROUP 3: Navigation with next() / prev()
	 *
	 * The add() inserts after the current pointer. After adding
	 * 10, 20, 30 the list order (from the initial node) is:
	 *   10 -> 20 -> 30 -> (back to 10)
	 *
	 * Current pointer is at 30. So:
	 *   next() -> 10  (wraps around)
	 *   next() -> 20
	 *   next() -> 30
	 * ================================================================ */

	/* Move to next: should wrap around to 10 */
	obj = afc_circular_list_next(cl);
	print_res("next() from 30 -> 10",
		(void *)(long)10,
		(void *)(long)obj,
		0);

	/* Move to next: should be 20 */
	obj = afc_circular_list_next(cl);
	print_res("next() from 10 -> 20",
		(void *)(long)20,
		(void *)(long)obj,
		0);

	/* Move to next: should be 30 */
	obj = afc_circular_list_next(cl);
	print_res("next() from 20 -> 30",
		(void *)(long)30,
		(void *)(long)obj,
		0);

	/* Move to next again: should wrap to 10 again */
	obj = afc_circular_list_next(cl);
	print_res("next() wraps to 10",
		(void *)(long)10,
		(void *)(long)obj,
		0);

	print_row();

	/* ================================================================
	 * GROUP 4: Navigation with prev()
	 *
	 * Currently at 10. prev() should go to 30 (wrapping backward).
	 * ================================================================ */

	obj = afc_circular_list_prev(cl);
	print_res("prev() from 10 -> 30",
		(void *)(long)30,
		(void *)(long)obj,
		0);

	obj = afc_circular_list_prev(cl);
	print_res("prev() from 30 -> 20",
		(void *)(long)20,
		(void *)(long)obj,
		0);

	obj = afc_circular_list_prev(cl);
	print_res("prev() from 20 -> 10",
		(void *)(long)10,
		(void *)(long)obj,
		0);

	print_row();

	/* ================================================================
	 * GROUP 5: Full circular traversal verification
	 *
	 * Starting from 10, calling next() 3 times should return
	 * to the same element (circularity).
	 * ================================================================ */

	/* We are at 10. Navigate 3 steps forward. */
	afc_circular_list_next(cl); /* -> 20 */
	afc_circular_list_next(cl); /* -> 30 */
	obj = afc_circular_list_next(cl); /* -> 10 (full circle) */
	print_res("full circle returns to 10",
		(void *)(long)10,
		(void *)(long)obj,
		0);

	print_row();

	/* ================================================================
	 * GROUP 6: Delete current item
	 *
	 * Currently at 10. Deleting it should move to the next item
	 * and return its data. The list becomes: 20 -> 30 -> (cycle)
	 * ================================================================ */

	obj = afc_circular_list_del(cl);
	print_res("del() at 10 returns next",
		(void *)(long)20,
		(void *)(long)obj,
		0);

	print_res("count after del",
		(void *)(long)2,
		(void *)(long)cl->count,
		0);

	/* Current should now be 20 */
	obj = afc_circular_list_obj(cl);
	print_res("obj() == 20 after del",
		(void *)(long)20,
		(void *)(long)obj,
		0);

	/* Verify circular behavior with 2 items */
	obj = afc_circular_list_next(cl);
	print_res("next() -> 30",
		(void *)(long)30,
		(void *)(long)obj,
		0);

	obj = afc_circular_list_next(cl);
	print_res("next() -> 20 (cycle)",
		(void *)(long)20,
		(void *)(long)obj,
		0);

	print_row();

	/* ================================================================
	 * GROUP 7: Delete until empty
	 * ================================================================ */

	/* Delete 20, should return 30 */
	obj = afc_circular_list_del(cl);
	print_res("del() at 20 returns 30",
		(void *)(long)30,
		(void *)(long)obj,
		0);

	print_res("count == 1",
		(void *)(long)1,
		(void *)(long)cl->count,
		0);

	/* Delete 30, should return NULL (list becomes empty) */
	obj = afc_circular_list_del(cl);
	print_res("del() last item returns NULL",
		(void *)(long)0,
		(void *)(long)obj,
		0);

	print_res("count == 0 after all dels",
		(void *)(long)0,
		(void *)(long)cl->count,
		0);

	/* Pointer should be NULL now */
	print_res("pointer is NULL when empty",
		(void *)(long)1,
		(void *)(long)(cl->pointer == NULL),
		0);

	print_row();

	/* ================================================================
	 * GROUP 8: Clear operation
	 * Re-add items and then clear
	 * ================================================================ */

	afc_circular_list_add(cl, (void *)(long)100);
	afc_circular_list_add(cl, (void *)(long)200);
	afc_circular_list_add(cl, (void *)(long)300);

	print_res("count == 3 before clear",
		(void *)(long)3,
		(void *)(long)cl->count,
		0);

	res = afc_circular_list_clear(cl);
	print_res("clear() result",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("count == 0 after clear",
		(void *)(long)0,
		(void *)(long)cl->count,
		0);

	print_row();

	/* ================================================================
	 * GROUP 9: Max element limit enforcement
	 * ================================================================ */

	/* Delete and create a new list with max limit of 2 */
	afc_circular_list_delete(cl);
	cl = afc_circular_list_new();
	afc_circular_list_init(cl, 2);

	/* Add up to max */
	res = afc_circular_list_add(cl, (void *)(long)1);
	print_res("add(1) under limit",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	res = afc_circular_list_add(cl, (void *)(long)2);
	print_res("add(2) at limit",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* Third add should fail with max elems error */
	res = afc_circular_list_add(cl, (void *)(long)3);
	print_res("add(3) over limit fails",
		(void *)(long)AFC_CIRCULAR_LIST_ERR_MAX_ELEMS,
		(void *)(long)res,
		0);

	/* Count should still be 2 */
	print_res("count still 2 after fail",
		(void *)(long)2,
		(void *)(long)cl->count,
		0);

	/* Existing items should be intact */
	obj = afc_circular_list_obj(cl);
	print_res("obj() still valid",
		(void *)(long)2,
		(void *)(long)obj,
		0);

	/* ================================================================
	 * Summary and cleanup
	 * ================================================================ */

	print_summary();

	afc_circular_list_delete(cl);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
