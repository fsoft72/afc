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

/**
 * test_readargs.c - Comprehensive tests for the ReadArgs argument parser class.
 *
 * Tests cover:
 *   - Object creation and deletion
 *   - Simple two-field parsing
 *   - Required fields (/A modifier)
 *   - Keyword fields (/K modifier)
 *   - Switch fields (/S modifier)
 *   - Numeric fields (/N modifier)
 *   - Multi-value fields (/M modifier)
 *   - get_by_name() and get_by_pos() access
 *   - Quoted strings in input
 *   - Clear and re-parse
 */

#include "test_utils.h"
#include "../src/readargs.h"
#include "../src/list.h"

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* ---- Test 1: Object creation ---- */
	ReadArgs *ra = afc_readargs_new();
	print_res("readargs_new() not NULL",
		(void *)(long)1,
		(void *)(long)(ra != NULL),
		0);

	print_row();

	/* ---- Test 2: Simple two required fields ---- */
	/* Template: "SOURCE/A,DEST/A"  Input: "fileA fileB" */
	{
		int res = afc_readargs_parse(ra, "SOURCE/A,DEST/A", "fileA fileB");
		print_res("parse simple -> NO_ERROR",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
	}

	/* ---- Test 3: get_by_name for SOURCE ---- */
	{
		char *val = (char *)afc_readargs_get_by_name(ra, "SOURCE");
		print_res("get_by_name('SOURCE')",
			(void *)"fileA",
			(void *)val,
			1);
	}

	/* ---- Test 4: get_by_name for DEST ---- */
	{
		char *val = (char *)afc_readargs_get_by_name(ra, "DEST");
		print_res("get_by_name('DEST')",
			(void *)"fileB",
			(void *)val,
			1);
	}

	/* ---- Test 5: get_by_pos for position 0 ---- */
	{
		char *val = (char *)afc_readargs_get_by_pos(ra, 0);
		print_res("get_by_pos(0) -> 'fileA'",
			(void *)"fileA",
			(void *)val,
			1);
	}

	/* ---- Test 6: get_by_pos for position 1 ---- */
	{
		char *val = (char *)afc_readargs_get_by_pos(ra, 1);
		print_res("get_by_pos(1) -> 'fileB'",
			(void *)"fileB",
			(void *)val,
			1);
	}

	print_row();

	/* ---- Test 7: Keyword arguments with explicit naming ---- */
	/* Template: "SOURCE/A,DEST/A" Input: "DEST outfile SOURCE infile" */
	{
		int res = afc_readargs_parse(ra, "SOURCE/A,DEST/A", "DEST outfile SOURCE infile");
		print_res("parse keyword order",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		char *src = (char *)afc_readargs_get_by_name(ra, "SOURCE");
		print_res("keyword SOURCE",
			(void *)"infile",
			(void *)src,
			1);

		char *dst = (char *)afc_readargs_get_by_name(ra, "DEST");
		print_res("keyword DEST",
			(void *)"outfile",
			(void *)dst,
			1);
	}

	print_row();

	/* ---- Test 8: Switch field ---- */
	/* Template: "CMD/A,VERBOSE/S,FILE/A" Input: "run VERBOSE test.txt" */
	{
		int res = afc_readargs_parse(ra, "CMD/A,VERBOSE/S,FILE/A", "run VERBOSE test.txt");
		print_res("parse with switch",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		char *cmd = (char *)afc_readargs_get_by_name(ra, "CMD");
		print_res("CMD -> 'run'",
			(void *)"run",
			(void *)cmd,
			1);

		/* Switch returns (void *)TRUE when set */
		long verbose = (long)afc_readargs_get_by_name(ra, "VERBOSE");
		print_res("VERBOSE switch is TRUE",
			(void *)(long)TRUE,
			(void *)(long)verbose,
			0);

		char *file = (char *)afc_readargs_get_by_name(ra, "FILE");
		print_res("FILE -> 'test.txt'",
			(void *)"test.txt",
			(void *)file,
			1);
	}

	/* ---- Test 9: Switch field NOT set ---- */
	{
		afc_readargs_parse(ra, "CMD/A,VERBOSE/S,FILE/A", "run test.txt");

		/* Switch returns FALSE (0) when not set */
		long verbose = (long)afc_readargs_get_by_name(ra, "VERBOSE");
		print_res("VERBOSE switch is FALSE",
			(void *)(long)FALSE,
			(void *)(long)verbose,
			0);
	}

	print_row();

	/* ---- Test 10: Keyword required (/K modifier) ---- */
	/* Template: "COMMAND/A,MAXBYTES/K/N"  Input: "search MAXBYTES 2048" */
	{
		int res = afc_readargs_parse(ra, "COMMAND/A,MAXBYTES/K/N", "search MAXBYTES 2048");
		print_res("parse /K/N keyword",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		char *cmd = (char *)afc_readargs_get_by_name(ra, "COMMAND");
		print_res("COMMAND -> 'search'",
			(void *)"search",
			(void *)cmd,
			1);

		long maxbytes = (long)afc_readargs_get_by_name(ra, "MAXBYTES");
		print_res("MAXBYTES -> 2048",
			(void *)(long)2048,
			(void *)(long)maxbytes,
			0);
	}

	print_row();

	/* ---- Test 11: Quoted strings ---- */
	/* Quoted strings should be treated as a single token */
	{
		int res = afc_readargs_parse(ra, "NAME/A,DESC/A", "hello \"world wide web\"");
		print_res("parse quoted string",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		char *name = (char *)afc_readargs_get_by_name(ra, "NAME");
		print_res("NAME -> 'hello'",
			(void *)"hello",
			(void *)name,
			1);

		char *desc = (char *)afc_readargs_get_by_name(ra, "DESC");
		print_res("DESC -> 'world wide web'",
			(void *)"world wide web",
			(void *)desc,
			1);
	}

	print_row();

	/* ---- Test 12: Multi-value fields (/M) ---- */
	/* Template: "FILES/M/A,DEST/K/A"  Input: "f1 f2 f3 DEST /tmp/out" */
	{
		int res = afc_readargs_parse(ra, "FILES/M/A,DEST/K/A", "f1 f2 f3 DEST /tmp/out");
		print_res("parse /M multi-value",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		/* Multi-value returns a List */
		struct afc_list *files = (struct afc_list *)afc_readargs_get_by_name(ra, "FILES");
		print_res("FILES is not NULL",
			(void *)(long)1,
			(void *)(long)(files != NULL),
			0);

		if (files != NULL)
		{
			/* Check the first file */
			char *f1 = (char *)afc_list_first(files);
			print_res("FILES[0] -> 'f1'",
				(void *)"f1",
				(void *)f1,
				1);

			/* Check the second file */
			char *f2 = (char *)afc_list_next(files);
			print_res("FILES[1] -> 'f2'",
				(void *)"f2",
				(void *)f2,
				1);

			/* Check the third file */
			char *f3 = (char *)afc_list_next(files);
			print_res("FILES[2] -> 'f3'",
				(void *)"f3",
				(void *)f3,
				1);
		}

		char *dest = (char *)afc_readargs_get_by_name(ra, "DEST");
		print_res("DEST -> '/tmp/out'",
			(void *)"/tmp/out",
			(void *)dest,
			1);
	}

	print_row();

	/* ---- Test 13: Clear and re-parse ---- */
	{
		afc_readargs_clear(ra);

		/* After clear, get_by_name should return NULL */
		void *val = afc_readargs_get_by_name(ra, "SOURCE");
		print_res("after clear -> NULL",
			(void *)(long)0,
			(void *)(long)(val == NULL ? 0 : 1),
			0);

		/* Re-parse with new data */
		int res = afc_readargs_parse(ra, "X/A", "hello");
		print_res("re-parse -> NO_ERROR",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		char *x = (char *)afc_readargs_get_by_name(ra, "X");
		print_res("re-parse X -> 'hello'",
			(void *)"hello",
			(void *)x,
			1);
	}

	/* ---- Test 14: NULL input string ---- */
	{
		int res = afc_readargs_parse(ra, "X/A", NULL);
		print_res("parse NULL input",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
	}

	/* ---- Test 15: Empty input string ---- */
	{
		int res = afc_readargs_parse(ra, "X/A", "");
		print_res("parse empty input",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
	}

	print_summary();

	/* Cleanup */
	afc_readargs_delete(ra);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
