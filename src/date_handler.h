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
#ifndef AFC_DATE_HANDLER_H
#define AFC_DATE_HANDLER_H
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>

#include "base.h"

/* DateHandler'Magic' value: 'DATE' */
#define AFC_DATE_HANDLER_MAGIC ('D' << 24 | 'A' << 16 | 'T' << 8 | 'E')

/* DateHandler Base  */
#define AFC_DATE_HANDLER_BASE 0x1000

#define AFC_DATE_HANDLER_ERR_INVALID_DATE AFC_DATE_HANDLER_BASE + 1

enum
{
	AFC_DATE_HANDLER_TAG_SEC = 1,
	AFC_DATE_HANDLER_TAG_MIN,
	AFC_DATE_HANDLER_TAG_HOUR,
	AFC_DATE_HANDLER_TAG_DAY,
	AFC_DATE_HANDLER_TAG_MONTH,
	AFC_DATE_HANDLER_TAG_YEAR,
	AFC_DATE_HANDLER_TAG_YDAY,
	AFC_DATE_HANDLER_TAG_DAYLIGHT
};

enum
{
	AFC_DATE_HANDLER_MODE_FULL = 1,
	AFC_DATE_HANDLER_MODE_YYYYMMDD,
	AFC_DATE_HANDLER_MODE_MMDDYYYY,
	AFC_DATE_HANDLER_MODE_DDMMYYYY,
	AFC_DATE_HANDLER_MODE_TEXT
};

struct afc_date_handler
{
	unsigned long magic; /* DateHandler Magic Value */

	long julian_date;

	int day;
	int month;
	int year;

	char **week_names;
	char **month_names;
};

typedef struct afc_date_handler DateHandler;

/* Function Prototypes */
#define afc_date_handler_delete(dh)   \
	if (dh)                           \
	{                                 \
		_afc_date_handler_delete(dh); \
		dh = NULL;                    \
	}

DateHandler *afc_date_handler_new(void);
int _afc_date_handler_delete(DateHandler *dh);
int afc_date_handler_clear(DateHandler *dh);

int afc_date_handler_set(DateHandler *dh, int year, int month, int day);
int afc_date_handler_set_today(DateHandler *dh);
int afc_date_handler_set_julian(DateHandler *dh, long jd);
int afc_date_handler_is_valid(DateHandler *dh, int year, int month, int day);
int afc_date_handler_get_day_of_week(DateHandler *dh);
long afc_date_handler_get_julian(DateHandler *dh);
int afc_date_handler_add_days(DateHandler *dh, int days);
int afc_date_handler_to_string(DateHandler *dh, char *dest, int mode);
#endif
