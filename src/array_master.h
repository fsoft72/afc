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
#ifndef AFC_ARRAY_MASTER_H
#define AFC_ARRAY_MASTER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "base.h"
#include "string.h"
#include "exceptions.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* AFC afc_array_master Magic Number */
#define AFC_ARRAY_MASTER_MAGIC ( 'A' << 24 | 'R' << 16 | 'R' << 8 | 'A' )

/* AFC afc_array_master Base value for constants */
#define AFC_ARRAY_MASTER_BASE 0x8000


/* Errors for afc_array_master */

#define AFC_ARRAY_MASTER_DEFAULT_ITEMS       100

enum { 	AFC_ARRAY_MASTER_ADD_HERE = AFC_ARRAY_MASTER_BASE + 1,
	AFC_ARRAY_MASTER_ADD_TAIL,
	AFC_ARRAY_MASTER_ADD_HEAD 
     };

#define AFC_ARRAY_MASTER_SORT_ELEMENT(kind,varname)	( kind ) ( * ( kind * ) varname )

struct afc_array_master 
{
  	unsigned long magic;

	void * * mem;

  	unsigned long int max_items;
  	unsigned long int current_pos;
  	unsigned long int num_items;

	BOOL is_sorted;
	BOOL before_first;

	int ( *func_clear ) ( void * );
	void ( *custom_sort ) ( void * base, size_t nmemb, size_t size, int ( *compar ) ( const void *, const void *) );
};

typedef struct afc_array_master ArrayMaster;

#define afc_array_master_new(am)	_afc_array_master_new ( __FILE__, __FUNCTION__, __LINE__ )
#define afc_array_master_delete(am)	if ( am ) { _afc_array_master_delete ( am ); am = NULL; }

struct afc_array_master * _afc_array_master_new ( const char * file, const char * func, const unsigned int line );
int    _afc_array_master_delete   ( struct afc_array_master * );
int    afc_array_master_clear    ( struct afc_array_master * );
int    afc_array_master_init     ( ArrayMaster *, unsigned long );
int    afc_array_master_add      ( ArrayMaster *, void *, int );
void * afc_array_master_item     ( ArrayMaster *, unsigned long );
void * afc_array_master_first    ( ArrayMaster * );
void * afc_array_master_next     ( ArrayMaster * );
void * afc_array_master_prev     ( ArrayMaster * );
void * afc_array_master_last 	 ( ArrayMaster * );
void * afc_array_master_obj      ( ArrayMaster * );
short  afc_array_master_is_first ( ArrayMaster * );
short  afc_array_master_is_last  ( ArrayMaster * );
short  afc_array_master_is_empty ( ArrayMaster * );
void * afc_array_master_del      ( ArrayMaster * );
void * afc_array_master_sort     ( ArrayMaster * am, int ( *comp ) ( const void *, const void * ) );
// unsigned long int afc_array_master_current_pos ( ArrayMaster *  );
#define afc_array_master_pos(am)	  ( am ? am->current_pos : 0 )
#define afc_array_master_current_pos(am)  ( am ? am->current_pos : 0 )
#define afc_array_master_succ(am) afc_array_master_next ( am )
#define afc_array_master_num_items(am) afc_array_master_len ( am )
#define afc_array_master_add_tail(am,itm) afc_array_master_add ( am, itm, AFC_ARRAY_MASTER_ADD_TAIL )
#define afc_array_master_add_head(am,itm) afc_array_master_add ( am, itm, AFC_ARRAY_MASTER_ADD_HEAD )
#define afc_array_master_insert(am,itm) afc_array_master_add ( am, itm, AFC_ARRAY_MASTER_ADD_HERE )
unsigned long int afc_array_master_len ( ArrayMaster *  );
int afc_array_master_set_clear_func ( ArrayMaster * am, int ( *func ) ( void * ) );
int afc_array_master_for_each ( ArrayMaster * am, int ( *func ) ( ArrayMaster * am, int pos, void * v, void * info ), void * info );
int afc_array_master_set_custom_sort ( ArrayMaster * am, void ( * func )  ( void * base, size_t nmemb, size_t size, int ( *compar ) ( const void *, const void *) ) );
int afc_array_master_before_first ( ArrayMaster * am );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
