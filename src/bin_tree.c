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
#include "bin_tree.h"

static const char class_name [] = "Binary Tree";

static int afc_bin_tree_int_compare ( void * v1, void * v2 );
static BinTreeNode * afc_bin_tree_int_create_node ( void * key, void * val );
static BinTreeNode * afc_bin_tree_int_insert ( BinTree * bt, BinTreeNode * root, void * key, void * val );
static int afc_bin_tree_int_inorder ( BinTree * bt, BinTreeNode * root, int (*visit) ( BinTree * bt, BinTreeNode * ) );
static int afc_bin_tree_int_preorder ( BinTree * bt, BinTreeNode * root, int (*visit) ( BinTree * bt, BinTreeNode * ) );
static int afc_bin_tree_int_postorder ( BinTree * bt, BinTreeNode * root, int (*visit) ( BinTree * bt, BinTreeNode * ) );

static int afc_bin_tree_int_del_key ( BinTree * bt, BinTreeNode * * root, BinTreeNode * * keypos, void * key );
static int afc_bin_tree_int_del_node ( BinTree * bt, BinTreeNode * * p );
static int afc_bin_tree_int_node_delete ( BinTree * bt, BinTreeNode * n );

BinTree * afc_bin_tree_new ( void )
{
TRY ( BinTree * )

	BinTree * bt = afc_malloc ( sizeof ( BinTree ) );

	if ( bt == NULL ) RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "bin_tree", NULL );

	bt->magic = AFC_BIN_TREE_MAGIC;
	bt->compare = afc_bin_tree_int_compare;

	RETURN ( bt );

EXCEPT
	afc_bin_tree_delete ( bt );

FINALLY

ENDTRY
}

int afc_bin_tree_clear ( BinTree * bt )
{
	if ( bt == NULL ) return ( AFC_LOG_FAST ( AFC_ERR_NULL_POINTER ) );
	if ( bt->magic != AFC_BIN_TREE_MAGIC ) return ( AFC_LOG_FAST ( AFC_ERR_INVALID_POINTER ) );

	return ( afc_bin_tree_int_postorder ( bt, bt->root, afc_bin_tree_int_node_delete ) );
}

int _afc_bin_tree_delete ( BinTree * bt )
{
	int res;

	if ( ( res = afc_bin_tree_clear ( bt ) ) != AFC_ERR_NO_ERROR ) return ( res );

	afc_free ( bt );

	return ( AFC_ERR_NO_ERROR );
}


// Returns TRUE if the TREE is EMPTY
short afc_bin_tree_is_empty ( BinTree * bt )
{
	return ( bt->root == NULL );
}

// Searches the tree for the key "key". If found, returns the value of key
// if not found, returns "NULL"
void * afc_bin_tree_get ( BinTree * bt, void * key )
{
	BinTreeNode * pos;
	int res;

	if ( ( pos = bt->root ) == NULL ) return ( NULL );

	while ( pos && ( ( res = bt->compare ( key, pos->key ) ) != 0 ) )
	{
		if ( res < 0 )
			pos = pos->left;
		else
			pos = pos->right;
	}

	bt->curr_pos = pos;

	if ( pos ) return ( pos->val );

	return ( NULL );	
}

// Insert a new node
int afc_bin_tree_insert ( BinTree * bt, void * key, void * val )
{
	BinTreeNode * n;

	n = afc_bin_tree_int_insert ( bt, bt->root, key, val );

	if ( bt->root == NULL ) bt->root = n;

	return ( AFC_ERR_NO_ERROR );
}

int afc_bin_tree_traverse ( BinTree * bt, int mode, int ( *visit ) ( BinTree *, BinTreeNode *  ) )
{
	switch ( mode )
	{
		case AFC_BIN_TREE_MODE_INORDER:   return ( afc_bin_tree_int_inorder ( bt, bt->root, visit ) );
		case AFC_BIN_TREE_MODE_PREORDER:  return ( afc_bin_tree_int_preorder ( bt, bt->root, visit ) );
		case AFC_BIN_TREE_MODE_POSTORDER: return ( afc_bin_tree_int_postorder ( bt, bt->root, visit ) );
	}

	return ( AFC_BIN_TREE_ERR_INVALID_MODE );
}

int afc_bin_tree_del ( BinTree * bt, void * key )
{
	BinTreeNode * keypos;

	return ( afc_bin_tree_int_del_key ( bt, & bt->root, & keypos, key ) );
}

int afc_bin_tree_set_compare_func ( BinTree * bt, int ( *comp ) ( void * val1, void * val2 ) )
{
	bt->compare = comp;
	return ( AFC_ERR_NO_ERROR );
}

int afc_bin_tree_set_clear_func ( BinTree * bt, void ( *clear ) ( void * key, void * val ) )
{
	bt->freenode = clear;

	return ( AFC_ERR_NO_ERROR );
}

// ===================================================================================================
// INTERNAL FUNCTIONS
// ===================================================================================================

static int afc_bin_tree_int_compare ( void * v1, void * v2 )
{
	if ( ( long int ) v1 < ( long int ) v2 ) return ( -1 );
	if ( ( long int ) v1 > ( long int ) v2 ) return ( 1 );

	return ( 0 );
}

static BinTreeNode * afc_bin_tree_int_create_node ( void * key, void * val )
{
	BinTreeNode * n;

	if ( ( n = afc_malloc ( sizeof ( BinTreeNode ) ) ) == NULL )
	{
		AFC_LOG_FAST ( AFC_ERR_NO_MEMORY );
		return ( NULL );
	}
	
	n->key = key;
	n->val = val;

	return ( n );	
}

static BinTreeNode * afc_bin_tree_int_insert ( BinTree * bt, BinTreeNode * root, void * key, void * val )
{
	if ( root == NULL )
		root = afc_bin_tree_int_create_node ( key, val );
	else
	{
		if ( bt->compare ( key, root->key ) < 0 )
			root->left = afc_bin_tree_int_insert ( bt, root->left, key, val );
		else
			root->right = afc_bin_tree_int_insert ( bt, root->right, key, val );
	}

	return ( root );
}

static int afc_bin_tree_int_inorder ( BinTree * bt, BinTreeNode * root, int (*visit) ( BinTree *, BinTreeNode * ) )
{
	if ( root )
	{
		afc_bin_tree_int_inorder ( bt, root->left, visit );
		visit ( bt, root );
		afc_bin_tree_int_inorder ( bt, root->right, visit );
	}

	return ( AFC_ERR_NO_ERROR );
}

static int afc_bin_tree_int_preorder ( BinTree * bt, BinTreeNode * root, int (*visit) ( BinTree *, BinTreeNode * ) )
{
	if ( root )
	{
		visit ( bt, root );
		afc_bin_tree_int_preorder ( bt, root->left, visit );
		afc_bin_tree_int_preorder ( bt, root->right, visit );
	}

	return ( AFC_ERR_NO_ERROR );
}

static int afc_bin_tree_int_postorder ( BinTree * bt, BinTreeNode * root, int (*visit) ( BinTree *, BinTreeNode * ) )
{
	if ( root )
	{
		afc_bin_tree_int_postorder ( bt, root->left, visit );
		afc_bin_tree_int_postorder ( bt, root->right, visit );
		visit ( bt, root );
	}

	return ( AFC_ERR_NO_ERROR );
}

static int afc_bin_tree_int_del_key ( BinTree * bt, BinTreeNode * * root, BinTreeNode * * keypos, void * key )
{
	int res;

	if ( * root == NULL ) return ( AFC_ERR_NO_ERROR );

	res = bt->compare ( key, (* root)->key );

	if ( res == 0 )
	{
		* keypos = * root;
		afc_bin_tree_int_del_node ( bt, root );
	} else if ( res < 0 )
		afc_bin_tree_int_del_key ( bt,  &( *root )->left, keypos, key );
	else
		afc_bin_tree_int_del_key ( bt,  &( *root )->right, keypos, key );

	return ( AFC_ERR_NO_ERROR );
}

static int afc_bin_tree_int_del_node ( BinTree * bt, BinTreeNode * * p )
{
	BinTreeNode * r = * p, * q;

	if ( r == NULL ) return ( AFC_ERR_NO_ERROR );

	if ( r->right == NULL )
	{
		*p = r->left;
		afc_bin_tree_int_node_delete ( bt, r );
	} else if ( r->left == NULL ) {
		*p = r->right;
		afc_bin_tree_int_node_delete ( bt, r );
	} else {
		for ( q = r->right; q->left; q = q->left ) ;

		q->left = r->left;
		*p = r->right;

		afc_bin_tree_int_node_delete ( bt, r );
	}

	return ( AFC_ERR_NO_ERROR );
}

static int afc_bin_tree_int_node_delete ( BinTree * bt, BinTreeNode * n )
{
	if ( n == NULL ) return ( AFC_ERR_NO_ERROR );

	if ( bt->freenode ) bt->freenode ( n->key, n->val );
	afc_free ( n );

	return ( AFC_ERR_NO_ERROR );
}

#ifdef TEST_CLASS
int visit ( BinTree * bt, BinTreeNode * node )
{
	printf ( "Key: %d - Val: %d\n", ( int ) node->key, ( int ) node->val );

	return ( AFC_ERR_NO_ERROR );
}

void freenode ( void * key, void * val )
{
	printf ( "Free: key: %d - %d\n", ( int ) key, ( int ) val );
}

int main ( int argc, char * argv [] )
{
	AFC * afc;
	BinTree * bt;

	afc = afc_new ();

	afc_track_mallocs ( afc );

	afc_set_tags ( afc, AFC_TAG_LOG_LEVEL, AFC_LOG_WARNING, AFC_TAG_END );

	// afc_malloc ( 100 );

	bt = afc_bin_tree_new ();

	afc_bin_tree_set_clear_func ( bt, freenode );

	afc_bin_tree_insert ( bt, ( void * ) 1, ( void * ) 1 );
	afc_bin_tree_insert ( bt, ( void * ) 10, ( void * ) 10 );
	afc_bin_tree_insert ( bt, ( void * ) 5, ( void * ) 5 );
	afc_bin_tree_insert ( bt, ( void * ) 7, ( void * ) 7 );
	afc_bin_tree_insert ( bt, ( void * ) 6, ( void * ) 6 );
	afc_bin_tree_insert ( bt, ( void * ) 11, ( void * ) 11 );
	afc_bin_tree_insert ( bt, ( void * ) 6, ( void * ) 6 );

	afc_bin_tree_del ( bt, ( void * ) 5 );

	printf ( "In:\n" );
	afc_bin_tree_traverse ( bt, AFC_BIN_TREE_MODE_INORDER, visit );

	printf ( "Pre:\n" );
	afc_bin_tree_traverse ( bt, AFC_BIN_TREE_MODE_PREORDER, visit );

	printf ( "Post:\n" );
	afc_bin_tree_traverse ( bt, AFC_BIN_TREE_MODE_POSTORDER, visit );

	afc_bin_tree_delete ( bt );
	afc_delete ( afc );
	return  ( 0 );
}
#endif
