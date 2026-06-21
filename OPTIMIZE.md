# Optimization TODO

> Generated on 2026-06-21. Items sorted by importance.

## Critical

- [ ] **Split string.c into smaller modules** — The 2211-line string.c is a god module that handles string creation, manipulation, conversion, pattern matching, and more. Split into `string_core.c` (create/copy/delete), `string_ops.c` (upper/lower/trim/concat), `string_search.c` (instr/index_of/replace), and `string_conv.c` (radix/utf8/hash).
  - File(s): `src/string.c`

- [ ] **Fix broken string functions** — Tests reveal several functions return wrong results: `afc_string_last_index_of()` returns -1 when it should find matches, `afc_string_pad_start/end()` don't pad correctly, `afc_string_trim_start/end()` return empty strings. These functions need debugging.
  - File(s): `src/string.c`

- [x] **Replace unsafe sprintf/strcpy/strcat with bounded variants** — Multiple files use `sprintf`, `strcpy`, and `strcat` without bounds checking. Use `snprintf`, `strncpy`, `strncat` or AFC's own `afc_string_make`/`afc_string_copy`/`afc_string_add` instead.
  - File(s): `src/list.c:1496`, `src/md5.c:283`, `src/regexp.c:200,207,579,580`, `src/cmd_parser.c:941`, `src/dirmaster.c:1111`, `src/fileops.c:871`

- [ ] **Add NULL pointer checks to all public API functions** — Many functions don't validate input pointers before dereferencing. For example, `afc_string_right()` calls `strlen(src)` without checking if `src` is NULL. Each public function should validate all pointer parameters at entry.
  - File(s): `src/string.c`, `src/array.c`, `src/list.c`, `src/dictionary.c`

- [ ] **Standardize error return conventions** — Some functions return NULL on error (string functions), others return int error codes (class functions), and some return both depending on context. Document and enforce a consistent convention per function category.
  - File(s): All source files

## High

- [ ] **Extract common new/delete/clear boilerplate into macros** — Every AFC class repeats the same pattern: allocate struct, set magic, allocate sub-objects, handle cleanup on failure. Create a macro like `AFC_CLASS_NEW(type, magic)` and `AFC_CLASS_DELETE(obj, magic)` to reduce this 20-30 line pattern to 2-3 lines.
  - File(s): `src/base.h`, all class `.c` files

- [ ] **Replace hardcoded buffer sizes with named constants** — Magic numbers like `1024`, `255`, `50`, `30` appear throughout for buffer sizes. Define constants like `AFC_BUF_SMALL`, `AFC_BUF_MEDIUM`, `AFC_BUF_LARGE` in `base.h`.
  - File(s): `src/smtp.c`, `src/http_client.c`, `src/cgi_manager.c`, `src/inet_client.c`

- [ ] **Consolidate CRLF injection checks** — The `_afc_smtp_check_crlf`, `_afc_ftp_client_check_crlf`, and `_afc_cgi_check_crlf` functions are identical. Extract to a single shared function in `base.c` or a new `utils.c`.
  - File(s): `src/smtp.c`, `src/ftp_client.c`, `src/cgi_manager.c`

- [ ] **Fix afc_string_trim() boundary bug** — The trim function decrements index `y` in a while loop checking `s[y]` but doesn't verify `y >= 0`, which can underflow for empty strings or strings with only whitespace, causing out-of-bounds read.
  - File(s): `src/string.c:548-555`

- [ ] **Use memmove() instead of memcpy() for overlapping regions** — `afc_string_copy()` uses `memcpy()` which is undefined for overlapping memory. If source and dest overlap (e.g., self-copy), this can corrupt data. Use `memmove()` instead.
  - File(s): `src/string.c:305`

- [ ] **Remove unused TRY/EXCEPT exception macros** — The custom exception handling system in `exceptions.h` uses goto-based flow control that makes code hard to follow. Many classes don't use it consistently. Consider replacing with simple `if/return` patterns.
  - File(s): `src/exceptions.h`, all class `.c` files

## Medium

- [ ] **Add thread safety to global state** — `__internal_afc_base` is a global pointer modified without synchronization. Multiple threads creating AFC instances will race. Either document single-thread-only usage or add mutex protection.
  - File(s): `src/base.c:35`

- [ ] **Optimize afc_string_comp() loop** — The comparison function has a complex loop condition with multiple checks per iteration. Simplify to a standard `strncmp`-like loop for clarity and performance.
  - File(s): `src/string.c:370-375`

- [ ] **Consolidate afc_string_resize_copy and afc_string_resize_add** — These two functions are nearly identical (both resize and copy/add). Merge into one function with a mode parameter or have resize_add call resize_copy then add.
  - File(s): `src/string.c:1070-1110`

- [ ] **Replace manual UTF-8 validation with simpler state machine** — The `_seems_utf8()` function uses nested loops and manual byte counting. A state-machine approach would be clearer and handle edge cases better.
  - File(s): `src/string.c:1150-1190`

- [x] **Add input validation to afc_string_radix()** — The function doesn't validate that `radix >= 2` (radix 0 or 1 would cause infinite loop or division by zero). Add bounds check at function entry.
  - File(s): `src/string.c:660`

- [ ] **Fix memory leak in afc_string_temp() on MINGW path** — If `mkstemp()` fails after `tempnam()` succeeds, the `name` pointer from `tempnam()` is not freed (it's freed with `free()`, not `afc_free()`).
  - File(s): `src/string.c:1020-1050`

- [ ] **Document magic number constants** — Each class defines magic numbers (e.g., `AFC_MAGIC`, `AFC_STRING_MAGIC`) but doesn't document what the hex values represent or why they were chosen. Add comments explaining the encoding.
  - File(s): `src/base.h`, `src/string.h`, `src/array.h`, `src/list.h`

- [ ] **Standardize function naming convention** — Some internal functions use `_afc_` prefix, others use `afc_` with `_internal_` in the name. Pick one convention and apply consistently.
  - File(s): All source files

- [ ] **Add overflow protection to integer arithmetic** — `afc_string_resize_copy()` multiplies `max * 2` which can overflow. Add overflow checks before arithmetic operations on sizes.
  - File(s): `src/string.c:1080`

- [ ] **Replace busy-wait patterns in network code** — Some network functions loop waiting for data without yielding or checking timeouts properly. Use `select()` or `poll()` with appropriate timeout values.
  - File(s): `src/inet_client.c`, `src/smtp.c`, `src/ftp_client.c`

## Low / Nice to have

- [x] **Add unit tests for edge cases** — Current tests cover happy paths. Add tests for: NULL inputs, zero-length strings, maximum-size strings, concurrent access, and memory exhaustion scenarios.
  - File(s): `src/test_area/string/test_02.c`

- [ ] **Generate API documentation from source comments** — The source has extensive `@node` documentation comments. Set up Doxygen or similar to auto-generate HTML/PDF docs.
  - File(s): All source files

- [ ] **Add pkg-config support** — Create an `afc.pc.in` file for pkg-config integration, making it easier for downstream projects to find and link AFC.
  - File(s): (new file: `afc.pc.in`)

- [ ] **Consider using C99/C11 features** — The codebase uses C89-style declarations at function tops. Modern C allows mixed declarations and code, `restrict` keyword for optimization, and `_Static_assert` for compile-time checks.
  - File(s): All source files

- [ ] **Add CI/CD pipeline configuration** — Add GitHub Actions or similar CI config to automatically build and run tests on push/PR.
  - File(s): (new file: `.github/workflows/ci.yml`)
