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
#ifndef AFC_MD5_H
#define AFC_MD5_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "afc.h"

/* MD5'Magic' value: 'MD5X' */
#define AFC_MD5_MAGIC ( 'M' << 24 | 'D' << 16 | '5' << 8 | 'X' )

/* MD5 Base  */
#define AFC_MD5_BASE 0x8100

#define AFC_MD5_ERR_FILE_NOT_FOUND	AFC_MD5_BASE + 1

struct afc_md5
{
  	unsigned  magic;     				/* MD5 Magic Value */
	unsigned  int state [ 4 ];                          /* state (ABCD) */
  	unsigned  int count [ 2 ];        		 	/* number of bits, modulo 2^64 (lsb first) */
  	unsigned char buffer [ 64 ];                         	/* input buffer */

	unsigned char digest [ 16 ];
	char * result;
};

typedef struct afc_md5 MD5;

/* Function Prototypes */
#define afc_md5_delete(md5)	if ( md5 ) { _afc_md5_delete ( md5 ); md5 = NULL; }

MD5 * afc_md5_new ( void );
int _afc_md5_delete ( MD5 * m );
int afc_md5_clear ( MD5 * m );
const char * afc_md5_digest ( MD5 * m );
int afc_md5_update( MD5 *, unsigned char * , unsigned int );
int afc_md5_encode_file ( MD5 * m, const char * fname );

#endif
