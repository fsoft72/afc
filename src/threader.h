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
#ifndef AFC_THREADER_H
#define AFC_THREADER_H
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>

#include "base.h"
#include "strings.h"
#include "array_master.h"
#include "dictionary.h"
#include "hash_master.h"

#ifdef MINGW
	#define sleep _sleep
#endif

/* Threader'Magic' value: 'THRE' */
#define AFC_THREADER_MAGIC ( 'T' << 24 | 'H' << 16 | 'R' << 8 | 'E' )

/* Threader Base  */
#define AFC_THREADER_BASE 0x12000

/* ERROR MESSAGES */
enum { 	AFC_THREADER_ERR_CREATE_THREAD = AFC_THREADER_BASE + 1, 		// pthread_create failed
	AFC_THREADER_ERR_THREAD_NOT_FOUND,					// Cannot find the thread defined
	AFC_THREADER_ERR_LOCK, 							// pthread_mutex_lock failed
	AFC_THREADER_ERR_LOCK_NOT_FOUND,					// Desired lock was not found
	AFC_THREADER_ERR_LOCK_BUSY						// Lock already in use
};

/* MACROS */
#define AFC_THREADER_CANCEL_ENABLE(td)		\
		pthread_setcancelstate ( PTHREAD_CANCEL_ENABLE, NULL ); \
		td->cancel_enabled = TRUE;

#define AFC_THREADER_CANCEL_DISABLE(td)		\
		pthread_setcancelstate ( PTHREAD_CANCEL_DISABLE, NULL ); \
		td->cancel_enabled = FALSE;

#define AFC_THREADER_CANCEL_ASYNC(td)		\
		pthread_setcanceltype  ( PTHREAD_CANCEL_ASYNCHRONOUS, NULL ); \
		td->cancel_deferred = FALSE;

#define AFC_THREADER_CANCEL_DEFERRED(td)	\
		pthread_setcanceltype  ( PTHREAD_CANCEL_DEFERRED, NULL );	\
		td->cancel_deferred = TRUE;

struct afc_threader
{
	unsigned long magic;     /* Threader Magic Value */
	Dictionary * threads;
	Dictionary * mutex;
	ArrayMaster * thread_stack;
};

typedef struct afc_threader Threader;

struct afc_threader_data
{
	Threader * th;		/* Pointer to the main Threader class */

	ArrayMaster * locks;	// List of all locks by this thread 

	pthread_t thread;
	void * info;

	short  waiting;		// If TRUE, then pthread_join() has been already called on it
	short  can_lock;	// if FALSE, the thread cannot create locks

	short  cancel_enabled;	// If TRUE, the thread can be cancelled
	short  cancel_deferred;	// If TRUE, then cancel mode is deferred on it
};

typedef struct afc_threader_data ThreaderData;

typedef void * (* ThreaderFunc ) ( void * );

/* Function Prototypes */
#define afc_threader_delete(th)	if ( th ) { _afc_threader_delete ( th ); th = NULL; }

Threader * afc_threader_new (void);
int _afc_threader_delete ( Threader * t );
int afc_threader_clear ( Threader * t );

int afc_threader_add ( Threader * th, const char * name, ThreaderFunc func, void * info );
int afc_threader_wait ( Threader * th );
int afc_threader_cancel ( Threader * th, const char * thread_name );

int afc_threader_thread_lock ( ThreaderData * td, char * lock_name, short wait );
int afc_threader_thread_unlock ( ThreaderData * td, char * lock_name );
#endif
