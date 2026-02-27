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
 * test_cmd_parser.c - Comprehensive tests for the AFC CommandParser module.
 *
 * The CommandParser uses scheme-like syntax: (command args (child ...)).
 * Commands are registered via DynamicClassMaster plugins that provide
 * open_callback, close_callback, and get_template methods.
 *
 * Tests cover creation/deletion, registering commands via DynamicClassMaster,
 * parsing simple scripts, nested commands, argument retrieval, clearing,
 * edge cases like empty scripts and unknown commands.
 */

#include "test_utils.h"
#include "../src/cmd_parser.h"
#include "../src/dynamic_class_master.h"

/* Counters to track callback invocations */
static int OPEN_COUNT = 0;
static int CLOSE_COUNT = 0;
static int CHILD_OPEN_COUNT = 0;
static int CHILD_CLOSE_COUNT = 0;

/* Storage for the last parsed arguments */
static char LAST_TITLE[256] = {0};
static char LAST_WIDTH[64] = {0};

/* Reference to the CommandParser, needed inside callbacks to get args */
static CommandParser *GLOBAL_CMDP = NULL;

/* ----------------------------------------------------------------
 * "box" command plugin - accepts TITLE and W parameters
 * ---------------------------------------------------------------- */

/**
 * _box_cb_open - Open callback for the "box" command.
 *
 * Reads TITLE and W arguments from the parser and stores them
 * in static buffers for test verification.
 */
static int _box_cb_open(DynamicClass *dyn)
{
	OPEN_COUNT++;

	char *title = (char *)afc_cmd_parser_arg_get_by_name(GLOBAL_CMDP, "TITLE");
	char *w = (char *)afc_cmd_parser_arg_get_by_name(GLOBAL_CMDP, "W");

	if (title)
	{
		strncpy(LAST_TITLE, title, sizeof(LAST_TITLE) - 1);
		LAST_TITLE[sizeof(LAST_TITLE) - 1] = '\0';
	}
	if (w)
	{
		strncpy(LAST_WIDTH, w, sizeof(LAST_WIDTH) - 1);
		LAST_WIDTH[sizeof(LAST_WIDTH) - 1] = '\0';
	}

	return AFC_ERR_NO_ERROR;
}

/**
 * _box_cb_close - Close callback for the "box" command.
 *
 * Simply increments the close counter.
 */
static int _box_cb_close(DynamicClass *dyn)
{
	CLOSE_COUNT++;
	return AFC_ERR_NO_ERROR;
}

/**
 * _box_get_template - Returns the argument template for the "box" command.
 *
 * Template "TITLE W" means the parser expects two positional arguments.
 */
static int _box_get_template(DynamicClass *dyn)
{
	dyn->result = "TITLE W";
	return AFC_ERR_NO_ERROR;
}

/**
 * _box_new_instance - Creates a new DynamicClass instance for the "box" plugin.
 *
 * Registers the open_callback, close_callback, and get_template methods.
 */
static DynamicClass *_box_new_instance(void)
{
	DynamicClass *dyn = afc_dynamic_class_new();
	if (dyn == NULL) return NULL;

	afc_dynamic_class_add_method(dyn, "open_callback", NULL, _box_cb_open);
	afc_dynamic_class_add_method(dyn, "close_callback", NULL, _box_cb_close);
	afc_dynamic_class_add_method(dyn, "get_template", NULL, _box_get_template);

	return dyn;
}

/**
 * _box_del_instance - Deletes a "box" plugin instance.
 */
static int _box_del_instance(DynamicClass *dyn)
{
	if (dyn != NULL)
		afc_dynamic_class_delete(dyn);
	return AFC_ERR_NO_ERROR;
}

/* ----------------------------------------------------------------
 * "item" command plugin - no parameters
 * ---------------------------------------------------------------- */

/**
 * _item_cb_open - Open callback for the "item" command.
 */
static int _item_cb_open(DynamicClass *dyn)
{
	CHILD_OPEN_COUNT++;
	return AFC_ERR_NO_ERROR;
}

/**
 * _item_cb_close - Close callback for the "item" command.
 */
static int _item_cb_close(DynamicClass *dyn)
{
	CHILD_CLOSE_COUNT++;
	return AFC_ERR_NO_ERROR;
}

/**
 * _item_get_template - Returns empty template (no arguments).
 */
static int _item_get_template(DynamicClass *dyn)
{
	dyn->result = "";
	return AFC_ERR_NO_ERROR;
}

/**
 * _item_new_instance - Creates a new DynamicClass instance for "item".
 */
static DynamicClass *_item_new_instance(void)
{
	DynamicClass *dyn = afc_dynamic_class_new();
	if (dyn == NULL) return NULL;

	afc_dynamic_class_add_method(dyn, "open_callback", NULL, _item_cb_open);
	afc_dynamic_class_add_method(dyn, "close_callback", NULL, _item_cb_close);
	afc_dynamic_class_add_method(dyn, "get_template", NULL, _item_get_template);

	return dyn;
}

/**
 * _item_del_instance - Deletes an "item" plugin instance.
 */
static int _item_del_instance(DynamicClass *dyn)
{
	if (dyn != NULL)
		afc_dynamic_class_delete(dyn);
	return AFC_ERR_NO_ERROR;
}

/**
 * _reset_counters - Resets all static test counters and buffers.
 */
static void _reset_counters(void)
{
	OPEN_COUNT = 0;
	CLOSE_COUNT = 0;
	CHILD_OPEN_COUNT = 0;
	CHILD_CLOSE_COUNT = 0;
	memset(LAST_TITLE, 0, sizeof(LAST_TITLE));
	memset(LAST_WIDTH, 0, sizeof(LAST_WIDTH));
}

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ----------------------------------------------------------------
	 * 1. Creation and initial state
	 * ---------------------------------------------------------------- */
	CommandParser *cmdp = afc_cmd_parser_new();
	print_res("cmd_parser_new != NULL",
		(void *)(long)1, (void *)(long)(cmdp != NULL), 0);

	/* Magic number should be set */
	print_res("magic == CMDP",
		(void *)(long)AFC_CMD_PARSER_MAGIC,
		(void *)(long)cmdp->magic, 0);

	/* Set the global reference for callbacks */
	GLOBAL_CMDP = cmdp;

	print_row();

	/* ----------------------------------------------------------------
	 * 2. Register commands via DynamicClassMaster
	 * ---------------------------------------------------------------- */
	DynamicClassMaster *dcm = afc_dynamic_class_master_new();
	print_res("dcm_new != NULL",
		(void *)(long)1, (void *)(long)(dcm != NULL), 0);

	/* Add the "box" and "item" command plugins */
	int res = afc_dynamic_class_master_add(dcm, "box", NULL,
		_box_new_instance, _box_del_instance, NULL);
	print_res("add box plugin OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	res = afc_dynamic_class_master_add(dcm, "item", NULL,
		_item_new_instance, _item_del_instance, NULL);
	print_res("add item plugin OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* Register all plugins with the CommandParser */
	res = afc_cmd_parser_add_commands(cmdp, dcm);
	print_res("add_commands returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 3. Parse a simple script with one command
	 * ---------------------------------------------------------------- */
	_reset_counters();

	char *script1 = afc_string_dup("(box Hello 640)");
	res = afc_cmd_parser_parse_string(cmdp, script1, cmdp);
	print_res("parse simple script OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* Verify the open callback was called once */
	print_res("box open called once",
		(void *)(long)1, (void *)(long)OPEN_COUNT, 0);

	/* Verify the close callback was called once */
	print_res("box close called once",
		(void *)(long)1, (void *)(long)CLOSE_COUNT, 0);

	/* Verify parsed arguments */
	print_res("parsed TITLE == Hello",
		"Hello", LAST_TITLE, 1);
	print_res("parsed W == 640",
		"640", LAST_WIDTH, 1);

	afc_string_delete(script1);

	print_row();

	/* ----------------------------------------------------------------
	 * 4. Parse a script with nested commands
	 * ---------------------------------------------------------------- */
	_reset_counters();

	char *script2 = afc_string_dup(
		"(box MyWindow 800"
		"(item)"
		"(item)"
		")");
	res = afc_cmd_parser_parse_string(cmdp, script2, cmdp);
	print_res("parse nested script OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* box should open and close once */
	print_res("nested: box open == 1",
		(void *)(long)1, (void *)(long)OPEN_COUNT, 0);
	print_res("nested: box close == 1",
		(void *)(long)1, (void *)(long)CLOSE_COUNT, 0);

	/* item should open and close twice */
	print_res("nested: item open == 2",
		(void *)(long)2, (void *)(long)CHILD_OPEN_COUNT, 0);
	print_res("nested: item close == 2",
		(void *)(long)2, (void *)(long)CHILD_CLOSE_COUNT, 0);

	/* Verify the box arguments */
	print_res("nested: TITLE == MyWindow",
		"MyWindow", LAST_TITLE, 1);
	print_res("nested: W == 800",
		"800", LAST_WIDTH, 1);

	afc_string_delete(script2);

	print_row();

	/* ----------------------------------------------------------------
	 * 5. Edge case: parse NULL script
	 * ---------------------------------------------------------------- */
	res = afc_cmd_parser_parse_string(cmdp, NULL, NULL);
	print_res("parse NULL script != OK",
		(void *)(long)1, (void *)(long)(res != AFC_ERR_NO_ERROR), 0);

	print_row();

	/* ----------------------------------------------------------------
	 * 6. Edge case: parse empty script
	 * ---------------------------------------------------------------- */
	char *empty = afc_string_dup("");
	res = afc_cmd_parser_parse_string(cmdp, empty, NULL);
	print_res("parse empty != OK",
		(void *)(long)1, (void *)(long)(res != AFC_ERR_NO_ERROR), 0);
	afc_string_delete(empty);

	print_row();

	/* ----------------------------------------------------------------
	 * 7. Edge case: parse script with unknown command
	 * ---------------------------------------------------------------- */
	_reset_counters();
	char *script_unknown = afc_string_dup("(unknown_cmd)");
	res = afc_cmd_parser_parse_string(cmdp, script_unknown, cmdp);
	print_res("unknown cmd returns err",
		(void *)(long)AFC_CMD_PARSER_ERR_CMD_UNKNOWN, (void *)(long)res, 0);

	/* No callbacks should have been invoked */
	print_res("no open on unknown cmd",
		(void *)(long)0, (void *)(long)OPEN_COUNT, 0);
	afc_string_delete(script_unknown);

	print_row();

	/* ----------------------------------------------------------------
	 * 8. Edge case: unmatched brackets
	 * ---------------------------------------------------------------- */
	_reset_counters();
	char *script_unmatched = afc_string_dup("(box Test 100");
	res = afc_cmd_parser_parse_string(cmdp, script_unmatched, cmdp);
	print_res("unmatched bracket err",
		(void *)(long)AFC_CMD_PARSER_ERR_UNMATCHED_OPEN_BRACKET,
		(void *)(long)res, 0);
	afc_string_delete(script_unmatched);

	print_row();

	/* ----------------------------------------------------------------
	 * 9. afc_cmd_parser_clear() and reuse
	 *
	 * Note: clear() will delete the plugin instances from the classes
	 * dictionary via afc_dynamic_class_master_delete_instance(), so
	 * we must re-register commands after clearing.
	 * ---------------------------------------------------------------- */
	res = afc_cmd_parser_clear(cmdp);
	print_res("clear returns OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* Re-register commands after clear */
	res = afc_cmd_parser_add_commands(cmdp, dcm);
	print_res("re-add_commands OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);

	/* Parse again after re-registration */
	_reset_counters();
	char *script3 = afc_string_dup("(box Reused 320)");
	res = afc_cmd_parser_parse_string(cmdp, script3, cmdp);
	print_res("parse after clear OK",
		(void *)(long)AFC_ERR_NO_ERROR, (void *)(long)res, 0);
	print_res("reuse: box open == 1",
		(void *)(long)1, (void *)(long)OPEN_COUNT, 0);
	print_res("reuse: TITLE == Reused",
		"Reused", LAST_TITLE, 1);
	afc_string_delete(script3);

	print_row();

	/* ----------------------------------------------------------------
	 * 10. Edge case: clear on NULL
	 * ---------------------------------------------------------------- */
	res = afc_cmd_parser_clear(NULL);
	print_res("clear(NULL) returns err",
		(void *)(long)1, (void *)(long)(res != AFC_ERR_NO_ERROR), 0);

	/* ----------------------------------------------------------------
	 * Cleanup and summary
	 *
	 * Order matters: cmd_parser must be deleted first since it holds
	 * references to instances created by the DynamicClassMaster.
	 * ---------------------------------------------------------------- */
	print_summary();

	afc_cmd_parser_delete(cmdp);
	afc_dynamic_class_master_delete(dcm);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
