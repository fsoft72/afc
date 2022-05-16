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
#ifndef AFC_MEM_TRACKER_H
#define AFC_MEM_TRACKER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#ifndef AFC_BASE_H
#include "base.h"
#endif

#include "string.h"
#include "exceptions.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* AFC afc_mem_tracker Magic Number */
#define AFC_MEM_TRACKER_MAGIC ( 'A' << 24 | 'R' << 16 | 'R' << 8 | 'A' )

/* AFC afc_mem_tracker Base value for constants */
#define AFC_MEM_TRACKER_BASE 0x8000


/* Errors for afc_mem_tracker */

#define AFC_MEM_TRACKER_DEFAULT_ITEMS       100

enum {	AFC_MEM_TRACKER_ADD_HERE = AFC_MEM_TRACKER_BASE + 1,
	AFC_MEM_TRACKER_ADD_TAIL,
	AFC_MEM_TRACKER_ADD_HEAD
};

struct _mt_data
{
	void * mem;
	size_t size;
	char * file;
	char * func;
	unsigned int line;
};

typedef struct _mt_data MemTrackData;

struct _memtrack {
	MemTrackData * * data;
	unsigned int data_cur;
	unsigned int data_max;

	unsigned int * free;
	int free_cur;
	int free_max;

	char show_mallocs;
	char show_frees;

	unsigned int allocs;
	unsigned int frees;
	unsigned int alloc_bytes;
};

typedef struct _memtrack MemTracker;

// #define afc_mem_tracker_delete(mt)	if ( mt ) { _afc_mem_tracker_delete ( mt ); mt = NULL; }
#define afc_mem_tracker_delete(mt) _afc_mem_tracker_delete ( mt )
#define afc_mem_tracker_free(mt,mem) _afc_mem_tracker_free ( mt, mem, __FILE__, __FUNCTION__, __LINE__ )
#define afc_mem_tracker_update_size(mt,mem,new_mem,size) _afc_mem_tracker_update_size ( mt, mem, new_mem, size, __FILE__, __FUNCTION__, __LINE__ )

MemTracker * afc_mem_tracker_new ( void );
void _afc_mem_tracker_delete ( MemTracker * mt );
// int    afc_mem_tracker_clear    ( struct afc_mem_tracker * ) ;
void * afc_mem_tracker_malloc   ( MemTracker *, size_t size, const char * file, const char * func, const unsigned int line );
void   _afc_mem_tracker_free     ( MemTracker *, void * mem, const char * file, const char * func, const unsigned int line );
void _afc_mem_tracker_update_size ( MemTracker * mt, void * mem, void * new_mem, size_t size, const char * file, const char * func, const unsigned int line );
// int afc_mem_tracker_dump_stats ( MemTracker * mt, char detailed );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
