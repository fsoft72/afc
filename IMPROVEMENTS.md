# IMPROVEMENTS

This file captures concrete follow-up work found during a quick code review on 2026-03-09.

Baseline checked during the review:
- `make` in `src/` succeeds and produces the library cleanly.
- `make` in `tests/` fails immediately because `tests/test_utils.h` is missing.
- `CHANGES.md` already documents a large batch of fixes from 2026-03-04, so the list below focuses on remaining work rather than recently closed items.

## High-priority bugs and security fixes

### 1. Harden TLS in `src/inet_client.c`
- **Evidence:** `afc_inet_client_enable_ssl()` creates the TLS context and connects, but the code shown around `src/inet_client.c:656-718` never enables certificate verification and never sets SNI.
- **Impact:** HTTPS / SMTPS / POP3S clients can accept untrusted certificates, and some modern virtual-hosted TLS endpoints may fail without SNI.
- **Improve:** call `SSL_CTX_set_verify()`, load system CA roots, check the verification result after the handshake, and set the hostname with `SSL_set_tlsext_host_name()` before `SSL_connect()`.

### 2. Replace legacy DNS resolution
- **Evidence:** `src/inet_client.c:294-305` still uses `gethostbyname()`.
- **Impact:** no IPv6 support, poorer thread-safety, and outdated resolver behavior.
- **Improve:** migrate to `getaddrinfo()` and teach the client open/connect path to iterate returned addresses.

### 3. Stop logging cleartext passwords
- **Evidence:** `src/pop3.c:294` logs `Sending password: %s`; `src/ftp_client.c:647` logs the FTP password too.
- **Impact:** credentials can leak to stderr, logs, test output, and CI artifacts.
- **Improve:** remove the password value from debug output entirely or replace it with a redacted message.

### 4. Fix a likely one-byte overflow in `src/inet_server.c`
- **Evidence:** `recv()` reads up to `afc_string_max(data->buf)` bytes at `src/inet_server.c:288`, then `data->buf[nbytes] = '\0'` is written at `src/inet_server.c:305`.
- **Impact:** if the socket fills the whole AFC string buffer, the terminator is written one byte past the allocation.
- **Improve:** reserve one byte for the terminator before calling `recv()`, mirroring the safer pattern already used in `afc_inet_client_get()`.

### 5. Consume full multi-line SMTP responses
- **Evidence:** `_afc_smtp_get_response()` in `src/smtp.c:330-355` reads one chunk and parses only the first three digits.
- **Impact:** multi-line `EHLO` responses can be truncated, which can hide advertised capabilities such as `STARTTLS` or supported auth mechanisms.
- **Improve:** keep reading until the SMTP continuation marker ends (three digits followed by a space instead of `-`).

### 6. Fix the stack-position cleanup logic in `src/list.c`
- **Evidence:** `src/list.c:518-523` mixes `t` and `i` incorrectly in the inner loop:
  - the loop condition uses `t < nm->sposcount`
  - the increment uses `i++`
  - the assignment still indexes with `t`
- **Impact:** the stack-position cache can be corrupted, and the loop looks capable of running incorrectly when entries are removed.
- **Improve:** rewrite the compaction loop to use a single index variable and guard `nm->sposcount` before subtracting 1.

## Performance and scalability work

### 7. Replace byte-by-byte string copies with bulk copies
- **Evidence:** `afc_string_copy()` at `src/string.c:316-318` and `afc_string_add()` at `src/string.c:1038-1040` copy one byte at a time.
- **Impact:** these are hot-path helpers and pay unnecessary per-byte overhead.
- **Improve:** use `memcpy()` after the length has been clamped and validated.

### 8. Rework `MemTracker` lookup costs
- **Evidence:** `_memtrack_find()` in `src/mem_tracker.c:255-271` linearly scans every tracked allocation; allocation metadata also duplicates static strings with `strdup()` at `src/mem_tracker.c:103-106`.
- **Impact:** tracking overhead grows badly as allocation counts rise.
- **Improve:** keep a hash/index by pointer for O(1) lookup and store file/function pointers directly instead of duplicating compile-time string literals.

### 9. Remove allocation churn from case-insensitive sorts
- **Evidence:** `src/string_list.c:1073-1089` and `src/dirmaster.c:1218-1233` allocate temporary strings, uppercase them, compare them, and free them on every comparison.
- **Impact:** sorts create avoidable heap traffic and slow down badly on larger inputs.
- **Improve:** use a case-insensitive comparator (`strcasecmp()` or an AFC equivalent) instead of allocating temporary normalized copies.

### 10. Stop rebuilding Base64 lookup tables on every call
- **Evidence:** `src/base64.c:294-304` rebuilds the encode table, and `src/base64.c:372-383` rebuilds the decode table.
- **Impact:** repeated encode/decode calls do redundant work.
- **Improve:** make the lookup tables `static const` and initialize them once.

### 11. Batch HTTP request writes
- **Evidence:** `_afc_http_client_send_request()` in `src/http_client.c:911-964` sends the request line, each header, the blank line, and the body with separate `afc_inet_client_send()` calls.
- **Impact:** extra syscall overhead and more fragmented TCP writes.
- **Improve:** build the full request in memory when practical, then send it in one or a few larger writes.

## Build, test, and maintenance gaps

### 12. Repair the standalone test build
- **Evidence:** `tests/Makefile:25` depends on `test_utils.h`, but the `tests/` directory contains `test_utils.c` and no header file.
- **Impact:** `make` in `tests/` stops immediately with `No rule to make target 'test_utils.h'`.
- **Improve:** add the missing header, move the shared test helper into `tests/`, or update the Makefile to use the intended location.

### 13. Fix broken documentation links in `README.md`
- **Evidence:** `README.md:248-249` links to `ai/afc.md` and `ai/guidelines.md`, but there is no `ai/` directory in the repository root.
- **Impact:** the main README points contributors to docs that do not exist in the checkout.
- **Improve:** either restore those files or update the README to point at the real documentation location.

### 14. Clean up small build-system issues
- **Evidence:** `src/Makefile:24` and `src/Makefile:37` both add `-O2`; `src/fileops.h:23-26` includes `stdio.h` and `errno.h` twice.
- **Impact:** low-risk noise, but it adds maintenance friction and makes the build look less polished.
- **Improve:** remove duplicate flags/includes and add header dependency tracking (`-MMD -MP`) so header edits trigger recompilation reliably.

## Nice follow-up investigations

These are worth reviewing next, but I would handle them after the items above:

1. `src/base.c:128` uses a global `__internal_afc_base`, which limits thread isolation and makes the library harder to reason about in multi-context programs.
2. `src/http_client.c` should likely enforce a maximum response-header size to avoid unbounded memory growth on malicious or broken servers.
3. `src/ftp_client.c:992` still references `afc_inet_client_get_binary`, which matches the test/build note in `tests/Makefile` and should either be implemented or removed cleanly.
