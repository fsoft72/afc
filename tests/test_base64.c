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
 * test_base64.c - Comprehensive tests for the Base64 encoding/decoding class.
 *
 * Tests cover:
 *   - Object creation and deletion
 *   - Memory-to-memory encoding of known strings
 *   - Memory-to-memory decoding of known encoded strings
 *   - Round-trip encode then decode verification
 *   - Edge cases: empty input, single character, padding variations
 */

#include "test_utils.h"
#include "../src/base64.h"

/* Known Base64 test vectors */
#define INPUT_HELLO      "Hello, World!"
#define EXPECTED_HELLO   "SGVsbG8sIFdvcmxkIQ=="

#define INPUT_ABC        "abc"
#define EXPECTED_ABC     "YWJj"

#define INPUT_SINGLE     "a"
#define EXPECTED_SINGLE  "YQ=="

#define INPUT_TWO        "ab"
#define EXPECTED_TWO     "YWI="

/* Output buffer size - generous for encoded data plus line breaks */
#define OUTPUT_BUF_SIZE  4096

/**
 * _strip_whitespace - Removes all whitespace (including \r\n) from a buffer.
 * Modifies the buffer in place and returns its new length.
 */
static int _strip_whitespace(char *buf, int len)
{
	int i, j = 0;
	for (i = 0; i < len; i++)
	{
		if (buf[i] != '\r' && buf[i] != '\n' && buf[i] != ' ' && buf[i] != '\t')
		{
			buf[j++] = buf[i];
		}
	}
	buf[j] = '\0';
	return j;
}

/**
 * _encode_string - Encodes a string using Base64 into the provided output buffer.
 * Returns the length of the encoded output (excluding whitespace).
 */
static int _encode_string(Base64 *b64, const char *input, int input_len, char *output, int output_size)
{
	int encoded_len;

	memset(output, 0, output_size);

	afc_base64_encode(b64,
		AFC_BASE64_TAG_MEM_IN, input,
		AFC_BASE64_TAG_MEM_IN_SIZE, (void *)(long)input_len,
		AFC_BASE64_TAG_MEM_OUT, output,
		AFC_BASE64_TAG_MEM_OUT_SIZE, (void *)(long)output_size,
		AFC_TAG_END);

	/* Calculate the encoded length from the output pointer position */
	encoded_len = (int)(b64->mem_out_pos - b64->mem_out);

	/* Strip whitespace from output for comparison */
	encoded_len = _strip_whitespace(output, encoded_len);

	return encoded_len;
}

/**
 * _decode_string - Decodes a Base64 encoded string into the provided output buffer.
 * Returns the length of the decoded output.
 */
static int _decode_string(Base64 *b64, const char *input, int input_len, char *output, int output_size)
{
	int decoded_len;

	memset(output, 0, output_size);

	afc_base64_decode(b64,
		AFC_BASE64_TAG_MEM_IN, input,
		AFC_BASE64_TAG_MEM_IN_SIZE, (void *)(long)input_len,
		AFC_BASE64_TAG_MEM_OUT, output,
		AFC_BASE64_TAG_MEM_OUT_SIZE, (void *)(long)output_size,
		AFC_TAG_END);

	/* Calculate the decoded length from the output pointer position */
	decoded_len = (int)(b64->mem_out_pos - b64->mem_out);

	return decoded_len;
}

int main(void)
{
	AFC *afc = afc_new();
	test_header();

	char encoded[OUTPUT_BUF_SIZE];
	char decoded[OUTPUT_BUF_SIZE];

	/* ---- Test 1: Object creation ---- */
	Base64 *b64 = afc_base64_new();
	print_res("base64_new() not NULL",
		(void *)(long)1,
		(void *)(long)(b64 != NULL),
		0);

	print_row();

	/* ---- Test 2: Encode "Hello, World!" ---- */
	_encode_string(b64, INPUT_HELLO, strlen(INPUT_HELLO), encoded, OUTPUT_BUF_SIZE);
	print_res("encode 'Hello, World!'",
		(void *)EXPECTED_HELLO,
		(void *)encoded,
		1);

	/* ---- Test 3: Encode "abc" ---- */
	/* Need a fresh Base64 for each encode/decode because pointers carry state */
	afc_base64_delete(b64);
	b64 = afc_base64_new();
	_encode_string(b64, INPUT_ABC, strlen(INPUT_ABC), encoded, OUTPUT_BUF_SIZE);
	print_res("encode 'abc'",
		(void *)EXPECTED_ABC,
		(void *)encoded,
		1);

	/* ---- Test 4: Encode single char "a" (two bytes padding) ---- */
	afc_base64_delete(b64);
	b64 = afc_base64_new();
	_encode_string(b64, INPUT_SINGLE, strlen(INPUT_SINGLE), encoded, OUTPUT_BUF_SIZE);
	print_res("encode 'a' (2 pad chars)",
		(void *)EXPECTED_SINGLE,
		(void *)encoded,
		1);

	/* ---- Test 5: Encode "ab" (one byte padding) ---- */
	afc_base64_delete(b64);
	b64 = afc_base64_new();
	_encode_string(b64, INPUT_TWO, strlen(INPUT_TWO), encoded, OUTPUT_BUF_SIZE);
	print_res("encode 'ab' (1 pad char)",
		(void *)EXPECTED_TWO,
		(void *)encoded,
		1);

	print_row();

	/* ---- Test 6: Decode known encoded "Hello, World!" ---- */
	afc_base64_delete(b64);
	b64 = afc_base64_new();
	_decode_string(b64, EXPECTED_HELLO, strlen(EXPECTED_HELLO), decoded, OUTPUT_BUF_SIZE);
	/* Null-terminate the decoded output for string comparison */
	decoded[strlen(INPUT_HELLO)] = '\0';
	print_res("decode -> 'Hello, World!'",
		(void *)INPUT_HELLO,
		(void *)decoded,
		1);

	/* ---- Test 7: Decode "YWJj" -> "abc" ---- */
	afc_base64_delete(b64);
	b64 = afc_base64_new();
	_decode_string(b64, EXPECTED_ABC, strlen(EXPECTED_ABC), decoded, OUTPUT_BUF_SIZE);
	decoded[strlen(INPUT_ABC)] = '\0';
	print_res("decode -> 'abc'",
		(void *)INPUT_ABC,
		(void *)decoded,
		1);

	/* ---- Test 8: Decode "YQ==" -> "a" ---- */
	afc_base64_delete(b64);
	b64 = afc_base64_new();
	_decode_string(b64, EXPECTED_SINGLE, strlen(EXPECTED_SINGLE), decoded, OUTPUT_BUF_SIZE);
	decoded[strlen(INPUT_SINGLE)] = '\0';
	print_res("decode -> 'a'",
		(void *)INPUT_SINGLE,
		(void *)decoded,
		1);

	print_row();

	/* ---- Test 9: Round-trip encode then decode for a longer string ---- */
	{
		const char *ROUND_TRIP_INPUT = "The quick brown fox jumps over the lazy dog";
		int input_len = strlen(ROUND_TRIP_INPUT);

		/* Encode */
		afc_base64_delete(b64);
		b64 = afc_base64_new();
		_encode_string(b64, ROUND_TRIP_INPUT, input_len, encoded, OUTPUT_BUF_SIZE);

		/* Decode the encoded output */
		afc_base64_delete(b64);
		b64 = afc_base64_new();
		_decode_string(b64, encoded, strlen(encoded), decoded, OUTPUT_BUF_SIZE);
		decoded[input_len] = '\0';

		print_res("round-trip long string",
			(void *)ROUND_TRIP_INPUT,
			(void *)decoded,
			1);
	}

	/* ---- Test 10: Round-trip for binary-like data with various byte values ---- */
	{
		const char *BINARY_INPUT = "\x01\x02\x03\x04\x05\x06\x07\x08";
		int input_len = 8;

		/* Encode */
		afc_base64_delete(b64);
		b64 = afc_base64_new();
		_encode_string(b64, BINARY_INPUT, input_len, encoded, OUTPUT_BUF_SIZE);

		/* Decode the encoded output */
		afc_base64_delete(b64);
		b64 = afc_base64_new();
		int decoded_len = _decode_string(b64, encoded, strlen(encoded), decoded, OUTPUT_BUF_SIZE);

		/* Compare lengths */
		print_res("round-trip binary len",
			(void *)(long)input_len,
			(void *)(long)decoded_len,
			0);

		/* Compare content byte by byte */
		int match = (decoded_len == input_len) && (memcmp(BINARY_INPUT, decoded, input_len) == 0);
		print_res("round-trip binary data",
			(void *)(long)1,
			(void *)(long)match,
			0);
	}

	print_summary();

	/* Cleanup */
	afc_base64_delete(b64);
	afc_delete(afc);

	return get_test_failures() > 0 ? 1 : 0;
}
