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
#ifndef AFC_READARGS_H
#define AFC_READARGS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <stdarg.h>

#include "base.h"
#include "nodemaster.h"
#include "stringnode.h"
#include "string.h"

#ifdef MINGW
	#define index strchr
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* AFC ReadArgs Base Value */
#define AFC_READARGS_BASE 0x5000

/* AFC ReadArgs Magic Value: READ */
#define AFC_READARGS_MAGIC ( 'R' << 24 | 'E' << 16 | 'A' << 8 | 'D' )

#define AFC_READARGS_MAX_FIELD_NAME 30

enum { 	AFC_READARGS_MODE_REQUIRED = 1,
	AFC_READARGS_MODE_KEYWORD,
	AFC_READARGS_MODE_NUMERIC, 
	AFC_READARGS_MODE_SWITCH,
	AFC_READARGS_MODE_MULTI 
};

// ERRORS
enum {	AFC_READARGS_ERR_MISSING_KEYWORD  = 1,
	AFC_READARGS_ERR_REQUIRED,
	AFC_READARGS_ERR_NO_NUMERIC_FIELD,
	AFC_READARGS_ERR_NOT_A_NUMBER,
	AFC_READARGS_ERR_HELP_REQUESTED
};


struct afc_readargs 
{
  unsigned long magic;
	
  struct afc_nodemaster * fields;
  struct afc_nodemaster * str;

  struct afc_stringnode * global_split ;
  struct afc_stringnode * local_split;

  char * buffer;
};

struct afc_readargs_data
{
  struct afc_nodemaster * multi;

  char * name;
  void * data;

  short  is_switch;
  short  is_required;
  short  is_numeric;
  short  is_keyword;
  short  need_keyword;
};

typedef struct afc_readargs ReadArgs;

#define afc_readargs_delete(ra)	if ( ra ) { _afc_readargs_delete ( ra ); ra = NULL; }

ReadArgs * afc_readargs_new (void);
int    _afc_readargs_delete ( ReadArgs *  );

int    afc_readargs_parse ( ReadArgs *, const char *, const char * );
int    afc_readargs_clear ( ReadArgs * );
void * afc_readargs_get_by_name ( ReadArgs * , const char *  );
void * afc_readargs_get_by_pos  ( ReadArgs * , int     );
int afc_readargs_parse_cmd_line ( ReadArgs * rdargs, char * templ, int argc, char * argv[] );
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
