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
/*  AFCTools.h  $ 19/05/97 FR MT $  */

#ifndef AFC_STRING_H
#define AFC_STRING_H

#include <malloc.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "base.h"

#ifndef MINGW
#include <fnmatch.h>
#endif

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>

#ifdef MINGW
#include <fcntl.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define ALL (~0L)

/* Defined for afc_string_hash() */
#define afc_tools_internal_mix(a, b, c) \
  {                                     \
    a -= b;                             \
    a -= c;                             \
    a ^= (c >> 13);                     \
    b -= c;                             \
    b -= a;                             \
    b ^= (a << 8);                      \
    c -= a;                             \
    c -= b;                             \
    c ^= (b >> 13);                     \
    a -= b;                             \
    a -= c;                             \
    a ^= (c >> 12);                     \
    b -= c;                             \
    b -= a;                             \
    b ^= (a << 16);                     \
    c -= a;                             \
    c -= b;                             \
    c ^= (b >> 5);                      \
    a -= b;                             \
    a -= c;                             \
    a ^= (c >> 3);                      \
    b -= c;                             \
    b -= a;                             \
    b ^= (a << 10);                     \
    c -= a;                             \
    c -= b;                             \
    c ^= (b >> 15);                     \
  }

#define afc_string_delete(str) \
  if (str)                     \
  {                            \
    _afc_string_delete(str);   \
    str = NULL;                \
  }
#define afc_string_new(size) _afc_string_new(size, __FILE__, __FUNCTION__, __LINE__)
#define afc_string_dup(str) _afc_string_dup(str, __FILE__, __FUNCTION__, __LINE__)

  /* Function Prototypes */

  char *_afc_string_new(unsigned long numchars, const char *file, const char *func, const unsigned int line);
  char *_afc_string_delete(char *location);
  char *afc_string_copy(char *deststr, const char *sourcestr, unsigned long len);
  unsigned long afc_string_max(const char *str);
  unsigned long afc_string_len(const char *str);
  char *afc_string_mid(char *dest, const char *src, unsigned long fromchar, unsigned long numchars);
  // #define afc_string_comp(s1,s2,chars)  strncmp ( s1, s2, chars )
  signed long afc_string_comp(const char *s1, const char *s2, long chars);
  char *afc_string_upper(char *str);
  char *afc_string_lower(char *s);
  char *afc_string_trim(char *s);
  char *afc_string_instr(const char *str, const char *match, unsigned long startpos);
  char *afc_string_left(char *dest, const char *src, long len);
  char *afc_string_right(char *dest, const char *src, long len);
  unsigned long afc_string_reset_len(const char *str);
  int afc_string_pattern_match(const char *str, const char *pattern, short nocase);
  int afc_string_radix(char *dest, long n, int radix);
  unsigned long int afc_string_hash(register const unsigned char *k, register unsigned long int turbolence);
  char *_afc_string_dup(const char *str, const char *file, const char *func, const unsigned int line);
  char *afc_string_make(char *dest, const char *fmt, ...);
  char *afc_string_fget(char *dest, FILE *fh);
  char *afc_string_add(char *dest, const char *source, unsigned long len);
  char *afc_string_clear(char *dest);
  char *afc_string_temp(const char *path);
  char *afc_string_resize_copy(char **dest, const char *str);
  char *afc_string_resize_add(char **dest, const char *str);
  char *afc_string_dirname(const char *path);
  char *afc_string_basename(const char *path);
  char *afc_string_utf8_to_latin1(const char *utf8);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
