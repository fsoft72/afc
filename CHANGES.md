# CHANGES.md

## December 4, 2025

### New Features

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
