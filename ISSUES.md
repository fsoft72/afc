# AFC Security Issues

**Analysis Date:** 2026-02-27
**Scope:** Full codebase security audit of `src/` directory

---

## Critical

### 1. Missing SSL/TLS Certificate Validation

- **File:** `src/inet_client.c` (lines 539-587)
- **Type:** Man-in-the-Middle (MITM)
- **Description:** The SSL implementation never calls `SSL_CTX_set_verify()` or checks `SSL_get_verify_result()`. All TLS connections accept any certificate, making every encrypted connection vulnerable to interception.
- **Fix:** Call `SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL)` and verify `SSL_get_verify_result()` returns `X509_V_OK` after handshake.

### 2. CRLF Injection in SMTP Commands

- **File:** `src/smtp.c` (lines 690, 707, 775-784)
- **Type:** Protocol Command Injection
- **Description:** Email addresses and message content are interpolated directly into SMTP commands without sanitizing `\r\n` sequences. An attacker can inject arbitrary SMTP commands via crafted email addresses (e.g., `user@example.com\r\nRCPT TO:<attacker@evil.com>`).
- **Fix:** Strip or reject `\r` and `\n` characters from all user-supplied values before inserting them into protocol commands.

### 3. CRLF Injection in FTP Commands

- **File:** `src/ftp_client.c` (lines 178, 258, 598, 616)
- **Type:** Protocol Command Injection
- **Description:** Filenames, paths, usernames, and passwords are interpolated into FTP commands without CRLF filtering, enabling arbitrary FTP command injection.
- **Fix:** Strip or reject `\r` and `\n` characters from all parameters before inserting them into FTP commands.

### 4. CRLF Injection in POP3 Commands

- **File:** `src/pop3.c` (lines 272-278)
- **Type:** Protocol Command Injection
- **Description:** Login credentials are directly interpolated into POP3 `USER` and `PASS` commands without filtering, enabling protocol command injection.
- **Fix:** Strip or reject `\r` and `\n` characters from credentials before use.

### 5. Plaintext Credential Storage Without Secure Erasure

- **Files:** `src/smtp.c` (lines 79-80), `src/pop3.c` (line 69)
- **Type:** Credential Exposure
- **Description:** Passwords are stored as plaintext in structs and freed without secure erasure (`memset`/`explicit_bzero`). Credentials are recoverable from process memory or core dumps.
- **Fix:** Use `explicit_bzero()` on password buffers before freeing them.

### 6. Arbitrary Code Execution via Plugin Loading

- **Files:** `src/dynamic_class_master.c` (line 236), `src/dbi_manager.c` (line 208)
- **Type:** DLL/SO Injection, Remote Code Execution
- **Description:** `dlopen()` is called with `RTLD_GLOBAL` on user-influenced paths. The DBI manager constructs paths with `sprintf(buf, "%s/%s", modules_path, library_name)` from controllable inputs, enabling loading of arbitrary shared objects.
- **Fix:** Validate and canonicalize paths before `dlopen()`, remove `RTLD_GLOBAL` flag, restrict allowed plugin directories.

### 7. XSS in CGI Debug Output

- **File:** `src/cgi_manager.c` (lines 794-799)
- **Type:** Cross-Site Scripting (XSS)
- **Description:** User-controlled dictionary keys and values are written directly to HTML output via `printf()` without any HTML encoding in `afc_cgi_manager_internal_dump()`.
- **Fix:** HTML-encode all user data (`<`, `>`, `&`, `"`, `'`) before writing to HTML output.

### 8. Buffer Overflow in UTF-8 Conversion

- **File:** `src/string.c` (lines 1280-1323)
- **Type:** Buffer Overflow
- **Description:** `afc_string_utf8_to_latin1()` allocates `strlen(utf8) + 20` bytes but has no bounds checking in the conversion loop. The write index `xpos` can exceed the allocated buffer size.
- **Fix:** Track allocated size and check bounds before each write in the conversion loop.

### 9. Memory Corruption via Unchecked readlink()

- **File:** `src/dirmaster.c` (lines 867-868)
- **Type:** Buffer Overflow / Memory Corruption
- **Description:** `readlink()` returns -1 on error but the return value is never checked. When -1 is used as an array index (`tmpbuf[-1] = 0`), it causes an out-of-bounds write. This is followed by unbounded `strcat()` on the corrupted buffer.
- **Fix:** Check `readlink()` return value for -1 before using it as an index.

### 10. HTTP Response Splitting via Content-Type

- **File:** `src/cgi_manager.c` (line 751)
- **Type:** HTTP Response Splitting
- **Description:** The `content_type` field is injected into HTTP response headers without CRLF sanitization, enabling response splitting and cache poisoning.
- **Fix:** Strip `\r` and `\n` from all values inserted into HTTP headers.

### 11. Library Code Calls exit()

- **File:** `src/base64.c` (lines 240, 396, 407, 479)
- **Type:** Denial of Service
- **Description:** Library code calls `exit()` on file errors, terminating the entire host application. A library must never call `exit()`.
- **Fix:** Return error codes (`AFC_BASE64_ERR_*`) and let the caller decide how to handle the error.

### 12. Heap Buffer Overflow in InetClient Response

- **File:** `src/inet_client.c` (line 351)
- **Type:** Heap Buffer Overflow
- **Description:** After `recv()` or `SSL_read()` returns `bytes`, the code writes `ic->buf[bytes] = '\0'`. If `bytes` equals `afc_string_max(ic->buf)`, this writes beyond the allocated buffer.
- **Fix:** Ensure `recv`/`SSL_read` reads at most `max - 1` bytes, or allocate buffer with +1 size.

### 13. Unchecked realloc() Loses Original Pointer

- **File:** `src/mem_tracker.c` (lines 230, 237)
- **Type:** Memory Corruption / Memory Leak
- **Description:** `realloc()` return value is assigned directly to `mt->data`. If `realloc()` fails and returns NULL, the original pointer is lost, causing a memory leak and subsequent NULL dereference.
- **Fix:** Store return value in a temporary variable and check for NULL before assignment.

---

## High

### 14. Unbounded sprintf() in Date Handler

- **File:** `src/date_handler.c` (lines 429-456)
- **Type:** Buffer Overflow
- **Description:** `afc_date_handler_to_string()` uses `sprintf()` into a caller-provided buffer with no size parameter. Long locale-specific day/month names can overflow the destination buffer.
- **Fix:** Accept a buffer size parameter and use `snprintf()`.

### 15. Integer Overflow in String Resize

- **File:** `src/string.c` (lines 1142-1144)
- **Type:** Integer Overflow / Undersized Allocation
- **Description:** `afc_string_resize_copy()` doubles the buffer with `max * 2`, which can wrap around to a small value near `UINT_MAX`, causing an undersized allocation followed by overflow.
- **Fix:** Check for overflow before doubling: `if (max > UINT_MAX / 2) return error;`.

### 16. Unvalidated Content-Length in CGI POST Handling

- **File:** `src/cgi_manager.c` (lines 893-906)
- **Type:** Integer Handling / Buffer Overflow / DoS
- **Description:** `atoi()` on HTTP `Content-Length` returns 0 on invalid input, does not detect negative values or overflow, and has no maximum size limit. A crafted `CONTENT_LENGTH` header can cause buffer overflow or memory exhaustion.
- **Fix:** Replace `atoi()` with `strtol()` with full error checking, validate range, and enforce a maximum POST size.

### 17. Fixed Buffer Overflow in File Operations

- **File:** `src/fileops.c` (lines 890-916)
- **Type:** Buffer Overflow
- **Description:** Fixed 4096-byte buffer used with `sprintf("%s/%s", dest_path, dirname)`. Paths can exceed this size, especially via symlinks.
- **Fix:** Use dynamic allocation or `snprintf()` with `PATH_MAX`.

### 18. Unbounded strcat() in Symlink Handling

- **File:** `src/dirmaster.c` (lines 870-871)
- **Type:** Buffer Overflow
- **Description:** `strcat(info->name, tmpbuf)` appends symlink targets (up to `PATH_MAX` bytes) to a fixed-size name field without bounds checking.
- **Fix:** Use `strncat()` or dynamically sized buffers with length tracking.

### 19. Stack Buffer Overflow in DBI Module Path

- **File:** `src/dbi_manager.c` (lines 191, 208)
- **Type:** Stack Buffer Overflow
- **Description:** `sprintf(buf[1024], "%s/%s", modules_path, library_name)` with no bounds checking. Long paths overflow the stack buffer.
- **Fix:** Use `snprintf(buf, sizeof(buf), ...)` and validate total path length.

### 20. TOCTOU Race Condition in Symlink Handling

- **File:** `src/dirmaster.c` (lines 861-867)
- **Type:** Time-of-Check-Time-of-Use (TOCTOU)
- **Description:** `lstat()` checks if a file is a symlink, then `readlink()` is called later. An attacker can replace the symlink target between these calls, enabling arbitrary file reads.
- **Fix:** Use `readlinkat()` with `O_NOFOLLOW` or `fstatat()` to avoid the race window.

### 21. Unbounded sprintf() in Date Formatting

- **File:** `src/dirmaster.c` (lines 949-970)
- **Type:** Buffer Overflow
- **Description:** Multiple unbounded `sprintf()` calls for date formatting, plus `strcpy(str, "#undefined")` in the default case. No buffer size is passed or checked.
- **Fix:** Accept buffer size parameter and use `snprintf()`.

### 22. Weak TLS Configuration

- **File:** `src/inet_client.c` (lines 539-587)
- **Type:** Weak Cryptography
- **Description:** No `SSL_CTX_set_cipher_list()` call and no minimum TLS version enforcement. OpenSSL defaults may include weak or deprecated algorithms.
- **Fix:** Set minimum TLS 1.2 with `SSL_CTX_set_min_proto_version()` and configure a strong cipher list.

### 23. Insecure AUTH PLAIN Credential Handling

- **File:** `src/smtp.c` (lines 432-523)
- **Type:** Credential Exposure
- **Description:** AUTH PLAIN constructs plaintext credentials in intermediate buffers that are not securely erased before being freed. Base64-encoded credentials also remain in memory.
- **Fix:** Use `explicit_bzero()` on all intermediate credential buffers before freeing.

### 24. Cookie Header Injection

- **File:** `src/cgi_manager.c` (lines 763-770)
- **Type:** HTTP Header Injection
- **Description:** Cookie domain and path attributes are set from unvalidated input. An attacker can inject CRLF sequences to manipulate headers or create additional Set-Cookie directives.
- **Fix:** Validate domain/path values and strip `\r\n` characters.

### 25. Race Condition in Server FD Set

- **File:** `src/inet_server.c` (lines 250-290)
- **Type:** Race Condition
- **Description:** No mutex protection on FD set modifications while iterating in `afc_inet_server_process()`. In multithreaded use, concurrent modification causes undefined behavior.
- **Fix:** Add mutex locking around FD set access and modification.

### 26. Socket File Descriptor Check Against Wrong Value

- **File:** `src/inet_client.c` (lines 257-258)
- **Type:** Resource Leak
- **Description:** Code checks `if (ic->sockfd)` but socket fd 0 is valid (stdin). Socket errors return -1, not 0. A valid socket with fd=0 will never be closed.
- **Fix:** Check `if (ic->sockfd >= 0)` or track socket state with a separate boolean flag.

### 27. Unsafe atoi() on Potentially NULL Pointers (POP3)

- **File:** `src/pop3.c` (lines 285-286)
- **Type:** NULL Pointer Dereference
- **Description:** `afc_string_list_item()` can return NULL, which is passed directly to `atoi()` without checking, causing undefined behavior.
- **Fix:** Check for NULL before calling `atoi()`, or use `strtol()` with error checking.

### 28. Race Condition in Hash Table Sorting

- **File:** `src/hash.c` (lines 253-254)
- **Type:** Race Condition
- **Description:** In multi-threaded environments, two threads can both see `is_sorted == FALSE` and both call sort simultaneously, causing data corruption.
- **Fix:** Add mutex synchronization around hash operations, or document that hash tables are not thread-safe.

### 29. Unsafe afc_string_max() on Non-AFC Strings

- **File:** `src/string.c` (lines 205-208)
- **Type:** Out-of-Bounds Read
- **Description:** `afc_string_max()` dereferences memory before the string pointer to read metadata. If called with a regular C string (not allocated via `afc_string_new()`), this reads uninitialized or invalid memory.
- **Fix:** Add magic number validation before dereferencing metadata.

### 30. strtok() Not Thread-Safe in SMTP

- **File:** `src/smtp.c` (lines 648-668)
- **Type:** Race Condition
- **Description:** `strtok()` uses static internal state and is not thread-safe. Multiple threads using SMTP simultaneously will corrupt each other's parsing state.
- **Fix:** Use `strtok_r()` (POSIX thread-safe version).

---

## Medium

### 31. Integer Overflow in Memory Tracker Realloc

- **File:** `src/mem_tracker.c` (lines 263-272)
- **Type:** Integer Overflow
- **Description:** `data_max * 2` can overflow when computing realloc size, causing an undersized allocation.
- **Fix:** Check for overflow before doubling: `if (data_max > SIZE_MAX / (2 * sizeof(MemTrackData *))) return error;`.

### 32. SSRF via FTP PASV Response

- **File:** `src/ftp_client.c` (lines 646-740)
- **Type:** Server-Side Request Forgery (SSRF)
- **Description:** PASV response IP parsing trusts the server-provided IP address. A malicious FTP server can direct the client to connect to arbitrary internal hosts.
- **Fix:** Validate that the PASV IP matches the original server IP or restrict to the same host.

### 33. Port Confusion in FTP PASV Parsing

- **File:** `src/ftp_client.c` (lines 714-730)
- **Type:** Integer Truncation
- **Description:** `atoi()` result is cast to `u_char`, silently truncating values above 255 and producing incorrect port numbers.
- **Fix:** Validate that parsed values are in the range 0-255 before casting.

### 34. SMTP Response Code Parsing Without Validation

- **File:** `src/smtp.c` (lines 312-335)
- **Type:** Protocol Parsing Error
- **Description:** Response code parsing assumes the first 3 characters are digits. Non-digit characters cause `atoi()` to return 0, indistinguishable from a real error.
- **Fix:** Validate that the first 3 bytes are ASCII digits before converting.

### 35. POP3 Unbounded Multi-line Response

- **File:** `src/pop3.c` (lines 453-474)
- **Type:** Denial of Service
- **Description:** Multi-line responses are read until a lone "." is encountered with no size limit. A malicious server can cause memory exhaustion by sending unlimited lines.
- **Fix:** Enforce a maximum response size limit.

### 36. PostgreSQL Connection String Never Used

- **File:** `src/dbi/postgresql.c` (lines 150-154)
- **Type:** Logic Bug / Potential Injection
- **Description:** The connection string is constructed with user parameters but never passed to `PQconnectdb("")` (empty string used instead). If fixed naively, the direct string interpolation is vulnerable to injection.
- **Fix:** Use `PQconnectdbParams()` with parameterized values instead of string interpolation.

### 37. No SSL Handshake Timeout

- **File:** `src/inet_client.c` (lines 203-209)
- **Type:** Denial of Service
- **Description:** Socket timeout only applies to socket operations, not to `SSL_connect()`. A slow or malicious server can cause indefinite hangs during TLS handshake.
- **Fix:** Set a timer or use non-blocking SSL with `select()`/`poll()` and a timeout.

### 38. Hardcoded EHLO Hostname

- **File:** `src/smtp.c` (line 404)
- **Type:** Protocol Non-compliance
- **Description:** SMTP uses hardcoded `EHLO localhost` instead of the actual FQDN, violating RFC 5321. Strict SMTP servers may reject the connection.
- **Fix:** Use the system's actual hostname or allow configuration via a tag.

### 39. Memory Leak in SMTP AUTH Error Paths

- **File:** `src/smtp.c` (lines 456-469)
- **Type:** Memory Leak
- **Description:** If `afc_base64_encode()` fails or memory allocation fails partway through `_afc_smtp_auth_plain()`, not all previously allocated memory is freed.
- **Fix:** Use consistent cleanup pattern, ensuring all error returns free allocated resources.

### 40. Integer Overflow in SMTP AUTH Buffer Size

- **File:** `src/smtp.c` (line 435)
- **Type:** Integer Overflow
- **Description:** `auth_len = 1 + username_len + 1 + password_len` can overflow if username or password lengths are near `SIZE_MAX`, resulting in an undersized buffer allocation.
- **Fix:** Add overflow checks: `if (username_len > SIZE_MAX - password_len - 2) return error;`.

### 41. Dichotomous Search Can Loop Infinitely

- **File:** `src/hash.c` (lines 260-277)
- **Type:** Denial of Service
- **Description:** If the array is corrupted or hash values are malformed, the binary search could enter an infinite loop. The termination condition relies on `min > max` which may not trigger if values wrap.
- **Fix:** Add an iteration counter limit.

### 42. Missing Magic Number Validation in Internal Functions

- **Files:** Multiple
- **Type:** Undefined Behavior
- **Description:** Many internal functions (prefixed with `_`) do not validate magic numbers before operating on structures. If called after memory corruption, they operate on invalid data.
- **Fix:** Add magic number checks in internal functions or document preconditions.

### 43. Insufficient Input Validation in HTTP Client

- **File:** `src/http_client.c`
- **Type:** Buffer Overflow / DoS
- **Description:** HTTP client does not validate URL length or components before parsing. A crafted URL could cause buffer overflows in internal parsing functions.
- **Fix:** Add URL length checks and validate components before copying to fixed-size buffers.

---

## Low

### 44. Memory Tracker Not Thread-Safe

- **File:** `src/mem_tracker.c`
- **Type:** Race Condition
- **Description:** Memory tracker maintains global state without mutex protection. In multi-threaded applications, this causes race conditions in tracking data.
- **Fix:** Add mutex protection to all memory tracker operations.

### 45. Base64 eol Buffer Not Declared const

- **File:** `src/base64.c` (line 22)
- **Type:** Defensive Programming
- **Description:** `static char eol[] = "\r\n"` is not declared `const`. While current code does not modify it, the pattern is unsafe against future changes.
- **Fix:** Declare as `static const char eol[] = "\r\n"`.

### 46. Inconsistent Error Handling Strategy

- **Files:** Multiple
- **Type:** API Design
- **Description:** Some functions return error codes, others return NULL, others call `exit()`. No consistent error handling strategy across the library.
- **Fix:** Standardize on error code returns and document error conditions for all public APIs.

### 47. Missing const Qualifiers on Read-Only Parameters

- **Files:** Multiple
- **Type:** API Design
- **Description:** Many functions that do not modify their parameters lack `const` qualifiers (e.g., `afc_string_comp()` should take `const char *`).
- **Fix:** Add `const` to all read-only parameters.

### 48. Resource Cleanup Gaps in Constructors

- **Files:** Multiple
- **Type:** Memory Leak
- **Description:** Some `_new()` functions do not properly clean up all resources on partial failure. If allocation of the Nth member fails, previously allocated members may not be freed.
- **Fix:** Use consistent cleanup pattern in all constructors.

### 49. Platform Detection at Compile Time May Not Match Runtime

- **File:** `src/string.c` (lines 39-43)
- **Type:** Portability
- **Description:** Path separator is set at compile time via `#ifdef MINGW`. This may not match the runtime environment (e.g., Cygwin on Windows).
- **Fix:** Detect path separator at runtime or provide a configuration option.

---

## Summary

| Severity | Count |
|----------|-------|
| Critical | 13    |
| High     | 17    |
| Medium   | 13    |
| Low      | 6     |
| **Total** | **49** |

---

## Recommendations

### Immediate Priority

1. **Replace all `sprintf()`, `strcpy()`, `strcat()`** with bounded variants (`snprintf()`, `strncpy()`, `strncat()`) throughout the entire codebase.
2. **Add SSL certificate verification** in `inet_client.c`.
3. **Sanitize all protocol inputs** by stripping `\r\n` from user-controlled data before inserting into SMTP, FTP, POP3, and HTTP commands.
4. **Remove all `exit()` calls** from library code and return error codes instead.

### High Priority

5. **Validate all external input** — replace `atoi()` with `strtol()` with error checking, add bounds validation on Content-Length, paths, and numeric parameters.
6. **Secure credential handling** — use `explicit_bzero()` on all password and credential buffers before freeing.
7. **HTML-encode CGI output** — escape `<`, `>`, `&`, `"`, `'` in all user data written to HTML.
8. **Check all return values** — especially `readlink()`, `malloc()`, and `realloc()`.

### Medium Priority

9. **Restrict plugin paths** — validate and canonicalize paths before `dlopen()`, remove `RTLD_GLOBAL` flag.
10. **Fix the PostgreSQL driver** — use `PQconnectdbParams()` instead of string interpolation.
11. **Add size limits** to all unbounded read loops (POP3 multi-line, CGI POST body).
12. **Set minimum TLS 1.2** and configure strong cipher suites.
13. **Add mutex protection** to shared state in `inet_server.c`, `hash.c`, and `mem_tracker.c`.

### Testing Recommendations

1. **Fuzzing:** Use AFL or libFuzzer on network protocol parsers (SMTP, POP3, FTP, HTTP).
2. **Static Analysis:** Run Coverity, Clang Static Analyzer, or Cppcheck.
3. **Dynamic Analysis:** Use AddressSanitizer, MemorySanitizer, UndefinedBehaviorSanitizer.
4. **Thread Safety:** Use ThreadSanitizer to detect race conditions.
5. **Valgrind:** Run all tests under Valgrind to detect memory errors.
