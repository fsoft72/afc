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
 * test_str_error.c - Tests for per-thread last-error reporting.
 *
 * The AFC base object is a process-wide global; afc_str_error() must return
 * the error logged by the calling thread, not one clobbered by another
 * thread through the shared base.
 */

#include <pthread.h>

#include "test_utils.h"
#include "../src/base.h"

static void *worker(void *arg)
{
	const char *msg = (const char *)arg;

	afc_log(__internal_afc_base, AFC_LOG_ERROR, 1, "Test", "worker", msg, NULL);

	/* Give the main thread a chance to clobber the shared base */
	usleep(100000);

	/* The thread must still see its own message */
	return (void *)(strcmp(afc_str_error(), msg) == 0 ? NULL : (void *)1);
}

int main(void)
{
	AFC *afc = afc_new();
	pthread_t th;
	void *ret;
	static const char *thread_msg = "thread specific error";

	test_header();

	print_res("str_error not NULL", (void *)1, (void *)(long)(afc_str_error() != NULL), 0);

	/* Log from the main thread and verify */
	afc_log(afc, AFC_LOG_ERROR, 2, "Test", "main", "main error", NULL);
	print_res("main sees own error", (void *)"main error", (void *)afc_str_error(), 1);

	/* A worker thread logs its own error; the global base is shared, but
	   each thread must keep its own last-error view */
	pthread_create(&th, NULL, worker, (void *)thread_msg);
	usleep(10000);
	afc_log(afc, AFC_LOG_ERROR, 3, "Test", "main", "clobbered", NULL);
	pthread_join(th, &ret);
	print_res("worker kept own error", (void *)0, (void *)ret, 0);

	/* Main thread still sees its own message */
	print_res("main error intact", (void *)"clobbered", (void *)afc_str_error(), 1);

	print_summary();
	afc_delete(afc);
	return get_test_failures();
}
