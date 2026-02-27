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
 * test_tree.c - Comprehensive tests for the Tree (generic tree) class.
 *
 * The Tree class implements a multi-way tree where each node can
 * have multiple children and siblings. Nodes are linked in both
 * a tree hierarchy and a flat insertion-order list.
 *
 * Tests cover:
 *   - Creation and deletion
 *   - afc_tree_is_empty() on new and populated trees
 *   - afc_tree_insert() for root-level nodes
 *   - afc_subtree_insert_child() for adding children
 *   - afc_subtree_insert_sibling() for adding siblings
 *   - afc_tree_traverse() in PREORDER and POSTORDER modes
 *   - afc_subtree_delete() for removing a subtree
 *   - afc_tree_clear() for removing all nodes
 *   - Edge cases and structure verification
 */

#include "test_utils.h"
#include "../src/tree.h"

/* Maximum number of items we collect during traversal */
#define MAX_TRAVERSE_ITEMS 32

/* Global arrays used by the visitor callback to record traversal order */
static long _traverse_vals[MAX_TRAVERSE_ITEMS];
static int _traverse_count = 0;

/**
 * _reset_traverse - Reset the traversal recording arrays.
 */
static void _reset_traverse(void)
{
	_traverse_count = 0;
}

/**
 * _visitor - Visitor callback for afc_tree_traverse().
 * Records each visited node's val into the global array.
 */
static int _visitor(TreeNode *n)
{
	if (_traverse_count < MAX_TRAVERSE_ITEMS)
		_traverse_vals[_traverse_count++] = (long)n->val;

	return AFC_ERR_NO_ERROR;
}

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ================================================================
	 * GROUP 1: Creation and empty state
	 * ================================================================ */

	Tree *t = afc_tree_new();

	/* Verify tree was created (non-NULL) */
	print_res("new() returns non-NULL",
		(void *)(long)1,
		(void *)(long)(t != NULL),
		0);

	/* A freshly created tree must be empty */
	print_res("is_empty() on new tree",
		(void *)(long)1,
		(void *)(long)afc_tree_is_empty(t),
		0);

	print_row();

	/* ================================================================
	 * GROUP 2: Insert root-level nodes
	 *
	 * We build this tree structure:
	 *
	 *   Root level:  5 --- 1 --- 8
	 *                |           |
	 *   Children:   6  7       9  12
	 *               |
	 *              11
	 *
	 * Insert order:
	 *   5 (first root), child 6, child 7,
	 *   sibling 1, sibling 8,
	 *   child of 6: 11, child of 8: 9, child of 8: 12
	 * ================================================================ */

	/* Insert first root node with value 5 */
	TreeNode *node_5 = afc_tree_insert(t, (void *)(long)5);
	print_res("insert(5) non-NULL",
		(void *)(long)1,
		(void *)(long)(node_5 != NULL),
		0);

	/* Tree should no longer be empty */
	print_res("is_empty() after insert",
		(void *)(long)0,
		(void *)(long)afc_tree_is_empty(t),
		0);

	/* Verify the node's value */
	print_res("node_5 val == 5",
		(void *)(long)5,
		(void *)(long)node_5->val,
		0);

	print_row();

	/* ================================================================
	 * GROUP 3: Insert children and siblings
	 * ================================================================ */

	/* Add child 6 under node 5 */
	TreeNode *node_6 = afc_subtree_insert_child(node_5, (void *)(long)6);
	print_res("insert_child(5, 6) non-NULL",
		(void *)(long)1,
		(void *)(long)(node_6 != NULL),
		0);

	/* Verify parent relationship */
	print_res("node_6 parent == node_5",
		(void *)(long)1,
		(void *)(long)(node_6->parent == node_5),
		0);

	/* Add child 7 under node 5 (sibling of 6) */
	TreeNode *node_7 = afc_subtree_insert_child(node_5, (void *)(long)7);
	print_res("insert_child(5, 7) non-NULL",
		(void *)(long)1,
		(void *)(long)(node_7 != NULL),
		0);

	/* 7 should be a sibling of 6 */
	print_res("node_6 r_sibling == node_7",
		(void *)(long)1,
		(void *)(long)(node_6->r_sibling == node_7),
		0);

	/* Add root-level sibling 1 (sibling of 5) */
	TreeNode *node_1 = afc_tree_insert(t, (void *)(long)1);
	print_res("insert(1) sibling non-NULL",
		(void *)(long)1,
		(void *)(long)(node_1 != NULL),
		0);

	/* Add root-level sibling 8 */
	TreeNode *node_8 = afc_tree_insert(t, (void *)(long)8);
	print_res("insert(8) sibling non-NULL",
		(void *)(long)1,
		(void *)(long)(node_8 != NULL),
		0);

	/* Add child 11 under node 6 (grandchild of 5) */
	TreeNode *node_11 = afc_subtree_insert_child(node_6, (void *)(long)11);
	print_res("insert_child(6, 11) non-NULL",
		(void *)(long)1,
		(void *)(long)(node_11 != NULL),
		0);

	/* Add children 9 and 12 under node 8 */
	TreeNode *node_9 = afc_subtree_insert_child(node_8, (void *)(long)9);
	print_res("insert_child(8, 9) non-NULL",
		(void *)(long)1,
		(void *)(long)(node_9 != NULL),
		0);

	TreeNode *node_12 = afc_subtree_insert_child(node_8, (void *)(long)12);
	print_res("insert_child(8, 12) non-NULL",
		(void *)(long)1,
		(void *)(long)(node_12 != NULL),
		0);

	print_row();

	/* ================================================================
	 * GROUP 4: Structure verification
	 * ================================================================ */

	/* node_5 should be the first node in the tree */
	print_res("first node is 5",
		(void *)(long)5,
		(void *)(long)t->first->val,
		0);

	/* node_5's first child should be 6 */
	print_res("5's first child is 6",
		(void *)(long)6,
		(void *)(long)node_5->child->val,
		0);

	/* node_5's last child should be 7 */
	print_res("5's last child is 7",
		(void *)(long)7,
		(void *)(long)node_5->last_child->val,
		0);

	/* node_8's first child should be 9 */
	print_res("8's first child is 9",
		(void *)(long)9,
		(void *)(long)node_8->child->val,
		0);

	/* node_8's last child should be 12 */
	print_res("8's last child is 12",
		(void *)(long)12,
		(void *)(long)node_8->last_child->val,
		0);

	/* node_6's child should be 11 */
	print_res("6's child is 11",
		(void *)(long)11,
		(void *)(long)node_6->child->val,
		0);

	/* 11 should have no children */
	print_res("11 has no children",
		(void *)(long)1,
		(void *)(long)(node_11->child == NULL),
		0);

	/* 1 should have no children */
	print_res("1 has no children",
		(void *)(long)1,
		(void *)(long)(node_1->child == NULL),
		0);

	print_row();

	/* ================================================================
	 * GROUP 5: PREORDER traversal
	 *
	 * Tree:
	 *   5 --- 1 --- 8
	 *   |           |
	 *   6  7       9  12
	 *   |
	 *  11
	 *
	 * Preorder visits root first, then recursively children.
	 * For root-level siblings, each subtree is visited in order.
	 *
	 * Expected: 5, 6, 11, 7, 1, 8, 9, 12
	 * ================================================================ */

	_reset_traverse();
	int res = afc_tree_traverse(t, AFC_TREE_MODE_PREORDER, _visitor);
	print_res("traverse PREORDER result",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("preorder count == 8",
		(void *)(long)8,
		(void *)(long)_traverse_count,
		0);

	/* First visited in preorder should be root 5 */
	print_res("preorder[0] == 5",
		(void *)(long)5,
		(void *)(long)_traverse_vals[0],
		0);

	/* Then child 6 */
	print_res("preorder[1] == 6",
		(void *)(long)6,
		(void *)(long)_traverse_vals[1],
		0);

	/* Then grandchild 11 */
	print_res("preorder[2] == 11",
		(void *)(long)11,
		(void *)(long)_traverse_vals[2],
		0);

	/* Then sibling 7 of 6 */
	print_res("preorder[3] == 7",
		(void *)(long)7,
		(void *)(long)_traverse_vals[3],
		0);

	/* Root sibling 1 */
	print_res("preorder[4] == 1",
		(void *)(long)1,
		(void *)(long)_traverse_vals[4],
		0);

	/* Root sibling 8 */
	print_res("preorder[5] == 8",
		(void *)(long)8,
		(void *)(long)_traverse_vals[5],
		0);

	/* Child 9 of 8 */
	print_res("preorder[6] == 9",
		(void *)(long)9,
		(void *)(long)_traverse_vals[6],
		0);

	/* Child 12 of 8 */
	print_res("preorder[7] == 12",
		(void *)(long)12,
		(void *)(long)_traverse_vals[7],
		0);

	print_row();

	/* ================================================================
	 * GROUP 6: POSTORDER traversal
	 *
	 * Postorder visits children first, then root.
	 * Expected: 11, 6, 7, 5, 1, 9, 12, 8
	 * ================================================================ */

	_reset_traverse();
	res = afc_tree_traverse(t, AFC_TREE_MODE_POSTORDER, _visitor);
	print_res("traverse POSTORDER result",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("postorder count == 8",
		(void *)(long)8,
		(void *)(long)_traverse_count,
		0);

	/* Deepest leaf visited first: 11 */
	print_res("postorder[0] == 11",
		(void *)(long)11,
		(void *)(long)_traverse_vals[0],
		0);

	/* Then its parent 6 after all its children */
	print_res("postorder[1] == 6",
		(void *)(long)6,
		(void *)(long)_traverse_vals[1],
		0);

	/* Then 7 (leaf sibling of 6) */
	print_res("postorder[2] == 7",
		(void *)(long)7,
		(void *)(long)_traverse_vals[2],
		0);

	/* Then the root of first subtree: 5 */
	print_res("postorder[3] == 5",
		(void *)(long)5,
		(void *)(long)_traverse_vals[3],
		0);

	/* 1 is a leaf root */
	print_res("postorder[4] == 1",
		(void *)(long)1,
		(void *)(long)_traverse_vals[4],
		0);

	/* Last visited root: 8 (visited after its children 9, 12) */
	print_res("postorder[7] == 8",
		(void *)(long)8,
		(void *)(long)_traverse_vals[7],
		0);

	print_row();

	/* ================================================================
	 * GROUP 7: Invalid traversal mode
	 * The LEVEL mode traversal calls afc_subtree_traverse which
	 * returns AFC_TREE_ERR_INVALID_MODE for unsupported modes.
	 * However AFC_TREE_MODE_LEVEL is defined. Let's check the
	 * actual implementation handles LEVEL mode. From tree.c,
	 * only PREORDER and POSTORDER are handled in the switch.
	 * ================================================================ */

	_reset_traverse();
	res = afc_tree_traverse(t, AFC_TREE_MODE_LEVEL, _visitor);
	print_res("traverse LEVEL returns ERR",
		(void *)(long)AFC_TREE_ERR_INVALID_MODE,
		(void *)(long)res,
		0);

	print_row();

	/* ================================================================
	 * GROUP 8: Delete a subtree (node_5 and its descendants)
	 *
	 * Before: 5(6(11),7) --- 1 --- 8(9,12)
	 * After:  1 --- 8(9,12)
	 *
	 * Note: After subtree delete, we need to revalidate our
	 * node pointers since node_5, node_6, node_7, node_11 are freed.
	 * ================================================================ */

	afc_subtree_delete(node_5);

	/* Verify tree is not empty (still has nodes 1, 8, 9, 12) */
	print_res("not empty after subtree del",
		(void *)(long)0,
		(void *)(long)afc_tree_is_empty(t),
		0);

	/* Traverse remaining nodes in preorder: expect 1, 8, 9, 12 */
	_reset_traverse();
	afc_tree_traverse(t, AFC_TREE_MODE_PREORDER, _visitor);

	print_res("remaining count after del",
		(void *)(long)4,
		(void *)(long)_traverse_count,
		0);

	print_res("remaining[0] == 1",
		(void *)(long)1,
		(void *)(long)_traverse_vals[0],
		0);

	print_res("remaining[1] == 8",
		(void *)(long)8,
		(void *)(long)_traverse_vals[1],
		0);

	print_res("remaining[2] == 9",
		(void *)(long)9,
		(void *)(long)_traverse_vals[2],
		0);

	print_res("remaining[3] == 12",
		(void *)(long)12,
		(void *)(long)_traverse_vals[3],
		0);

	print_row();

	/* ================================================================
	 * GROUP 9: Add more nodes after subtree deletion to verify
	 * the tree is still functional.
	 * ================================================================ */

	/* Insert a new root-level sibling */
	TreeNode *node_50 = afc_tree_insert(t, (void *)(long)50);
	print_res("insert(50) after del non-NULL",
		(void *)(long)1,
		(void *)(long)(node_50 != NULL),
		0);

	/* Add a child to the new node */
	TreeNode *node_55 = afc_subtree_insert_child(node_50, (void *)(long)55);
	print_res("insert_child(50, 55) non-NULL",
		(void *)(long)1,
		(void *)(long)(node_55 != NULL),
		0);

	/* Preorder now: 1, 8, 9, 12, 50, 55 */
	_reset_traverse();
	afc_tree_traverse(t, AFC_TREE_MODE_PREORDER, _visitor);

	print_res("count after re-insert == 6",
		(void *)(long)6,
		(void *)(long)_traverse_count,
		0);

	print_row();

	/* ================================================================
	 * GROUP 10: Clear the tree and verify emptiness
	 * ================================================================ */

	res = afc_tree_clear(t);
	print_res("clear() result",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_res("is_empty() after clear",
		(void *)(long)1,
		(void *)(long)afc_tree_is_empty(t),
		0);

	/* Traversal on empty tree should visit nothing */
	_reset_traverse();
	afc_tree_traverse(t, AFC_TREE_MODE_PREORDER, _visitor);
	print_res("preorder count after clear",
		(void *)(long)0,
		(void *)(long)_traverse_count,
		0);

	print_row();

	/* ================================================================
	 * GROUP 11: Re-insert after clear to verify reuse
	 * ================================================================ */

	TreeNode *root = afc_tree_insert(t, (void *)(long)99);
	print_res("insert(99) after clear",
		(void *)(long)1,
		(void *)(long)(root != NULL),
		0);

	print_res("is_empty() after re-insert",
		(void *)(long)0,
		(void *)(long)afc_tree_is_empty(t),
		0);

	afc_subtree_insert_child(root, (void *)(long)88);
	afc_subtree_insert_child(root, (void *)(long)77);

	_reset_traverse();
	afc_tree_traverse(t, AFC_TREE_MODE_PREORDER, _visitor);
	print_res("reuse preorder count == 3",
		(void *)(long)3,
		(void *)(long)_traverse_count,
		0);

	print_res("reuse preorder[0] == 99",
		(void *)(long)99,
		(void *)(long)_traverse_vals[0],
		0);

	print_res("reuse preorder[1] == 88",
		(void *)(long)88,
		(void *)(long)_traverse_vals[1],
		0);

	print_res("reuse preorder[2] == 77",
		(void *)(long)77,
		(void *)(long)_traverse_vals[2],
		0);

	/* ================================================================
	 * Summary and cleanup
	 * ================================================================ */

	print_summary();

	afc_tree_delete(t);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
