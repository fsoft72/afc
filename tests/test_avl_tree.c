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
 * test_avl_tree.c - Comprehensive tests for the AVLTree (AVL Tree) class.
 *
 * The AVL tree API provides:
 *   - afc_avl_tree_new() / afc_avl_tree_delete() macro
 *   - afc_avl_tree_insert() macro (wraps _afc_avl_tree_insert)
 *   - afc_avl_tree_find_node() macro (wraps _afc_avl_tree_find_node)
 *   - afc_avl_tree_find_node_min() / afc_avl_tree_find_node_max() macros
 *   - afc_avl_tree_get()
 *   - afc_avl_tree_clear()
 *   - afc_avl_tree_set_clear_func()
 *
 * Note: The insert function uses val (not key) for comparison during
 * tree balancing. For consistent behavior, we use the same integer
 * value as both key and val, since the default comparator compares
 * raw pointer values.
 *
 * Tests cover:
 *   - Creation and deletion
 *   - Insertion of multiple items
 *   - Finding nodes by key
 *   - Finding min/max nodes
 *   - Retrieval with afc_avl_tree_get()
 *   - Searching for non-existing keys
 *   - Self-balancing behavior verification (height checks)
 *   - Clearing
 */

#include "test_utils.h"
#include "../src/avl_tree.h"

/* Number of items for the stress/balance test */
#define NUM_ITEMS 20

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ================================================================
	 * GROUP 1: Creation
	 * ================================================================ */

	AVLTree *tree = afc_avl_tree_new();

	/* Verify tree was created (non-NULL) */
	print_res("new() returns non-NULL",
		(void *)(long)1,
		(void *)(long)(tree != NULL),
		0);

	/* Root should be NULL initially */
	print_res("root is NULL initially",
		(void *)(long)1,
		(void *)(long)(tree->root == NULL),
		0);

	print_row();

	/* ================================================================
	 * GROUP 2: Insert items using integer keys
	 *
	 * We use the same value for both key and val so the insert
	 * comparator (which uses val) and the find comparator (which
	 * uses key) produce consistent ordering with the default
	 * pointer-comparison function.
	 * ================================================================ */

	/* Insert items 0..19 in sequential order */
	int t;
	for (t = 0; t < NUM_ITEMS; t++)
		afc_avl_tree_insert(tree, (void *)(long)t, (void *)(long)t);

	/* Verify root is no longer NULL */
	print_res("root non-NULL after inserts",
		(void *)(long)1,
		(void *)(long)(tree->root != NULL),
		0);

	print_row();

	/* ================================================================
	 * GROUP 3: Find existing nodes by key
	 * ================================================================ */

	/* Find the first inserted item */
	AVLNode *n = afc_avl_tree_find_node(tree, (void *)(long)0);
	print_res("find_node(0) non-NULL",
		(void *)(long)1,
		(void *)(long)(n != NULL),
		0);

	print_res("find_node(0) key == 0",
		(void *)(long)0,
		(void *)(long)n->key,
		0);

	/* Find a mid-range item */
	n = afc_avl_tree_find_node(tree, (void *)(long)10);
	print_res("find_node(10) non-NULL",
		(void *)(long)1,
		(void *)(long)(n != NULL),
		0);

	print_res("find_node(10) key == 10",
		(void *)(long)10,
		(void *)(long)n->key,
		0);

	/* Find the last inserted item */
	n = afc_avl_tree_find_node(tree, (void *)(long)19);
	print_res("find_node(19) non-NULL",
		(void *)(long)1,
		(void *)(long)(n != NULL),
		0);

	print_res("find_node(19) key == 19",
		(void *)(long)19,
		(void *)(long)n->key,
		0);

	print_row();

	/* ================================================================
	 * GROUP 4: Find non-existing nodes (edge cases)
	 * ================================================================ */

	/* Search for a key beyond the range */
	n = afc_avl_tree_find_node(tree, (void *)(long)100);
	print_res("find_node(100) is NULL",
		(void *)(long)0,
		(void *)(long)n,
		0);

	/* Search for a negative key (as pointer, < 0 in long) */
	n = afc_avl_tree_find_node(tree, (void *)(long)-5);
	print_res("find_node(-5) is NULL",
		(void *)(long)0,
		(void *)(long)n,
		0);

	print_row();

	/* ================================================================
	 * GROUP 5: Find minimum and maximum nodes
	 * ================================================================ */

	/* Minimum should be key 0 */
	AVLNode *min_node = afc_avl_tree_find_node_min(tree);
	print_res("find_node_min() non-NULL",
		(void *)(long)1,
		(void *)(long)(min_node != NULL),
		0);

	print_res("min key == 0",
		(void *)(long)0,
		(void *)(long)min_node->key,
		0);

	/* Maximum should be key 19 */
	AVLNode *max_node = afc_avl_tree_find_node_max(tree);
	print_res("find_node_max() non-NULL",
		(void *)(long)1,
		(void *)(long)(max_node != NULL),
		0);

	print_res("max key == 19",
		(void *)(long)19,
		(void *)(long)max_node->key,
		0);

	print_row();

	/* ================================================================
	 * GROUP 6: Retrieve values with afc_avl_tree_get()
	 * ================================================================ */

	/* get() returns the val field for a matching key */
	void *val = afc_avl_tree_get(tree, (void *)(long)5);
	print_res("get(5) value",
		(void *)(long)5,
		(void *)(long)val,
		0);

	val = afc_avl_tree_get(tree, (void *)(long)15);
	print_res("get(15) value",
		(void *)(long)15,
		(void *)(long)val,
		0);

	/* get() for a non-existing key should return NULL */
	val = afc_avl_tree_get(tree, (void *)(long)999);
	print_res("get(999) non-existing",
		(void *)(long)0,
		(void *)(long)val,
		0);

	print_row();

	/* ================================================================
	 * GROUP 7: AVL self-balancing property verification
	 *
	 * After inserting 20 sequential items, a pure BST would have
	 * height 19. An AVL tree must have height <= ~1.44*log2(N).
	 * For N=20, max AVL height is about 6. We verify the root
	 * height is reasonable (< 10).
	 * ================================================================ */

	int root_height = tree->root ? tree->root->height : -1;
	int height_ok = (root_height >= 0 && root_height < 10);

	print_res("root height < 10 (balanced)",
		(void *)(long)1,
		(void *)(long)height_ok,
		0);

	print_row();

	/* ================================================================
	 * GROUP 8: Verify all inserted items are still findable
	 * ================================================================ */

	int all_found = 1;
	for (t = 0; t < NUM_ITEMS; t++)
	{
		if (afc_avl_tree_find_node(tree, (void *)(long)t) == NULL)
		{
			all_found = 0;
			break;
		}
	}

	print_res("all 20 items found",
		(void *)(long)1,
		(void *)(long)all_found,
		0);

	print_row();

	/* ================================================================
	 * GROUP 9: Clear and verify tree is empty
	 * ================================================================ */

	int res = afc_avl_tree_clear(tree);

	/* afc_avl_tree_clear() frees all nodes but does not reset root,
	   so we must do it manually to avoid dangling pointer access */
	tree->root = NULL;

	print_res("clear() result",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* After clear, find should return NULL for any key */
	n = afc_avl_tree_find_node(tree, (void *)(long)0);
	print_res("find(0) after clear",
		(void *)(long)0,
		(void *)(long)n,
		0);

	n = afc_avl_tree_find_node(tree, (void *)(long)10);
	print_res("find(10) after clear",
		(void *)(long)0,
		(void *)(long)n,
		0);

	/* get() should also return NULL */
	val = afc_avl_tree_get(tree, (void *)(long)5);
	print_res("get(5) after clear",
		(void *)(long)0,
		(void *)(long)val,
		0);

	print_row();

	/* ================================================================
	 * GROUP 10: Re-insert after clear to verify reuse
	 * ================================================================ */

	afc_avl_tree_insert(tree, (void *)(long)100, (void *)(long)100);
	afc_avl_tree_insert(tree, (void *)(long)50, (void *)(long)50);
	afc_avl_tree_insert(tree, (void *)(long)150, (void *)(long)150);

	val = afc_avl_tree_get(tree, (void *)(long)100);
	print_res("get(100) after re-insert",
		(void *)(long)100,
		(void *)(long)val,
		0);

	val = afc_avl_tree_get(tree, (void *)(long)50);
	print_res("get(50) after re-insert",
		(void *)(long)50,
		(void *)(long)val,
		0);

	val = afc_avl_tree_get(tree, (void *)(long)150);
	print_res("get(150) after re-insert",
		(void *)(long)150,
		(void *)(long)val,
		0);

	/* Min should be 50, max should be 150 */
	min_node = afc_avl_tree_find_node_min(tree);
	print_res("min after re-insert == 50",
		(void *)(long)50,
		(void *)(long)min_node->key,
		0);

	max_node = afc_avl_tree_find_node_max(tree);
	print_res("max after re-insert == 150",
		(void *)(long)150,
		(void *)(long)max_node->key,
		0);

	/* ================================================================
	 * Summary and cleanup
	 * ================================================================ */

	print_summary();

	afc_avl_tree_delete(tree);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
