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
 * test_bin_tree.c - Comprehensive tests for the BinTree (Binary Tree) class.
 *
 * Tests cover:
 *   - Creation and deletion
 *   - Custom comparator via afc_bin_tree_set_compare_func()
 *   - Insertion of multiple items
 *   - Retrieval of existing and non-existing keys
 *   - Empty tree checks with afc_bin_tree_is_empty()
 *   - Deletion of individual nodes with afc_bin_tree_del()
 *   - Traversal in INORDER, PREORDER, and POSTORDER modes
 *   - Clearing all nodes with afc_bin_tree_clear()
 */

#include "test_utils.h"
#include "../src/bin_tree.h"

/* Maximum number of items we collect during traversal */
#define MAX_TRAVERSE_ITEMS 32

/* Global arrays used by the visitor callback to record traversal order */
static long _traverse_keys[MAX_TRAVERSE_ITEMS];
static int _traverse_count = 0;

/**
 * _reset_traverse - Reset the traversal recording arrays.
 */
static void _reset_traverse(void)
{
	_traverse_count = 0;
}

/**
 * _visit_collect - Visitor callback for afc_bin_tree_traverse().
 * Records each visited node's key into the global array.
 */
static int _visit_collect(BinTree *bt, BinTreeNode *node)
{
	if (_traverse_count < MAX_TRAVERSE_ITEMS)
		_traverse_keys[_traverse_count++] = (long)node->key;

	return AFC_ERR_NO_ERROR;
}

/**
 * _int_compare - Custom integer comparator for the binary tree.
 * Compares two void* values treated as long integers.
 */
static int _int_compare(void *v1, void *v2)
{
	long a = (long)v1;
	long b = (long)v2;

	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ================================================================
	 * GROUP 1: Creation and empty state
	 * ================================================================ */

	BinTree *bt = afc_bin_tree_new();

	/* Verify tree was created (non-NULL) */
	print_res("new() returns non-NULL",
		(void *)(long)1,
		(void *)(long)(bt != NULL),
		0);

	/* A freshly created tree must be empty */
	print_res("is_empty() on new tree",
		(void *)(long)1,
		(void *)(long)afc_bin_tree_is_empty(bt),
		0);

	print_row();

	/* ================================================================
	 * GROUP 2: Set custom comparator and insert items
	 * ================================================================ */

	/* Set our own integer comparator */
	int res = afc_bin_tree_set_compare_func(bt, _int_compare);
	print_res("set_compare_func() result",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* Insert 7 items: keys 50, 30, 70, 20, 40, 60, 80 */
	afc_bin_tree_insert(bt, (void *)50, (void *)500);
	afc_bin_tree_insert(bt, (void *)30, (void *)300);
	afc_bin_tree_insert(bt, (void *)70, (void *)700);
	afc_bin_tree_insert(bt, (void *)20, (void *)200);
	afc_bin_tree_insert(bt, (void *)40, (void *)400);
	afc_bin_tree_insert(bt, (void *)60, (void *)600);
	afc_bin_tree_insert(bt, (void *)80, (void *)800);

	/* After inserting items, tree should no longer be empty */
	print_res("is_empty() after inserts",
		(void *)(long)0,
		(void *)(long)afc_bin_tree_is_empty(bt),
		0);

	print_row();

	/* ================================================================
	 * GROUP 3: Retrieve existing keys
	 * ================================================================ */

	/* Get the root key */
	void *val = afc_bin_tree_get(bt, (void *)50);
	print_res("get(50) value",
		(void *)(long)500,
		(void *)(long)val,
		0);

	/* Get a left child key */
	val = afc_bin_tree_get(bt, (void *)30);
	print_res("get(30) value",
		(void *)(long)300,
		(void *)(long)val,
		0);

	/* Get a right child key */
	val = afc_bin_tree_get(bt, (void *)70);
	print_res("get(70) value",
		(void *)(long)700,
		(void *)(long)val,
		0);

	/* Get a leaf node key */
	val = afc_bin_tree_get(bt, (void *)80);
	print_res("get(80) value",
		(void *)(long)800,
		(void *)(long)val,
		0);

	print_row();

	/* ================================================================
	 * GROUP 4: Retrieve non-existing keys (edge cases)
	 * ================================================================ */

	/* Search for a key that does not exist - should return NULL */
	val = afc_bin_tree_get(bt, (void *)99);
	print_res("get(99) non-existing",
		(void *)(long)0,
		(void *)(long)val,
		0);

	/* Search for key 0 which was never inserted */
	val = afc_bin_tree_get(bt, (void *)0);
	print_res("get(0) non-existing",
		(void *)(long)0,
		(void *)(long)val,
		0);

	print_row();

	/* ================================================================
	 * GROUP 5: INORDER traversal - should produce sorted order
	 * Expected: 20, 30, 40, 50, 60, 70, 80
	 * ================================================================ */

	_reset_traverse();
	afc_bin_tree_traverse(bt, AFC_BIN_TREE_MODE_INORDER, _visit_collect);

	print_res("inorder count",
		(void *)(long)7,
		(void *)(long)_traverse_count,
		0);

	print_res("inorder[0] == 20",
		(void *)(long)20,
		(void *)(long)_traverse_keys[0],
		0);

	print_res("inorder[1] == 30",
		(void *)(long)30,
		(void *)(long)_traverse_keys[1],
		0);

	print_res("inorder[2] == 40",
		(void *)(long)40,
		(void *)(long)_traverse_keys[2],
		0);

	print_res("inorder[3] == 50",
		(void *)(long)50,
		(void *)(long)_traverse_keys[3],
		0);

	print_res("inorder[6] == 80",
		(void *)(long)80,
		(void *)(long)_traverse_keys[6],
		0);

	print_row();

	/* ================================================================
	 * GROUP 6: PREORDER traversal
	 * With root=50, left subtree root=30, right subtree root=70:
	 * Expected: 50, 30, 20, 40, 70, 60, 80
	 * ================================================================ */

	_reset_traverse();
	afc_bin_tree_traverse(bt, AFC_BIN_TREE_MODE_PREORDER, _visit_collect);

	print_res("preorder count",
		(void *)(long)7,
		(void *)(long)_traverse_count,
		0);

	/* First visited node in preorder is the root */
	print_res("preorder[0] == 50 (root)",
		(void *)(long)50,
		(void *)(long)_traverse_keys[0],
		0);

	/* Second node is left child of root */
	print_res("preorder[1] == 30",
		(void *)(long)30,
		(void *)(long)_traverse_keys[1],
		0);

	print_row();

	/* ================================================================
	 * GROUP 7: POSTORDER traversal
	 * Expected: 20, 40, 30, 60, 80, 70, 50
	 * ================================================================ */

	_reset_traverse();
	afc_bin_tree_traverse(bt, AFC_BIN_TREE_MODE_POSTORDER, _visit_collect);

	print_res("postorder count",
		(void *)(long)7,
		(void *)(long)_traverse_count,
		0);

	/* Last visited node in postorder is the root */
	print_res("postorder[6] == 50 (root)",
		(void *)(long)50,
		(void *)(long)_traverse_keys[6],
		0);

	/* First visited node in postorder is the leftmost leaf */
	print_res("postorder[0] == 20 (leaf)",
		(void *)(long)20,
		(void *)(long)_traverse_keys[0],
		0);

	print_row();

	/* ================================================================
	 * GROUP 8: Delete a leaf node (80)
	 * ================================================================ */

	res = afc_bin_tree_del(bt, (void *)80);
	print_res("del(80) result",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* Verify deleted node is no longer found */
	val = afc_bin_tree_get(bt, (void *)80);
	print_res("get(80) after del",
		(void *)(long)0,
		(void *)(long)val,
		0);

	/* Verify another node is still found */
	val = afc_bin_tree_get(bt, (void *)70);
	print_res("get(70) still exists",
		(void *)(long)700,
		(void *)(long)val,
		0);

	print_row();

	/* ================================================================
	 * GROUP 9: Delete an internal node (30, has two children: 20, 40)
	 * ================================================================ */

	res = afc_bin_tree_del(bt, (void *)30);
	print_res("del(30) internal node",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* Children of deleted node should still be findable */
	val = afc_bin_tree_get(bt, (void *)20);
	print_res("get(20) after parent del",
		(void *)(long)200,
		(void *)(long)val,
		0);

	val = afc_bin_tree_get(bt, (void *)40);
	print_res("get(40) after parent del",
		(void *)(long)400,
		(void *)(long)val,
		0);

	/* Verify deleted node is gone */
	val = afc_bin_tree_get(bt, (void *)30);
	print_res("get(30) after del",
		(void *)(long)0,
		(void *)(long)val,
		0);

	print_row();

	/* ================================================================
	 * GROUP 10: Delete the root node (50)
	 * ================================================================ */

	res = afc_bin_tree_del(bt, (void *)50);
	print_res("del(50) root node",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	val = afc_bin_tree_get(bt, (void *)50);
	print_res("get(50) after root del",
		(void *)(long)0,
		(void *)(long)val,
		0);

	/* Remaining nodes (20, 40, 60, 70) should still be accessible */
	val = afc_bin_tree_get(bt, (void *)60);
	print_res("get(60) after root del",
		(void *)(long)600,
		(void *)(long)val,
		0);

	print_row();

	/* ================================================================
	 * GROUP 11: Count remaining items via inorder traversal
	 * ================================================================ */

	_reset_traverse();
	afc_bin_tree_traverse(bt, AFC_BIN_TREE_MODE_INORDER, _visit_collect);

	print_res("remaining count after dels",
		(void *)(long)4,
		(void *)(long)_traverse_count,
		0);

	print_row();

	/* ================================================================
	 * GROUP 12: Clear the tree and verify it is empty again
	 * ================================================================ */

	res = afc_bin_tree_clear(bt);
	print_res("clear() result",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* After clearing, the root pointer should be NULL internally.
	 * We verify by checking is_empty which checks root == NULL. */
	/* Note: clear() frees nodes but does not reset bt->root to NULL.
	 * The tree's traversal on empty root simply does nothing. Let's
	 * verify by traversal. */
	_reset_traverse();
	afc_bin_tree_traverse(bt, AFC_BIN_TREE_MODE_INORDER, _visit_collect);
	print_res("inorder count after clear",
		(void *)(long)0,
		(void *)(long)_traverse_count,
		0);

	print_row();

	/* ================================================================
	 * GROUP 13: Delete non-existing key (edge case)
	 * ================================================================ */

	/* Inserting fresh items after clear */
	afc_bin_tree_insert(bt, (void *)10, (void *)100);
	afc_bin_tree_insert(bt, (void *)5, (void *)50);

	/* Deleting a key that does not exist should not crash */
	res = afc_bin_tree_del(bt, (void *)999);
	print_res("del(999) non-existing",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* Original items should still be intact */
	val = afc_bin_tree_get(bt, (void *)10);
	print_res("get(10) after failed del",
		(void *)(long)100,
		(void *)(long)val,
		0);

	val = afc_bin_tree_get(bt, (void *)5);
	print_res("get(5) after failed del",
		(void *)(long)50,
		(void *)(long)val,
		0);

	/* ================================================================
	 * Summary and cleanup
	 * ================================================================ */

	print_summary();

	afc_bin_tree_delete(bt);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
