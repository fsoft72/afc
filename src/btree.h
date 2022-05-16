#ifndef AFC_BTREE_H
#define AFC_BTREE_H

#include "base.h"
#include "exceptions.h"


#define AFC_BTREE_MAGIC	( 'B' << 24 | 'T' << 16 | 'R' << 8 | 'E' )
#define AFC_BTREE_BASE	0xa000
enum
{
	AFC_BTREE_DUPLICATE_KEY = AFC_BTREE_BASE,
	AFC_BTREE_KEY_NOT_PRESENT ,
	AFC_BTREE_ERR_OPEN_FILE,
	AFC_BTREE_ERR_WRITE_FILE,
	AFC_BTREE_ERR_READING_FILE,
	AFC_BTREE_ERR_READING_FIELDS,
	AFC_BTREE_PD_TRUE,
	AFC_BTREE_PD_FALSE
};

struct afc_btree_node
{
	unsigned short			counter;
	void **				entry;
	struct afc_btree_node ** 	branch;

};

typedef struct afc_btree_node BTreeNode;

struct afc_btree
{
	unsigned long 		magic;

	unsigned short		max;
	unsigned short		min;	
	BTreeNode	*	root;
	BOOL (* lower_than) ( void * , void * );
	BOOL (* equal) (void *, void * );

	int (* create_key) (void *, void *, unsigned short * ); // user defined function that writes entry key in a buffer pointed by the first pointer and put size in space linked by pointed the second
	int (* write_node) ( FILE *, void * ); // user defined function for writing entry data on file
	int (* read_key) (void **, void *, unsigned short ); // user defined function that reads key from buffer and put it's value in appropriate struct pointed by the first pointer 
	int (* read_node) ( void **, FILE *, unsigned short ); // user defined function for writing entry data on file
	void *			key_buffer;
	FILE * 			file;
	FILE *			data_file;
	unsigned long 		lev;
	int ( * func_clear ) ( void * );
};

typedef struct afc_btree BTree;

BTree * afc_btree_new ( void );
int _afc_btree_delete ( BTree * btr );
int afc_btree_init ( BTree * btr, unsigned short m, BOOL  (* lower_than) ( void * , void * ), BOOL (* equal) (void *, void * ), int (* create_key) (void *, void *, unsigned short * ), int (* write_node) ( FILE *, void * ), int (* read_key) (void **, void *, unsigned short ), int (* read_node) ( void **, FILE *, unsigned short ) );
int afc_btree_clear   ( BTree * btr );
int afc_btree_set_clear_func ( BTree * btr, int ( *func ) ( void * ) );
int afc_btree_write ( BTree * btr, char * fname, char * f_data_name );
int afc_btree_read ( BTree * btr, char * fname, char * f_data_name );

int afc_btree_add ( BTree * btr, void * entry );
int afc_btree_del ( BTree * btr, void * entry );
void * afc_btree_find ( BTree * btr, void * entry );

#define afc_btree_delete(btr)	if ( btr ) { _afc_btree_delete ( btr ); btr = NULL; }
#endif
