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
#ifndef AFC_BIN_TREE_H
#define AFC_BIN_TREE_H

#include "base.h"
#include "exceptions.h"

/* AFC BinTree Magic Number */
#define AFC_BIN_TREE_MAGIC ( 'B' << 24 | 'I' << 16 | 'N' << 8 | 'T' )

/* AFC BinTree Base value for constants */
#define AFC_BIN_TREE_BASE 0x2100

enum {	AFC_BIN_TREE_ERR_INVALID_MODE = AFC_BIN_TREE_BASE + 1
};

enum {	AFC_BIN_TREE_MODE_INORDER = AFC_BIN_TREE_BASE + 1,
	AFC_BIN_TREE_MODE_PREORDER,
	AFC_BIN_TREE_MODE_POSTORDER
};

typedef struct afc_bin_tree_node BinTreeNode;

struct afc_bin_tree_node
{
	BinTreeNode * left;
	BinTreeNode * right;
	void * key;
	void * val;
};

struct afc_bin_tree
{
	unsigned long magic;

	BinTreeNode * root;
	BinTreeNode * curr_pos;

	int (*compare) 	  ( void * val1, void * val2 );
	void (*freenode)  ( void * key, void * val );
};

typedef struct afc_bin_tree BinTree;

#define afc_bin_tree_delete(bt)	if ( bt ) { _afc_bin_tree_delete ( bt ); bt = NULL; }

BinTree * afc_bin_tree_new ( void );
int afc_bin_tree_clear ( BinTree * bt );
int _afc_bin_tree_delete ( BinTree * bt );
short afc_bin_tree_is_empty ( BinTree * bt );
void * afc_bin_tree_get ( BinTree * bt, void * key );
int afc_bin_tree_insert ( BinTree * bt, void * key, void * val );
int afc_bin_tree_traverse ( BinTree * bt, int mode, int ( *visit ) ( BinTree *, BinTreeNode *  ) );
int afc_bin_tree_del ( BinTree * bt, void * key );
int afc_bin_tree_set_compare_func ( BinTree * bt, int ( *comp ) ( void * val1, void * val2 ) );
int afc_bin_tree_set_clear_func ( BinTree * bt, void ( *clear ) ( void * key, void * val ) );

#endif
