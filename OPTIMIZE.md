# Optimization TODO

> Generated on 2026-03-10. Items sorted by importance.
> Consolidates the still-open items from IMPROVEMENTS.md plus additional findings.
> All items resolved as of 2026-03-10; notes mark items that were verified as
> already fixed in the codebase during this pass.

## Critical

- [x] **Harden TLS: no certificate verification or SNI** — fixed: `SSL_CTX_set_verify(SSL_VERIFY_PEER)` with default CA paths, SNI via `SSL_set_tlsext_host_name()`, hostname check via `SSL_set1_host()`, post-handshake `SSL_get_verify_result()` check, new `AFC_INET_CLIENT_ERR_SSL_VERIFY`. New test: `tests/test_inet_ssl.c`.
  - File(s): `src/inet_client.c`

- [x] **SIGPIPE can kill any process using the library** — fixed: `MSG_NOSIGNAL` on all `send()` calls. New test: `tests/test_sigpipe.c`.
  - File(s): `src/inet_client.c`, `src/inet_server.c:327`

- [x] **One-byte buffer overflow in inet_server receive path** — verified already fixed: `recv()` reads at most `afc_string_max(buf) - 1`, reserving space for the terminator.
  - File(s): `src/inet_server.c:288`

- [x] **Password printed in debug log** — verified already fixed: no password value appears in any log or printf in smtp.c.
  - File(s): `src/smtp.c`

- [x] **Broken index arithmetic in list stack-position cleanup** — verified already fixed: the compaction loop uses a single index and guards `sposcount`.
  - File(s): `src/list.c:518-523`

- [x] **Truncated multi-line SMTP responses** — verified already fixed: `_afc_smtp_get_response()` loops until the "NNN " final line.
  - File(s): `src/smtp.c:330-355`

## High

- [x] **Replace `gethostbyname()` with `getaddrinfo()`** — verified already fixed: `afc_inet_client_resolve()` uses `getaddrinfo()` with AF_UNSPEC and the connect path iterates results.
  - File(s): `src/inet_client.c`

- [x] **Byte-by-byte copies on hot string paths** — verified already fixed: `afc_string_copy()`/`afc_string_add()` use `memmove()`/`memcpy()`.
  - File(s): `src/string.c`

- [x] **Repair the standalone test build** — fixed: added missing `tests/run_all.sh` used by `make run`; repairing it surfaced (and we fixed) three real bugs: `afc_string_comp()` partial compare, inverted `afc_string_list_sort()` comparators, and a mem_tracker rehash bug after realloc.
  - File(s): `tests/run_all.sh`, `src/string.c`, `src/string_list.c`, `src/mem_tracker.c`

- [x] **Global base object prevents multi-context/thread use** — fixed within API compatibility: the base stays a global by design, but error state is now per-thread via `afc_str_error()` (thread-local) backing `AFC_STR_ERROR()`. New test: `tests/test_str_error.c`.
  - File(s): `src/base.c`, `src/base.h`

- [x] **Allocation churn in case-insensitive comparators** — verified already fixed: comparators use `strcasecmp()` directly.
  - File(s): `src/string_list.c`, `src/dirmaster.c`

- [x] **Unbounded response-header growth in HTTP client** — fixed: `AFC_HTTP_CLIENT_MAX_HEADERS` (100) and `AFC_HTTP_CLIENT_MAX_HEADER_SIZE` (64KB) limits, new `AFC_HTTP_CLIENT_ERR_HEADERS_TOO_LARGE`. New test: `tests/test_http_headers.c`.
  - File(s): `src/http_client.c`, `src/http_client.h`

## Medium

- [x] **MemTracker linear scan and strdup overhead** — verified already fixed: hash-based lookup (`_memtrack_hash_find`) is in place.
  - File(s): `src/mem_tracker.c`

- [x] **Base64 lookup tables rebuilt on every call** — verified already fixed: tables are `static const`.
  - File(s): `src/base64.c:26-35`

- [x] **Fragmented HTTP request writes** — verified already fixed: `_afc_http_client_send_request()` builds the full request in one buffer and sends it in a single write.
  - File(s): `src/http_client.c`

- [x] **Missing `afc_inet_client_get_binary` referenced by ftp_client** — fixed: implemented the function (SSL-aware), added `ftp_client.o` to the library and `test_ftp_client` to the suite.
  - File(s): `src/inet_client.c`, `src/ftp_client.c`, `src/Makefile`, `tests/Makefile`

- [x] **Duplicated build flags and includes** — verified already fixed: single `-O2`, no duplicate includes in fileops.h.
  - File(s): `src/Makefile`, `src/fileops.h`

## Low / Nice to have

- [x] **Broken documentation links in README** — fixed: README points at CLAUDE.md; CLAUDE.md no longer references the deleted `ai/` docs.
  - File(s): `README.md`, `CLAUDE.md`

- [x] **Suppressing useful warnings in CFLAGS** — fixed: `-Wno-unused-*` flags removed, all warning sites cleaned (`(void)` casts, dead variables removed, pcre third-party warnings suppressed in pcre/Makefile only). Library builds with zero warnings under `-Wall -Wextra`.
  - File(s): `src/Makefile`, `src/exceptions.h`, multiple `src/*.c`

- [x] **Stale TODO/FIXME comments** — fixed: `afc_base64_fwrite()` reports empty-buffer errors, cgi_manager documents the valueless-key behavior and errors on invalid mode.
  - File(s): `src/base64.c`, `src/cgi_manager.c`
