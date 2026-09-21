# Optimization TODO

> Generated on 2026-03-10. Items sorted by importance.
> Consolidates the still-open items from IMPROVEMENTS.md plus additional findings.

## Critical

- [ ] **Harden TLS: no certificate verification or SNI** — `afc_inet_client_enable_ssl()` creates the SSL context and connects but never enables certificate verification and never sets SNI, so HTTPS/SMTPS clients accept untrusted certificates and fail against virtual-hosted endpoints.
  - File(s): `src/inet_client.c` (around lines 656-718)

- [ ] **SIGPIPE can kill any process using the library** — `afc_inet_client_send()` calls `send()` without `MSG_NOSIGNAL` and nothing in the library ignores `SIGPIPE`, so writing to a closed socket terminates the host application.
  - File(s): `src/inet_client.c:437`

- [ ] **One-byte buffer overflow in inet_server receive path** — `recv()` fills the full AFC string capacity and then `data->buf[nbytes] = '\0'` writes one byte past the allocation when the buffer is completely full.
  - File(s): `src/inet_server.c:288,305`

- [ ] **Password printed in debug log** — a debug log statement includes the SMTP password value, leaking credentials into logs.
  - File(s): `src/smtp.c`

- [ ] **Broken index arithmetic in list stack-position cleanup** — the compaction loop mixes `t` and `i` (condition uses `t`, increment uses `i++`, assignment still indexes `t`), corrupting the stack-position cache when entries are removed.
  - File(s): `src/list.c:518-523`

- [ ] **Truncated multi-line SMTP responses** — `_afc_smtp_get_response()` reads a single chunk and parses only the first 3 digits, so multi-line EHLO replies are truncated and advertised capabilities (STARTTLS, AUTH methods) can be missed. Read until "3 digits + space" terminator.
  - File(s): `src/smtp.c:330-355`

## High

- [ ] **Replace `gethostbyname()` with `getaddrinfo()`** — the resolver path has no IPv6 support, poor thread-safety, and outdated behavior; iterate the `getaddrinfo()` results when connecting.
  - File(s): `src/inet_client.c:294-305`

- [ ] **Byte-by-byte copies on hot string paths** — `afc_string_copy()` and `afc_string_add()` copy one byte at a time; use `memcpy()` after length clamping.
  - File(s): `src/string.c:316-318, 1038-1040`

- [ ] **Repair the standalone test build** — `make` in `tests/` fails immediately because `tests/test_utils.h` does not exist; add the header or fix the Makefile.
  - File(s): `tests/Makefile:25`

- [ ] **Global base object prevents multi-context/thread use** — `__internal_afc_base` is a single global, so error state and logging are shared across all users and threads.
  - File(s): `src/base.c:128`

- [ ] **Allocation churn in case-insensitive comparators** — string_list and dirmaster sort comparators allocate, uppercase, compare, and free temporary strings per comparison; use `strcasecmp()` instead.
  - File(s): `src/string_list.c:1073-1089`, `src/dirmaster.c:1218-1233`

- [ ] **Unbounded response-header growth in HTTP client** — no maximum header size is enforced, allowing memory exhaustion from malicious or broken servers.
  - File(s): `src/http_client.c`

## Medium

- [ ] **MemTracker linear scan and strdup overhead** — `_memtrack_find()` linearly scans all tracked allocations and metadata duplicates compile-time strings with `strdup()`; use a pointer hash for O(1) lookup and store the literal pointers.
  - File(s): `src/mem_tracker.c:103-106, 255-271`

- [ ] **Base64 lookup tables rebuilt on every call** — both encode and decode rebuild their lookup tables per call; make them `static const` initialized once.
  - File(s): `src/base64.c:294-304, 372-383`

- [ ] **Fragmented HTTP request writes** — `_afc_http_client_send_request()` issues a separate `send()` per header; build the full request in memory and send in one or a few writes.
  - File(s): `src/http_client.c:911-964`

- [ ] **Missing `afc_inet_client_get_binary` referenced by ftp_client** — the referenced function is not implemented; implement it or remove the reference cleanly.
  - File(s): `src/ftp_client.c:992`

- [ ] **Duplicated build flags and includes** — `-O2` is added twice in the Makefile and `fileops.h` includes `stdio.h`/`errno.h` twice; also add `-MMD -MP` header dependency tracking.
  - File(s): `src/Makefile:24,37`, `src/fileops.h:23-26`

## Low / Nice to have

- [ ] **Broken documentation links in README** — README links to `ai/afc.md` and `ai/guidelines.md`, which do not exist in the checkout; restore the files or fix the links.
  - File(s): `README.md:248-249`

- [ ] **Suppressing useful warnings in CFLAGS** — `-Wno-unused-label -Wno-unused-parameter -Wno-unused-but-set-variable` hide real dead code; clean the code and re-enable the warnings.
  - File(s): `src/Makefile:24`

- [ ] **Stale TODO/FIXME comments** — a few long-standing TODO/FIXME notes remain (base64 error messages, cgi_manager null-key handling) that should be resolved or ticketed.
  - File(s): `src/base64.c:207`, `src/cgi_manager.c:1038,1052`
