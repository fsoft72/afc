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
#include "date_handler.h"

static const char class_name [] = "DateHandler";

static int  month_totals 	[]  = { 0, 0,  31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
static char * def_week_days 	[]  = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char * def_month_names	[]  = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"  };

// static int afc_date_handler_internal_adjust_days ( DateHandler * dh );
static int afc_date_handler_internal_to_julian ( DateHandler * dh );

#define AFC_DH_IS_LEAP(year) ( ( ( year % 4 == 0 ) && ( year % 100 != 0 ) ) || ( ( year % 400 == 0 ) ) ?  TRUE : FALSE );

/*
@config
	TITLE:     DateHandler
	VERSION:   1.00
	AUTHOR:    Fabio Rotondo - fabio@rotondo.it
@endnode

@node quote
	*Ticking away the moments that make up a dull-day.*
	
	Pink Floyd
@endnode

@node intro
DateHandler is a class that handles dates.  Main features are:

- Ability to check if a given date is valid or not. 
- Ability to add or remove days to the current date.
- Julian date handling

Like all AFC classes, you can instance a new  DateHandler by calling afc_date_handler_new (), 
and free it with afc_date_handler_delete ().

To set the date, use afc_date_handler_set(), afc_date_handler_set_today() or afc_date_handler_set_julian().
You can the change the date using afc_date_handler_add_days() and getting it as a string with
afc_date_handler_to_string().
@endnode
*/

// {{{ afc_date_handler_new ()
/*
@node afc_date_handler_new

           NAME: afc_date_handler_new () - Initializes a new DateHandler instance.

       SYNOPSIS: DateHandler * afc_date_handler_new ()

    DESCRIPTION: This function initializes a new DateHandler instance.

          INPUT: NONE

        RESULTS: a valid inizialized DateHandler structure. NULL in case of errors.

       SEE ALSO: - afc_date_handler_delete()

@endnode
*/
DateHandler * afc_date_handler_new ( void )
{
	DateHandler * dh = ( DateHandler * ) afc_malloc ( sizeof ( DateHandler ) );

  	if ( dh == NULL )
   	{
    		AFC_LOG_FAST ( AFC_ERR_NO_MEMORY );
    		return ( NULL );
  	}

  	dh->magic = AFC_DATE_HANDLER_MAGIC;
	
	dh->week_names 	= def_week_days;
	dh->month_names	= def_month_names;

  	return ( dh );
}
// }}}
// {{{ afc_date_handler_delete ( dh )
/*
@node afc_date_handler_delete

           NAME: afc_date_handler_delete ( dh )  - Disposes a valid DateHandler instance.

       SYNOPSIS: int afc_date_handler_delete ( DateHandler * dh)

    DESCRIPTION: This function frees an already alloc'd DateHandler structure.

          INPUT: - dh  - Pointer to a valid afc_date_handler class.

        RESULTS: should be AFC_ERR_NO_ERROR

          NOTES: - this method calls: afc_date_handler_clear()

       SEE ALSO: - afc_date_handler_new()
                 - afc_date_handler_clear()
@endnode

*/
int _afc_date_handler_delete ( DateHandler * dh ) 
{
  	int afc_res; 

  	if ( ( afc_res = afc_date_handler_clear ( dh ) ) != AFC_ERR_NO_ERROR ) return ( afc_res );

  	afc_free ( dh );

  	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_date_handler_clear ( dh )
/*
@node afc_date_handler_clear

           NAME: afc_date_handler_clear ( dh )  - Clears all stored data

       SYNOPSIS: int afc_date_handler_clear ( DateHandler * dh)

    DESCRIPTION: Use this function to clear all stored data in the current dh instance.

          INPUT: - dh    - Pointer to a valid afc_date_handler instance.

        RESULTS: should be AFC_ERR_NO_ERROR
                
       SEE ALSO: - afc_date_handler_delete()
                 
@endnode
*/
int afc_date_handler_clear ( DateHandler * dh ) 
{
  	if ( dh == NULL ) return ( AFC_ERR_NULL_POINTER );
 
  	if ( dh->magic != AFC_DATE_HANDLER_MAGIC ) return ( AFC_ERR_INVALID_POINTER );



  	/* Custom Clean-up code should go here */


  	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_date_handler_set ( dh, year, month, day )
/*
@node afc_date_handler_set

           NAME: afc_date_handler_set ( dh, year, month, day )  - Set a new date

       SYNOPSIS: int afc_date_handler_set ( DateHandler * dh, int year, int month, int day )

    DESCRIPTION: This method sets the date specified with /year/, /month/ and /day/ in the current
		 DateHandler instance.

          INPUT: - dh    	- Pointer to a valid afc_date_handler instance.
		 - year  	- Date's Year
		 - month 	- Date's month
		 - day		- Date's day

        RESULTS: - AFC_ERR_NO_ERROR			- No error occurred
		 - AFC_DATE_HANDLER_ERR_INVALID_DATE	- Provided date is not valid
                
       SEE ALSO: - afc_date_handler_set_today()
		 - afc_date_handler_set_julian()
		 - afc_date_handler_is_valid()
@endnode
*/
int afc_date_handler_set ( DateHandler * dh, int year, int  month, int day )
{
	int res;

	if ( ( res = afc_date_handler_is_valid ( dh, year, month, day ) ) != AFC_ERR_NO_ERROR ) return ( res );

	dh->year 	= year;
	dh->month 	= month;
	dh->day 	= day;

	afc_date_handler_internal_to_julian ( dh );

	return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_date_handler_set_today ( dh )
/*
@node afc_date_handler_set_today

           NAME: afc_date_handler_set_today ( dh )  - Set today's date

       SYNOPSIS: int afc_date_handler_set_today ( DateHandler * dh )

    DESCRIPTION: This method sets today's date in the current DateHandler instance.

          INPUT: - dh    	- Pointer to a valid afc_date_handler instance.

        RESULTS: - AFC_ERR_NO_ERROR			- No error occurred
		 - AFC_DATE_HANDLER_ERR_INVALID_DATE	- Provided date is not valid
                
       SEE ALSO: - afc_date_handler_set()
		 - afc_date_handler_set_julian()
@endnode
*/
int afc_date_handler_set_today ( DateHandler * dh )
{
	time_t time_val;
        struct tm *tm_ptr;

        time ( &time_val );
        tm_ptr = localtime ( & time_val );

	return ( afc_date_handler_set ( dh, 1900 + tm_ptr->tm_year, tm_ptr->tm_mon + 1, tm_ptr->tm_mday ) );
}
// }}}
// {{{ afc_date_handler_set_julian ( dh, julian_date )
/*
@node afc_date_handler_set_julian

           NAME: afc_date_handler_set_julian ( dh, julian_date )  - Set Julian Date

       SYNOPSIS: int afc_date_handler_set_julian ( DateHandler * dh, long julian_date )

    DESCRIPTION: This method sets the date using the Julian Date rappresentation.

          INPUT: - dh    	- Pointer to a valid afc_date_handler instance.
		 - julian_date	- The Julian date to be set.

        RESULTS: - AFC_ERR_NO_ERROR			- No error occurred
		 - AFC_DATE_HANDLER_ERR_INVALID_DATE	- Provided date is not valid
                
       SEE ALSO: - afc_date_handler_set()
		 - afc_date_handler_set_today()
@endnode
*/
int afc_date_handler_set_julian ( DateHandler * dh, long jd )
{
	long L, N, I, J;
	int day, month, year;

	L = jd + 68569;
    	N = ( long )  ( ( 4 * L ) /146097 );
    	L = L - ( ( long ) ( ( 146097 * N + 3 ) / 4 ) );
    	I = ( long )  ( ( 4000 * ( L + 1 ) / 1461001 ) );
    	L = L - ( long ) ( ( 1461 * I ) / 4 ) + 31;
    	J = ( long ) ( ( 80 * L ) / 2447 );

    	day = L - ( long ) ( ( 2447 * J ) / 80 );
    	L = ( long ) ( J / 11 );
    	month = J + 2 - 12 * L;
    	year = 100 * ( N - 49 ) + I + L ;

	return ( afc_date_handler_set ( dh, year, month, day ) );	
}
// }}}
// {{{ afc_date_handler_is_valid ( dh, year, month, day )
/*
@node afc_date_handler_is_valid

           NAME: afc_date_handler_is_valid ( dh, year, month, day )  - Validate a date

       SYNOPSIS: int afc_date_handler_is_valid ( DateHandler * dh, int year, int month, int day )

    DESCRIPTION: This method checks whether the provided date is valid or not. This is the same
		 validation check performed by afc_date_handler_set() before actually setting the
		 date inside the given instance of DateHandler.

          INPUT: - dh    	- Pointer to a valid afc_date_handler instance.
		 - year  	- Date's Year
		 - month 	- Date's month
		 - day		- Date's day

        RESULTS: - AFC_ERR_NO_ERROR			- No error occurred
		 - AFC_DATE_HANDLER_ERR_INVALID_DATE	- Provided date is not valid
                
       SEE ALSO: - afc_date_handler_set()
		 - afc_date_handler_set_today()
@endnode
*/
int afc_date_handler_is_valid ( DateHandler * dh, int year, int month, int day )
{
	int is_leap, month_days ;

	if ( month > 12 || day > 31 ) return ( AFC_LOG ( AFC_LOG_WARNING, AFC_DATE_HANDLER_ERR_INVALID_DATE, "Invalid month or day", NULL ) );

	is_leap = AFC_DH_IS_LEAP ( year );

        month_days = month_totals[ month+1 ] -  month_totals[ month ];

        if ( ( month == 2 ) && is_leap ) month_days ++;

        if ( year  < 0  ||
             month < 1  ||  month > 12  ||
             day   < 1  ||  day   > month_days ) return ( AFC_LOG ( AFC_LOG_WARNING, AFC_DATE_HANDLER_ERR_INVALID_DATE, "Invalid date", NULL ) );

        return ( AFC_ERR_NO_ERROR );
}
// }}}
// {{{ afc_date_handler_get_day_of_week ( dh )
/*
@node afc_date_handler_get_day_of_week

           NAME: afc_date_handler_get_day_of_week ( dh )  - Returns day of week

       SYNOPSIS: int afc_date_handler_get_day_of_week ( DateHandler * dh )

    DESCRIPTION: Returns the ordinal value of the day of week.
		 Values range from 0 to 6: 0 = "Sunday", 1 = Monday and so on.

          INPUT: - dh    	- Pointer to a valid afc_date_handler instance.

        RESULTS: - the ordinal value of the day of week
		
       SEE ALSO: - afc_date_handler_get_julian()
		
@endnode
*/
int afc_date_handler_get_day_of_week ( DateHandler * dh )
{
	return ( ( ( int ) ( dh->julian_date + 1.5 ) ) % 7 );
}
// }}}
// {{{ afc_date_handler_get_julian ( dh )
/*
@node afc_date_handler_get_julian

           NAME: afc_date_handler_get_julian ( dh )  - Returns the Julian Date

       SYNOPSIS: long afc_date_handler_get_julian ( DateHandler * dh )

    DESCRIPTION: This method returns the Julian value of the date set using
		 afc_date_handler_set() or afc_date_handler_set_today().

          INPUT: - dh    	- Pointer to a valid afc_date_handler instance.

        RESULTS: - the ordinal value of the day of week, 0 = "Sunday", 1 = Monday and so on.
		
       SEE ALSO: - afc_date_handler_get_day_of_week()
		 - afc_date_handler_set()
		 - afc_date_handler_set_today()
		
@endnode
*/
long afc_date_handler_get_julian ( DateHandler * dh )
{
	return ( dh->julian_date );
}
// }}}
// {{{ afc_date_handler_add_days ( dh, days )
/*
@node afc_date_handler_add_days

           NAME: afc_date_handler_add_days ( dh, days )  - Add more days to current date

       SYNOPSIS: int afc_date_handler_add_days ( DateHandler * dh, int days )

    DESCRIPTION: This method adds /days/ to the current date. /days/ can be a negative or positive value.

          INPUT: - dh    	- Pointer to a valid afc_date_handler instance.
		 - days		- Days to add (or subtract) to the current date.

        RESULTS: - AFC_ERR_NO_ERROR
		
       SEE ALSO: - afc_date_handler_set()
		 - afc_date_handler_set_today()
		
@endnode
*/
int afc_date_handler_add_days ( DateHandler * dh, int days )
{
	dh->julian_date += days;

	return ( afc_date_handler_set_julian ( dh, dh->julian_date ) );
}
// }}}
// {{{ afc_date_handler_to_string ( dh, dest, mode )
/*
@node afc_date_handler_to_string

           NAME: afc_date_handler_to_string ( dh, dest, mode )  - Converts current date to string

       SYNOPSIS: int afc_date_handler_to_string ( DateHandler * dh, char * dest, int mode )

    DESCRIPTION: This method converts the current date in DateHandler into a string, using
		 the format specified with the /mode/ variable.

          INPUT: - dh    	- Pointer to a valid afc_date_handler instance.
		 - dest		- Destination buffer where to copy the string to.
		 - mode		- Valid string format. Values are:

					+ AFC_DATE_HANDLER_MODE_FULL	 	- A full date: wday day month year (dd/mm/yyyy)
					+ AFC_DATE_HANDLER_MODE_YYYYMMDD	- A string in YYYY/MM/DD format
					+ AFC_DATE_HANDLER_MODE_MMDDYYYY  	- A string in MM/DD/YYYY format
					+ AFC_DATE_HANDLER_MODE_DDMMYYYY   	- A string in DD/MM/YYYY format
					+ AFC_DATE_HANDLER_MODE_TEXT		- A string: wday day month year 

        RESULTS: - AFC_ERR_NO_ERROR
		
       SEE ALSO: 
		
		
@endnode
*/
int afc_date_handler_to_string ( DateHandler * dh, char * dest, int mode )
{

	switch ( mode )
	{
		case AFC_DATE_HANDLER_MODE_TEXT:
			sprintf ( dest, "%s %2.2d %s %d", dh->week_names [ afc_date_handler_get_day_of_week ( dh ) ], 
					  dh->day, dh->month_names [ dh->month -1 ],
					  dh->year );
			break;

		case AFC_DATE_HANDLER_MODE_FULL:
			sprintf ( dest, "%s %2.2d %s %d (%2.2d/%2.2d/%4.4d)", dh->week_names [ afc_date_handler_get_day_of_week ( dh ) ], 
					  dh->day, dh->month_names [ dh->month -1 ],
					  dh->year,
					  dh->day, dh->month, dh->year);
			break;


		case AFC_DATE_HANDLER_MODE_MMDDYYYY:
			sprintf ( dest, "%2.2d/%2.2d/%4.4d", dh->month, dh->day, dh->year );
			break;

		case AFC_DATE_HANDLER_MODE_DDMMYYYY:
			sprintf ( dest, "%2.2d/%2.2d/%4.4d", dh->day, dh->month, dh->year );
			break;

		case AFC_DATE_HANDLER_MODE_YYYYMMDD:
		default:
			sprintf ( dest, "%4.4d/%2.2d/%2.2d", dh->year, dh->month, dh->day );
			break;
	}
					

	return ( AFC_ERR_NO_ERROR );
}
// }}}


/* ============================================================================================
   INTERNAL FUNCTIONS
============================================================================================ */
// {{{ afc_date_handler_internal_do_julian ( dh )
static int afc_date_handler_internal_to_julian ( DateHandler * dh )
{
	int m, d, y;

	m = dh->month;
	y = dh->year;
	d = dh->day;

        if ( m < 3 )
        {
      		m += 12;
      		--y;
        }

	dh->julian_date = ( d + ( 153 * m - 457 ) / 5 + 365 * y + ( y / 4 ) - ( y / 100 ) + ( y / 400 ) + 1721119 );

	return ( AFC_ERR_NO_ERROR );
}
// }}}

#ifdef TEST_CLASS
// {{{ TEST_CLASS
int main ( int argc, char * argv[] )
{
	AFC * afc = afc_new ();
	char buf [ 1024 ];
  	DateHandler * dh = afc_date_handler_new();

  	if ( dh == NULL ) 
  	{
    		fprintf ( stderr, "Init of class DateHandler failed.\n" );
    		return ( 1 );
  	}

	// afc_date_handler_set ( dh, 1972, 1, 10 );
	afc_date_handler_set_today ( dh );
	afc_date_handler_to_string ( dh, buf, 0 );
	printf ( "Date: %s\n", buf );

	afc_date_handler_set ( dh, 1972, 2, 30 );
	afc_date_handler_add_days ( dh, 366 );
	afc_date_handler_to_string ( dh, buf, 0 );


	printf ( "Date: %s\n", buf );

	// afc_date_handler_debug_dump ( dh );

  	afc_date_handler_delete ( dh );
  	afc_delete ( afc );

  	return ( 0 ); 
}
// }}}
#endif
