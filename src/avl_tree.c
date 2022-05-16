/* 
 * Advanced Foundation Classes
 * Copyright (C) 2000/2004  Fabio Rotondo 
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
#include "avl_tree.h"

static const char class_name[] = "AVLTree";

#define _HEIGHT(node)	( node ? node->height : -1 )
#define _MAX(lh,rh)	( lh > rh ? lh : rh )

static int afc_avl_tree_int_clear ( AVLTree * t, AVLNode * node );
static AVLNode * afc_avl_tree_int_single_rotate_with_left ( AVLTree * tree, AVLNode * node2 );
static AVLNode * afc_avl_tree_int_single_rotate_with_right ( AVLTree * tree, AVLNode * node1 );
static AVLNode * afc_avl_tree_int_double_rotate_with_left ( AVLTree * tree, AVLNode * node3 );
static AVLNode * afc_avl_tree_int_double_rotate_with_right ( AVLTree * tree, AVLNode * node1 );
static int afc_avl_tree_int_default_compare ( void * v1, void * v2 );

AVLTree * afc_avl_tree_new ( void )
{
TRY ( AVLTree * )

	AVLTree * avl = NULL;

	if ( ( avl = afc_malloc ( sizeof ( AVLTree ) ) )  == NULL ) RAISE_FAST ( AFC_ERR_NO_MEMORY, "avl tree" );

	avl->comp = afc_avl_tree_int_default_compare;

	RETURN ( avl );
EXCEPT
	afc_avl_tree_delete ( avl );
	RETURN ( NULL );

FINALLY

ENDTRY
}

int _afc_avl_tree_delete ( AVLTree * t )
{
	afc_avl_tree_clear ( t );
	afc_free ( t );

	return ( AFC_ERR_NO_ERROR );
}

int afc_avl_tree_clear ( AVLTree * t )
{
	afc_avl_tree_int_clear ( t, t->root );

	return ( AFC_ERR_NO_ERROR );
}


AVLNode * _afc_avl_tree_find_node ( AVLTree * avl, AVLNode * node, void * x )
{
	int v;

	if ( node == NULL ) return ( NULL );

	v = avl->comp ( x, node->key );

	if ( v < 0 ) return ( _afc_avl_tree_find_node ( avl, node->left, x ) );
	if ( v > 0 ) return ( _afc_avl_tree_find_node ( avl, node->right, x ) );

	return node;
}

AVLNode * _afc_avl_tree_find_node_min ( AVLTree * avl, AVLNode * node )
{
	if ( node != NULL ) while ( node->left != NULL ) node = node->left;

	return ( node );
}

AVLNode * _afc_avl_tree_find_node_max ( AVLTree * avl, AVLNode * node )
{
	if ( node != NULL ) while ( node->right != NULL ) node = node->right;

	return ( node );
}

AVLNode * _afc_avl_tree_insert ( AVLTree * tree, AVLNode * node, void * key, void * val )
{
TRY ( AVLNode * )
	int v;

	if ( node == NULL )
	{
		// Create and return one-node tree

		if ( ( node = afc_malloc ( sizeof ( AVLNode ) ) ) == NULL ) RAISE_FAST ( AFC_ERR_NO_MEMORY, "node" );

		node->key = key;
		node->val = val;
	} else {
		v = tree->comp ( val, node->key );

		if ( v < 0 )
		{
			node->left = _afc_avl_tree_insert ( tree, node->left, key, val );
			if ( ( _HEIGHT ( node->left ) - _HEIGHT ( node->right ) ) == 2 )
			{
				if ( tree->comp ( val, node->left->key ) < 0 )
					node = afc_avl_tree_int_single_rotate_with_left ( tree, node );
				else
					node = afc_avl_tree_int_double_rotate_with_left ( tree, node );
			}
		}

		if ( v > 0 )
		{
			node->right = _afc_avl_tree_insert ( tree, node->right, key, val );
			if ( ( _HEIGHT ( node->right ) -  _HEIGHT ( node->left ) ) == 2 )
			{
				if ( tree->comp ( val, node->right->key ) > 0 )
					node = afc_avl_tree_int_single_rotate_with_right ( tree, node );
				else
					node = afc_avl_tree_int_double_rotate_with_right ( tree, node );
			}
		} 

		// We do nothing if val is already present in the AVL Tree
	}

	node->height = _MAX ( _HEIGHT ( node->left ), _HEIGHT ( node->right ) ) + 1;

	RETURN ( node );
EXCEPT
	RETURN ( NULL );

FINALLY

ENDTRY
}

int afc_avl_tree_set_clear_func ( AVLTree * tree, void (*clear) (void *v) )
{
	tree->clear = clear;

	return ( AFC_ERR_NO_ERROR );
}

void * afc_avl_tree_get ( AVLTree * tree, void * key )
{
	AVLNode * n = _afc_avl_tree_find_node ( tree, tree->root, key );

	if ( n == NULL ) return ( NULL );

	return ( n->val );
}

/* ========================================================================================
	INTERNAL FUNCTIONS
======================================================================================== */

static int afc_avl_tree_int_clear ( AVLTree * t, AVLNode * node )
{
	if ( node != NULL )
    	{
		afc_avl_tree_int_clear ( t, node->left );
		afc_avl_tree_int_clear ( t, node->right );

		// Calls the clear function
		if ( t->clear ) t->clear ( node->val );
		afc_free ( node );
    	}

    	return ( AFC_ERR_NO_ERROR );
}

static AVLNode * afc_avl_tree_int_single_rotate_with_left ( AVLTree * tree, AVLNode * node2 )
{
    	AVLNode * node1;

    	node1 	 = node2->left;
    	node2->left  = node1->right;
    	node1->right = node2;

    	node2->height = _MAX ( _HEIGHT( node2->left ), _HEIGHT( node2->right ) ) + 1;
    	node1->height = _MAX ( _HEIGHT( node1->left ), node2->height ) + 1;

    	return node1;  
}

static AVLNode * afc_avl_tree_int_single_rotate_with_right ( AVLTree * tree, AVLNode * node1 )
{
	AVLNode * node2;

    	node2 	     = node1->right;
    	node1->right = node2->left;
    	node2->left  = node1;

    	node1->height = _MAX ( _HEIGHT ( node1->left ),  _HEIGHT ( node1->right ) ) + 1;
    	node2->height = _MAX ( _HEIGHT ( node2->right ), node1->height ) + 1;

    	return node2;
}

static AVLNode * afc_avl_tree_int_double_rotate_with_left ( AVLTree * tree, AVLNode * node3 )
{
    	/* Rotate between node1 and node2 */
    	node3->left = afc_avl_tree_int_single_rotate_with_right ( tree, node3->left );

    	/* Rotate between node3 and node2 */
    	return afc_avl_tree_int_single_rotate_with_left ( tree, node3 );
}

static AVLNode * afc_avl_tree_int_double_rotate_with_right ( AVLTree * tree, AVLNode * node1 )
{
    	/* Rotate between node3 and node2 */
    	node1->right = afc_avl_tree_int_single_rotate_with_left ( tree, node1->right );

    	/* Rotate between node1 and node2 */
	return afc_avl_tree_int_single_rotate_with_right ( tree, node1 );
}

static int afc_avl_tree_int_default_compare ( void * v1, void * v2 )
{
	if ( v1 > v2 ) return ( 1 );
	if ( v1 < v2 ) return ( -1 );

	return ( 0 );
}

#ifdef TEST_CLASS
void clear_func ( void * s )
{
	afc_string_delete ( s );
}

int main ( int argc, char * argv [] )
{
	AFC * afc = afc_new ();
	AVLTree * tree;
	AVLNode * n;
	int t;
	char * s;

	afc_track_mallocs ( afc );
	tree = afc_avl_tree_new ();

	afc_avl_tree_set_clear_func ( tree, clear_func );

	for ( t = 0; t < 20; t ++ )
	{
		s = afc_string_new ( 20 );
		afc_string_make ( s, "Item: %d", t );

		afc_avl_tree_insert ( tree, ( void * ) t, s );
	}

	for ( t = 0; t < 20; t ++ )
	{
		n = afc_avl_tree_find_node ( tree, ( void * ) t );
		if ( ( int ) n->key  == t )
			printf ( "Found: %d - %s\n", ( int ) n->key, ( char * ) n->val );
		else
			printf ( "Miss: %d - %s\n", ( int ) n->key, ( char * ) n->val );
	}

	n = afc_avl_tree_find_node_min ( tree );
	printf ( "Minimum: %d - %s\n", ( int ) n->key, ( char * ) n->val );

	n = afc_avl_tree_find_node_max ( tree );
	printf ( "Max: %d - %s\n", ( int ) n->key, ( char * ) n->val );

	printf ( "Get: %s\n", ( char * ) afc_avl_tree_get ( tree, ( void * ) 10 ) );

	
#ifdef DEBUG_MEM
	// Just to be sure MemTracker works...
	afc_malloc ( 100 );
#endif

	afc_avl_tree_delete ( tree );
	afc_delete ( afc );

	return ( 0 );
}
#endif
