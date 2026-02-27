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
 * test_fileops.c - Comprehensive tests for the FileOperations class.
 *
 * Tests cover:
 *   - Object creation and deletion
 *   - Creating temp files and checking existence
 *   - Copying files and verifying both exist
 *   - Deleting files and verifying removal
 *   - Creating directories with afc_fileops_mkdir()
 *   - Deleting directories
 *   - Moving/renaming files
 *   - Non-existent file returns NOT_FOUND
 *   - All temp files/dirs use /tmp/afc_test_* paths for safety
 */

#include "test_utils.h"
#include "../src/fileops.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

/* Temp paths for safety - all under /tmp */
#define TEST_DIR       "/tmp/afc_test_dir"
#define TEST_FILE1     "/tmp/afc_test_file1.txt"
#define TEST_FILE2     "/tmp/afc_test_file2.txt"
#define TEST_FILE3     "/tmp/afc_test_file3.txt"
#define TEST_FILE_MOVE "/tmp/afc_test_file_moved.txt"
#define TEST_CONTENT   "Hello, AFC FileOps test!\n"

/**
 * _create_test_file - Creates a test file with known content at the given path.
 * Returns 0 on success, -1 on failure.
 */
static int _create_test_file(const char *path)
{
	FILE *fh = fopen(path, "w");
	if (!fh) return -1;
	fprintf(fh, "%s", TEST_CONTENT);
	fclose(fh);
	return 0;
}

/**
 * _file_has_content - Checks whether a file contains the expected content.
 * Returns 1 if content matches, 0 otherwise.
 */
static int _file_has_content(const char *path, const char *expected)
{
	FILE *fh = fopen(path, "r");
	if (!fh) return 0;

	char buf[1024];
	memset(buf, 0, sizeof(buf));
	fread(buf, 1, sizeof(buf) - 1, fh);
	fclose(fh);

	return (strcmp(buf, expected) == 0);
}

/**
 * _cleanup_all - Removes all temp files and dirs created during tests.
 * Silently ignores errors for files that do not exist.
 */
static void _cleanup_all(void)
{
	unlink(TEST_FILE1);
	unlink(TEST_FILE2);
	unlink(TEST_FILE3);
	unlink(TEST_FILE_MOVE);
	rmdir(TEST_DIR);
}

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	/* Clean up any leftover artifacts from previous runs */
	_cleanup_all();

	/* ---- Test 1: Object creation ---- */
	FileOperations *fo = afc_fileops_new();
	print_res("fileops_new() not NULL",
		(void *)(long)1,
		(void *)(long)(fo != NULL),
		0);

	print_row();

	/* ---- Test 2: Non-existent file returns NOT_FOUND ---- */
	{
		int res = afc_fileops_exists(fo, "/tmp/afc_test_nonexistent_file_xyz");
		print_res("exists non-existent -> ERR",
			(void *)(long)AFC_FILEOPS_ERR_NOT_FOUND,
			(void *)(long)res,
			0);
	}

	/* ---- Test 3: Create a temp file and check existence ---- */
	{
		int created = _create_test_file(TEST_FILE1);
		print_res("create test file1",
			(void *)(long)0,
			(void *)(long)created,
			0);

		int res = afc_fileops_exists(fo, TEST_FILE1);
		print_res("exists(file1) -> OK",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
	}

	print_row();

	/* ---- Test 4: Copy a file and verify both exist ---- */
	{
		int res = afc_fileops_copy(fo, TEST_FILE1, TEST_FILE2);
		print_res("copy file1 -> file2",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		/* Original should still exist */
		res = afc_fileops_exists(fo, TEST_FILE1);
		print_res("file1 still exists",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		/* Copy should exist */
		res = afc_fileops_exists(fo, TEST_FILE2);
		print_res("file2 exists",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		/* Copy should have the same content */
		int match = _file_has_content(TEST_FILE2, TEST_CONTENT);
		print_res("file2 content matches",
			(void *)(long)1,
			(void *)(long)match,
			0);
	}

	print_row();

	/* ---- Test 5: Delete a file and verify it is gone ---- */
	{
		int res = afc_fileops_del(fo, TEST_FILE2);
		print_res("del(file2) -> OK",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		res = afc_fileops_exists(fo, TEST_FILE2);
		print_res("file2 gone -> NOT_FOUND",
			(void *)(long)AFC_FILEOPS_ERR_NOT_FOUND,
			(void *)(long)res,
			0);
	}

	print_row();

	/* ---- Test 6: Create a directory ---- */
	{
		int res = afc_fileops_mkdir(fo, TEST_DIR);
		print_res("mkdir -> OK",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		/* Directory should exist */
		res = afc_fileops_exists(fo, TEST_DIR);
		print_res("dir exists",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		/* Verify it is actually a directory */
		struct stat st;
		stat(TEST_DIR, &st);
		int is_dir = S_ISDIR(st.st_mode);
		print_res("is a directory",
			(void *)(long)1,
			(void *)(long)is_dir,
			0);
	}

	/* ---- Test 7: mkdir on existing dir (should not error with default settings) ---- */
	{
		/* block_mkdir_exists defaults to FALSE, so this should succeed */
		int res = afc_fileops_mkdir(fo, TEST_DIR);
		print_res("mkdir existing -> OK",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);
	}

	/* ---- Test 8: Delete the directory ---- */
	{
		int res = afc_fileops_del(fo, TEST_DIR);
		print_res("del(dir) -> OK",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		res = afc_fileops_exists(fo, TEST_DIR);
		print_res("dir gone -> NOT_FOUND",
			(void *)(long)AFC_FILEOPS_ERR_NOT_FOUND,
			(void *)(long)res,
			0);
	}

	print_row();

	/* ---- Test 9: Move (rename) a file ---- */
	{
		/* Create file3 first */
		_create_test_file(TEST_FILE3);

		int res = afc_fileops_move(fo, TEST_FILE3, TEST_FILE_MOVE);
		print_res("move file3 -> moved",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		/* Original should be gone */
		res = afc_fileops_exists(fo, TEST_FILE3);
		print_res("file3 gone after move",
			(void *)(long)AFC_FILEOPS_ERR_NOT_FOUND,
			(void *)(long)res,
			0);

		/* Moved file should exist */
		res = afc_fileops_exists(fo, TEST_FILE_MOVE);
		print_res("moved file exists",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		/* Moved file should have the same content */
		int match = _file_has_content(TEST_FILE_MOVE, TEST_CONTENT);
		print_res("moved content matches",
			(void *)(long)1,
			(void *)(long)match,
			0);

		/* Clean up the moved file */
		afc_fileops_del(fo, TEST_FILE_MOVE);
	}

	print_row();

	/* ---- Test 10: Copy a directory with files ---- */
	{
		/* Create a directory with a file in it */
		afc_fileops_mkdir(fo, TEST_DIR);
		_create_test_file("/tmp/afc_test_dir/inner.txt");

		/* Copy the entire directory */
		const char *DEST_DIR = "/tmp/afc_test_dir_copy";
		int res = afc_fileops_copy(fo, TEST_DIR, DEST_DIR);
		print_res("copy dir -> OK",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		/* Destination dir should exist */
		res = afc_fileops_exists(fo, DEST_DIR);
		print_res("dest dir exists",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		/* File inside copied dir should exist */
		res = afc_fileops_exists(fo, "/tmp/afc_test_dir_copy/inner.txt");
		print_res("inner file exists",
			(void *)(long)AFC_ERR_NO_ERROR,
			(void *)(long)res,
			0);

		/* Clean up both directories */
		afc_fileops_del(fo, DEST_DIR);
		afc_fileops_del(fo, TEST_DIR);
	}

	print_row();

	/* ---- Test 11: Delete non-existent file returns error ---- */
	{
		int res = afc_fileops_del(fo, "/tmp/afc_test_nonexistent_del");
		/* del on non-existent file should return an error from stat */
		print_res("del non-existent -> ERR",
			(void *)(long)1,
			(void *)(long)(res != AFC_ERR_NO_ERROR),
			0);
	}

	/* Final cleanup of any remaining files */
	_cleanup_all();

	print_summary();

	/* Cleanup */
	afc_fileops_delete(fo);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
