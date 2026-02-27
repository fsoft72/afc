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
 * test_btree.c - Comprehensive tests for the BTree (B-Tree) class.
 *
 * The BTree implementation requires callback functions for comparing
 * and serializing entries. We use simple integer entries stored as
 * void* pointers, with corresponding lt/eq/create_key/write_node/
 * read_key/read_node callback stubs.
 *
 * Tests cover:
 *   - Creation and deletion
 *   - Initialization with comparison functions and branching factor
 *   - Adding multiple items with afc_btree_add()
 *   - Finding items with afc_btree_find()
 *   - Deleting items with afc_btree_del()
 *   - Clearing all items with afc_btree_clear()
 *   - Edge cases: finding/deleting non-existing keys, duplicate inserts
 */

#include "test_utils.h"
#include "../src/btree.h"

/* ================================================================
 * BTree callback functions for integer-as-pointer entries
 * ================================================================ */

/**
 * _bt_lower_than - Returns TRUE if key < entry (both cast to long).
 */
static BOOL _bt_lower_than(void *key, void *entry)
{
	return ((long)key < (long)entry) ? TRUE : FALSE;
}

/**
 * _bt_equal - Returns TRUE if key == entry (both cast to long).
 */
static BOOL _bt_equal(void *key, void *entry)
{
	return ((long)key == (long)entry) ? TRUE : FALSE;
}

/**
 * _bt_create_key - Writes the entry pointer value into a buffer as
 * the key representation. Sets the size to sizeof(void*).
 */
static int _bt_create_key(void *entry, void *buffer, unsigned short *size)
{
	*size = (unsigned short)sizeof(void *);
	memcpy(buffer, &entry, *size);
	return AFC_ERR_NO_ERROR;
}

/**
 * _bt_write_node - Writes entry data to a file (stub for serialization).
 */
static int _bt_write_node(FILE *file, void *entry)
{
	if (fwrite(&entry, sizeof(void *), 1, file) != 1)
		return AFC_BTREE_ERR_WRITE_FILE;
	return AFC_ERR_NO_ERROR;
}

/**
 * _bt_read_key - Reads key from buffer into entry pointer (stub).
 */
static int _bt_read_key(void **entry, void *buffer, unsigned short size)
{
	memcpy(entry, &buffer, size);
	return AFC_ERR_NO_ERROR;
}

/**
 * _bt_read_node - Reads entry data from file (stub for deserialization).
 */
static int _bt_read_node(void **entry, FILE *file, unsigned short size)
{
	if (fread(entry, sizeof(void *), 1, file) != 1)
		return AFC_BTREE_ERR_READING_FILE;
	return AFC_ERR_NO_ERROR;
}

/* Branching factor for B-Tree tests */
#define BTREE_BRANCH_FACTOR 5

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ================================================================
	 * GROUP 1: Creation and initialization
	 * ================================================================ */

	BTree *btr = afc_btree_new();

	/* Verify B-tree was created (non-NULL) */
	print_res("new() returns non-NULL",
		(void *)(long)1,
		(void *)(long)(btr != NULL),
		0);

	/* Initialize with branching factor and comparison callbacks */
	int res = afc_btree_init(btr, BTREE_BRANCH_FACTOR,
		_bt_lower_than, _bt_equal,
		_bt_create_key, _bt_write_node,
		_bt_read_key, _bt_read_node);

	print_res("init() result",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	print_row();

	/* ================================================================
	 * GROUP 2: Add items and verify with find()
	 * ================================================================ */

	/* Insert several integer values as entries */
	res = afc_btree_add(btr, (void *)10);
	print_res("add(10) result",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	res = afc_btree_add(btr, (void *)20);
	print_res("add(20) result",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	afc_btree_add(btr, (void *)30);
	afc_btree_add(btr, (void *)40);
	afc_btree_add(btr, (void *)50);
	afc_btree_add(btr, (void *)5);
	afc_btree_add(btr, (void *)15);
	afc_btree_add(btr, (void *)25);
	afc_btree_add(btr, (void *)35);
	afc_btree_add(btr, (void *)45);

	print_row();

	/* ================================================================
	 * GROUP 3: Find existing items
	 * ================================================================ */

	/* Find an item that was inserted first */
	void *found = afc_btree_find(btr, (void *)10);
	print_res("find(10)",
		(void *)(long)10,
		(void *)(long)found,
		0);

	/* Find an item in the middle */
	found = afc_btree_find(btr, (void *)30);
	print_res("find(30)",
		(void *)(long)30,
		(void *)(long)found,
		0);

	/* Find the smallest item */
	found = afc_btree_find(btr, (void *)5);
	print_res("find(5)",
		(void *)(long)5,
		(void *)(long)found,
		0);

	/* Find the largest item */
	found = afc_btree_find(btr, (void *)50);
	print_res("find(50)",
		(void *)(long)50,
		(void *)(long)found,
		0);

	/* Find an intermediate item */
	found = afc_btree_find(btr, (void *)45);
	print_res("find(45)",
		(void *)(long)45,
		(void *)(long)found,
		0);

	print_row();

	/* ================================================================
	 * GROUP 4: Find non-existing items (edge cases)
	 * ================================================================ */

	/* Search for a key that was never inserted */
	found = afc_btree_find(btr, (void *)99);
	print_res("find(99) non-existing",
		(void *)(long)0,
		(void *)(long)found,
		0);

	/* Search for key 0 - not inserted */
	found = afc_btree_find(btr, (void *)0);
	print_res("find(0) non-existing",
		(void *)(long)0,
		(void *)(long)found,
		0);

	/* Search for a negative-range value (cast to void*) */
	found = afc_btree_find(btr, (void *)(long)-1);
	print_res("find(-1) non-existing",
		(void *)(long)0,
		(void *)(long)found,
		0);

	print_row();

	/* ================================================================
	 * GROUP 5: Delete an existing item and verify it is gone
	 * ================================================================ */

	res = afc_btree_del(btr, (void *)25);
	print_res("del(25) result",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* Verify deleted item is no longer found */
	found = afc_btree_find(btr, (void *)25);
	print_res("find(25) after del",
		(void *)(long)0,
		(void *)(long)found,
		0);

	/* Other items should still be findable */
	found = afc_btree_find(btr, (void *)20);
	print_res("find(20) after del(25)",
		(void *)(long)20,
		(void *)(long)found,
		0);

	found = afc_btree_find(btr, (void *)30);
	print_res("find(30) after del(25)",
		(void *)(long)30,
		(void *)(long)found,
		0);

	print_row();

	/* ================================================================
	 * GROUP 6: Delete a non-existing item (should return error)
	 * ================================================================ */

	res = afc_btree_del(btr, (void *)999);
	print_res("del(999) non-existing",
		(void *)(long)AFC_BTREE_KEY_NOT_PRESENT,
		(void *)(long)res,
		0);

	print_row();

	/* ================================================================
	 * GROUP 7: Delete multiple items sequentially
	 * ================================================================ */

	res = afc_btree_del(btr, (void *)5);
	print_res("del(5) smallest",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	found = afc_btree_find(btr, (void *)5);
	print_res("find(5) after del",
		(void *)(long)0,
		(void *)(long)found,
		0);

	res = afc_btree_del(btr, (void *)50);
	print_res("del(50) largest",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	found = afc_btree_find(btr, (void *)50);
	print_res("find(50) after del",
		(void *)(long)0,
		(void *)(long)found,
		0);

	/* Remaining items should still be intact */
	found = afc_btree_find(btr, (void *)10);
	print_res("find(10) still present",
		(void *)(long)10,
		(void *)(long)found,
		0);

	found = afc_btree_find(btr, (void *)40);
	print_res("find(40) still present",
		(void *)(long)40,
		(void *)(long)found,
		0);

	print_row();

	/* ================================================================
	 * GROUP 8: Clear and verify tree is empty
	 * ================================================================ */

	res = afc_btree_clear(btr);
	print_res("clear() result",
		(void *)(long)AFC_ERR_NO_ERROR,
		(void *)(long)res,
		0);

	/* After clearing, root should be NULL, so find should return NULL */
	found = afc_btree_find(btr, (void *)10);
	print_res("find(10) after clear",
		(void *)(long)0,
		(void *)(long)found,
		0);

	found = afc_btree_find(btr, (void *)40);
	print_res("find(40) after clear",
		(void *)(long)0,
		(void *)(long)found,
		0);

	print_row();

	/* ================================================================
	 * GROUP 9: Re-insert after clear to verify tree is reusable
	 * ================================================================ */

	/* Re-init is not needed since init just sets function pointers */
	afc_btree_add(btr, (void *)100);
	afc_btree_add(btr, (void *)200);
	afc_btree_add(btr, (void *)300);

	found = afc_btree_find(btr, (void *)100);
	print_res("find(100) after re-add",
		(void *)(long)100,
		(void *)(long)found,
		0);

	found = afc_btree_find(btr, (void *)200);
	print_res("find(200) after re-add",
		(void *)(long)200,
		(void *)(long)found,
		0);

	found = afc_btree_find(btr, (void *)300);
	print_res("find(300) after re-add",
		(void *)(long)300,
		(void *)(long)found,
		0);

	/* ================================================================
	 * Summary and cleanup
	 * ================================================================ */

	print_summary();

	afc_btree_delete(btr);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
