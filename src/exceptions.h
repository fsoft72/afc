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
#ifndef AFC_EXCEPTIONS
#define AFC_EXCEPTIONS

#define TRY(type)	type __rc; \
			BOOL __in_exception = FALSE; \
			BOOL __in_finally = FALSE; \
			int __exception_type = 0;

#define EXCEPT		__afc_except:\
				if ( __in_exception == FALSE ) goto __afc_finally;\
				afc_dprintf ( "%s got an exception...\n", __FUNCTION__ );

#define FINALLY		__afc_finally:\
				__in_finally = TRUE; 

#define ENDTRY	__afc_exit:\
				return ( __rc ); \

#define RETURN(return_code)	{ __rc = return_code; \
				if ( __in_finally == FALSE ) { \
					goto __afc_finally; \
				} else { \
					goto __afc_exit;\
				}}

#define RAISE(level,err,descr,info)	\
		{ \
			__in_exception = TRUE; \
			__exception_type = err; \
			AFC_LOG ( level, err, descr, info ); \
			goto __afc_except; \
		}

#define RAISE_RC(level,err,descr,info,return_code)	\
		{ \
			__in_exception = TRUE; \
			__exception_type = err; \
			AFC_LOG ( level, err, descr, info ); \
			__rc = return_code; \
			goto __afc_except; \
		}

#define RAISE_FAST(err,info) \
		{ \
			__in_exception = TRUE; \
			__exception_type = err; \
			AFC_LOG_FAST_INFO ( err, info ); \
			goto __afc_except; \
		}

#define RAISE_FAST_RC(err,info,return_code) \
		{ \
			__in_exception = TRUE; \
			__exception_type = err; \
			AFC_LOG_FAST_INFO ( err, info ); \
			__rc = return_code; \
			goto __afc_except; \
		}

#define IS_EXCEPTION()	__in_exception
#define EXCEPTION_TYPE()  __exception_type

#endif
