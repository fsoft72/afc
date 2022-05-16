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

#include "threader.h"

// {{{ docs
/*
@config
	TITLE:     Threader
	VERSION:   1.00
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
@endnode

@node quote
	*Who are you going to believe, me or your own eyes?*

		Groucho Marx
@endnode

@node intro
Threader is a class based upon <classname>pthread</classname> aimed to ease the creation of
multi threaded applications under Linux. It is not to be considered a full replacement or interface
to the ``pthread`` library, since just the most used functions have been implemented.

Anyway, Threader should be a valid tool to create threaded application in a breeze.

Main features are:

	 - Ability to stop (cancel) a thread at any time
	 - Mutex (lock) support
	 - Asynchronous interface

Like all AFC classes, you can instance a new  Threader by calling afc_threader_new (), 
and free it with afc_threader_delete (). By deleting a Threader class, you also will delete all
running threads spawned by that Threader instance. 


To add threads to your application, simply call afc_threader_add (), and to cancel it call the afc_threader_cancel ().

After all your basic threads have been created, your app must call the afc_threader_wait() method that will wait until all running threads have finished.

Mutexes (locks) can be used to create *semaphores*. A *semaphore*
is a flag that grant or deny access to a specific resource in your application. It is useful when you want
to synchronize access of two or more threads to the same resource.

To lock a mutex, call afc_threader_thread_lock() and to release the lock the afc_threader_thread_unlock().
@endnode
*/
// }}}

static const char class_name [] = "Threader";

// {{{ statics
static ThreaderData * afc_threader_internal_data_new ( Threader * th, void * info );
static int afc_threader_internal_data_delete ( ThreaderData * td );
static int afc_threader_internal_data_del_lock ( ThreaderData * td, pthread_mutex_t * lock );
static int afc_threader_internal_data_clear ( ThreaderData * td );
static int afc_threader_internal_cancel_thread ( Threader * t, ThreaderData * td );
static int afc_threader_internal_remove_threads ( Threader * t );
static int afc_threader_internal_free_mutex ( Threader * t );
static int afc_threader_internal_free_threads ( Threader * t );
// static int afc_threader_internal_thread_init ( ThreaderData * td );
// }}}

// {{{ afc_threader_new ()
/*
@node afc_threader_new

	         NAME: afc_threader_new () - Initializes a new Threader instance.

	     SYNOPSIS: Threader * afc_threader_new ()

	  DESCRIPTION: This function initializes a new Threader instance.

	        INPUT: NONE

	      RESULTS: a valid inizialized Threader structure. NULL in case of errors.

	     SEE ALSO: - afc_threader_delete()

@endnode
*/
Threader * afc_threader_new ()
{
TRY ( Threader * )

	Threader * t = ( Threader * ) afc_malloc ( sizeof ( Threader ) );

	if ( t == NULL ) RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "Threader", NULL );
	t->magic = AFC_THREADER_MAGIC;

	if ( ( t->threads = afc_dictionary_new () ) == NULL ) 
		RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "threads", NULL );

	if ( ( t->mutex = afc_dictionary_new () ) == NULL ) 
		RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "mutex", NULL );

	if ( ( t->thread_stack = afc_array_master_new () ) == NULL )
		RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "stack", NULL );

	RETURN ( t );

EXCEPT
	afc_threader_delete ( t );

FINALLY

ENDTRY
}
// }}}
// {{{ afc_threader_delete ( t )
/*
@node afc_threader_delete

	         NAME: afc_threader_delete ( t )  - Disposes a valid Threader instance.

	     SYNOPSIS: int afc_threader_delete ( Threader * t)

	  DESCRIPTION: This function frees an already alloc'd Threader structure.

	        INPUT: - t  - Pointer to a valid afc_threader class.

	      RESULTS: should be AFC_ERR_NO_ERROR

	        NOTES: - this method calls: afc_threader_clear()

	     SEE ALSO: - afc_threader_new()
	               - afc_threader_clear()
@endnode
*/
int _afc_threader_delete ( Threader * t ) 
{
	int afc_res; 

	if ( ( afc_res = afc_threader_clear ( t ) ) != AFC_ERR_NO_ERROR ) return ( afc_res );

	afc_dictionary_delete ( t->threads );
	afc_dictionary_delete ( t->mutex );
	afc_array_master_delete ( t->thread_stack );

	afc_free ( t );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_threader_clear ( t )
/*
@node afc_threader_clear

	         NAME: afc_threader_clear ( t )  - Clears all stored data

	     SYNOPSIS: int afc_threader_clear ( Threader * t)

	  DESCRIPTION: Use this function to clear all stored data in the current t instance.

	        INPUT: - t    - Pointer to a valid afc_threader instance.

	      RESULTS: should be AFC_ERR_NO_ERROR
	              
	     SEE ALSO: - afc_threader_delete()
@endnode
*/
int afc_threader_clear ( Threader * t ) 
{
	if ( t == NULL ) return ( AFC_ERR_NULL_POINTER );
 
	if ( t->magic != AFC_THREADER_MAGIC ) return ( AFC_ERR_INVALID_POINTER );

	/* Custom Clean-up code should go here */
	afc_dprintf ( "%s: 1\n", __FUNCTION__ );
	afc_threader_internal_remove_threads ( t );
	afc_dprintf ( "%s: 2\n", __FUNCTION__ );
	afc_threader_internal_free_mutex ( t );
	afc_dprintf ( "%s: 3\n", __FUNCTION__ );
	afc_threader_internal_free_threads ( t );


	afc_dprintf ( "%s: 4\n", __FUNCTION__ );
	// if ( t->threads ) 	afc_dictionary_clear ( t->threads );
	afc_dprintf ( "%s: 5\n", __FUNCTION__ );
	if ( t->mutex ) 	afc_dictionary_clear ( t->mutex );
	afc_dprintf ( "%s: 6\n", __FUNCTION__ );
	if ( t->thread_stack ) 	afc_array_master_clear ( t->thread_stack );

	afc_dprintf ( "%s: 7\n", __FUNCTION__ );
	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_threader_add ( t, name, func, info )
/*
@node afc_threader_add

	         NAME: afc_threader_add ( t, name, func, info )  - Adds (and spawn) a new Thread

	     SYNOPSIS: int afc_threader_add ( Threader * t, char * name, ThreaderFunc func, void * info )

	  DESCRIPTION: Use this function to add a new running thread to your application.

	        INPUT: - t    - Pointer to a valid afc_threader instance.
		       - name - Name of the thread. This is a unique name you have to provide
				to be able to reach the thread later on.
		       - func - Pointer to the starting function of the thread. 
		       - info - Additional info to be sent to the thread launcher function.

	      RESULTS: should be AFC_ERR_NO_ERROR
	              
	     SEE ALSO: - afc_threader_wait()
	               - afc_threader_cancel()
@endnode
*/
int afc_threader_add ( Threader * th, const char * name, ThreaderFunc func, void * info )
{
	ThreaderData * td;
	int ret;

	if ( afc_dictionary_has_key ( th->threads, name ) ) return ( AFC_ERR_NO_ERROR );
 	
	// Alloc needed data for a Threader Thread Data
	if ( ( td = afc_threader_internal_data_new ( th, info ) ) == NULL )
		return ( AFC_LOG_FAST ( AFC_ERR_NO_MEMORY ) );

	// Create the thread
	if ( ( ret = pthread_create ( &td->thread, NULL, func, td ) ) != 0 )
	{
		afc_string_make ( __internal_afc_base->tmp_string, "%d", ret );
		return ( AFC_LOG ( AFC_LOG_ERROR, AFC_THREADER_ERR_CREATE_THREAD, "pthread_create failed", __internal_afc_base->tmp_string ) );
	}

	// Add the thread to the internal Dictionary
	afc_dictionary_set ( th->threads, name, td );

	// Add the thread to the stack of threads running
	afc_array_master_add ( th->thread_stack, td, AFC_ARRAY_MASTER_ADD_TAIL );

	afc_dprintf ( "%s: Adding ThreaderData: %x\n", __FUNCTION__, ( int ) td );

	// fprintf ( stderr, "RUN: %s %d\n", name, ( int ) td );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_threader_wait ( t )
/*
@node afc_threader_wait

	         NAME: afc_threader_wait ( t )  - Waits for all thread to finish

	     SYNOPSIS: int afc_threader_wait ( Threader * t )

	  DESCRIPTION: You must call this function to pthread_join all threads you have created
		       and to wait for them to finish.

	        INPUT: - t    - Pointer to a valid afc_threader instance.

	      RESULTS: should be AFC_ERR_NO_ERROR
	              
	     SEE ALSO: - afc_threader_cancel()
		       - afc_threader_add()
@endnode
*/
int afc_threader_wait ( Threader * th )
{
	ThreaderData * td;

	// Scan all the thread created and add them to the list
	td = afc_array_master_last ( th->thread_stack );
	while ( td != NULL )
	{
		// If the thread is not joined yet, we join it
		if ( td->waiting == FALSE )
		{
			td->waiting = TRUE;
			pthread_join ( td->thread, NULL );
		}
		td = afc_array_master_prev ( th->thread_stack );
	}

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_threader_cancel ( t, thread_name )
/*
@node afc_threader_cancel

	         NAME: afc_threader_cancel ( t, thread_name )  - Aborts a running thread

	     SYNOPSIS: int afc_threader_cancel ( Threader * t, char * thread_name )

	  DESCRIPTION: You must call this function to abort a running thread.

	        INPUT: - t    - Pointer to a valid afc_threader instance.

	      RESULTS: should be AFC_ERR_NO_ERROR
	              
	     SEE ALSO: - afc_threader_cancel()
		       - afc_threader_add()
@endnode
*/
int afc_threader_cancel ( Threader * th, const char * thread_name )
{
	ThreaderData * td;
	int res;

	if ( ( td = afc_dictionary_get ( th->threads, thread_name ) ) == NULL )
		return ( AFC_LOG ( AFC_LOG_WARNING, AFC_THREADER_ERR_THREAD_NOT_FOUND, "thread not found", thread_name ) );


	res = afc_threader_internal_cancel_thread ( th, td );

	afc_dictionary_set ( th->threads, thread_name, NULL );
	// afc_threader_internal_data_delete ( td );

	return ( res );
}
// }}}
// {{{ afc_threader_thread_lock ( td, lock_name, wait )
/*
@node afc_threader_thread_lock

	         NAME: afc_threader_thread_lock ( td, lock_name, wait )  - Attemps to lock a Mutex

	     SYNOPSIS: int afc_threader_thread_lock ( ThreaderData * td, char * lock_name, short wait )

	  DESCRIPTION: 	Use this function to create or obtain a lock to a mutex. If the Mutex called 
		       	/lock_name/ already exists, Threader will try to grant you the lock. If the /lock_name/
		       	does not exist yet, Threader will create the new Mutex and lock it.
		
			If the Mutex already exists, it could already be locked by some other thread, and that's way
			you have to specify the /wait/ flag. If /wait/ is set to TRUE, your thread will attemp to obtain
			a lock to the Mutex, waiting until the Mutex is avaible. In other words, the thread stops its execution
			until it can obtain the lock. Keep in mind that this can cause deadlocks if your code is not written 
			correctly. 	

	        INPUT: - td    		- Pointer to a valid ThreaderData instance.
		       - lock_name	- Name of the lock (Mutex)
		       - wait		- If TRUE the function will wait until the Mutex is avaible.

	      RESULTS: 	this function returns AFC_ERR_NO_ERROR when the lock has been obtained.
		       	If there is not enough memory to create the Mutex, you'll get an AFC_ERR_NO_MEMORY.
		       	In all the other cases, you'll get the result value coming directly from pthread.
			As a rule of thumb, consider only AFC_ERR_NO_ERROR as the valid return code to 
			continue your program correctly.

		NOTES: - Once created, a new Mutex is not removed by the internal array of existing Mutexes.
			That's because the creation of a Mutex is cpu intensive and it is cheaper to keep it in
			memory rather then delete it. Furthermore when a thread afc_threader_thread_unlock() a Mutex
			there could be another thread waiting to obtain it, so we cannot free it as we like.
			Since a Mutex requires some resources, try to use as few locks as possible in your application.

			- lock_name is case sensitive. A lock called "mylock" and another called "MyLock" are two different
			  locks.
	              
	     SEE ALSO: - afc_threader_thread_unlock()
	               
@endnode
*/
int afc_threader_thread_lock ( ThreaderData * td, char * lock_name, short wait )
{
	Threader * th = td->th;
	pthread_mutex_t * mutex;
	int res;

	// If the thread cannot lock, simply return with no error
	if ( td->can_lock == FALSE ) return ( AFC_ERR_NO_ERROR );

	// If the mutex does not exists (ie. it is NULL)
	if ( ( mutex = afc_dictionary_get ( th->mutex, lock_name ) ) == NULL )
	{
		// we alloc one of pthread_mutex_t size
		if ( ( mutex = afc_malloc ( sizeof ( pthread_mutex_t ) ) ) == NULL )
			return ( AFC_LOG_FAST_INFO ( AFC_ERR_NO_MEMORY, "create mutex" ) );

		// add it to the th->mutex dictionary with the name specified in lock_name
		afc_dictionary_set ( th->mutex, lock_name, mutex );

		// initialize the mutex
		pthread_mutex_init ( mutex, NULL );
	}

	// Try to lock the mutex (according to the "wait" policy)
	if ( wait )
		res = pthread_mutex_lock ( mutex );
	else
		res = pthread_mutex_trylock ( mutex );


	// If res == 0, add the lock to the ones owned by the thread itself
	if ( res == 0 )
		afc_array_master_add ( td->locks, mutex, AFC_ARRAY_MASTER_ADD_TAIL );

	return ( res );
}
// }}}
// {{{ afc_threader_thread_unlock ( td, lock_name )
/*
@node afc_threader_thread_unlock

	         NAME: afc_threader_thread_unlock ( td, lock_name )  - Attemps to unlock a Mutex

	     SYNOPSIS: int afc_threader_thread_unlock ( ThreaderData * td, char * lock_name )

	  DESCRIPTION: 	Use this function to release a lock to the Mutex called /lock_name/.

	        INPUT: - td    		- Pointer to a valid ThreaderData instance.
		       - lock_name	- Name of the lock (Mutex)

	      RESULTS: 	- this function returns AFC_ERR_NO_ERROR when the lock has been successfully released.
			- If the lock cannot be found, you'll get a AFC_THREADER_ERR_LOCK_NOT_FOUND error.
			  This error can only occurs when the /lock_name/ is invalid (remember that lock names are
			  case sensitive).

		NOTES:	- The lock is released but the Mutex is not freed because some other thread could be there
			  waiting to the Mutex to be free to lock by itself.

	     SEE ALSO: - afc_threader_thread_lock()
@endnode
*/
int afc_threader_thread_unlock ( ThreaderData * td, char * lock_name )
{
	Threader * th = td->th;
	pthread_mutex_t * mutex;
	// int oldtype;
	int res;

	// If the thread cannot lock, simply return
	if ( td->can_lock == FALSE ) return ( AFC_ERR_NO_ERROR );

	// get the mutex by the dictionary of all alloc'd mutex
	if ( ( mutex = afc_dictionary_get ( th->mutex, lock_name ) ) == NULL )
		return ( AFC_LOG ( AFC_LOG_ERROR, AFC_THREADER_ERR_LOCK_NOT_FOUND, "lock not found", lock_name ) );

	// delete the mutex from the pool of the thread
	if ( afc_threader_internal_data_del_lock ( td, mutex ) != AFC_ERR_NO_ERROR ) 
		return ( AFC_THREADER_ERR_LOCK_NOT_FOUND );

	res = pthread_mutex_unlock ( mutex );

	return ( res );
}
// }}}


// ----------------------------------------------------------------------------------------------------------------
// INTERNAL FUNCTIONS
// ----------------------------------------------------------------------------------------------------------------
/*
static int afc_threader_internal_thread_init ( ThreaderData * td )
{
	pthread_setcanceltype ( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );

	return ( AFC_ERR_NO_ERROR );
}
*/

// {{{ afc_threader_internal_data_new ( th, info )
static ThreaderData * afc_threader_internal_data_new ( Threader * th, void * info )
{
	ThreaderData * td;

	if ( ( td = afc_malloc ( sizeof ( ThreaderData ) ) ) == NULL )
	{
		AFC_LOG_FAST ( AFC_ERR_NO_MEMORY );
		return ( NULL );
	}

	if ( ( td->locks = afc_array_master_new () ) == NULL )
	{
		AFC_LOG_FAST_INFO ( AFC_ERR_NO_MEMORY, "locks" );
		afc_threader_internal_data_delete ( td );
		return ( NULL );
	}

	td->th   = th;
	td->info = info;

	// Default values
	td->cancel_enabled  = TRUE;
	td->cancel_deferred = FALSE;
	td->can_lock	    = TRUE;

	return ( td );
}
// }}}
// {{{ afc_threader_internal_data_clear ( td )
static int afc_threader_internal_data_clear ( ThreaderData * td )
{
	pthread_mutex_t * lock;

	if ( td == NULL ) return ( AFC_ERR_NO_ERROR );

	td->cancel_enabled = FALSE;
	td->can_lock = FALSE;

	if ( td->locks )
	{
		lock = afc_array_master_first ( td->locks );
		while ( lock )
		{
			pthread_mutex_unlock ( lock );
			lock = afc_array_master_next ( td->locks );
		}

		afc_array_master_clear ( td->locks );
	}

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_threader_internal_data_delete ( td )
static int afc_threader_internal_data_delete ( ThreaderData * td )
{
	if ( td == NULL ) return ( AFC_ERR_NO_ERROR );

	afc_threader_internal_data_clear ( td );

	afc_array_master_delete ( td->locks );
	afc_free ( td );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_threader_internal_data_del_lock ( td, lock )
static int afc_threader_internal_data_del_lock ( ThreaderData * td, pthread_mutex_t * lock )
{
	pthread_mutex_t * ilock;

	// fprintf ( stderr, "DEL LOCK: %d - %d\n", ( int ) td->thread, ( int ) lock );

	ilock = afc_array_master_first ( td->locks );
	while ( ilock )
	{
		if ( lock == ilock )
		{
			afc_array_master_del ( td->locks );
			return ( AFC_ERR_NO_ERROR );
		}

		ilock = afc_array_master_next ( td->locks );
	}

	return ( AFC_THREADER_ERR_LOCK_NOT_FOUND );
}
// }}}
// {{{ afc_threader_internal_remove_threads ( t )
static int afc_threader_internal_remove_threads ( Threader * t )
{
	ThreaderData * td;

	afc_dprintf ( "%s: 1\n", __FUNCTION__ );
	td = afc_dictionary_first ( t->threads );
	while ( td )
	{
		afc_dprintf ( "%s: 2\n", __FUNCTION__ );
		afc_threader_internal_cancel_thread ( t, td );
		afc_dprintf ( "%s: 3\n", __FUNCTION__ );
		td = afc_dictionary_next ( t->threads );
	}	

	// fprintf ( stderr, "remove_threads end\n" );

	afc_dprintf ( "%s: 4\n", __FUNCTION__ );
	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_threader_internal_cancel_thread ( t, td )
static int afc_threader_internal_cancel_thread ( Threader * t, ThreaderData * td )
{	
	afc_dprintf ( "%s: 1\n", __FUNCTION__ );
	if ( td->cancel_enabled == FALSE ) return ( AFC_ERR_NO_ERROR );

	afc_dprintf ( "%s: 2\n", __FUNCTION__ );
	// fprintf ( stderr, "CANCEL for: %d - canc enabled: %d - deferred: %d\n", ( int ) td->thread, ( int ) td->cancel_enabled, ( int ) td->cancel_deferred );
	if ( ( td->cancel_enabled == TRUE ) && ( td->cancel_deferred == FALSE ) )
		afc_threader_internal_data_clear ( td );

	afc_dprintf ( "%s: 3\n", __FUNCTION__ );
	pthread_cancel ( td->thread );

	afc_dprintf ( "%s: 4\n", __FUNCTION__ );
	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_threader_internal_free_mutex ( t )
static int afc_threader_internal_free_mutex ( Threader * t )
{
	pthread_mutex_t * lock;

	afc_dprintf ( "%s: 1\n", __FUNCTION__ );
	lock = afc_dictionary_first ( t->mutex );
	while ( lock )
	{
		afc_dprintf ( "%s: 2\n", __FUNCTION__ );
		pthread_mutex_unlock ( lock );
		afc_dprintf ( "%s: 3\n", __FUNCTION__ );
		afc_free ( lock );

		afc_dprintf ( "%s: 4\n", __FUNCTION__ );
		lock = afc_dictionary_next ( t->mutex );
	}

	afc_dprintf ( "%s: 5\n", __FUNCTION__ );
	afc_dictionary_clear ( t->mutex );

	afc_dprintf ( "%s: 6\n", __FUNCTION__ );
	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_threader_internal_free_threads ( t )
static int afc_threader_internal_free_threads ( Threader * t )
{
	ThreaderData * td;

	afc_dprintf ( "%s: 1\n", __FUNCTION__ );
	td = afc_dictionary_first ( t->threads );
	while ( td )
	{
		afc_dprintf ( "%s: 2\n", __FUNCTION__ );
		afc_threader_internal_data_delete ( td );
		afc_dprintf ( "%s: 3\n", __FUNCTION__ );
		td = afc_dictionary_next ( t->threads );
	}

	afc_dprintf ( "%s: 4\n", __FUNCTION__ );

	return ( AFC_ERR_NO_ERROR );
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
#include <unistd.h>

void task1 ( ThreaderData * td )
{
	int * counter = td->info;

	afc_threader_thread_lock ( td, "hello", TRUE );
	afc_threader_thread_lock ( td, "world", TRUE );

	while ( *counter < 300 )
	{
		afc_threader_thread_lock ( td, "stop", TRUE );
		printf ( "*** task1 count: %d\n", *counter );
		(*counter)++;
		afc_threader_thread_unlock ( td, "stop" );
		sleep ( 1 );
	}
}

void task2 ( ThreaderData * td )
{
	int * counter = td->info;

	while ( *counter < 300 )
	{
		if ( afc_threader_thread_lock ( td, "stop", FALSE ) == AFC_ERR_NO_ERROR )
		{
			printf ( "task2 count: %d\n", *counter );
			(*counter)++;

			afc_threader_thread_unlock ( td, "stop" );

			if ( ! ( *counter % 10 ) ) sleep ( 1 );

			if ( *counter == 201 )
				afc_threader_cancel ( td->th, "task1" );

		} else {
			fprintf ( stderr, "task2: cannot lock \"stop\"\n" );
		}

	}
}

void task3 ( ThreaderData * td )
{
	printf ( "task3 sleeping...\n" );
	sleep ( 3 );

	printf ( "task3 killing all...\n" );
	afc_threader_clear ( td->th );
}

int main ( int argc, char * argv[] )
{
	AFC * afc = afc_new ();
	Threader * t;
	int c1 = 0, c2 = 0;

	afc_track_mallocs ( afc );

	afc_set_tags ( afc, AFC_TAG_LOG_LEVEL, AFC_LOG_WARNING,
		       AFC_TAG_END );


	if ( ( t = afc_threader_new () ) == NULL )
	{
	  fprintf ( stderr, "Init of class Threader failed.\n" );
	  return ( 1 );
	}

	afc_threader_add ( t, "task1", (ThreaderFunc) task1, &c1 );
	afc_threader_add ( t, "task2", (ThreaderFunc) task2, &c2 );
	afc_threader_add ( t, "task3", (ThreaderFunc) task3, NULL );

	afc_threader_wait ( t );

	afc_threader_delete ( t );

	printf ( "Task1: %d - Task2: %d\n", c1, c2 );

	afc_delete ( afc );

	return ( 0 ); 
}
// }}}
#endif
