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
#include "base64.h"

static const char eol[] = "\r\n"; /* End of line sequence */
static const char class_name[] = "Base64";

static int afc_base64_internal_inbuf(Base64 *b64);
static int afc_base64_internal_inchar(Base64 *b64);
static int afc_base64_internal_encode(Base64 *b64);
static int afc_base64_internal_insig(Base64 *b64);
static int afc_base64_internal_decode(Base64 *b64);
static int afc_base64_internal_parse_tags(Base64 *b64, int first_tag, va_list tags);
static int afc_base64_internal_write(Base64 *b64, const void *mem, int size);
static int afc_base64_internal_write_char(Base64 *b64, int c);

Base64 *afc_base64_new(void)
{
	Base64 *b64 = afc_malloc(sizeof(Base64));

	if (b64 == NULL)
	{
		AFC_LOG_FAST(AFC_ERR_NO_MEMORY);
		return (NULL);
	}

	b64->iocp = 256;
	b64->error_check = TRUE;

	return (b64);
}

int afc_base64_delete(Base64 *b64)
{
	if (b64 == NULL)
		return (AFC_LOG_FAST(AFC_ERR_NULL_POINTER));

	if (b64->file_in)
		afc_string_delete(b64->file_in);
	if (b64->file_out)
		afc_string_delete(b64->file_out);

	afc_free(b64);

	return (AFC_ERR_NO_ERROR);
}

int afc_base64_encode(Base64 *b64, int first_tag, ...)
{
	va_list tags;

	va_start(tags, first_tag);
	afc_base64_internal_parse_tags(b64, first_tag, tags);
	va_end(tags);

	if ((b64->mem_in == NULL) && (b64->file_in == NULL))
		return (AFC_ERR_NO_ERROR);
	if ((b64->mem_out == NULL) && (b64->file_out == NULL))
		return (AFC_ERR_NO_ERROR);

	if (b64->mem_in == NULL)
		if ((b64->fin = fopen(b64->file_in, "rb")) == NULL)
			return (AFC_LOG(AFC_LOG_ERROR, AFC_BASE64_ERR_FILE_INPUT, "Cannot read input file", b64->file_in));

	if (b64->mem_out == NULL)
		if ((b64->fout = fopen(b64->file_out, "wb")) == NULL)
			return (AFC_LOG(AFC_LOG_ERROR, AFC_BASE64_ERR_FILE_OUTPUT, "Cannot write output file", b64->file_out));

	afc_base64_internal_encode(b64);

	if (b64->fout)
		fclose(b64->fout);
	if (b64->fin)
		fclose(b64->fin);

	b64->fin = NULL;
	b64->fout = NULL;

	return (AFC_ERR_NO_ERROR);
}

int afc_base64_decode(Base64 *b64, int first_tag, ...)
{
	va_list tags;

	va_start(tags, first_tag);
	afc_base64_internal_parse_tags(b64, first_tag, tags);
	va_end(tags);

	if ((b64->mem_in == NULL) && (b64->file_in == NULL))
		return (AFC_ERR_NO_ERROR);
	if ((b64->mem_out == NULL) && (b64->file_out == NULL))
		return (AFC_ERR_NO_ERROR);

	if (b64->mem_in == NULL)
		if ((b64->fin = fopen(b64->file_in, "rb")) == NULL)
			return (AFC_LOG(AFC_LOG_ERROR, AFC_BASE64_ERR_FILE_INPUT, "Cannot read input file", b64->file_in));

	if (b64->mem_out == NULL)
		if ((b64->fout = fopen(b64->file_out, "wb")) == NULL)
			return (AFC_LOG(AFC_LOG_ERROR, AFC_BASE64_ERR_FILE_OUTPUT, "Cannot write output file", b64->file_out));

	afc_base64_internal_decode(b64);

	if (b64->fout)
		fclose(b64->fout);
	if (b64->fin)
		fclose(b64->fin);

	b64->fin = NULL;
	b64->fout = NULL;

	return (AFC_ERR_NO_ERROR);
}

int afc_base64_set_tag(Base64 *b64, int tag, void *val)
{
	switch (tag)
	{
	case AFC_BASE64_TAG_MEM_IN:
		b64->mem_in = val;
		b64->mem_in_pos = val;
		break;

	case AFC_BASE64_TAG_MEM_IN_SIZE:
		b64->mem_in_size = (unsigned int)(long)val;
		break;

	case AFC_BASE64_TAG_MEM_OUT:
		b64->mem_out = val;
		b64->mem_out_pos = val;
		break;

	case AFC_BASE64_TAG_MEM_OUT_SIZE:
		b64->mem_out_size = (unsigned int)(long)val;
		break;

	case AFC_BASE64_TAG_FILE_IN:
		if (b64->file_in)
			afc_string_delete(b64->file_in);
		b64->file_in = afc_string_dup((char *)val);
		break;

	case AFC_BASE64_TAG_FILE_OUT:
		if (b64->file_out)
			afc_string_delete(b64->file_out);
		b64->file_out = afc_string_dup((char *)val);
		break;
	}

	return (AFC_ERR_NO_ERROR);
}

int afc_base64_fwrite(Base64 *b64, const char *fname, int what)
{
	FILE *fh;
	char *mem;
	unsigned int size;

	// TODO: error messages
	if ((what == AFC_BASE64_IN) && (b64->mem_in == NULL))
		return (AFC_ERR_NO_ERROR);
	if ((what == AFC_BASE64_OUT) && (b64->mem_out == NULL))
		return (AFC_ERR_NO_ERROR);

	if ((fh = fopen(fname, "wb")) == NULL)
		return (AFC_LOG(AFC_LOG_ERROR, AFC_BASE64_ERR_FILE_OUTPUT, "Canno write file", fname));

	if (what == AFC_BASE64_IN)
	{
		mem = b64->mem_in;
		size = b64->mem_in_pos - b64->mem_in;
	}
	else
	{
		mem = b64->mem_out;
		size = b64->mem_out_pos - b64->mem_out;
	}

	fwrite(mem, size, 1, fh);
	fclose(fh);
	return (AFC_ERR_NO_ERROR);
}

/* ===============================================================================
   INTERNAL FUNCTIONS
=============================================================================== */

/*  INBUF  --  Fill input buffer with data  */
static int afc_base64_internal_inbuf(Base64 *b64)
{
	int l;

	if (b64->at_eof)
		return (FALSE);

	if (b64->mem_in)
	{
		if (b64->mem_in_pos >= b64->mem_in + b64->mem_in_size)
		{
			b64->at_eof = TRUE;
			return (AFC_BASE64_ERR_EOF);
		}

		l = sizeof(b64->io_buffer);
		if (l + b64->mem_in_pos > b64->mem_in + b64->mem_in_size)
			l = b64->mem_in_size - (b64->mem_in_pos - b64->mem_in);

		afc_dprintf("IN size: %d\n", l);

		memcpy(b64->io_buffer, b64->mem_in_pos, l);
		b64->mem_in_pos += l;
	}
	else
	{
		l = fread(b64->io_buffer, 1, sizeof(b64->io_buffer), b64->fin); /* Read input buffer */

		if (l <= 0)
		{
			if (ferror(b64->fin))
			{
				b64->at_eof = TRUE;
				return (AFC_BASE64_ERR_READ_ERROR);
			}

			b64->at_eof = TRUE;
			return (AFC_BASE64_ERR_EOF);
		}
	}

	b64->size = l;
	b64->iocp = 0;

	return (AFC_ERR_NO_ERROR);
}

/*  INCHAR  --	Return next character from input  */

static int afc_base64_internal_inchar(Base64 *b64)
{
	if (b64->iocp >= b64->size)
		if (afc_base64_internal_inbuf(b64) != AFC_ERR_NO_ERROR)
		{
			b64->at_eof = TRUE;
			return EOF;
		}

	return (b64->io_buffer[b64->iocp++]);
}

/*  OCHAR  --  Output an encoded character, inserting line breaks
		   where required.	*/
static int afc_base64_internal_ochar(Base64 *b64, int c)
{
	if (b64->line_len >= LINELEN)
	{
		afc_base64_internal_write(b64, eol, 2);
		b64->line_len = 0;
	}

	afc_base64_internal_write_char(b64, c);
	b64->line_len += 1;

	return (AFC_ERR_NO_ERROR);
}

/*  ENCODE  --	Encode binary file into base64.  */

static int afc_base64_internal_encode(Base64 *b64)
{
	int i, quit = FALSE;
	unsigned char igroup[3], ogroup[4];
	int c, n;

	/* Fill dtable with character encodings.  */

	for (i = 0; i < 26; i++)
	{
		b64->dtable[i] = 'A' + i;
		b64->dtable[26 + i] = 'a' + i;
	}

	for (i = 0; i < 10; i++)
		b64->dtable[52 + i] = '0' + i;

	b64->dtable[62] = '+';
	b64->dtable[63] = '/';

	while (!quit)
	{
		igroup[0] = igroup[1] = igroup[2] = 0;

		for (n = 0; n < 3; n++)
		{
			c = afc_base64_internal_inchar(b64);
			if (b64->at_eof)
			{
				quit = TRUE;
				break;
			}

			igroup[n] = (unsigned char)c;
		}

		if (n > 0)
		{
			ogroup[0] = b64->dtable[igroup[0] >> 2];
			ogroup[1] = b64->dtable[((igroup[0] & 3) << 4) | (igroup[1] >> 4)];
			ogroup[2] = b64->dtable[((igroup[1] & 0xF) << 2) | (igroup[2] >> 6)];
			ogroup[3] = b64->dtable[igroup[2] & 0x3F];

			/* Replace characters in output stream with "=" pad
		   characters if fewer than three characters were
		   read from the end of the input stream. */

			if (n < 3)
			{
				ogroup[3] = '=';
				if (n < 2)
					ogroup[2] = '=';
			}

			for (i = 0; i < 4; i++)
				afc_base64_internal_ochar(b64, ogroup[i]);
		}
	}

	afc_base64_internal_write(b64, eol, 2);

	return (AFC_ERR_NO_ERROR);
}

/*  INSIG  --  Return next significant input  */

static int afc_base64_internal_insig(Base64 *b64)
{
	int c;

	while (TRUE)
	{
		c = afc_base64_internal_inchar(b64);
		if (c == EOF || (c > ' '))
			return (c);
	}
}

/*  DECODE  --	Decode base64.	*/

static int afc_base64_internal_decode(Base64 *b64)
{
	register int i;
	unsigned char a[4], b[4], o[3];
	int c;

	for (i = 0; i < 255; i++)
		b64->dtable[i] = 0x80;
	for (i = 'A'; i <= 'Z'; i++)
		b64->dtable[i] = 0 + (i - 'A');
	for (i = 'a'; i <= 'z'; i++)
		b64->dtable[i] = 26 + (i - 'a');
	for (i = '0'; i <= '9'; i++)
		b64->dtable[i] = 52 + (i - '0');

	b64->dtable['+'] = 62;
	b64->dtable['/'] = 63;
	b64->dtable['='] = 0;

	while (TRUE)
	{
		for (i = 0; i < 4; i++)
		{
			c = afc_base64_internal_insig(b64);

			if (c == EOF)
			{
				if (b64->error_check && (i > 0))
				{
					return (AFC_BASE64_ERR_INCOMPLETE_INPUT);
				}
				return (AFC_BASE64_ERR_EOF);
			}

			if (b64->dtable[c] & 0x80)
			{
				if (b64->error_check)
				{
					return (AFC_BASE64_ERR_ILLEGAL_CHAR);
				}

				/* Ignoring errors: discard invalid character. */
				i--;
				continue;
			}

			a[i] = (unsigned char)c;
			b[i] = (unsigned char)b64->dtable[c];
		}

		o[0] = (b[0] << 2) | (b[1] >> 4);
		o[1] = (b[1] << 4) | (b[2] >> 2);
		o[2] = (b[2] << 6) | b[3];

		i = a[2] == '=' ? 1 : (a[3] == '=' ? 2 : 3);

		afc_base64_internal_write(b64, o, i);

		if (i < 3)
			return (AFC_ERR_NO_ERROR);
	}
}

static int afc_base64_internal_parse_tags(Base64 *b64, int first_tag, va_list tags)
{
	unsigned int tag;
	void *val;

	tag = first_tag;

	while (tag != AFC_TAG_END)
	{
		val = va_arg(tags, void *);
		afc_base64_set_tag(b64, tag, val);
		tag = va_arg(tags, int);
	}

	return (AFC_ERR_NO_ERROR);
}

static int afc_base64_internal_write(Base64 *b64, const void *mem, int size)
{
	if (b64->mem_out)
	{
		if (b64->mem_out_pos >= b64->mem_out + b64->mem_out_size)
			return (AFC_BASE64_ERR_OUT_OF_MEM); // This may be redundant
		if (b64->mem_out_pos + size > b64->mem_out + b64->mem_out_size)
			return (AFC_BASE64_ERR_OUT_OF_MEM);

		memcpy(b64->mem_out_pos, mem, size);
		b64->mem_out_pos += size;
	}
	else
		fwrite(mem, size, 1, b64->fout);

	return (AFC_ERR_NO_ERROR);
}

static int afc_base64_internal_write_char(Base64 *b64, int c)
{
	if (b64->mem_out)
	{
		if (b64->mem_out_pos >= b64->mem_out + b64->mem_out_size)
			return (AFC_BASE64_ERR_OUT_OF_MEM); // This may be redundant
		if (b64->mem_out_pos + 1 > b64->mem_out + b64->mem_out_size)
			return (AFC_BASE64_ERR_OUT_OF_MEM);

		*b64->mem_out_pos = (unsigned char)c;
		b64->mem_out_pos += 1;
	}
	else if (putc(((unsigned char)c), b64->fout) == EOF)
		return (AFC_BASE64_ERR_WRITE_ERROR);

	return (AFC_ERR_NO_ERROR);
}

#ifdef TEST_CLASS
int main()
{
	AFC *afc = afc_new();
	Base64 *b64 = afc_base64_new();
	char *mem;
	unsigned int len;
	// FILE * fh;

	/*
	fh = fopen ( "test_file.ok", "rb" );
	fseek ( fh, 0, SEEK_END );
	len = ftell ( fh );
	fseek ( fh, 0, SEEK_SET );

	mem = afc_malloc ( len );
	fread ( mem, len, 1, fh );
	fclose ( fh );

	afc_base64_encode ( b64, AFC_BASE64_TAG_MEM_IN, mem,
				 AFC_BASE64_TAG_MEM_IN_SIZE, len,
				 AFC_BASE64_TAG_FILE_OUT, "test.mem",
				 AFC_TAG_END );

	afc_free ( mem );

	mem = afc_malloc ( 50199 );
	afc_base64_decode ( b64, AFC_BASE64_TAG_FILE_IN, "test.mem",
				 AFC_BASE64_TAG_MEM_OUT, mem,
				 AFC_BASE64_TAG_MEM_OUT_SIZE, 50199,
				 AFC_TAG_END );
	*/

	len = 100 * 1024;
	mem = afc_malloc(len);

	afc_base64_encode(b64, AFC_BASE64_TAG_FILE_IN, "test_file.ok",
					  AFC_BASE64_TAG_MEM_OUT, mem,
					  AFC_BASE64_TAG_MEM_OUT_SIZE, len,
					  AFC_TAG_END);

	afc_base64_fwrite(b64, "test_file.enc.mem", AFC_BASE64_OUT);

	afc_free(mem);
	afc_base64_delete(b64);
	afc_delete(afc);

	return (0);
}
#endif
