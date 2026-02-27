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
 * test_threader.c - Comprehensive tests for the AFC Threader module.
 *
 * Tests cover creation/deletion, spawning threads that increment counters,
 * waiting for completion, verifying counters were modified, multiple threads
 * with shared data protected by mutex locks, and the clear function.
 *
 * Thread functions receive a ThreaderData* where td->info points to
 * user-supplied data (in our case, shared counters).
 */

#include "test_utils.h"
#include "../src/threader.h"

#include <unistd.h>
#include <pthread.h>

/* Number of increments each thread should perform */
#define INCREMENT_COUNT 100

/* Shared data structure for thread tests */
struct thread_test_data
{
	int counter;		  /* Shared counter incremented by threads */
	int thread_ran;		  /* Flag set to 1 when the thread body executes */
	pthread_mutex_t lock; /* POSIX mutex for protecting the counter */
};

/**
 * _simple_thread_func - A minimal thread function that sets a flag.
 *
 * Receives ThreaderData* where td->info points to a thread_test_data struct.
 * Sets the thread_ran flag to 1 and exits immediately.
 */
static void *_simple_thread_func(void *arg)
{
	ThreaderData *td = (ThreaderData *)arg;
	struct thread_test_data *data = (struct thread_test_data *)td->info;

	data->thread_ran = 1;

	return NULL;
}

/**
 * _counter_thread_func - Thread function that increments a counter safely.
 *
 * Uses the POSIX mutex from the shared data struct to protect increments.
 * Increments the counter INCREMENT_COUNT times.
 */
static void *_counter_thread_func(void *arg)
{
	ThreaderData *td = (ThreaderData *)arg;
	struct thread_test_data *data = (struct thread_test_data *)td->info;

	for (int i = 0; i < INCREMENT_COUNT; i++)
	{
		pthread_mutex_lock(&data->lock);
		data->counter++;
		pthread_mutex_unlock(&data->lock);
	}

	data->thread_ran = 1;

	return NULL;
}

/**
 * _slow_thread_func - Thread function that sleeps briefly then sets a flag.
 *
 * Used to test that afc_threader_wait() actually waits for completion.
 */
static void *_slow_thread_func(void *arg)
{
	ThreaderData *td = (ThreaderData *)arg;
	struct thread_test_data *data = (struct thread_test_data *)td->info;

	/* Sleep 100ms to simulate work */
	usleep(100000);

	data->counter = 42;
	data->thread_ran = 1;

	return NULL;
}

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ----------------------------------------------------------------
	 * 1. Creation and initial state
	 * ---------------------------------------------------------------- */
	Threader *th = afc_threader_new();
	print_res("threader_new != NULL",
		(void *)(long)1, (void *)(long)(th != NULL), 0);

	/* Magic number should be set */
	print_res("magic == THRE",
		(void *)(long)AFC_THREADER_MAGIC,
		(void *)(long)th->magic, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 2. Simple thread: spawn, wait, verify flag
	 * ---------------------------------------------------------------- */
	struct thread_test_data data1;
	data1.counter = 0;
	data1.thread_ran = 0;
	pthread_mutex_init(&data1.lock, NULL);

	int res = afc_threader_add(th, "simple", (ThreaderFunc)_simple_thread_func, &data1);
	print_res("add simple thread OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* Wait for all threads to finish */
	res = afc_threader_wait(th);
	print_res("wait returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* Verify the thread ran */
	print_res("simple thread ran",
		(void *)(long)1, (void *)(long)data1.thread_ran, 0);

	pthread_mutex_destroy(&data1.lock);

	print_row();

	/* ----------------------------------------------------------------
	 * 3. Clean up and create a fresh Threader for next tests
	 * ---------------------------------------------------------------- */
	afc_threader_delete(th);
	th = afc_threader_new();
	print_res("new threader after delete",
		(void *)(long)1, (void *)(long)(th != NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 4. Counter thread: verify counter was incremented
	 * ---------------------------------------------------------------- */
	struct thread_test_data data2;
	data2.counter = 0;
	data2.thread_ran = 0;
	pthread_mutex_init(&data2.lock, NULL);

	res = afc_threader_add(th, "counter1", (ThreaderFunc)_counter_thread_func, &data2);
	print_res("add counter thread OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	afc_threader_wait(th);

	print_res("counter thread ran",
		(void *)(long)1, (void *)(long)data2.thread_ran, 0);
	print_res("counter == INCREMENT_COUNT",
		(void *)(long)INCREMENT_COUNT, (void *)(long)data2.counter, 0);

	pthread_mutex_destroy(&data2.lock);

	print_row();

	/* ----------------------------------------------------------------
	 * 5. Clean up and create fresh Threader for multi-thread test
	 * ---------------------------------------------------------------- */
	afc_threader_delete(th);
	th = afc_threader_new();

	/* ----------------------------------------------------------------
	 * 6. Multiple threads sharing a counter with mutex protection
	 * ---------------------------------------------------------------- */
	struct thread_test_data data3;
	data3.counter = 0;
	data3.thread_ran = 0;
	pthread_mutex_init(&data3.lock, NULL);

	res = afc_threader_add(th, "worker_a", (ThreaderFunc)_counter_thread_func, &data3);
	print_res("add worker_a OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	res = afc_threader_add(th, "worker_b", (ThreaderFunc)_counter_thread_func, &data3);
	print_res("add worker_b OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	afc_threader_wait(th);

	/* Two threads each increment INCREMENT_COUNT times */
	int EXPECTED_TOTAL = INCREMENT_COUNT * 2;
	print_res("2 threads: counter total",
		(void *)(long)EXPECTED_TOTAL, (void *)(long)data3.counter, 0);

	pthread_mutex_destroy(&data3.lock);

	print_row();

	/* ----------------------------------------------------------------
	 * 7. Clean up and create fresh Threader for wait-verification test
	 * ---------------------------------------------------------------- */
	afc_threader_delete(th);
	th = afc_threader_new();

	/* ----------------------------------------------------------------
	 * 8. Slow thread: verify wait() actually blocks until completion
	 * ---------------------------------------------------------------- */
	struct thread_test_data data4;
	data4.counter = 0;
	data4.thread_ran = 0;
	pthread_mutex_init(&data4.lock, NULL);

	afc_threader_add(th, "slow", (ThreaderFunc)_slow_thread_func, &data4);
	afc_threader_wait(th);

	/* After wait, the slow thread should have set counter to 42 */
	print_res("slow thread completed",
		(void *)(long)1, (void *)(long)data4.thread_ran, 0);
	print_res("slow counter == 42",
		(void *)(long)42, (void *)(long)data4.counter, 0);

	pthread_mutex_destroy(&data4.lock);

	print_row();

	/* ----------------------------------------------------------------
	 * 9. Clean up and create fresh Threader for duplicate name test
	 * ---------------------------------------------------------------- */
	afc_threader_delete(th);
	th = afc_threader_new();

	/* ----------------------------------------------------------------
	 * 10. Duplicate thread name: second add should be ignored
	 * ---------------------------------------------------------------- */
	struct thread_test_data data5;
	data5.counter = 0;
	data5.thread_ran = 0;
	pthread_mutex_init(&data5.lock, NULL);

	struct thread_test_data data6;
	data6.counter = 0;
	data6.thread_ran = 0;
	pthread_mutex_init(&data6.lock, NULL);

	afc_threader_add(th, "dupname", (ThreaderFunc)_counter_thread_func, &data5);

	/* Adding another thread with the same name should be silently ignored */
	res = afc_threader_add(th, "dupname", (ThreaderFunc)_counter_thread_func, &data6);
	print_res("dup name add returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	afc_threader_wait(th);

	/* Only the first thread should have run */
	print_res("first dup thread ran",
		(void *)(long)1, (void *)(long)data5.thread_ran, 0);
	print_res("first dup counter ok",
		(void *)(long)INCREMENT_COUNT, (void *)(long)data5.counter, 0);

	/* The second thread data should be untouched */
	print_res("second dup never ran",
		(void *)(long)0, (void *)(long)data6.thread_ran, 0);

	pthread_mutex_destroy(&data5.lock);
	pthread_mutex_destroy(&data6.lock);

	print_row();

	/* ----------------------------------------------------------------
	 * 11. Edge case: clear on NULL
	 * ---------------------------------------------------------------- */
	res = afc_threader_clear(NULL);
	print_res("clear(NULL) returns err",
		(void *)(long)1, (void *)(long)(res != AFC_ERR_NO_ERROR), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 12. Clean up and create fresh Threader for clear-and-reuse test
	 * ---------------------------------------------------------------- */
	afc_threader_delete(th);
	th = afc_threader_new();

	/* ----------------------------------------------------------------
	 * 13. Clear and reuse: add thread, wait, clear, add new, wait again
	 * ---------------------------------------------------------------- */
	struct thread_test_data data7;
	data7.counter = 0;
	data7.thread_ran = 0;
	pthread_mutex_init(&data7.lock, NULL);

	afc_threader_add(th, "first_run", (ThreaderFunc)_simple_thread_func, &data7);
	afc_threader_wait(th);
	print_res("first_run thread ran",
		(void *)(long)1, (void *)(long)data7.thread_ran, 0);

	/* Clear the threader - this cancels/joins remaining threads */
	res = afc_threader_clear(th);
	print_res("clear after wait OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* Add and run a new thread after clear */
	struct thread_test_data data8;
	data8.counter = 0;
	data8.thread_ran = 0;
	pthread_mutex_init(&data8.lock, NULL);

	afc_threader_add(th, "second_run", (ThreaderFunc)_simple_thread_func, &data8);
	afc_threader_wait(th);
	print_res("second_run after clear ran",
		(void *)(long)1, (void *)(long)data8.thread_ran, 0);

	pthread_mutex_destroy(&data7.lock);
	pthread_mutex_destroy(&data8.lock);

	/* ----------------------------------------------------------------
	 * Cleanup and summary
	 * ---------------------------------------------------------------- */
	print_summary();

	afc_threader_delete(th);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
