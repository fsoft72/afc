# CHANGES.md

## January 15, 2026

### Security and Correctness Fixes

Fixed multiple security and correctness issues identified in code analysis (see ISSUES.md).

#### Critical Fixes

**base64.c - Removed exit() calls**
- Library code should never terminate the host application
- Replaced `exit(1)` with proper error returns
- Added new error codes: `AFC_BASE64_ERR_READ_ERROR`, `AFC_BASE64_ERR_WRITE_ERROR`, `AFC_BASE64_ERR_INCOMPLETE_INPUT`, `AFC_BASE64_ERR_ILLEGAL_CHAR`
- Added `const` qualifier to static `eol` variable
- Updated `afc_base64_internal_write()` to accept `const void*`

**inet_client.c - Buffer overflow and socket fd handling**
- Fixed buffer overflow: now reserves 1 byte for null terminator when reading from socket/SSL
- Initialize `sockfd` to -1 in constructor instead of leaving undefined
- Check `sockfd >= 0` instead of `sockfd != 0` (fd 0 is valid stdin)
- Reset `sockfd` to -1 and `fd` to NULL on close

**mem_tracker.c - Memory safety improvements**
- Check `realloc()` return value before using new pointer
- Use temp variable to preserve original pointer on failure
- Fix malloc size: use `sizeof(MemTrackData*)` not `sizeof(MemTrackData)`
- Fix memset size: multiply by element size
- Add NULL checks for all malloc/realloc calls
- Initialize all struct fields in constructor

#### High Priority Fixes

**smtp.c - Security and thread safety**
- Use `snprintf()` instead of `strncpy()` for response code extraction
- Use `strtok_r()` instead of `strtok()` for thread-safe recipient parsing
- Add integer overflow check for auth credential buffer size
- Use `memcpy()` instead of `strcpy()` for safer auth string building
- Add NULL checks for all memory allocations
- Fix memory leaks in error paths in `_afc_smtp_auth_plain()`
- Add defensive NULL checks for username/password

**pop3.c - NULL pointer safety**
- Check return value of `afc_string_list_item()` before passing to `atoi()`

**array.c - NULL pointer safety**
- Add NULL pointer check for array in `afc_array_item()`

---

### New Documentation

#### Comprehensive AFC Library Documentation for LLMs
Added complete API documentation designed for LLMs to understand and use all AFC library features.

**Files created:**
- `ai/afc.md` - Comprehensive API reference documentation

**Documentation covers:**
- **Core System**: AFC base object, memory management, exception handling
- **String Handling**: String class with JS-like API, StringList class
- **Data Structures**: Array, List, Hash, Dictionary, BinTree, AVLTree, BTree, Tree, CircularList
- **Networking (Linux)**: InetClient/InetServer, SMTP, POP3, HTTP, FTP clients
- **Utilities**: MD5, Base64, DateHandler, RegExp, FileOperations, DirMaster
- **Web Development**: CGIManager for CGI applications
- **Database (Linux)**: DBIManager with MySQL/PostgreSQL drivers
- **Plugin System (Linux)**: DynamicClass and DynamicClassMaster
- **Threading (Linux)**: Threader with mutex support

**Features:**
- Complete function signatures and usage examples
- Tag-based configuration reference for all classes
- Error codes and handling patterns
- Platform availability notes (Linux vs MinGW)
- Best practices for memory management
- Complete working examples

---

## December 4, 2025

### Bug Fixes

#### SMTP Client - Fixed Response Handling and Command Formatting
Fixed critical bugs in the SMTP client implementation that prevented it from working correctly.

**Files modified:**
- `src/smtp.c` - Fixed buffer handling and command formatting

**Bugs fixed:**
1. **Response buffer handling**: `_afc_smtp_get_response()` was not copying data from `InetClient->buf` to `SMTP->buf`, causing empty responses to be processed
2. **Buffer aliasing in EHLO commands**: Commands were using `smtp->tmp` as both source and destination, causing "bad syntax" errors
3. **Buffer aliasing in AUTH PLAIN**: Authentication command was reusing `smtp->tmp`, causing authentication failures
4. **Buffer aliasing in MAIL FROM/RCPT TO**: Send commands were reusing `smtp->tmp`, causing send failures

**Technical details:**
- Added `afc_string_copy(smtp->buf, smtp->ic->buf, ALL)` after `afc_inet_client_get()` to properly capture server responses
- Changed EHLO commands to pass string literals directly to `_afc_smtp_send_command()`
- Fixed `_afc_smtp_auth_plain()` to use a separate buffer for AUTH PLAIN command
- Fixed `afc_smtp_send()` to use separate buffers for MAIL FROM and RCPT TO commands

**Root cause:** `_afc_smtp_send_command()` internally uses `smtp->tmp` to format commands with CRLF. Any code that pre-formatted commands in `smtp->tmp` and then passed it as a parameter caused the same buffer to be used as both source and destination in `afc_string_make()`, resulting in corrupted commands.

These fixes enable the SMTP client to successfully:
- Connect to SMTP servers
- Perform STARTTLS negotiation (port 587)
- Authenticate with AUTH PLAIN and AUTH LOGIN
- Send MAIL FROM and RCPT TO commands

Testing verified with Amazon SES on port 587 with STARTTLS.

### New Features

#### SMTP Test Example with Settings File
Added a working SMTP test example in `src/test_area/smtp/` that demonstrates sending emails using configuration from a settings file.

**Files created:**
- `src/test_area/smtp/test_01.c` - SMTP test program with settings file parser
- `src/test_area/smtp/Makefile` - Build configuration
- `src/test_area/smtp/settings.txt.example` - Configuration template
- `src/test_area/smtp/README.md` - Usage documentation

**Bug fixes in test program:**
- Fixed settings parser to use `afc_string_dup()` for dictionary values (was storing pointers to reused buffer)
- Added cleanup function `_cleanup_string()` to properly free duplicated strings

**Features:**
- **Settings file parsing**: Reads SMTP configuration from `settings.txt`
- **Full SMTP workflow**: Connect, authenticate, send email, and quit
- **Comprehensive error handling**: Validates all required settings
- **Multiple authentication methods**: AUTH PLAIN, AUTH LOGIN, and no auth
- **TLS/SSL support**: STARTTLS (port 587) and direct SSL (port 465)
- **Configuration display**: Shows all settings before connecting
- **Example configurations**: Amazon SES, Gmail, and generic SMTP

**Usage:**
```bash
cd src/test_area/smtp
cp settings.txt.example settings.txt
# Edit settings.txt with your SMTP credentials
make
./test_01
```

The test sends a simple email verifying the SMTP client functionality with your configured provider.

#### Enhanced HTTP Client Class
Completely overhauled the existing HTTP client with full HTTP/1.1 support, including all major HTTP methods, automatic redirect following, SSL/TLS support, response parsing, and configurable timeouts.

**Files modified:**
- `src/http_client.h` - Enhanced with complete HTTP client API
- `src/http_client.c` - Completely rewritten with full HTTP protocol support

**Features:**
- **Full HTTP method support**: GET, POST, PUT, DELETE, PATCH, HEAD, OPTIONS
- **Request handling**:
  - Custom headers via Dictionary
  - Request body support with automatic Content-Length
  - HTTP/1.1 protocol with proper Host header
- **Response handling**:
  - Status code and message parsing
  - Response headers parsing (into Dictionary)
  - Response body capture with multiple transfer modes
  - Chunked transfer encoding support
  - Content-Length based reading
- **Automatic redirect following**:
  - Configurable via `AFC_HTTP_CLIENT_TAG_FOLLOW_REDIRECTS`
  - Configurable maximum redirects (default: 10)
  - Proper HTTP semantics (301/302/303 convert to GET)
  - 307/308 preserve original method
- **SSL/TLS support**: Both HTTP and HTTPS via InetClient's OpenSSL integration
- **URL parsing**: Automatic protocol, host, port, and path extraction
- **Timeout configuration**: Per-request timeout settings
- **Connection reuse**: Automatic connection management and reconnection when needed
- **Tag-based configuration**: Following AFC conventions

**Usage examples:**

Simple GET request:
```c
HttpClient *hc = afc_http_client_new();
afc_http_client_get(hc, "http://example.com/api/data");
printf("Status: %d\n", afc_http_client_get_status_code(hc));
printf("Body: %s\n", afc_http_client_get_response_body(hc));
afc_http_client_delete(hc);
```

POST request with JSON:
```c
HttpClient *hc = afc_http_client_new();
afc_http_client_set_header(hc, "Content-Type", "application/json");
const char *json = "{\"key\":\"value\"}";
afc_http_client_post(hc, "https://api.example.com/data", json, strlen(json));
afc_http_client_delete(hc);
```

With custom configuration:
```c
HttpClient *hc = afc_http_client_new();
afc_http_client_set_tags(hc,
    AFC_HTTP_CLIENT_TAG_TIMEOUT, (void *)30,
    AFC_HTTP_CLIENT_TAG_FOLLOW_REDIRECTS, (void *)TRUE,
    AFC_HTTP_CLIENT_TAG_MAX_REDIRECTS, (void *)5,
    AFC_TAG_END);
afc_http_client_set_header(hc, "User-Agent", "MyApp/1.0");
afc_http_client_get(hc, "http://example.com/");
afc_http_client_delete(hc);
```

**API Summary:**
- Configuration: `afc_http_client_set_tags()`, `afc_http_client_set_header()`
- HTTP methods: `afc_http_client_get()`, `afc_http_client_post()`, `afc_http_client_put()`, `afc_http_client_patch()`, `afc_http_client_delete_url()`, `afc_http_client_head()`, `afc_http_client_options()`
- Generic request: `afc_http_client_request()`
- Response access: `afc_http_client_get_status_code()`, `afc_http_client_get_status_message()`, `afc_http_client_get_response_body()`, `afc_http_client_get_response_headers()`, `afc_http_client_get_response_header()`

#### Timeout Support in InetClient
Added configurable timeout support to the base networking class for connection and I/O operations.

**Files modified:**
- `src/inet_client.h` - Added timeout field and `AFC_INET_CLIENT_TAG_TIMEOUT` tag
- `src/inet_client.c` - Implemented timeout via `setsockopt()` with `SO_RCVTIMEO` and `SO_SNDTIMEO`

**Features:**
- Per-connection timeout configuration
- Applies to both send and receive operations
- Timeout in seconds (0 = no timeout, default)
- Configurable via `afc_inet_client_set_tags()`

**Usage:**
```c
InetClient *ic = afc_inet_client_new();
afc_inet_client_set_tags(ic, AFC_INET_CLIENT_TAG_TIMEOUT, (void *)10, AFC_TAG_END);
afc_inet_client_open(ic, "example.com", 80);
```

#### SMTP Client Class
Added complete SMTP client implementation with SSL/TLS support for sending emails through services like Amazon SES.

**Files added:**
- `src/smtp.h` - SMTP client header with API definitions
- `src/smtp.c` - SMTP client implementation

**Features:**
- Full SMTP protocol implementation (HELO/EHLO, MAIL FROM, RCPT TO, DATA, QUIT)
- SSL/TLS support via two modes:
  - STARTTLS (port 587) - upgrades plain connection to TLS
  - Direct SSL (port 465) - SSL from connection start
- Authentication methods:
  - AUTH PLAIN (Amazon SES compatible)
  - AUTH LOGIN (Amazon SES compatible)
- High-level API: `afc_smtp_send_simple()` for easy email sending
- Low-level API: `afc_smtp_send()` for custom message formatting
- Tag-based configuration following AFC conventions
- Multiple recipient support (comma-separated)

**Usage example:**
```c
SMTP *smtp = afc_smtp_new();
afc_smtp_set_tags(smtp,
    AFC_SMTP_TAG_HOST, "email-smtp.us-east-1.amazonaws.com",
    AFC_SMTP_TAG_PORT, "587",
    AFC_SMTP_TAG_USERNAME, "your-username",
    AFC_SMTP_TAG_PASSWORD, "your-password",
    AFC_SMTP_TAG_USE_TLS, (void *)TRUE,
    AFC_SMTP_TAG_AUTH_METHOD, (void *)AFC_SMTP_AUTH_LOGIN,
    AFC_TAG_END);
afc_smtp_connect(smtp);
afc_smtp_authenticate(smtp);
afc_smtp_send_simple(smtp, "from@example.com", "to@example.com",
                     "Subject", "Body");
afc_smtp_quit(smtp);
afc_smtp_delete(smtp);
```

#### SSL/TLS Support in InetClient
Enhanced the base networking class with OpenSSL support.

**Files modified:**
- `src/inet_client.h` - Added SSL/TLS structures, error codes, and function prototypes
- `src/inet_client.c` - Implemented SSL/TLS functionality

**New functions:**
- `afc_inet_client_enable_ssl()` - Enable direct SSL connection (port 465)
- `afc_inet_client_start_tls()` - Upgrade to TLS via STARTTLS (port 587)
- `afc_inet_client_set_tags()` - Tag-based configuration
- `afc_inet_client_set_tag()` - Set individual configuration tags

**New error codes:**
- `AFC_INET_CLIENT_ERR_SSL_INIT` - SSL initialization failed
- `AFC_INET_CLIENT_ERR_SSL_CONNECT` - SSL handshake failed
- `AFC_INET_CLIENT_ERR_SSL_READ` - SSL read operation failed
- `AFC_INET_CLIENT_ERR_SSL_WRITE` - SSL write operation failed

**New tags:**
- `AFC_INET_CLIENT_TAG_USE_SSL` - Enable/disable SSL

**Implementation details:**
- Modified `afc_inet_client_get()` to use `SSL_read()` when SSL is active
- Modified `afc_inet_client_send()` to use `SSL_write()` when SSL is active
- Modified `afc_inet_client_close()` to properly clean up SSL connections
- Uses OpenSSL's `TLS_client_method()` for protocol negotiation

### Build System Changes

**Files modified:**
- `src/Makefile` - Added OpenSSL linker flags (`-lssl -lcrypto`) and smtp.o to OBJS list
- `src/afc.h` - Added `#include "smtp.h"` for automatic inclusion

### Documentation

**Files modified:**
- `CLAUDE.md` - Added comprehensive SMTP usage guide and OpenSSL dependency documentation

**New sections:**
- Dependencies > OpenSSL - Installation instructions for development headers
- Using SMTP Class - Complete usage guide with examples
- SSL/TLS Options - Configuration guide for different encryption modes
- Authentication Methods - Supported SMTP AUTH mechanisms

### Dependencies

**New requirement:**
- OpenSSL development libraries (`libssl-dev` on Debian/Ubuntu, `openssl-devel` on Red Hat/Fedora)
- Library now links against `-lssl -lcrypto`

### Compatibility Notes

- OpenSSL SSL/TLS support is only available on Linux (not in MinGW builds)
- SMTP class is only available on Linux (not in MinGW builds)
- Maintains backward compatibility for existing code not using SSL features
- All existing InetClient functionality remains unchanged when SSL is not enabled

### Testing

- Compiled successfully with gcc on Linux
- InetClient SSL functions compile without errors
- SMTP class compiles with only minor sign-comparison warnings (cosmetic)
- Library builds successfully with `make` command

### Future Enhancements

Potential areas for expansion:
- SMTP MIME support for attachments
- HTML email support
- Additional authentication methods (CRAM-MD5, OAUTH2)
- Connection pooling/reuse
- Test suite for SMTP functionality
- Example programs demonstrating Amazon SES integration
