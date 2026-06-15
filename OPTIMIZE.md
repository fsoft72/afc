# Optimization TODO

> Generated on 2026-06-15. Items sorted by importance.

## Critical

- [ ] **Thread-safety: Global mutable `__internal_afc_base` pointer** - The library uses a single global `__internal_afc_base` pointer (`src/base.c:128`) shared across all threads. Any concurrent access to AFC classes will cause data races. This is the single biggest architectural limitation.
  - File(s): `src/base.c`, `src/base.h`

- [x] **Use-after-free pattern in `_afc_free`** - `_afc_free()` writes `m[0] = '\0'` to memory before freeing it (`src/base.c:710-711`). If the memory was already freed or corrupted, this is undefined behavior. The write serves no useful purpose since the memory is about to be freed.
  - File(s): `src/base.c:700-717`

- [x] **Deprecated `gethostbyname()` is not thread-safe** - `afc_inet_client_resolve()` uses `gethostbyname()` (`src/inet_client.c:298`) which returns a pointer to static data and is not thread-safe. It also doesn't support IPv6. Should use `getaddrinfo()` instead.
  - File(s): `src/inet_client.c:294-305`

- [x] **Stack buffer overflow in `fileops_internal_scan_dir`** - Uses `strcpy()` and `strcat()` on fixed-size stack buffer `dirname[]` without bounds checking (`src/fileops.c:839-842`). A path longer than `AFC_FILEOPS_MAX_DIR_LEN` will overflow the stack.
  - File(s): `src/fileops.c:839-842`

- [x] **`afc_new()` bypasses memory tracking** - `afc_new()` uses raw `malloc()` (`src/base.c:153`) instead of `afc_malloc()`, so the base AFC allocation is never tracked by the memory tracker. This defeats leak detection for the most fundamental object.
  - File(s): `src/base.c:151-184`
  - Status: **Won't fix** - bootstrap problem: `afc_malloc()` depends on `__internal_afc_base` which is created by `afc_new()`. The base object is intentionally untracked.

## High

- [ ] **`afc_string_max()` reads arbitrary memory before string pointer** - `afc_string_max()` (`src/string.c:222`) reads `*((unsigned long *)(str - sizeof(unsigned long) * 2))`. If called on a regular C string or stack buffer, this reads garbage memory or causes a segfault. While a sanity check was added, the fundamental design is unsafe.
  - File(s): `src/string.c:213-231`

- [x] **Hash table sorts on every insertion** - `afc_hash_add()` (`src/hash.c:227`) calls `afc_array_sort()` after every single insertion. For N items, this is O(N^2 log N) total. Should batch-sort or use insertion-sort for maintaining sorted order.
  - File(s): `src/hash.c:212-230`
  - Status: **Fixed** - Now uses binary search to find insertion position and inserts directly, maintaining sorted order without full sort.

- [ ] **O(N) linear scan in memory tracker** - `_memtrack_find()` (`src/mem_tracker.c:255-271`) does a linear scan of all tracked allocations. For applications with many allocations, this becomes a bottleneck. A hash table lookup would be O(1).
  - File(s): `src/mem_tracker.c:255-271`

- [ ] **`strtok_r` modifies `to_list` in `afc_smtp_send`** - `strtok_r()` (`src/smtp.c:775`) destructively modifies the duplicated `to_list` string. While `to_list` is a duplicate, the pattern is error-prone. Consider using `afc_string_list_split()` which is already available.
  - File(s): `src/smtp.c:773-802`

- [x] **`sprintf` in test code without bounds checking** - Test code in `src/array.c:1029` uses `sprintf(buf, "%4.4d", t)` with a 15-byte buffer. While safe for the test values, it sets a bad example. Should use `snprintf()`.
  - File(s): `src/array.c:1029`

- [x] **`_afc_realloc` passes potentially-freed pointer to tracker** - `_afc_realloc()` (`src/base.c:763`) calls `_afc_mem_tracker_update_size()` with the old `mem` pointer after `realloc()` has already been called. If `realloc()` moved the memory, the old pointer is invalid. The pragma suppressing the warning hides a real bug.
  - File(s): `src/base.c:740-767`

## Medium

- [ ] **`BOOL` type is `char` on Linux, `int` on MINGW** - `src/base.h:53-56` defines `BOOL` differently per platform. This causes ABI incompatibility and subtle bugs when mixing code compiled on different platforms. Use `int` consistently or use `<stdbool.h>`.
  - File(s): `src/base.h:52-56`

- [ ] **`true`/`false` defined as `~0` and `0`** - `src/base.h:37-42` defines `true` as `~0` (all bits set) instead of `1`. While functionally correct in boolean contexts, it can cause surprises in integer operations and conflicts with C99 `<stdbool.h>`.
  - File(s): `src/base.h:36-50`

- [x] **`register` keyword used in multiple functions** - The `register` storage class specifier (`src/string.c:464,499,787`, `src/hash.c:258`, `src/cgi_manager.c:1109`) is deprecated in C++17 and ignored by modern compilers. It adds visual noise without benefit.
  - File(s): `src/string.c`, `src/hash.c`, `src/cgi_manager.c`

- [ ] **`afc_string_radix` uses fixed 1024-byte stack buffer** - `src/string.c:728` uses `char buf[1024]` for number conversion. Very large numbers in unusual bases could exceed this. Should use dynamic allocation or calculate required size.
  - File(s): `src/string.c:725-757`

- [x] **`afc_fileops_move` checks `errno` after `rename()` without checking return value first** - `src/fileops.c:441` checks `errno == EXDEV` but `rename()` may have succeeded (returned 0), in which case `errno` is undefined. Should check `res != 0` before examining `errno`.
  - File(s): `src/fileops.c:435-460`

- [ ] **`afc_array_del` off-by-one when array becomes empty** - `src/array.c:610-611` sets `current_pos = num_items - 1` which underflows to `ULONG_MAX` when `num_items` is 0 (unsigned). The subsequent check `num_items <= 0` catches it, but the intermediate state is problematic.
  - File(s): `src/array.c:578-619`

- [x] **No input validation on `afc_string_new(0)`** - Calling `afc_string_new(0)` allocates a string with 0 capacity. Subsequent operations like `afc_string_copy()` will silently truncate everything. Should either reject 0 or allocate a minimum size.
  - File(s): `src/string.c:133-151`

- [ ] **`afc_string_utf8_to_latin1` only handles 2-byte UTF-8 sequences** - `src/string.c:1321-1343` only handles characters in the range U+0080 to U+00C7 (2-byte sequences starting with 0xC0-0xC7). Characters from 0xC8-0xFF and 3/4-byte sequences are silently dropped.
  - File(s): `src/string.c:1294-1352`

- [ ] **`cgi_manager` stack-based `dirname`/`fullname` buffers** - `afc_fileops_internal_scan_dir()` (`src/fileops.c:828-829`) uses fixed `char dirname[AFC_FILEOPS_MAX_DIR_LEN]` and `char fullname[AFC_FILEOPS_MAX_DIR_LEN]` on the stack. Deep directory trees could overflow these.
  - File(s): `src/fileops.c:822-883`

## Low / Nice to have

- [x] **`AFC_LOG_FAST` default case uses magic number 1000** - `src/base.c:402` sets `level = 1000` as a sentinel for unknown errors. Should use a named constant like `AFC_LOG_LEVEL_MAX + 1`.
  - File(s): `src/base.c:376-424`

- [x] **Duplicate `_afc_dprintf` calls in `_afc_free`** - `src/base.c:706` calls `_afc_dprintf` which itself may call `afc_free` through `__internal_afc_base->fout`. This creates a potential recursion path if `fout` operations trigger memory allocation.
  - File(s): `src/base.c:700-717`

- [x] **`mem_tracker` test code references undefined functions** - `src/mem_tracker.c:357-368` test code calls `memtrack_new()`, `memtrack_add()`, `memtrack_del()` which don't exist. The test code is dead/broken.
  - File(s): `src/mem_tracker.c:353-370`

- [x] **`custom_sort` in array test uses hardcoded `4` bytes** - `src/array.c:1004-1006` test `custom_sort()` function hardcodes `memcpy(&temp, pos1, 4)` instead of using the `size` parameter. This only works for 32-bit pointer arrays.
  - File(s): `src/array.c:989-1010`

- [x] **No `const` correctness on many string functions** - Functions like `afc_string_upper()`, `afc_string_lower()`, `afc_string_trim()` modify their input but their parameter types don't always reflect this. Some callers pass `const char*` to functions that modify the string.
  - File(s): `src/string.c`
  - Status: **Won't fix** - Signatures are correct: functions that modify in-place take `char *`. The issue is about callers, not the library.

- [x] **Missing `#include <errno.h>` in `base.c`** - `src/base.c` uses `errno` indirectly through `strerror()` but relies on transitive includes. Should explicitly include `<errno.h>`.
  - File(s): `src/base.c`

- [x] **`afc_string_copy` returns pointer to end of string, not beginning** - `src/string.c:320` returns `dest` after incrementing it through the copy loop, so the returned pointer points past the end of the copied data. This is inconsistent with most string copy semantics.
  - File(s): `src/string.c:295-321`
  - Status: **Fixed** - Now returns `dest` (beginning of string).

- [x] **`afc_string_add` returns pointer past end of string** - Same issue as `afc_string_copy` - `src/string.c:1042` returns `dest` after advancing it, pointing past the actual string data.
  - File(s): `src/string.c:1014-1043`
  - Status: **Fixed** - Now returns `dest` (beginning of string).

- [x] **Inconsistent error return patterns** - Some functions return `NULL` on error, others return `AFC_ERR_*` codes, and some return the error code passed as parameter. This makes error handling confusing for library users.
  - File(s): multiple files
  - Status: **Won't fix** - Would require a major API redesign and break existing code.

- [x] **`afc_array_internal_insert` uses `register` keyword** - `src/array.c:823` uses deprecated `register` storage class.
  - File(s): `src/array.c:821-831`

- [x] **Missing `fflush` after debug output** - `afc_debug()` and `afc_debug_adv()` (`src/base.c:461,509`) write to `fout` but don't flush, which can cause buffered output to be lost on crashes.
  - File(s): `src/base.c:454-514`

- [x] **`afc_fileops_clear` is empty** - `src/fileops.c:166-169` implements `afc_fileops_clear()` as a no-op, but the `FileOperations` struct may have state that should be reset.
  - File(s): `src/fileops.c:166-169`
  - Status: **Fixed** - Now resets all fields to defaults.
