#include "btree.h"

/*
@config
	TITLE:   BTree
	VERSION: 1.0
	AUTHOR:  Fabrizio PAstore - pastorefabrizio@libero.it
	AUTHOR:  Fabio Rotondo - fabio@rotondo.it
@endnode
*/

// {{{ docs
/*
@node quote
	*Put the quote here*

	Quote Author
@endnode

@node intro
	BTree documentation introduction should go here.
	Use the reST syntax to create docs.

=================
BTree File Format
=================

=====  ====  ==============================
  BTree File Format (structure header)
-------------------------------------------
Pos    Size  Description
=====  ====  ==============================
0         4  Number of levels

4	  2  Branching factor

====  =====  ==============================


=====  ====  ==============================
  BTree File Format (structure fields)
-------------------------------------------
Pos    Size  Description
=====  ====  ==============================
0	  2  Number of elements

2	  2  Key Size

4+ks 	k s  Key

8+ks	  4  Offset of data in data file

12+ks     4  Dimension of data saved

=====  ====  ==============================


@endnode
*/
// }}}

static const char class_name[] = "BTree";
static int afc_btree_int_recursive_del(BTree *btr, BTreeNode *node);
static int afc_btree_int_node_del(BTree *btr, BTreeNode *node);

static int afc_btree_int_rec_delete_tree(BTree *btr, void *target, BTreeNode *current);
static int afc_btree_int_delete_tree(BTree *btr, void *target, BTreeNode *root);
static int afc_btree_int_remove(BTreeNode *current, unsigned long pos);
static int afc_btree_int_successor(BTree *btr, BTreeNode *current, unsigned long pos);
static int afc_btree_int_restore(BTree *btr, BTreeNode *current, unsigned long pos);
static int afc_btree_int_move_right(BTreeNode *current, unsigned long pos);
static int afc_btree_int_move_left(BTreeNode *current, unsigned long pos);
static int afc_btree_int_combine(BTree *btr, BTreeNode *current, unsigned long pos);
static int afc_btree_int_write(BTree *btr, unsigned long *lev, BTreeNode *node);
static int afc_btree_int_read(BTree *btr, BTreeNode **node, unsigned long *lev);

static int afc_btree_int_insert_tree(BTree *btr, void *new_entry);
static int afc_btree_int_split(BTree *btr, void *med_entry, BTreeNode *med_right, BTreeNode *current, unsigned short pos, void **new_median, BTreeNode **new_right);
static int afc_btree_int_new_node(BTree *btr, BTreeNode **btrn);
BTreeNode *afc_btree_int_search_tree(BTree *btr, void *target, BTreeNode *root, unsigned short *targetpos);
static BOOL afc_btree_int_search_node(BTree *btr, void *target, BTreeNode *current, unsigned short *pos);
static int afc_btree_int_push_down(BTree *btr, void *new_entry, BTreeNode *current, void **med_entry, BTreeNode **med_right);
static int afc_btree_int_push_in(void *med_entry, BTreeNode *med_right, BTreeNode *current, unsigned short pos);
// {{{ afc_btree_new ()
/*
@node afc_btree_new

				 NAME: afc_btree_new ()    - Initializes a new BTree instance.

			 SYNOPSIS: BTree * afc_btree_new ()

		  DESCRIPTION: This function initializes a new BTree instance.

				INPUT: NONE

			  RESULTS: a valid inizialized BTree instance. NULL in case of errors.

			 SEE ALSO: - afc_btree_delete()
					   - afc_btree_clear()
@endnode
*/
BTree *afc_btree_new(void)
{
	TRY(BTree *)

	BTree *btr = afc_malloc(sizeof(BTree));

	if (btr == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "btr", NULL);
	btr->magic = AFC_BTREE_MAGIC;
	if ((btr->key_buffer = (void *)afc_malloc(1024)) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "btr->root", NULL);

	RETURN(btr);

	EXCEPT
	afc_btree_delete(btr);

	FINALLY

	ENDTRY
}
// }}}
/*
@node afc_btree_init

				 NAME: afc_btree_init ( btr, m, lower_than, equal, create_key, write_node, read_key, read_node )

			 SYNOPSIS: int afc_btree_init ( BTree * btr, unsigned short m, BOOL  (* lower_than) ( void * , void * ), BOOL (* equal) (void *, void * ), int (* create_key) (void *, void *, unsigned short * ), int (* write_node) ( FILE *, void * ), int (* read_key) (void **, void *, unsigned short ), int (* read_node) ( void **, FILE *, unsigned short ) )

		  DESCRIPTION: This function initializes a new BTree instance.

				INPUT: - btr 	     - Pointer to a *valid* BTree instance.
			   - m	     - Branching factor
			   - lower_than  - function that returns TRUE if an entry is lower than another
			   - equal       - function that returns TRUE if two entries are equals
			   - create_key  - function that writes in a buffer key value
			   - write_node  - function that writes btree node entries ( usualli an ID ) on disk
			   - read_key    - function for key reading from file
			   - read_node   - function for btree node reading from file


			  RESULTS: a valid inizialized BTree instance. NULL in case of errors.

			 SEE ALSO: - afc_btree_delete()
					   - afc_btree_clear()
@endnode
*/
int afc_btree_init(BTree *btr, unsigned short m, BOOL (*lower_than)(void *, void *), BOOL (*equal)(void *, void *), int (*create_key)(void *, void *, unsigned short *), int (*write_node)(FILE *, void *), int (*read_key)(void **, void *, unsigned short), int (*read_node)(void **, FILE *, unsigned short))
{
	btr->max = m - 1;
	btr->min = (m - 1) / 2;
	btr->lower_than = lower_than;
	btr->equal = equal;
	btr->create_key = create_key;
	btr->write_node = write_node;
	btr->read_key = read_key;
	btr->read_node = read_node;
	return (AFC_ERR_NO_ERROR);
}

// {{{ afc_btree_delete ( btr )
/*
@node afc_btree_delete

				 NAME: afc_btree_delete ( btr ) - Dispose a BTree instance.

			 SYNOPSIS: int afc_btree_delete ( BTree * btr )

		  DESCRIPTION: Use this method to delete an object's instance.

				INPUT: - btr - Pointer to a *valid* BTree instance.

			  RESULTS: - AFC_ERR_NO_ERROR on success.

			 SEE ALSO: afc_btree_new()
@endnode
*/
int _afc_btree_delete(BTree *btr)
{
	int res;

	if (btr == NULL)
		return AFC_LOG_FAST(AFC_ERR_NULL_POINTER);

	if (btr->magic != AFC_BTREE_MAGIC)
		return AFC_LOG_FAST(AFC_ERR_INVALID_POINTER);

	if ((res = afc_btree_clear(btr)) != AFC_ERR_NO_ERROR)
		return (res);
	afc_free(btr->key_buffer);
	afc_free(btr);

	return (AFC_ERR_NO_ERROR);
}
// }}}
// {{{ afc_btree_clear ( btr )
/*
@node afc_btree_clear

				 NAME: afc_btree_clear ( btr ) - Frees all unused memory

			 SYNOPSIS: int afc_btree_clear ( BTree * btr )

		  DESCRIPTION: This method clears all data inside the BTree instance,
					   except the main classes.

				INPUT: - btr   - Pointer to a *valid* BTree instance.

			  RESULTS: - AFC_ERR_NO_ERROR on success.

			 SEE ALSO: - afc_btree_new()
			   - afc_btree_delete()
@endnode
*/
int afc_btree_clear(BTree *btr)
{
	if (btr == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));
	if (btr->magic != AFC_BTREE_MAGIC)
		return (AFC_LOG_FAST(AFC_ERR_INVALID_POINTER));
	if (btr->root != NULL)
	{
		afc_btree_int_recursive_del(btr, btr->root);
		afc_btree_int_node_del(btr, btr->root);
	}
	btr->file = NULL;
	btr->data_file = NULL;
	btr->root = NULL;
	return (AFC_ERR_NO_ERROR);
}
// }}}

/*
@node afc_btree_add

				 NAME: afc_btree_add ( btr, entry )

			 SYNOPSIS: int afc_btree_add ( BTree * btr, void * entry )

		  DESCRIPTION: This method add a key to the tree


				INPUT: - btr   - Pointer to a *valid* BTree instance.
			   - entry - Pointer to entry we want to add

			  RESULTS: - AFC_ERR_NO_ERROR on success.

			 SEE ALSO: - afc_btree_new()
			   - afc_btree_delete()
@endnode
*/
int afc_btree_add(BTree *btr, void *entry)
{
	return (afc_btree_int_insert_tree(btr, entry));
}

/*
@node afc_btree_del

				 NAME: afc_btree_del ( btr, entry )

			 SYNOPSIS: int afc_btree_del ( BTree * btr, void * entry )

		  DESCRIPTION: This method removes a key from the tree


				INPUT: - btr   - Pointer to a *valid* BTree instance.
			   - entry - Pointer to the entry we want to delete

			  RESULTS: - AFC_ERR_NO_ERROR on success.

			 SEE ALSO: - afc_btree_new()
			   - afc_btree_delete()
@endnode
*/

int afc_btree_del(BTree *btr, void *entry)
{
	return (afc_btree_int_delete_tree(btr, entry, btr->root));
}

/*
@node afc_btree_find

				 NAME: afc_btree_find ( btr, entry )

			 SYNOPSIS: void * afc_btree_find ( BTree * btr, void * entry )

		  DESCRIPTION: This method add a key to the tree


				INPUT: - btr   - Pointer to a *valid* BTree instance.
			   - entry - Pointer to the entry we want to find

			  RESULTS: - Pointer to entry we want to find
			   - NULL if not present
			 SEE ALSO:

@endnode
*/

void *afc_btree_find(BTree *btr, void *entry)
{
	BTreeNode *node;
	unsigned short pos;

	node = afc_btree_int_search_tree(btr, entry, btr->root, &pos);
	if (node == NULL)
		return (NULL);
	return (node->entry[pos]);
}

/*
@node afc_btree_write

				 NAME: afc_btree_write ( btr, fname, f_data_name )

			 SYNOPSIS: int afc_btree_write ( BTree * btr, char * fname, char * f_data_name )

		  DESCRIPTION: This method writes btree on disk


				INPUT: - btr   		- Pointer to a *valid* BTree instance.
			   - fname 		- File with btree structure
			   - f_data_name 	- File with entries data

		  RESULTS: - AFC_ERR_NO_ERROR on success.

			 SEE ALSO:

@endnode
*/
int afc_btree_write(BTree *btr, char *fname, char *f_data_name)
{

	TRY(int)
	unsigned long lev = 0;

	if ((btr->file = fopen(fname, "wb")) == NULL)
		RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_OPEN_FILE, "Could not open file", fname, AFC_BTREE_ERR_OPEN_FILE);

	if ((btr->data_file = fopen(f_data_name, "wb")) == NULL)
		RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_OPEN_FILE, "Could not open file", f_data_name, AFC_BTREE_ERR_OPEN_FILE);

	fseek(btr->file, sizeof(unsigned long), SEEK_SET);

	if (fwrite(&btr->max, sizeof(unsigned short), 1, btr->file) == 0)
		RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE);

	afc_btree_int_write(btr, &lev, btr->root);

	fseek(btr->file, 0, SEEK_SET);

	if (fwrite(&btr->lev, sizeof(unsigned long), 1, btr->file) == 0)
		RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE);

	RETURN(AFC_ERR_NO_ERROR);

	EXCEPT

	FINALLY

	fclose(btr->file);
	fclose(btr->data_file);
	ENDTRY
}

/*
@node afc_btree_read

				 NAME: afc_btree_read ( btr, fname, f_data_name )

			 SYNOPSIS: int afc_btree_read ( BTree * btr, char * fname, char * f_data_name )

		  DESCRIPTION: This method writes btree on disk


				INPUT: - btr   		- Pointer to a *valid* BTree instance.
			   - fname 		- File with btree structure
			   - f_data_name 	- File with entries data

		  RESULTS: - AFC_ERR_NO_ERROR on success.

			 SEE ALSO:

@endnode
*/
int afc_btree_read(BTree *btr, char *fname, char *f_data_name)
{
	TRY(int)
	unsigned long lev = 0;

	if ((btr->file = fopen(fname, "rb")) == NULL)
		RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_OPEN_FILE, "Could not open file", fname, AFC_BTREE_ERR_OPEN_FILE);

	if ((btr->data_file = fopen(f_data_name, "rb")) == NULL)
		RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_OPEN_FILE, "Could not open file", f_data_name, AFC_BTREE_ERR_OPEN_FILE);
	if ((fread(&(btr->lev), sizeof(unsigned long), 1, btr->file)) != 1)
		RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_READING_FILE, "Error reading record on file", 0, AFC_BTREE_ERR_READING_FIELDS);

	if ((fread(&(btr->max), sizeof(unsigned short), 1, btr->file)) != 1)
		RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_READING_FILE, "Error reading record on file", 0, AFC_BTREE_ERR_READING_FIELDS);

	btr->min = btr->max / 2;

	RETURN(afc_btree_int_read(btr, &btr->root, &lev));
	// RETURN ( AFC_ERR_NO_ERROR );
	EXCEPT

	FINALLY

	if (btr->file)
		fclose(btr->file);
	if (btr->data_file)
		fclose(btr->data_file);
	ENDTRY
}

/*
@node

	   NAME:

   SYNOPSIS:

	  SINCE: 1.10

DESCRIPTION: Use this command to set a clear function. The function will be called each time an
		 item is being deleted from the list with afc_hash_del() or afc_hash_clear().
		 To remove this function, pass a NULL value as function pointer.

	  INPUT: - am	- Pointer to a valid Hash class.
		 - func	- Function to be called in clearing operations.

	RESULTS: AFC_ERR_NO_ERROR

   SEE ALSO:

@endnode
*/
int afc_btree_set_clear_func(BTree *btr, int (*func)(void *))
{
	btr->func_clear = func;
	return (AFC_ERR_NO_ERROR);
}

/* ===============================================================================================================
	INTERNAL FUNCTIONS
=============================================================================================================== */

/*

search for target descending in the btree
btr 	-	Pointer to a valid btree structure
target  -	Poiter to the entry with the key we want to find ( entries can be defined as structures with a key and data fields )
root	-	Pointer to the node we want to start the search from
targetpos -	Position in the node returned

*/
BTreeNode *afc_btree_int_search_tree(BTree *btr, void *target, BTreeNode *root, unsigned short *targetpos)
{
	// check if node exist
	if (!root)
		return (NULL);
	// search key in current node
	else if (afc_btree_int_search_node(btr, target, root, targetpos))
		return root;
	// search for target key in node's children
	else
		return afc_btree_int_search_tree(btr, target, root->branch[*targetpos], targetpos);
}

/*

search for target descending in a node
btr 	-	Pointer to a valid btree structure
target  -	Poiter to the entry with the key we want to find ( entries can be defined as structures with a key and data fields )
current	-	Pointer to the node we want to examine
targetpos -	Position in the node returned

*/
static BOOL afc_btree_int_search_node(BTree *btr, void *target, BTreeNode *current, unsigned short *pos)
{

	// check if key is lower than first entry
	if ((btr->lower_than(target, current->entry[1])))
	// if ( btr->eval ( target, current->entry[1] ) < 0 )
	{
		*pos = 0;
		return FALSE;
	}
	else
	{

		// position pos pointer to the lowest greater element
		for (*pos = current->counter; btr->lower_than(target, current->entry[*pos]) && *pos > 1; (*pos)--)
			;

		// for ( *pos = current->counter; ( ( btr->eval ( target, current->entry[*pos]) ) < 0 ) && *pos > 1; ( * pos )-- );

		// check if the entry that is not bigger than target is equal
		return (btr->equal(target, current->entry[*pos]));
	}
}
/*
Insert an entry in the btree

btr 	-	Pointer to a valid BTree structure
new_entry -	Pointer to the entry we want to add
root	-	Pointer to the root node

*/

static int afc_btree_int_insert_tree(BTree *btr, void *new_entry)
{
	int ret;
	void *med_entry;
	BTreeNode *med_right;
	BTreeNode *new_root;
	if ((ret = afc_btree_int_push_down(btr, new_entry, btr->root, &med_entry, &med_right)) == AFC_BTREE_PD_TRUE)
	{
		// need new root
		afc_btree_int_new_node(btr, &new_root);

		new_root->counter = 1;

		// insert median key
		new_root->entry[1] = med_entry;

		// old root is new roots left branch
		new_root->branch[0] = btr->root;

		// set new roots right branch
		new_root->branch[1] = med_right;

		btr->root = new_root;
		return (AFC_ERR_NO_ERROR);
	}
	return ((ret == AFC_BTREE_DUPLICATE_KEY) ? ret : AFC_ERR_NO_ERROR);
}

/*
insert an entry in the tree, pushing down it until reaches the right position

btr	-	Pointer to a valid BTree structure
new_entry -	Pointer to the entry we want to add
current	-	Pointer to the node in which we are tryin to put new_entry
med_entry	Returned median entry  ( in case of a spit is nedded )
med_right	Pointer to med_entry right child

*/

static int afc_btree_int_push_down(BTree *btr, void *new_entry, BTreeNode *current, void **med_entry, BTreeNode **med_right)
{
	TRY(int)
	unsigned short pos;
	int ret;
	// passed a null node, caller function have only to set the first entry in it
	if (current == NULL)
	{
		*med_entry = new_entry;
		*med_right = NULL;
		RETURN(AFC_BTREE_PD_TRUE);
	}
	else
	{

		// search if key passed is already present in this node, return first bigger entry in pos
		if (afc_btree_int_search_node(btr, new_entry, current, &pos))
			RAISE_RC(AFC_LOG_WARNING, AFC_BTREE_DUPLICATE_KEY, "Inserting duplicate key", NULL, AFC_BTREE_DUPLICATE_KEY);

		// insert key in correct branch
		if ((ret = afc_btree_int_push_down(btr, new_entry, current->branch[pos], med_entry, med_right)) == AFC_BTREE_PD_TRUE)
		{
			// if branch is null and current node is not full insert in current node
			if (current->counter < btr->max)
			{
				afc_btree_int_push_in(*med_entry, *med_right, current, pos);
				RETURN(AFC_BTREE_PD_FALSE);
			}
			else
			{
				// if current node is full split
				afc_btree_int_split(btr, *med_entry, *med_right, current, pos, med_entry, med_right);
				RETURN(AFC_BTREE_PD_TRUE);
			}
		}
		RETURN(ret);
	}

	EXCEPT

	FINALLY

	ENDTRY
}

/*

Insert an entry in a node in the passed position

med_entry	New entry
med_right	Right child
current	-	Pointer to the node in which we want to put new_entry
pos -	Position in the node

*/
static int afc_btree_int_push_in(void *med_entry, BTreeNode *med_right, BTreeNode *current, unsigned short pos)
{
	unsigned short i;

	// move all entry one position right
	for (i = current->counter; i > pos; i--)
	{
		current->entry[i + 1] = current->entry[i];
		current->branch[i + 1] = current->branch[i];
	}

	// insert new entry
	current->entry[pos + 1] = med_entry;
	current->branch[pos + 1] = med_right;
	current->counter++;

	return (AFC_ERR_NO_ERROR);
}

/*
split a node in 2 parts

btr 	-	Pointer to a valid BTree structure
med_entry	Parent entry for current node
med_right	Parent entry's right child
current		Pointer to the node we want to split
pos		Position in which new entry has to be added
new_median 	Returned pointer to new_median entry
new_right	Returned pointer to new median's right child

*/

static int afc_btree_int_split(BTree *btr, void *med_entry, BTreeNode *med_right, BTreeNode *current, unsigned short pos, void **new_median, BTreeNode **new_right)
{
	unsigned short i;
	unsigned short median;

	if (pos <= btr->min)
		median = btr->min;
	else
		median = btr->min + 1;

	// create new node neeede for split
	afc_btree_int_new_node(btr, new_right);

	// move entry bigger than median key to new node
	for (i = median + 1; i <= btr->max; i++)
	{

		(*new_right)->entry[i - median] = current->entry[i];
		(*new_right)->branch[i - median] = current->branch[i];
		current->entry[i] = NULL;
		current->branch[i] = NULL;
	}
	(*new_right)->counter = btr->max - median;
	current->counter = median;

	// if new key was lower than median key insert in old node
	if (pos <= btr->min)
		afc_btree_int_push_in(med_entry, med_right, current, pos);
	else
		// insert in new created
		afc_btree_int_push_in(med_entry, med_right, *new_right, pos - median);

	// set new median key
	*new_median = current->entry[current->counter];

	// new node 's fist element left branch is old median right brand
	(*new_right)->branch[0] = current->branch[current->counter];
	current->branch[current->counter] = NULL;
	current->counter--;
	return (AFC_ERR_NO_ERROR);
}

/*
creates a new node

btr	-	Pointer to a valid BTree structure
btrn	-	Returned pointer of the new node

*/

static int afc_btree_int_new_node(BTree *btr, BTreeNode **btrn)
{
	TRY(int)
	// alloc node
	if ((*btrn = afc_malloc(sizeof(BTreeNode))) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "btr->root", AFC_ERR_NO_MEMORY);
	// alloc memory for entry
	if (((*btrn)->entry = afc_malloc(sizeof(char *) * (btr->max + 1))) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "btr->entry", AFC_ERR_NO_MEMORY);
	// alloc memory for branch
	if (((*btrn)->branch = afc_malloc(sizeof(char *) * (btr->max + 1))) == NULL)
		RAISE_FAST_RC(AFC_ERR_NO_MEMORY, "bt->branch", AFC_ERR_NO_MEMORY);
	(*btrn)->counter = 0;
	RETURN(AFC_ERR_NO_ERROR);

	EXCEPT

	FINALLY

	ENDTRY
}

/*
deletes a node and his entries

btr 	-	Pointer to a valid BTree structure
node	-	Pointer to the node we want to delete

*/

static int afc_btree_int_node_del(BTree *btr, BTreeNode *node)
{
	unsigned short pos;
	if (btr->func_clear)
	{
		for (pos = 1; pos <= node->counter; pos++)
			if (node->entry[pos])
				btr->func_clear(node->entry[pos]);
	}

	afc_free(node->entry);
	afc_free(node->branch);
	afc_free(node);
	return (AFC_ERR_NO_ERROR);
}

/*
deletes recusively all nodes and children of a (sub)tree

btr 	-	Pointer to a valid BTree structure
node	-	Pointer	to the node we are currently examining

*/

static int afc_btree_int_recursive_del(BTree *btr, BTreeNode *node)
{
	int pos;

	// for each node allocated
	for (pos = node->counter; pos >= 0; pos--)
	{

		if (node->branch[pos] != NULL)
		{
			// descend into branch
			afc_btree_int_recursive_del(btr, node->branch[pos]);

			// delete branch node
			afc_btree_int_node_del(btr, node->branch[pos]);
			node->branch[pos] = NULL;
		}
	}
	return (AFC_ERR_NO_ERROR);
}

/*
deletes an entry from the tree

btr 	-	Pointer to a valid BTree structure
target 	-	Entry structure that we want to delete, only a set key is necessary
root 	-	Root of the tree

*/

static int afc_btree_int_delete_tree(BTree *btr, void *target, BTreeNode *root)
{
	BTreeNode *old_root;
	int ret;
	if ((ret = afc_btree_int_rec_delete_tree(btr, target, root)) != AFC_ERR_NO_ERROR)
		return (ret);
	if (!root)
		return (AFC_ERR_NO_ERROR);
	// check if target was at root
	if (root->counter == 0)
	{
		old_root = root;
		root = root->branch[0];
		// restore
		// afc_btree_int_restore ( btr, old_root, 0 );
		afc_btree_int_node_del(btr, old_root);
		btr->root = root;
	}
	return (AFC_ERR_NO_ERROR);
}

/*
deletes recursively an entry from a (sub)tree

btr 	-	Pointer to a valid BTree structure
target	-	Entry structure taht we want to delete
current	-	Node we are currently examining

*/

static int afc_btree_int_rec_delete_tree(BTree *btr, void *target, BTreeNode *current)
{
	TRY(int)
	unsigned short pos;
	int ret;

	// if current node is null the key is not present
	if (current == NULL)
	{
		RAISE_RC(AFC_LOG_WARNING, AFC_BTREE_KEY_NOT_PRESENT, " Key not in btree ", NULL, AFC_BTREE_KEY_NOT_PRESENT);
	}
	// search key in the current node
	if (afc_btree_int_search_node(btr, target, current, &pos))
	{
		// if key is present and node is not a leaf
		if (current->branch[pos - 1])
		{
			// find and change deleted key with his successor
			afc_btree_int_successor(btr, current, pos);

			// delete the key just moved
			afc_btree_int_rec_delete_tree(btr, current->entry[pos], current->branch[pos]);
		}
		else
		{
			// current node is a leaf
			/*if ( btr->func_clear )
			{
				aux_key = current->entry[pos];
				btr->func_clear ( current->entry [ pos ] );
			}*/
			afc_btree_int_remove(current, pos);
		}
	}
	else
		// key is not present in current node, search in possible branch
		if ((ret = afc_btree_int_rec_delete_tree(btr, target, current->branch[pos])) != AFC_ERR_NO_ERROR)
			// if key is not found
			RETURN(ret);

	// if not a leaf
	if (current->branch[pos])
	{
		// check if the branch in wich we substitute the key has less elemnt tha min
		if (current->branch[pos]->counter < btr->min)
			// restore
			afc_btree_int_restore(btr, current, pos);
	}

	RETURN(AFC_ERR_NO_ERROR);
	EXCEPT

	FINALLY

	ENDTRY
}

/*
removes an entry from a node

current 	-	 Pointer to node we want to delete key from
pos		-	Position of the key we want to delete

*/

static int afc_btree_int_remove(BTreeNode *current, unsigned long pos)
{
	int i;
	for (i = pos + 1; i <= current->counter; i++)
	{
		current->entry[i - 1] = current->entry[i];
		current->branch[i - 1] = current->branch[i];
		current->branch[i] = NULL;
		current->entry[i] = NULL;
	}
	current->counter--;
	return (AFC_ERR_NO_ERROR);
}

/*
find successor of a key

current 	-	Pointer of node we start from
pos		-	Position of key

*/

static int afc_btree_int_successor(BTree *btr, BTreeNode *current, unsigned long pos)
{
	BTreeNode *leaf;

	// find lowest key possible
	for (leaf = current->branch[pos]; leaf->branch[0]; leaf = leaf->branch[0])
		;

	if (btr->func_clear)
		btr->func_clear(current->entry[pos]);

	// put lowest entry in current
	current->entry[pos] = leaf->entry[1];

	return (AFC_ERR_NO_ERROR);
}

/*
restore a node if one of his branches has less than allowed elements

btr	-	Pointer to a valid BTree structure
current	-	Pointer to the node
pos 	-	Position of the branch with too less entries

*/

static int afc_btree_int_restore(BTree *btr, BTreeNode *current, unsigned long pos)
{
	// leftmost case
	if (pos == 0)
	{
		// if right branch has more than min elements
		if (current->branch[1]->counter > btr->min)
			//
			afc_btree_int_move_left(current, 1);
		else
			// combines two btranches
			afc_btree_int_combine(btr, current, 1);
		return (AFC_ERR_NO_ERROR);
	}
	// rightmost case
	if (pos == current->counter)
	{
		// if left branch has more elements than min
		if (current->branch[pos - 1]->counter > btr->min)
			afc_btree_int_move_right(current, pos);
		else
			afc_btree_int_combine(btr, current, pos);
		return (AFC_ERR_NO_ERROR);
	}
	// if key is in the node
	// if left branch has more elements than min
	if (current->branch[pos - 1]->counter > btr->min)
	{
		afc_btree_int_move_right(current, pos);
		return (AFC_ERR_NO_ERROR);
	}
	//
	if (current->branch[pos + 1]->counter > btr->min)
	{
		afc_btree_int_move_left(current, pos + 1);
		return (AFC_ERR_NO_ERROR);
	}
	afc_btree_int_combine(btr, current, pos);
	return (AFC_ERR_NO_ERROR);
}

/*
move all entries of a node one position right (control on node lenght must be done in caller func )

current		-	Node in which we want to shift
pos		-	Position to start from

*/

static int afc_btree_int_move_right(BTreeNode *current, unsigned long pos)
{
	int c;
	BTreeNode *t;
	t = current->branch[pos];
	// move all right branch's key one position right, we know that counter is lower than max
	for (c = t->counter; c > 0; c--)
	{
		t->entry[c + 1] = t->entry[c];
		t->branch[c + 1] = t->branch[c];
	}
	t->branch[1] = t->branch[0];

	// up counter
	t->counter++;

	// sets first right branch 's entry as current node pointed entry
	t->entry[1] = current->entry[pos];

	t = current->branch[pos - 1];

	// sets current node entry as his left child bigger entry
	current->entry[pos] = t->entry[t->counter];

	// set current node fist element left branch as moved entry right branch
	current->branch[pos]->branch[0] = t->branch[t->counter];
	t->branch[t->counter] = NULL;

	// decrease counter for left child
	t->counter--;
	return (AFC_ERR_NO_ERROR);
}

/*
move all entries of a node one position left (control on node lenght must be done in caller func )

current		-	Node in which we want to shift
pos		-	Position to start from

*/

static int afc_btree_int_move_left(BTreeNode *current, unsigned long pos)
{
	int c;
	BTreeNode *t;
	t = current->branch[pos - 1];
	t->counter++;
	// set this entry as new entry for his left child
	t->entry[t->counter] = current->entry[pos];
	t->branch[t->counter] = current->branch[pos]->branch[0];
	t = current->branch[pos];
	// put current node right child's first entry in current first entry
	current->entry[pos] = t->entry[1];
	// move all right child entries one position left
	t->branch[0] = t->branch[1];
	t->branch[1] = NULL;
	t->counter--;
	for (c = 1; c <= t->counter; c++)
	{
		t->entry[c] = t->entry[c + 1];
		t->branch[c] = t->branch[c + 1];
	}
	return (AFC_ERR_NO_ERROR);
}

/*
combines two records with less than allowed entries

btr	-	Pointer to valid BTree
current	-	Pointer to parent of two nodes
pos	-	Position of righter of them

*/

static int afc_btree_int_combine(BTree *btr, BTreeNode *current, unsigned long pos)
{
	int c;
	BTreeNode *right;
	BTreeNode *left;
	right = current->branch[pos];
	left = current->branch[pos - 1];
	left->counter++;

	void *paux;

	// move current entry as last left branch entry
	left->entry[left->counter] = current->entry[pos];

	// set last left branch entry euqal to right entry first branch
	left->branch[left->counter] = right->branch[0];
	// move all right elements to left node
	for (c = 1; c <= right->counter; c++)
	{
		left->counter++;
		left->entry[left->counter] = right->entry[c];
		left->branch[left->counter] = right->branch[c];
		right->entry[c] = NULL;
		right->branch[c] = NULL;
		paux = left->branch[left->counter];
	}
	for (c = pos; c <= current->counter; c++)
	{
		current->entry[c] = current->entry[c + 1];
		current->branch[c] = current->branch[c + 1];
	}
	// decrease current counter
	current->counter--;
	// delete right node (now it is void )
	afc_btree_int_node_del(btr, right);
	return (AFC_ERR_NO_ERROR);
}

/*
@node afc_btree_int_write

				 NAME: afc_btree_int_write ( btr, lev, node )

			 SYNOPSIS: static int afc_btree_int_write ( BTree * btr, unsigned long *lev, BTreeNode * node )

		  DESCRIPTION: This method writes a node on the record


				INPUT: - btr   - Pointer to a *valid* BTree instance.
			   - lev   - Pointer to variable where level is stored
			   - node  - Pointer to the node that have to be saved

			  RESULTS: - Pointer to entry we want to find
			   - NULL if not present
			 SEE ALSO:

@endnode
*/

static int afc_btree_int_write(BTree *btr, unsigned long *lev, BTreeNode *node)
{
	TRY(int)
	unsigned long pos;
	unsigned short key_size;
	unsigned long size;
	unsigned long offset;
	// check if node exist
	if (node == NULL)
		RETURN(AFC_ERR_NO_ERROR);

	// update level if necessary
	if ((*lev) > btr->lev)
		btr->lev = *lev;
	(*lev)++;

	if (fwrite(&node->counter, sizeof(unsigned short), 1, btr->file) == 0)
		RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE);

	for (pos = 1; pos <= node->counter; pos++)
	{
		if (btr->create_key(node->entry[pos], btr->key_buffer, &key_size) != AFC_ERR_NO_ERROR)
			RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE); // change to  return create_key code

		if (fwrite(&key_size, sizeof(unsigned short), 1, btr->file) == 0)
			RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE);
		if (fwrite(btr->key_buffer, key_size, 1, btr->file) == 0)
			RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE);

		// current position in data file
		offset = ftell(btr->data_file);
		if (fwrite(&offset, sizeof(unsigned long), 1, btr->file) == 0)
			RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE);

		// write data informations
		if (btr->write_node(btr->data_file, node->entry[pos]) != AFC_ERR_NO_ERROR)
			RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE);

		// calculate size of written data
		size = ftell(btr->data_file) - offset;
		if (fwrite(&size, sizeof(unsigned long), 1, btr->file) == 0)
			RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE);
	}
	for (pos = 0; pos <= node->counter; pos++)
	{

		if (node->branch[pos] != NULL)
		{
			afc_btree_int_write(btr, lev, node->branch[pos]);
		}
		else
			// node is a leaf
			break;
	}
	(*lev)--;
	RETURN(AFC_ERR_NO_ERROR);
	EXCEPT

	FINALLY

	ENDTRY
}

/*
reads btree nodes from file

btr	-	pointer to a valid BTree structure
node 	-	pointer to node we want to start adding readed nodes
lev	-	deep counter

*/

static int afc_btree_int_read(BTree *btr, BTreeNode **node, unsigned long *lev)
{
	TRY(int)
	int pos;
	unsigned short key_size;
	unsigned long size;
	unsigned long offset;

	afc_btree_int_new_node(btr, node);
	if ((fread(&((*node)->counter), sizeof(unsigned short), 1, btr->file)) != 1)
		RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_READING_FILE, "Error reading record on file", 0, AFC_BTREE_ERR_READING_FIELDS);

	for (pos = 1; pos <= (*node)->counter; pos++)
	{
		if ((fread(&key_size, sizeof(unsigned short), 1, btr->file)) != 1)
			RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_READING_FILE, "Error reading record on file", 0, AFC_BTREE_ERR_READING_FIELDS);

		if ((fread(btr->key_buffer, key_size, 1, btr->file)) != 1)
			RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_READING_FILE, "Error reading record on file", 0, AFC_BTREE_ERR_READING_FIELDS);
		btr->read_key(&((*node)->entry[pos]), btr->key_buffer, key_size);
		if ((fread(&offset, sizeof(unsigned long), 1, btr->file)) != 1)
			RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_READING_FILE, "Error reading record on file", 0, AFC_BTREE_ERR_READING_FIELDS);

		if ((fread(&size, sizeof(unsigned long), 1, btr->file)) != 1)
			RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_READING_FILE, "Error reading record on file", 0, AFC_BTREE_ERR_READING_FIELDS);

		fseek(btr->data_file, offset, SEEK_SET);
		btr->read_node(&((*node)->entry[pos]), btr->data_file, size);
	}
	// control if leaf
	if ((*lev) < btr->lev)
	{
		(*lev)++;
		for (pos = 0; pos <= (*node)->counter; pos++)
		{
			afc_btree_int_read(btr, &(*node)->branch[pos], lev);
		}
		(*lev)--;
	}

	RETURN(AFC_ERR_NO_ERROR);
	EXCEPT

	FINALLY

	ENDTRY
}
// {{{ TEST_CLASS

#ifdef TEST_CLASS
struct chiave_s
{
	char *key;
	unsigned long id;
};

int create_key(void *entry, void *buffer, unsigned short *size)
{
	*size = (unsigned long)sizeof(void *);
	memcpy(buffer, &entry, *size);
	return (AFC_ERR_NO_ERROR);
}

int create_key_s(void *entry, void *buffer, unsigned short *size)
{
	*size = afc_string_len(((struct chiave_s *)entry)->key);
	memcpy(buffer, ((struct chiave_s *)entry)->key, *size);
	return (AFC_ERR_NO_ERROR);
}

int write_node(FILE *file, void *entry)
{
	TRY(int)
	if (fwrite(&entry, sizeof(void *), 1, file) != 1)
		RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE);
	RETURN(AFC_ERR_NO_ERROR);
	EXCEPT

	FINALLY

	ENDTRY
}

int write_node_s(FILE *file, void *entry)
{
	TRY(int)
	if (fwrite(&((struct chiave_s *)entry)->id, sizeof(unsigned long), 1, file) != 1)
		RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE);
	RETURN(AFC_ERR_NO_ERROR);
	EXCEPT

	FINALLY

	ENDTRY
}

int read_key(void **entry, void *buffer, unsigned short size)
{

	memcpy(entry, &buffer, size);
	return (AFC_ERR_NO_ERROR);
}

int read_key_s(void **entry, void *buffer, unsigned short size)
{
	struct chiave_s *str;
	*entry = (struct chiave_s *)afc_malloc(sizeof(struct chiave_s));
	str = *entry;
	((struct chiave_s *)(*entry))->key = afc_string_new(size + 1);
	memcpy(((struct chiave_s *)(*entry))->key, buffer, size);
	((struct chiave_s *)(*entry))->key[size] = '\0';
	afc_string_reset_len(((struct chiave_s *)(*entry))->key);
	return (AFC_ERR_NO_ERROR);
}

int read_node(void **entry, FILE *file, unsigned short size)
{
	TRY(int)
	if (fread(entry, sizeof(void *), 1, file) != 1)
		RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE);
	RETURN(AFC_ERR_NO_ERROR);
	EXCEPT

	FINALLY

	ENDTRY
}

int read_node_s(void **entry, FILE *file, unsigned short size)
{
	TRY(int)
	if (fread(&((((struct chiave_s *)*entry))->id), sizeof(unsigned long), 1, file) != 1)
		RAISE_RC(AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE);
	RETURN(AFC_ERR_NO_ERROR);
	EXCEPT

	FINALLY

	ENDTRY
}

int print_recursive(int *lev, BTreeNode *node)
{
	int pos;

	if (node == NULL)
		return (AFC_ERR_NO_ERROR);
	(*lev)++;

	printf("livello %d \n", *lev);
	for (pos = 1; pos <= node->counter; pos++)
		printf(" %c ", (int)node->entry[pos]);
	printf("\n");
	for (pos = 0; pos <= node->counter; pos++)
	{
		if (node->branch[pos] != NULL)
		{
			// printf ( "figlio %d\n", pos );
			print_recursive(lev, node->branch[pos]);
			printf(" \n");
		}
	}
	(*lev)--;
	return (AFC_ERR_NO_ERROR);
}

int print_recursive_s(int *lev, BTreeNode *node)
{
	int pos;
	struct chiave_s *str;
	if (node == NULL)
		return (AFC_ERR_NO_ERROR);
	(*lev)++;

	printf("livello %d \n", *lev);
	for (pos = 1; pos <= node->counter; pos++)
	{
		str = node->entry[pos];
		printf(" %s ", ((struct chiave_s *)(node->entry[pos]))->key);
	}
	printf("\n");
	for (pos = 0; pos <= node->counter; pos++)
	{
		if (node->branch[pos] != NULL)
		{
			// printf ( "figlio %d\n", pos );

			print_recursive_s(lev, node->branch[pos]);
			printf(" \n");
		}
	}
	(*lev)--;
	return (AFC_ERR_NO_ERROR);
}

/*
int eval ( void * key, void * entry )
{
	if ( key < entry )
		return ( -1 );
	if ( key == entry )
		return ( 0 );
	return ( 1 );
}
*/

BOOL lt(void *key, void *entry)
{
	return ((key < entry) ? 1 : 0);
}

BOOL eq(void *key, void *entry)
{
	return ((key == entry) ? 1 : 0);
}

BOOL lt_s(void *key, void *entry)
{
	if (strcmp(((struct chiave_s *)key)->key, ((struct chiave_s *)entry)->key) < 0)
		return (TRUE);
	return (FALSE);
}

BOOL eq_s(void *key, void *entry)
{
	if ((strcmp(((struct chiave_s *)key)->key, ((struct chiave_s *)entry)->key)) == 0)
		return (TRUE);
	return (FALSE);
}

int clear(void *p)
{
	// struct chiave_s *k =(struct chiave_s*) p;
	afc_string_delete(((struct chiave_s *)p)->key);
	afc_free(p);
	return (AFC_ERR_NO_ERROR);
}

struct chiave_s *crea_chiave_struct(char *s, unsigned long id)
{
	struct chiave_s *str;
	str = afc_malloc(sizeof(struct chiave_s));
	str->key = afc_string_dup(s);
	str->id = id;
	return (str);
}

int main(int argc, char *argv[])
{
	AFC *afc;
	BTree *btr;
	struct chiave_s *search;
	int liv = 0;
	afc = afc_new();
	afc_track_mallocs(afc);
	afc_set_tags(afc, AFC_TAG_LOG_LEVEL,
				 AFC_LOG_WARNING,
				 AFC_TAG_SHOW_MALLOCS, TRUE,
				 AFC_TAG_END);

	if (argc != 3)
	{
		printf(" test create branch ");
		printf(" create: 0 for create, 1 for reading ");
		printf(" branch: insert branbching factor ");
		return (AFC_ERR_NO_ERROR);
	}
	btr = afc_btree_new();
	afc_btree_init(btr, atoi(argv[2]), lt_s, eq_s, create_key_s, write_node_s, read_key_s, read_node_s);
	// afc_btree_init ( btr, atoi( argv [ 2 ] ), lt, eq, create_key, write_node, read_key, read_node );

	afc_btree_set_clear_func(btr, clear);

	if (atoi(argv[1]) == 0)
	{
		afc_btree_add(btr, crea_chiave_struct("a", 1));
		afc_btree_add(btr, crea_chiave_struct("g", 1));
		afc_btree_add(btr, crea_chiave_struct("f", 1));
		afc_btree_add(btr, crea_chiave_struct("b", 1));
		afc_btree_add(btr, crea_chiave_struct("k", 1));

		afc_btree_add(btr, crea_chiave_struct("d", 1));
		afc_btree_add(btr, crea_chiave_struct("h", 1));
		afc_btree_add(btr, crea_chiave_struct("m", 1));
		afc_btree_add(btr, crea_chiave_struct("j", 1));
		afc_btree_add(btr, crea_chiave_struct("e", 1));
		afc_btree_add(btr, crea_chiave_struct("s", 1));
		afc_btree_add(btr, crea_chiave_struct("i", 1));
		afc_btree_add(btr, crea_chiave_struct("r", 1));
		afc_btree_add(btr, crea_chiave_struct("x", 1));
		afc_btree_add(btr, crea_chiave_struct("c", 1));
		afc_btree_add(btr, crea_chiave_struct("l", 1));
		afc_btree_add(btr, crea_chiave_struct("n", 1));
		afc_btree_add(btr, crea_chiave_struct("t", 1));
		afc_btree_add(btr, crea_chiave_struct("u", 1));
		afc_btree_add(btr, crea_chiave_struct("p", 1));

		/*
		afc_btree_add ( btr, (void *) 'a' );
		afc_btree_add ( btr, (void *) 'g' );
		afc_btree_add ( btr, (void *) 'f' );
		afc_btree_add ( btr, (void *) 'b' );
		afc_btree_add ( btr, (void *) 'k' );
		print_recursive ( &liv, btr->root);
		afc_btree_add ( btr, (void *) 'd' );
		afc_btree_add ( btr, (void *) 'h' );
		afc_btree_add ( btr, (void *) 'm' );
		print_recursive ( &liv, btr->root);
		afc_btree_add ( btr, (void *) 'j' );
		print_recursive ( &liv, btr->root);
		afc_btree_add ( btr, (void *) 'e' );
		afc_btree_add ( btr, (void *) 's' );
		afc_btree_add ( btr, (void *) 'i' );
		afc_btree_add ( btr, (void *) 'r' );
		print_recursive ( &liv, btr->root);
		afc_btree_add ( btr, (void *) 'x' );
		print_recursive ( &liv, btr->root);
		afc_btree_add ( btr, (void *) 'c' );
		afc_btree_add ( btr, (void *) 'l' );
		afc_btree_add ( btr, (void *) 'n' );
		afc_btree_add ( btr, (void *) 't' );
		afc_btree_add ( btr, (void *) 'u' );
		print_recursive ( &liv, btr->root);
		afc_btree_add ( btr, (void *) 'p' );*/
		// print_recursive_s ( &liv, btr->root);
		// printf ( " find : %c ", ( int )afc_btree_find ( btr, (void *) 'z'));
		// afc_btree_del ( btr, (void *) 'a');
		print_recursive_s(&liv, btr->root);
		afc_btree_write(btr, "test_btree.save", "test_btree.data");
	}
	else
	{
		afc_btree_read(btr, "test_btree.save", "test_btree.data");
		print_recursive_s(&liv, btr->root);
		// fflush ( stdout );
	}
	search = crea_chiave_struct("z", 1);
	if (afc_btree_del(btr, search) != AFC_ERR_NO_ERROR)
		printf("\nKey not present");
	clear(search);
	print_recursive_s(&liv, btr->root);
	afc_btree_delete(btr);
	afc_delete(afc);
	return (0);
}

#endif
// }}}
