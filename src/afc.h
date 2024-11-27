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
#ifndef AFC_LIBS_H
#define AFC_LIBS_H

#include "base.h"
#include "exceptions.h"
#include "mem_tracker.h"
#include "list.h"
#include "stringlist.h"
#include "readargs.h"
#include "regexp.h"
#include "hash_master.h"
#include "array.h"
#include "dictionary.h"
#include "cgi_manager.h"
#include "string.h"
#include "fileops.h"
#include "bin_tree.h"
#include "date_handler.h"
#include "circular_list.h"
#include "btree.h"
#include "md5.h"
#include "base64.h"

#ifndef MINGW

#include "dirmaster.h"
#include "dynamic_class_master.h"
#include "dynamic_class.h"
#include "cmd_parser.h"
#include "inet_client.h"
#include "inet_server.h"
#include "dbi_manager.h"
#include "pop3.h"

#endif

#endif
