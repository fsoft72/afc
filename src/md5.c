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
#include "md5.h"

/*
@config
	TITLE:     MD5
	VERSION:   1.00
	AUTHOR:    Lara Savoini - savo76@tin.it
	AUTHOR:	   Fabio Rotondo - fabio@rotondo.it
@endnode
*/

static const char class_name [] = "Md5";

static int afc_md5_internal_transform ( unsigned  int * state, unsigned char * block );
static int afc_md5_internal_encode (unsigned char *output, unsigned  int *input, unsigned int len);
static int afc_md5_internal_decode ( unsigned  int *output, unsigned char *input, unsigned int len );
static int afc_md5_internal_init ( MD5 * m );


// {{{ docs
/*
@node quote
	*Who are you going to believe, me or your own eyes?*

		Groucho Marx
@endnode

@node intro
MD5 is a class that can compute the md5 value for strings and files.
@endnode

@node history
	- 1.00:		Initial Release
@endnode
*/
// }}}

// {{{ macros
static unsigned char PADDING [ 64 ] = { 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* Constants for MD5Transform routine.  */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

/* F, G, H and I are basic MD5 functions.  */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.  */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.  Rotation is separate from addition to prevent recomputation.  */
#define FF(a, b, c, d, x, s, ac) \
	{ \
	 (a) += F ((b), (c), (d)) + (x) + (unsigned  int)(ac); \
	 (a) = ROTATE_LEFT ((a), (s)); \
	 (a) += (b); \
  	}
#define GG(a, b, c, d, x, s, ac) \
	{ \
 	(a) += G ((b), (c), (d)) + (x) + (unsigned  int)(ac); \
 	(a) = ROTATE_LEFT ((a), (s)); \
 	(a) += (b); \
  	}
#define HH(a, b, c, d, x, s, ac) \
	{ \
 	(a) += H ((b), (c), (d)) + (x) + (unsigned  int)(ac); \
 	(a) = ROTATE_LEFT ((a), (s)); \
 	(a) += (b); \
  	}
#define II(a, b, c, d, x, s, ac) \
	{ \
 	(a) += I ((b), (c), (d)) + (x) + (unsigned  int)(ac); \
 	(a) = ROTATE_LEFT ((a), (s)); \
 	(a) += (b); \
  	}
// }}}

// {{{ afc_md5_new
/*
@node afc_md5_new

           NAME: afc_md5_new () - Initializes a new MD5 instance.

       SYNOPSIS: MD5 * afc_md5_new ()

    DESCRIPTION: This function initializes a new MD5 instance.

          INPUT: NONE

        RESULTS: a valid inizialized MD5 structure. NULL in case of errors.

       SEE ALSO: - afc_md5_delete()

@endnode
*/
MD5 * afc_md5_new ( void )
{
TRY ( MD5 * )

	MD5 * m = ( MD5 * ) afc_malloc ( sizeof ( MD5 ) );

  	if ( m == NULL ) RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "md5", NULL );
  	m->magic = AFC_MD5_MAGIC;

	if ( ( m->result = afc_string_new ( 33 ) ) == NULL ) RAISE_FAST_RC ( AFC_ERR_NO_MEMORY, "result", NULL );
	afc_md5_internal_init ( m );
	RETURN ( m );

EXCEPT
	afc_md5_delete ( m );

FINALLY

ENDTRY
}
// }}}
// {{{ afc_md5_delete ( m )
/*
@node afc_md5_delete

           NAME: afc_md5_delete ( m )  - Disposes a valid MD5 instance.

       SYNOPSIS: int afc_md5_delete ( MD5 * m)

    DESCRIPTION: This function frees an already alloc'd MD5 structure.

          INPUT: - m  - Pointer to a valid afc_MD5 class.

        RESULTS: should be AFC_ERR_NO_ERROR

          NOTES: - this method calls: afc_md5_clear()

       SEE ALSO: - afc_md5_new()
                 - afc_md5_clear()
@endnode
*/
int _afc_md5_delete ( MD5 * m ) 
{
  	int afc_res; 

  	if ( ( afc_res = afc_md5_clear ( m ) ) != AFC_ERR_NO_ERROR ) return ( afc_res );
	afc_string_delete ( m->result );
  	afc_free ( m );

  	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_md5_clear ( m )
/*
@node afc_md5_clear

           NAME: afc_md5_clear ( m )  - Clears all stored data

       SYNOPSIS: int afc_md5_clear ( MD5 * m)

    DESCRIPTION: Use this function to clear all stored data in the current m instance.

          INPUT: - m    - Pointer to a valid afc_MD5 instance.

        RESULTS: should be AFC_ERR_NO_ERROR
                
       SEE ALSO: - afc_md5_delete()
                 
@endnode
*/
int afc_md5_clear ( MD5 * m ) 
{
  	if ( m == NULL ) return ( AFC_ERR_NULL_POINTER );
 
  	if ( m->magic != AFC_MD5_MAGIC ) return ( AFC_ERR_INVALID_POINTER );

	afc_md5_internal_init ( m );

  	/* Custom Clean-up code should go here */

  	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_md5_update ( m, input, input_len )
int afc_md5_update ( MD5 * m, unsigned char * input, unsigned int input_len )
{
 	 unsigned int i, index, part_len;

  	/* Compute number of bytes mod 64 */
  	index = ( unsigned int ) ( ( m->count [ 0 ] >> 3 ) & 0x3F );

  	/* Update number of bits */
  	if ( ( m->count [ 0 ] += ( ( unsigned  int ) input_len << 3 ) ) < ( ( unsigned  int ) input_len << 3 ) ) m->count [ 1 ] ++;
  
	m->count [ 1 ] += ( ( unsigned  int ) input_len >> 29 );

	part_len = 64 - index;

  	/* Transform as many times as possible.  */
	if ( input_len >= part_len )
	{
		memcpy ( ( char * ) & m->buffer [ index ], ( char * ) input, part_len );
		afc_md5_internal_transform ( m->state, m->buffer );

		for ( i = part_len; i + 63 < input_len; i += 64 ) afc_md5_internal_transform ( m->state, &input [ i ] );

		index = 0;
	} else 
		i = 0;

	/* Buffer remaining input */
  	memcpy ( ( char * ) & m->buffer [ index ] , ( char * ) & input [ i ], input_len-i );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_md5_digest ( MD5 * m )
/* MD5 finalization. Ends an MD5 message-digest operation, writing the the message digest and zeroizing the context.  */
const char * afc_md5_digest ( MD5 * m )
{
	unsigned char bits[8];
  	unsigned int index, padLen, t;
	char buf [ 3 ];

	/* Save number of bits */
	afc_md5_internal_encode ( bits , m->count , 8 );

	/* Pad out to 56 mod 64.  */
	index = ( unsigned int ) ( ( m->count [ 0 ] >> 3 ) & 0x3f );
	padLen = ( index < 56 ) ? ( 56 - index ) : ( 120 - index );
	afc_md5_update ( m, PADDING, padLen );

	/* Append length (before padding) */
	afc_md5_update ( m, bits, 8 );

	/* Store state in digest */
	afc_md5_internal_encode ( m->digest, m->state, 16 );

	/* Copy the code inside the 32 bytes string 'result' to allow easy handling of the result */
	afc_string_clear ( m->result );
	
	for ( t = 0; t < 16; t ++ ) 
	{
		sprintf ( buf, "%2.2x", ( unsigned char ) m->digest [ t ] );
		afc_string_add ( m->result, buf, ALL );
	}

	return ( m->result );
}
// }}}
// {{{ afc_md5_encode_file ( m, fname )
int afc_md5_encode_file ( MD5 * m, const char * fname )
{
	FILE * 	fh;
	unsigned char * buf;
	int    	len, bytes;
	long	file_len;

	if ( ( fh = fopen ( fname, "r" ) ) == NULL )
		return ( AFC_LOG ( AFC_LOG_ERROR, AFC_MD5_ERR_FILE_NOT_FOUND, "Cannot read file", fname ) );

	fseek ( fh, 0, SEEK_END );
	file_len = ftell ( fh );
	fseek ( fh, 0, SEEK_SET );

	if ( ( buf = afc_malloc ( 1024 ) ) == NULL )
	{
		fclose ( fh );
		return ( AFC_LOG_FAST ( AFC_ERR_NO_MEMORY ) );
	}

	afc_md5_clear ( m );

	while ( ! feof ( fh ) )
	{
		bytes = fread ( buf, 1024, 1, fh );
		if ( bytes < 0 ) break;

		file_len -= 1024;

		if ( file_len < 0 ) 
			len = file_len + 1024;
		else
			len = 1024;

		afc_md5_update ( m, buf, len );

		memset ( buf, 0, 1024 );
	}

	fclose ( fh );
	afc_free ( buf );

	return ( AFC_ERR_NO_ERROR );
}
// }}}

// ===========================================================================================================
// INTERNAL FUNCTIONS
// ===========================================================================================================

// {{{ afc_md5_internal_transform ( state, block )
/* MD5 basic transformation. Transforms state based on block.  */ 
static int afc_md5_internal_transform ( unsigned  int * state, unsigned char * block )
{
	unsigned  int a = state [ 0 ], b = state [ 1 ], c = state [ 2 ], d = state [ 3 ], x [ 16 ];

  	afc_md5_internal_decode ( x, block, 64 );

	  /* Round 1 */
	FF ( a, b, c, d, x [ 0 ], S11, 0xd76aa478 ); /* 1 */
	FF ( d, a, b, c, x [ 1 ], S12, 0xe8c7b756 ); /* 2 */
	FF ( c, d, a, b, x [ 2 ], S13, 0x242070db ); /* 3 */
	FF ( b, c, d, a, x [ 3 ], S14, 0xc1bdceee ); /* 4 */
	FF ( a, b, c, d, x [ 4 ], S11, 0xf57c0faf ); /* 5 */
	FF ( d, a, b, c, x [ 5 ], S12, 0x4787c62a ); /* 6 */
	FF ( c, d, a, b, x [ 6 ], S13, 0xa8304613 ); /* 7 */
	FF ( b, c, d, a, x [ 7 ], S14, 0xfd469501 ); /* 8 */
	FF ( a, b, c, d, x [ 8 ], S11, 0x698098d8 ); /* 9 */
	FF ( d, a, b, c, x [ 9 ], S12, 0x8b44f7af ); /* 10 */
	FF ( c, d, a, b, x [ 10 ], S13, 0xffff5bb1 ); /* 11 */
	FF ( b, c, d, a, x [ 11 ], S14, 0x895cd7be ); /* 12 */
	FF ( a, b, c, d, x [ 12 ], S11, 0x6b901122 ); /* 13 */
	FF ( d, a, b, c, x [ 13 ], S12, 0xfd987193 ); /* 14 */
	FF ( c, d, a, b, x [ 14 ], S13, 0xa679438e ); /* 15 */
	FF ( b, c, d, a, x [ 15 ], S14, 0x49b40821 ); /* 16 */

	 /* Round 2 */
	GG ( a, b, c, d, x [ 1 ], S21, 0xf61e2562 ); /* 17 */
	GG ( d, a, b, c, x [ 6 ], S22, 0xc040b340 ); /* 18 */
	GG ( c, d, a, b, x [ 11 ], S23, 0x265e5a51 ); /* 19 */
	GG ( b, c, d, a, x [ 0 ], S24, 0xe9b6c7aa ); /* 20 */
	GG ( a, b, c, d, x [ 5 ], S21, 0xd62f105d ); /* 21 */
	GG ( d, a, b, c, x [ 10 ], S22,  0x2441453 ); /* 22 */
	GG ( c, d, a, b, x [ 15 ], S23, 0xd8a1e681 ); /* 23 */
	GG ( b, c, d, a, x [ 4 ], S24, 0xe7d3fbc8 ); /* 24 */
	GG ( a, b, c, d, x [ 9 ], S21, 0x21e1cde6 ); /* 25 */ 
	GG ( d, a, b, c, x [ 14 ], S22, 0xc33707d6 ); /* 26 */
	GG ( c, d, a, b, x [ 3 ], S23, 0xf4d50d87 ); /* 27 */
	GG ( b, c, d, a, x [ 8 ], S24, 0x455a14ed ); /* 28 */
	GG ( a, b, c, d, x [ 13 ], S21, 0xa9e3e905 ); /* 29 */
	GG ( d, a, b, c, x [ 2 ], S22, 0xfcefa3f8 ); /* 30 */
	GG ( c, d, a, b, x [ 7 ], S23, 0x676f02d9 ); /* 31 */
	GG ( b, c, d, a, x [ 12 ], S24, 0x8d2a4c8a ); /* 32 */

	  /* Round 3 */
	HH ( a, b, c, d, x [ 5 ], S31, 0xfffa3942 ); /* 33 */
	HH ( d, a, b, c, x [ 8 ], S32, 0x8771f681 ); /* 34 */
	HH ( c, d, a, b, x [ 11 ], S33, 0x6d9d6122 ); /* 35 */
	HH ( b, c, d, a, x [ 14 ], S34, 0xfde5380c ); /* 36 */
	HH ( a, b, c, d, x [ 1 ], S31, 0xa4beea44 ); /* 37 */
	HH ( d, a, b, c, x [ 4 ], S32, 0x4bdecfa9 ); /* 38 */
	HH ( c, d, a, b, x [ 7 ], S33, 0xf6bb4b60 ); /* 39 */
	HH ( b, c, d, a, x [ 10 ], S34, 0xbebfbc70 ); /* 40 */
	HH ( a, b, c, d, x [ 13 ], S31, 0x289b7ec6 ); /* 41 */
	HH ( d, a, b, c, x [ 0 ], S32, 0xeaa127fa ); /* 42 */
	HH ( c, d, a, b, x [ 3 ], S33, 0xd4ef3085 ); /* 43 */
	HH ( b, c, d, a, x [ 6 ], S34,  0x4881d05 ); /* 44 */
	HH ( a, b, c, d, x [ 9 ], S31, 0xd9d4d039 ); /* 45 */
	HH ( d, a, b, c, x [ 12 ], S32, 0xe6db99e5 ); /* 46 */
	HH ( c, d, a, b, x [ 15 ], S33, 0x1fa27cf8 ); /* 47 */
	HH ( b, c, d, a, x [ 2 ], S34, 0xc4ac5665 ); /* 48 */

	  /* Round 4 */
	II ( a, b, c, d, x [ 0 ], S41, 0xf4292244 ); /* 49 */
	II ( d, a, b, c, x [ 7 ], S42, 0x432aff97 ); /* 50 */
	II ( c, d, a, b, x [ 14 ], S43, 0xab9423a7 ); /* 51 */
	II ( b, c, d, a, x [ 5 ], S44, 0xfc93a039 ); /* 52 */
	II ( a, b, c, d, x [ 12 ], S41, 0x655b59c3 ); /* 53 */
	II ( d, a, b, c, x [ 3 ], S42, 0x8f0ccc92 ); /* 54 */
	II ( c, d, a, b, x [ 10 ], S43, 0xffeff47d ); /* 55 */
	II ( b, c, d, a, x [ 1 ], S44, 0x85845dd1 ); /* 56 */
	II ( a, b, c, d, x [ 8 ], S41, 0x6fa87e4f ); /* 57 */
	II ( d, a, b, c, x [ 15 ], S42, 0xfe2ce6e0 ); /* 58 */
	II ( c, d, a, b, x [ 6 ], S43, 0xa3014314 ); /* 59 */
	II ( b, c, d, a, x [ 13 ], S44, 0x4e0811a1 ); /* 60 */
	II ( a, b, c, d, x [ 4 ], S41, 0xf7537e82 ); /* 61 */
	II ( d, a, b, c, x [ 11 ], S42, 0xbd3af235 ); /* 62 */
	II ( c, d, a, b, x [ 2 ], S43, 0x2ad7d2bb ); /* 63 */
	II ( b, c, d, a, x [ 9 ], S44, 0xeb86d391 ); /* 64 */

	state [ 0 ] += a;
	state [ 1 ] += b;
	state [ 2 ] += c;
	state [ 3 ] += d;

	 /* Zeroize sensitive information. */
	//memset ( x, 0, sizeof ( x ) );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_md5_internal_encode ( output, input, len )
/* Encodes input (unsigned  int) into output (unsigned char). Assumes len is a multiple of 4.  */
static int afc_md5_internal_encode (unsigned char *output, unsigned  int *input, unsigned int len)
{
	unsigned int i, j;

	for ( i = 0, j = 0; j < len; i++, j += 4 ) {
			output [ j ] = ( unsigned char ) ( input [ i ] & 0xff );
			output [ j+1 ] = ( unsigned char ) ( ( input [ i ] >> 8 ) & 0xff );
			output [ j+2 ] = ( unsigned char ) ( ( input [ i ] >> 16 ) & 0xff );
			output [ j+3 ] = ( unsigned char ) ( ( input [ i ] >> 24 ) & 0xff );
  		}

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_md5_internal_decode ( output, input, len )
/* Decodes input (unsigned char) into output (unsigned  int). Assumes len is a multiple of 4. */
static int afc_md5_internal_decode ( unsigned  int *output, unsigned char *input, unsigned int len )
{
	unsigned int i, j;
	for ( i = 0, j = 0; j < len; i++, j += 4 )
	output [ i ] = ( ( unsigned  int ) input [ j ] ) | ( ( ( unsigned  int ) input [ j+1 ] ) << 8) | ( ( ( unsigned  int )input [ j+2 ] ) << 16 ) | ( ( ( unsigned  int ) input [ j+3 ] ) << 24 );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_md5_internal_init ( m )
static int afc_md5_internal_init ( MD5 * m )
{
 	m->count [ 0 ] = m->count [ 1 ] = 0;

  	// Load magic initialization constants.
	m->state [ 0 ] = 0x67452301;
  	m->state [ 1 ] = 0xefcdab89;
  	m->state [ 2 ] = 0x98badcfe;
  	m->state [ 3 ] = 0x10325476;

	return ( AFC_ERR_NO_ERROR );
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
int main ( int argc, char * argv[] )
{
  	AFC * afc = afc_new ();
  	MD5 * m;

	afc_track_mallocs ( afc );

	afc_set_tags (afc, AFC_TAG_LOG_LEVEL, AFC_LOG_WARNING, 
			   AFC_TAG_SHOW_MALLOCS, FALSE,
			   AFC_TAG_SHOW_FREES,   FALSE, 
		           AFC_TAG_END);

	m = afc_md5_new ();

  	if ( m == NULL ) 
  	{
    		fprintf ( stderr, "Init of class MD5 failed.\n" );
    		return ( 1 );
  	}


	afc_md5_update ( m, ( unsigned char * ) "a", 1 );
	printf ( "RES: %s - Expected: 0cc175b9c0f1b6a831c399e269772661\n", afc_md5_digest ( m ) );

	afc_md5_clear ( m );
	afc_md5_update ( m, ( unsigned char * ) "ciao lara", 9 );
	printf ( "RES: %s - Expected: fcf5da192c4765a744dd665b902ff1b0\n", afc_md5_digest ( m ) );

	afc_md5_encode_file ( m, "md5.o" );
	printf ( "RES: %s\n", afc_md5_digest ( m ) );

  	afc_md5_delete ( m );
  	afc_delete ( afc );

  	return ( 0 ); 
}
// }}}
#endif
