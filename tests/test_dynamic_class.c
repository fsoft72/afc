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
 * test_dynamic_class.c - Comprehensive tests for the AFC DynamicClass module.
 *
 * Tests cover creation/deletion, adding methods with callbacks, executing
 * methods with arguments, setting/getting variables of different kinds
 * (numeric, string, pointer), overwriting variables, clearing, and
 * edge cases like executing non-existent methods.
 */

#include "test_utils.h"
#include "../src/dynamic_class.h"

/* Static counter incremented by test methods to verify execution */
static int METHOD_CALL_COUNT = 0;

/* Static variable to capture the last argument received by a method */
static void *LAST_ARG_RECEIVED = NULL;

/**
 * _test_method_simple - A simple test callback that increments a counter.
 *
 * This method takes no arguments (beyond the end sentinel) and simply
 * increments the global METHOD_CALL_COUNT to prove it was called.
 */
static int _test_method_simple(DynamicClass *dc)
{
	METHOD_CALL_COUNT++;
	return AFC_ERR_NO_ERROR;
}

/**
 * _test_method_with_args - A test callback that reads arguments from the array.
 *
 * Reads the first argument from dc->args and stores it in LAST_ARG_RECEIVED.
 * Increments METHOD_CALL_COUNT as well.
 */
static int _test_method_with_args(DynamicClass *dc)
{
	METHOD_CALL_COUNT++;
	LAST_ARG_RECEIVED = afc_array_first(dc->args);
	return AFC_ERR_NO_ERROR;
}

/**
 * _test_method_returns_error - A test callback that returns a custom error.
 *
 * Used to verify that execute() propagates the return code from the method.
 */
static int _test_method_returns_error(DynamicClass *dc)
{
	METHOD_CALL_COUNT++;
	return 42;
}

/**
 * _test_method_reads_var - A test callback that reads a DynamicClass variable.
 *
 * Reads the variable "counter" and stores its value in dc->result for
 * external verification.
 */
static int _test_method_reads_var(DynamicClass *dc)
{
	METHOD_CALL_COUNT++;
	dc->result = afc_dynamic_class_get_var(dc, "counter");
	dc->result_type = AFC_DYNAMIC_CLASS_RESULT_TYPE_INTEGER;
	return AFC_ERR_NO_ERROR;
}

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ----------------------------------------------------------------
	 * 1. Creation and initial state
	 * ---------------------------------------------------------------- */
	DynamicClass *dc = afc_dynamic_class_new();
	print_res("dynamic_class_new != NULL",
		(void *)(long)1, (void *)(long)(dc != NULL), 0);

	/* Magic number should be set correctly */
	print_res("magic == DYNC",
		(void *)(long)AFC_DYNAMIC_CLASS_MAGIC,
		(void *)(long)dc->magic, 0);

	/* add_arg_end should default to TRUE */
	print_res("add_arg_end default TRUE",
		(void *)(long)TRUE, (void *)(long)dc->add_arg_end, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 2. afc_dynamic_class_add_method() - add a simple method
	 * ---------------------------------------------------------------- */
	int res = afc_dynamic_class_add_method(dc, "hello", NULL, _test_method_simple);
	print_res("add_method returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* Verify the method can be found */
	DynamicClassMethodData *dcmd = afc_dynamic_class_find_method(dc, "hello");
	print_res("find_method hello != NULL",
		(void *)(long)1, (void *)(long)(dcmd != NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 3. afc_dynamic_class_execute() - call the simple method
	 * ---------------------------------------------------------------- */
	METHOD_CALL_COUNT = 0;
	res = afc_dynamic_class_execute(dc, "hello");
	print_res("execute hello returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);
	print_res("method was called once",
		(void *)(long)1, (void *)(long)METHOD_CALL_COUNT, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 4. Execute the same method multiple times
	 * ---------------------------------------------------------------- */
	METHOD_CALL_COUNT = 0;
	afc_dynamic_class_execute(dc, "hello");
	afc_dynamic_class_execute(dc, "hello");
	afc_dynamic_class_execute(dc, "hello");
	print_res("3 executions counted",
		(void *)(long)3, (void *)(long)METHOD_CALL_COUNT, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 5. Add and execute a method with arguments
	 * ---------------------------------------------------------------- */
	afc_dynamic_class_add_method(dc, "with_args", "P", _test_method_with_args);
	METHOD_CALL_COUNT = 0;
	LAST_ARG_RECEIVED = NULL;

	/* Pass a pointer argument to the method */
	void *TEST_VALUE = (void *)(long)12345;
	res = afc_dynamic_class_execute(dc, "with_args", TEST_VALUE);
	print_res("execute with_args OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);
	print_res("method received argument",
		(void *)(long)12345, (void *)(long)(long)LAST_ARG_RECEIVED, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 6. Execute a method that returns a custom error code
	 * ---------------------------------------------------------------- */
	afc_dynamic_class_add_method(dc, "fail", NULL, _test_method_returns_error);
	METHOD_CALL_COUNT = 0;
	res = afc_dynamic_class_execute(dc, "fail");
	print_res("execute fail returns 42",
		(void *)(long)42, (void *)(long)res, 0);
	print_res("fail method was called",
		(void *)(long)1, (void *)(long)METHOD_CALL_COUNT, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 7. Execute non-existent method returns error
	 * ---------------------------------------------------------------- */
	res = afc_dynamic_class_execute(dc, "nonexistent");
	print_res("execute unknown != NO_ERR",
		(void *)(long)1, (void *)(long)(res != AFC_ERR_NO_ERROR), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 8. afc_dynamic_class_set_var() / get_var() - numeric variable
	 * ---------------------------------------------------------------- */
	res = afc_dynamic_class_set_var(dc,
		AFC_DYNAMIC_CLASS_VAR_KIND_NUM, "counter", (void *)(long)99);
	print_res("set_var num returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	void *val = afc_dynamic_class_get_var(dc, "counter");
	print_res("get_var counter == 99",
		(void *)(long)99, (void *)(long)(long)val, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 9. set_var / get_var - string variable
	 * ---------------------------------------------------------------- */
	res = afc_dynamic_class_set_var(dc,
		AFC_DYNAMIC_CLASS_VAR_KIND_STRING, "name", "TestClass");
	print_res("set_var string returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	val = afc_dynamic_class_get_var(dc, "name");
	print_res("get_var name == TestClass",
		"TestClass", (char *)val, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 10. set_var / get_var - pointer variable
	 * ---------------------------------------------------------------- */
	int SOME_DATA = 777;
	res = afc_dynamic_class_set_var(dc,
		AFC_DYNAMIC_CLASS_VAR_KIND_POINTER, "data_ptr", &SOME_DATA);
	print_res("set_var pointer OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	int *retrieved_ptr = (int *)afc_dynamic_class_get_var(dc, "data_ptr");
	print_res("get_var pointer matches",
		(void *)(long)777, (void *)(long)*retrieved_ptr, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 11. Overwrite an existing variable
	 * ---------------------------------------------------------------- */
	afc_dynamic_class_set_var(dc,
		AFC_DYNAMIC_CLASS_VAR_KIND_NUM, "counter", (void *)(long)200);
	val = afc_dynamic_class_get_var(dc, "counter");
	print_res("overwrite counter == 200",
		(void *)(long)200, (void *)(long)(long)val, 0);

	/* Overwrite string variable */
	afc_dynamic_class_set_var(dc,
		AFC_DYNAMIC_CLASS_VAR_KIND_STRING, "name", "Updated");
	val = afc_dynamic_class_get_var(dc, "name");
	print_res("overwrite name == Updated",
		"Updated", (char *)val, 1);

	print_row();

	/* ----------------------------------------------------------------
	 * 12. Get non-existent variable returns NULL
	 * ---------------------------------------------------------------- */
	val = afc_dynamic_class_get_var(dc, "nonexistent_var");
	print_res("get unknown var == NULL",
		(void *)(long)0, (void *)(long)(val != NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 13. Method that reads a variable via the DynamicClass pointer
	 * ---------------------------------------------------------------- */
	afc_dynamic_class_add_method(dc, "read_var", NULL, _test_method_reads_var);
	afc_dynamic_class_set_var(dc,
		AFC_DYNAMIC_CLASS_VAR_KIND_NUM, "counter", (void *)(long)555);
	METHOD_CALL_COUNT = 0;

	res = afc_dynamic_class_execute(dc, "read_var");
	print_res("read_var method OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);
	print_res("result from method == 555",
		(void *)(long)555, (void *)(long)(long)dc->result, 0);
	print_res("result_type == INTEGER",
		(void *)(long)AFC_DYNAMIC_CLASS_RESULT_TYPE_INTEGER,
		(void *)(long)dc->result_type, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 14. afc_dynamic_class_clear() resets methods and variables
	 * ---------------------------------------------------------------- */
	res = afc_dynamic_class_clear(dc);
	print_res("clear returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* After clear, find_method should return NULL */
	dcmd = afc_dynamic_class_find_method(dc, "hello");
	print_res("find_method after clear",
		(void *)(long)0, (void *)(long)(dcmd != NULL), 0);

	/* After clear, get_var should return NULL */
	val = afc_dynamic_class_get_var(dc, "counter");
	print_res("get_var after clear == NULL",
		(void *)(long)0, (void *)(long)(val != NULL), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 15. Add methods again after clear - verify reuse
	 * ---------------------------------------------------------------- */
	afc_dynamic_class_add_method(dc, "reuse", NULL, _test_method_simple);
	METHOD_CALL_COUNT = 0;
	res = afc_dynamic_class_execute(dc, "reuse");
	print_res("execute after clear OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);
	print_res("reuse method called",
		(void *)(long)1, (void *)(long)METHOD_CALL_COUNT, 0);

	/* ----------------------------------------------------------------
	 * Cleanup and summary
	 * ---------------------------------------------------------------- */
	print_summary();

	afc_dynamic_class_delete(dc);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
