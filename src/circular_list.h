#ifndef AFC_CIRCULAR_LIST_H
#define AFC_CIRCULAR_LIST_H

#include "base.h"
#include "exceptions.h"


#define AFC_CIRCULAR_LIST_MAGIC	( 'C' << 24 | 'L' << 16 | 'I' << 8 | 'S' )
#define AFC_CIRCULAR_LIST_BASE	0x1000
enum
{	
	AFC_CIRCULAR_LIST_ERR_MAX_ELEMS = AFC_CIRCULAR_LIST_BASE
};

struct afc_circular_list_node
{
	char *				data;
	struct afc_circular_list_node *	next;
	struct afc_circular_list_node * prev;
};

typedef struct afc_circular_list_node CircularListNode;

struct afc_circular_list
{
	unsigned long 		magic;
	CircularListNode *	pointer;
	int (*func_clear) (void*);
	int			count;
	int			max_elems;
};

typedef struct afc_circular_list CircularList;

CircularList * afc_circular_list_new ( void );
int _afc_circular_list_delete ( CircularList * cl );
int afc_circular_list_clear   ( CircularList * cl );
void * afc_circular_list_del ( CircularList * cl );
#define afc_circular_list_delete(cl)	if ( cl ) { _afc_circular_list_delete ( cl ); cl = NULL; }

int afc_circular_list_set_clear_func ( CircularList * cl, int ( *func ) ( void * ) );
void * afc_circular_list_init ( CircularList * cl, int max_elems );
//void * afc_circular_list_obj ( CircularList * cl );
void * afc_circular_list_next ( CircularList * cl );
void * afc_circular_list_prev( CircularList * cl );
int afc_circular_list_add ( CircularList * cl, void * data );
int afc_circular_list_set_clear_func ( CircularList * am, int ( *func ) ( void * ) );
#define afc_circular_list_obj(cl) (cl->pointer? cl->pointer->data : NULL )
#endif
