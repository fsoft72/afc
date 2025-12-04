# CLAUDE.md

Important: read [Guidelines](./ai/guidelines.md) before working on this repository.

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

AFC (Advanced Foundation Classes) is a C library providing foundation classes for application development. Originally "Amiga Foundation Classes," it now targets Linux and open source platforms. AFC provides building blocks like strings, arrays, lists, hash tables, networking, database abstraction, and more.

The library uses:
- **Tag-based API system**: Parameters are passed using tags (similar to varargs) allowing API extension without breaking compatibility
- **Standardized naming**: All public APIs follow `afc_<class>_<action>` pattern (e.g., `afc_string_copy`, `afc_array_add`)
- **Memory tracking**: Built-in memory leak detection via `mem_tracker` class
- **C++ bindings**: Available but not compiled by default

## Building and Installation

### Standard Build
```bash
# Build both static and shared libraries
make

# Install libraries and headers (as root)
make install
```

Libraries install to `/usr/local/lib`, binaries to `/usr/local/bin`, headers to `/usr/local/include/afc/`.

### MinGW Build
```bash
make DEFINE=MINGW mingw_all
```
MinGW builds include a subset of AFC (no networking, DBI, or dynamic class features).

### C++ Bindings
```bash
make cpp
make cpp_install
```

### Clean Build
```bash
make clean
```

## Testing

### Building Tests

Individual module tests are in `src/test_area/<module>/`:
```bash
# Build all tests for a specific module
cd src/test_area/string
make

# Build specific test
make test_01

# Build with leak detection
make leak
```

### Running Tests

Tests are standalone executables:
```bash
cd src/test_area/string
./test_01
```

Each test uses `test_utils.h` helper functions (`test_header()`, `print_res()`, etc.).

### Building Individual Module Tests from src/

From `src/` directory, you can build test executables:
```bash
# Examples from src/Makefile
make bin_tree.test
make array.test
make md5.test
```

## Architecture

### Core Base System

- **base.h/base.c**: Foundation class providing `AFC` base object, error codes, magic numbers, and common macros
- **exceptions.h**: Error handling definitions
- **mem_tracker.h/mem_tracker.c**: Memory leak detection and tracking

### String Handling

- **string.h/string.c**: Dynamic string class with bounds checking, JavaScript String compatibility
- **string_list.h/string_list.c**: List of strings (specialized Node Master)

### Data Structures

- **array.h/array.c**: Dynamic arrays with custom sorting
- **list.h/list.c**: Generic linked list (Node Master) with sorting
- **hash.h/hash.c**: Hash table foundation with dichotomous search
- **dictionary.h/dictionary.c**: Hash-based key-value store (like Python dicts)
- **btree.h/btree.c**: B-tree implementation
- **bin_tree.h/bin_tree.c**: Binary tree
- **avl_tree.h/avl_tree.c**: Self-balancing AVL tree
- **tree.h/tree.c**: Generic tree structure
- **circular_list.h/circular_list.c**: Circular linked list

### Networking (Linux only)

- **inet_client.h/inet_client.c**: TCP/IP client foundation with SSL/TLS support (OpenSSL)
- **inet_server.h/inet_server.c**: TCP/IP server foundation
- **pop3.h/pop3.c**: POP3 client implementation
- **smtp.h/smtp.c**: SMTP client with TLS/SSL support (STARTTLS and direct SSL), AUTH PLAIN/LOGIN
- **ftp_client.h/ftp_client.c**: FTP client
- **http_client.h/http_client.c**: HTTP client

### Database (Linux only)

- **dbi_manager.h/dbi_manager.c**: Database abstraction layer
- **dbi/mysql.c**: MySQL driver (builds as `mysql.so`)
- **dbi/postgresql.c**: PostgreSQL driver (builds as `postgresql.so`)

DBI drivers are dynamic plugins loaded at runtime.

### Web Development

- **cgi_manager.h/cgi_manager.c**: CGI application helper (GET/POST/cookies)

### File System

- **fileops.h/fileops.c**: File operations (copy, delete, rename, chmod, chown)
- **dirmaster.h/dirmaster.c**: Directory scanning and file info (Linux only)

### Utilities

- **readargs.h/readargs.c**: String parsing with patterns/templates
- **regexp.h/regexp.c**: Regular expressions with sed-like replace
- **cmd_parser.h/cmd_parser.c**: Scheme-like custom language parser (Linux only)
- **date_handler.h/date_handler.c**: Date validation and arithmetic
- **md5.h/md5.c**: MD5 hashing
- **base64.h/base64.c**: Base64 encoding/decoding
- **threader.h/threader.c**: Multi-threading support (Linux only)

### Plugin System (Linux only)

- **dynamic_class.h/dynamic_class.c**: Plugin client interface
- **dynamic_class_master.h/dynamic_class_master.c**: Plugin host/loader

### PCRE Integration

`src/pcre/` contains embedded Perl-Compatible Regular Expression library used by regexp class.

## Code Structure

### Header Inclusion

All AFC classes are accessible via:
```c
#include <afc/afc.h>
```

For C++:
```c
#include <afc/afcpp.h>
```

### API Patterns

- **Object creation**: `afc_<class>_new()`
- **Object deletion**: `afc_<class>_delete(<obj>)`
- **Operations**: `afc_<class>_<action>(<obj>, ...)`
- **Tag-based calls**: Many functions accept `TAG_END` terminated varargs

### Internal/Private Functions

Internal functions start with `_` prefix (e.g., `_afc_string_internal_resize()`).

### Test Class Pattern

Many `.c` files have embedded test code:
```c
#ifdef TEST_CLASS
int main() {
    // Test code here
}
#endif
```

Build tests using `-DTEST_CLASS` flag.

## Important Conventions

- **Magic numbers**: Each class validates objects using magic numbers (e.g., `AFC_MAGIC`)
- **Error handling**: Functions return error codes from `AFC_ERR_*` enum
- **Memory management**: Use `afc_malloc()`, `afc_free()` wrappers for tracking
- **Platform differences**: Use `#ifndef MINGW` for Linux-only features
- **Constants**: Use UPPERCASE with underscores (e.g., `AFC_TAG_END`)

## Dependencies

### OpenSSL

AFC now requires OpenSSL for SSL/TLS support in networking classes (inet_client, smtp). Install development headers:

```bash
# Debian/Ubuntu
sudo apt-get install libssl-dev

# Red Hat/Fedora
sudo yum install openssl-devel
```

The library links against `-lssl -lcrypto`.

## Using SMTP Class

The SMTP class provides email sending with Amazon SES and other SMTP servers.

### Basic Usage

```c
#include <afc/afc.h>

AFC *afc = afc_new();
SMTP *smtp = afc_smtp_new();

// Configure for Amazon SES with STARTTLS (port 587)
afc_smtp_set_tags(smtp,
    AFC_SMTP_TAG_HOST, "email-smtp.us-east-1.amazonaws.com",
    AFC_SMTP_TAG_PORT, "587",
    AFC_SMTP_TAG_USERNAME, "your-smtp-username",
    AFC_SMTP_TAG_PASSWORD, "your-smtp-password",
    AFC_SMTP_TAG_USE_TLS, (void *)TRUE,
    AFC_SMTP_TAG_AUTH_METHOD, (void *)AFC_SMTP_AUTH_LOGIN,
    AFC_TAG_END);

// Connect and authenticate
afc_smtp_connect(smtp);
afc_smtp_authenticate(smtp);

// Send email
afc_smtp_send_simple(smtp,
    "sender@example.com",
    "recipient@example.com",
    "Test Subject",
    "Email body content");

// Clean up
afc_smtp_quit(smtp);
afc_smtp_delete(smtp);
afc_delete(afc);
```

### SSL/TLS Options

- **STARTTLS (port 587)**: Set `AFC_SMTP_TAG_USE_TLS` to `TRUE` - upgrades plain connection to TLS
- **Direct SSL (port 465)**: Set `AFC_SMTP_TAG_USE_SSL` to `TRUE` - SSL from connection start
- **No encryption (port 25)**: Don't set either flag (not recommended for production)

### Authentication Methods

- `AFC_SMTP_AUTH_PLAIN`: AUTH PLAIN mechanism (Amazon SES compatible)
- `AFC_SMTP_AUTH_LOGIN`: AUTH LOGIN mechanism (Amazon SES compatible)
- `AFC_SMTP_AUTH_NONE`: No authentication (local/trusted servers only)

## Using afc-config

Helper script for compilation:
```bash
# Get compiler flags
afc-config --cflags

# Get linker flags
afc-config --libs

# In Makefile
CFLAGS=`afc-config --cflags`
LIBS=`afc-config --libs`
```

## License

AFC is released under GNU LGPL v2.1. Commercial licenses available from the author.
