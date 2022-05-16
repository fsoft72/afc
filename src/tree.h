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

#ifndef AFC_TREE_H
#define AFC_TREE_H

#include "base.h"
#include "exceptions.h"

/* AFC Tree Magic Number */
#define AFC_TREE_MAGIC ( 'T' << 24 | 'R' << 16 | 'E' << 8 | 'E' )

/* AFC Tree Base value for constants */
#define AFC_TREE_BASE 0xB100

enum {	AFC_TREE_ERR_INVALID_MODE = AFC_TREE_BASE + 1
};

enum {	AFC_TREE_MODE_LEVEL = AFC_TREE_BASE + 1,
	AFC_TREE_MODE_PREORDER,
	AFC_TREE_MODE_POSTORDER
};

typedef struct afc_tree_node TreeNode;

struct afc_tree
{
	unsigned long magic;

	TreeNode * first;
	TreeNode * last;

	TreeNode * current;

	int (*compare) 	  ( void * val1, void * val2 );
	void (*freenode)  ( void * val );
};

typedef struct afc_tree Tree;

struct afc_tree_node
{
	TreeNode * parent;

	TreeNode * l_sibling;
	TreeNode * r_sibling;
	
	TreeNode * child;
	TreeNode * last_child;

	TreeNode * list_next;
	TreeNode * list_prev;

	Tree * tree;

	void * val;
};

Tree * afc_tree_new ( void );
int _afc_tree_delete ( Tree * tree );
#define afc_tree_delete(t)  if ( t ) { _afc_tree_delete ( t ); t = NULL; }
int afc_tree_clear ( Tree * tree );
short afc_tree_is_empty ( Tree * tree );
TreeNode *afc_tree_insert ( Tree * tree, void * val );
#define AS_TREE(node) (node->tree)
int afc_tree_traverse ( Tree * t, int mode, int ( *visitor ) ( TreeNode *  ) );


#define afc_subtree_delete(t)	if ( t ) { _afc_subtree_delete ( t ); t = NULL; }
int _afc_subtree_delete ( TreeNode * subtree );
TreeNode *afc_subtree_insert_child ( TreeNode * parent, void * val );
TreeNode *afc_subtree_insert_sibling ( TreeNode * brother, void * val );
int afc_subtree_traverse ( TreeNode * t, int mode, int ( *visitor ) ( TreeNode * ) );




#endif
