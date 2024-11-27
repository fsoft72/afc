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
#include "tree.h"

static const char class_name[] = "Tree";

// ===================================================================================================
// INTERNAL FUNCTIONS
// ===================================================================================================

static int afc_tree_int_compare(void *v1, void *v2)
{
	if ((long int)v1 < (long int)v2)
		return (-1);
	if ((long int)v1 > (long int)v2)
		return (1);

	return (0);
}

static TreeNode *afc_tree_int_node_delete(TreeNode *n)
{
	Tree *tree;
	TreeNode *next;
	TreeNode *parent;

	if (n == NULL)
		return (NULL);

	tree = n->tree;
	next = n->list_next;
	parent = n->parent;

	if (tree->first == n)
		tree->first = next;
	if (tree->last == n)
		tree->last = n->list_prev;

	if (n->list_prev)
		n->list_prev->list_next = n->list_next;
	if (n->list_next)
		n->list_next->list_prev = n->list_prev;

	if (parent)
	{
		if (parent->last_child == n)
		{
			parent->last_child = n->l_sibling;
		}
		if (parent->child == n)
		{
			parent->child = n->r_sibling;
		}
	}

	if (n->l_sibling)
	{
		n->l_sibling->r_sibling = n->r_sibling;
	}
	if (n->r_sibling)
	{
		n->r_sibling->l_sibling = n->l_sibling;
	}

	if (tree->freenode)
		tree->freenode(n->val);
	afc_free(n);

	return (next);
}

static TreeNode *afc_tree_int_node_new(Tree *tree, void *val)
{
	TreeNode *n = afc_malloc(sizeof(TreeNode));
	n->tree = tree;
	n->val = val;
	return n;
}

static TreeNode *afc_tree_int_append_child(TreeNode *parent, TreeNode *child)
{
	child->parent = parent;

	if (parent->last_child)
	{
		child->l_sibling = parent->last_child;
		parent->last_child->r_sibling = child;
	}
	else
		parent->child = child;

	parent->last_child = child;

	return child;
}

// ===================================================================================================
// CLASS FUNCTIONS
// ===================================================================================================
Tree *afc_tree_new(void)
{
	TRY(Tree *)
	Tree *t;

	t = afc_malloc(sizeof(Tree));
	if (t == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "tree", NULL);

	t->magic = AFC_TREE_MAGIC;
	t->compare = afc_tree_int_compare;

	RETURN(t);

	EXCEPT
	afc_tree_delete(t);

	FINALLY

	ENDTRY
}

int _afc_tree_delete(Tree *t)
{
	int res;

	if ((res = afc_tree_clear(t)) != AFC_ERR_NO_ERROR)
		return (res);

	afc_free(t);

	return (AFC_ERR_NO_ERROR);
}

int afc_tree_clear(Tree *tree)
{
	TreeNode *current_node;

	if (tree == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));
	if (tree->magic != AFC_TREE_MAGIC)
		return (AFC_LOG_FAST(AFC_ERR_INVALID_POINTER));

	for (current_node = tree->first; current_node != NULL; current_node = afc_tree_int_node_delete(current_node))
		;

	tree->first = NULL;
	tree->last = NULL;
	tree->current = NULL;

	return (AFC_ERR_NO_ERROR);
}

TreeNode *afc_tree_insert(Tree *tree, void *val)
{
	if (tree->last == NULL)
		return tree->first = tree->last = afc_tree_int_node_new(tree, val);
	else
		return afc_subtree_insert_sibling(tree->first, val);
}

TreeNode *afc_subtree_insert_sibling(TreeNode *brother, void *val)
{
	Tree *tree;

	if (brother == NULL)
		return (NULL);
	tree = brother->tree;

	if (tree == NULL)
		return (NULL);
	if (tree->magic != AFC_TREE_MAGIC)
		return (NULL);

	TreeNode *sibling = afc_tree_int_node_new(tree, val);

	sibling->l_sibling = brother;
	if (brother->r_sibling)
	{
		sibling->r_sibling = brother->r_sibling;
		brother->r_sibling->l_sibling = sibling;
	}
	else if (brother->parent)
	{
		brother->parent->last_child = sibling;
	}
	brother->r_sibling = sibling;
	sibling->parent = brother->parent;

	sibling->list_prev = tree->last;
	tree->last = tree->last->list_next = sibling;

	return sibling;
}

TreeNode *afc_subtree_insert_child(TreeNode *parent, void *val)
{
	Tree *tree;

	if (parent == NULL)
		return (NULL);
	tree = parent->tree;

	if (tree == NULL)
		return (NULL);
	if (tree->magic != AFC_TREE_MAGIC)
		return (NULL);

	TreeNode *child = afc_tree_int_node_new(tree, val);

	afc_tree_int_append_child(parent, child);

	child->list_prev = tree->last;
	tree->last = tree->last->list_next = child;

	return child;
}

int afc_tree_node_preorder_visit(TreeNode *parent, int (*visitor)(TreeNode *))
{
	TreeNode *curr_node;

	if (!parent)
		return AFC_ERR_INVALID_POINTER;

	visitor(parent);

	for (curr_node = parent->child; curr_node != NULL; curr_node = curr_node->r_sibling)
		afc_tree_node_preorder_visit(curr_node, visitor);

	return AFC_ERR_NO_ERROR;
}

int afc_tree_node_postorder_visit(TreeNode *parent, int (*visitor)(TreeNode *))
{
	TreeNode *curr_node;

	if (!parent)
		return AFC_ERR_INVALID_POINTER;

	for (curr_node = parent->child; curr_node != NULL; curr_node = curr_node->r_sibling)
		afc_tree_node_postorder_visit(curr_node, visitor);

	visitor(parent);

	return AFC_ERR_NO_ERROR;
}

int afc_tree_traverse(Tree *t, int mode, int (*visitor)(TreeNode *))
{
	TreeNode *curr_node;

	if (t == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));
	if (t->magic != AFC_TREE_MAGIC)
		return (AFC_LOG_FAST(AFC_ERR_INVALID_POINTER));

	for (curr_node = t->first; curr_node != NULL; curr_node = curr_node->r_sibling)
		afc_subtree_traverse(curr_node, mode, visitor);

	return AFC_ERR_NO_ERROR;
}

int afc_subtree_traverse(TreeNode *n, int mode, int (*visitor)(TreeNode *))
{
	if (n == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));

	switch (mode)
	{
	case AFC_TREE_MODE_PREORDER:
		return (afc_tree_node_preorder_visit(n, visitor));
	case AFC_TREE_MODE_POSTORDER:
		return (afc_tree_node_postorder_visit(n, visitor));
	}

	return (AFC_TREE_ERR_INVALID_MODE);
}

short afc_tree_is_empty(Tree *tree)
{
	return (tree->first == NULL);
}

int _afc_subtree_delete(TreeNode *subtree)
{
	if (subtree == NULL)
		return (AFC_LOG_FAST(AFC_ERR_INVALID_POINTER));

	if (subtree->parent)
	{
		if (subtree->parent->last_child == subtree)
			subtree->parent->last_child = subtree->l_sibling;
		if (subtree->parent->child == subtree)
			subtree->parent->child = subtree->r_sibling;
	}

	return afc_subtree_traverse(subtree, AFC_TREE_MODE_POSTORDER, (int (*)(TreeNode *))afc_tree_int_node_delete);
}

#ifdef TEST_CLASS

#include <stdio.h>

int visitor(TreeNode *n)
{
	printf("%ld\n", (long int)n->val);
	return AFC_ERR_NO_ERROR;
}

int main()
{
	AFC *afc;
	Tree *t;
	TreeNode *first_head;

	afc = afc_new();
	afc_track_mallocs(afc);
	afc_set_tags(afc, AFC_TAG_LOG_LEVEL, AFC_LOG_WARNING, AFC_TAG_END);

	t = afc_tree_new();
	first_head = afc_tree_insert(t, 5);
	afc_subtree_insert_child(first_head, 6);
	afc_tree_insert(t, 1);
	afc_tree_traverse(t, AFC_TREE_MODE_POSTORDER, visitor);
	afc_subtree_delete(first_head);

	printf("----------\n");

	afc_subtree_insert_child(afc_tree_insert(t, 100), 10);
	afc_tree_traverse(t, AFC_TREE_MODE_PREORDER, visitor);

	afc_tree_delete(t);

	afc_delete(afc);
	return (0);
}

#endif
