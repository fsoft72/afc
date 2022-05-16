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
#ifndef AFC_AVL_H
#define AFC_AVL_H

#include "afc.h"

struct afc_avl_node
{
	void * key;

	struct afc_avl_node * left;
	struct afc_avl_node * right;
	int    height;

	void * val;
};

typedef struct afc_avl_node AVLNode;

struct afc_avl_tree
{
	unsigned long magic;

	AVLNode * root;

	int  ( *comp ) ( void * v1, void * v2 );
	void ( *clear ) ( void * v );
};

typedef struct afc_avl_tree AVLTree;

AVLTree * afc_avl_tree_new ( void );
int _afc_avl_tree_delete ( AVLTree * t );
int afc_avl_tree_clear ( AVLTree * t );
int afc_avl_tree_set_clear_func ( AVLTree * tree, void (*clear) (void *v) );
void * afc_avl_tree_get ( AVLTree * tree, void * key );

AVLNode * _afc_avl_tree_find_node ( AVLTree * avl, AVLNode * node, void * x );
AVLNode * _afc_avl_tree_find_node_min ( AVLTree * avl, AVLNode * node );
AVLNode * _afc_avl_tree_find_node_max ( AVLTree * avl, AVLNode * node );
AVLNode * _afc_avl_tree_insert ( AVLTree * tree, AVLNode * t, void * key, void * val );

#define afc_avl_tree_delete(avl)	if ( avl ) { _afc_avl_tree_delete ( avl ); avl = NULL; }

#define afc_avl_tree_find_node(avl,key)	 ( avl ? _afc_avl_tree_find_node ( avl, avl->root, key ) : NULL )
#define afc_avl_tree_find_node_min(avl)	 ( avl ? _afc_avl_tree_find_node_min ( avl, avl->root ) : NULL )
#define afc_avl_tree_find_node_max(avl)	 ( avl ? _afc_avl_tree_find_node_max ( avl, avl->root ) : NULL )
#define afc_avl_tree_insert(avl,key,val) ( avl ? avl->root = _afc_avl_tree_insert ( avl, avl->root, key, val ) : NULL )


#endif
