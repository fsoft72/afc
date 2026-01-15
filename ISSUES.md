# AFC Library Code Analysis - Security and Correctness Issues

**Analysis Date:** 2026-01-15
**Analyzed By:** Claude Code
**Scope:** Application code in src/ directory (non-test files prioritized)

---

## CRITICAL SEVERITY ISSUES

### 1. **Buffer Overflow in SMTP Response Parsing**
**File:** `src/smtp.c:328-332`
**Function:** `_afc_smtp_get_response()`

```c
char code_str[4];
strncpy(code_str, smtp->buf, 3);
code_str[3] = '\0';
```

**Issue:** While `code_str[3]` is explicitly set to null, if `smtp->buf` contains fewer than 3 characters (which is checked at line 324), the `strncpy` will not null-terminate if the source is exactly 3 characters without a null terminator. More critically, if `smtp->buf` is not properly null-terminated due to a network error, this could read beyond buffer boundaries.

**Impact:** Potential buffer overflow, undefined behavior
**Recommendation:** Use `snprintf()` instead: `snprintf(code_str, sizeof(code_str), "%.3s", smtp->buf);`

---

### 2. **strtok Modifies Input String - Memory Safety Issue**
**File:** `src/smtp.c:648-668`
**Function:** `afc_smtp_send()`

```c
char *to_list = afc_string_dup(smtp->to);
char *recipient = strtok(to_list, ",");
```

**Issue:** While the code correctly duplicates the string before using `strtok()`, `strtok()` is not thread-safe and modifies the string in place. If multiple threads use the same SMTP instance, this could cause race conditions.

**Impact:** Race condition in multi-threaded environments, potential memory corruption
**Recommendation:** Use `strtok_r()` (thread-safe version) instead, or implement custom string splitting.

---

### 3. **Library Code Calls exit() - Kills Host Process**
**File:** `src/base64.c:240, 396, 407, 479`
**Functions:** `afc_base64_internal_inbuf()`, `afc_base64_internal_decode()`

```c
if (ferror(b64->fin)) {
    // TODO: READ ERROR
    exit(1);
}
```

**Issue:** Library code should NEVER call `exit()`. This terminates the entire host application, which is unacceptable behavior for a library. Applications using this library cannot gracefully handle errors.

**Impact:** Application termination, loss of data, denial of service
**Recommendation:** Return error codes (`AFC_BASE64_ERR_*`) and let the caller decide how to handle the error.

---

### 4. **Buffer Overflow in InetClient Response Handling**
**File:** `src/inet_client.c:351`
**Function:** `afc_inet_client_get()`

```c
ic->buf[bytes] = '\0';
```

**Issue:** After `recv()` or `SSL_read()` returns `bytes`, the code writes a null terminator at position `ic->buf[bytes]`. However, if `bytes` equals `afc_string_max(ic->buf)`, this writes beyond the allocated buffer.

**Impact:** Heap buffer overflow, potential code execution
**Recommendation:** Check bounds before writing: `if (bytes < afc_string_max(ic->buf)) ic->buf[bytes] = '\0';` or allocate buffer with +1 size.

---

### 5. **Unchecked realloc() Failures**
**File:** `src/mem_tracker.c:230, 237`
**Functions:** `_memtrack_realloc()`, `_memtrack_realloc_free()`

```c
mt->data = realloc(mt->data, mt->data_max * sizeof(MemTrackData));
```

**Issue:** `realloc()` can fail and return NULL. If it fails, the original pointer is lost, causing a memory leak. Continuing to use the NULL pointer causes undefined behavior.

**Impact:** Memory corruption, segmentation fault, memory leak
**Recommendation:** Store return value in temporary variable and check for NULL before assignment.

---

### 6. **Socket File Descriptor Check Against Wrong Value**
**File:** `src/inet_client.c:257-258`
**Function:** `afc_inet_client_close()`

```c
if (ic->sockfd)
    close(ic->sockfd);
```

**Issue:** Socket file descriptors can be 0 (stdin). The code checks `if (ic->sockfd)` which treats 0 as "no socket". Socket errors return -1, not 0. A valid socket with fd=0 will never be closed.

**Impact:** Resource leak, file descriptor exhaustion
**Recommendation:** Check `if (ic->sockfd >= 0)` or track socket state with a separate boolean flag.

---

## HIGH SEVERITY ISSUES

### 7. **Potential NULL Pointer Dereference in SMTP AUTH**
**File:** `src/smtp.c:440-442`
**Function:** `_afc_smtp_auth_plain()`

```c
auth_str[0] = '\0';
strcpy(auth_str + 1, smtp->username);
auth_str[1 + afc_string_len(smtp->username)] = '\0';
strcpy(auth_str + 1 + afc_string_len(smtp->username) + 1, smtp->password);
```

**Issue:** While `smtp->username` and `smtp->password` are checked at line 583-584 in `afc_smtp_authenticate()`, the internal functions assume these are valid. If called directly (future refactoring), this causes NULL pointer dereference. Additionally, complex pointer arithmetic is error-prone.

**Impact:** Segmentation fault, potential buffer overflow
**Recommendation:** Add null checks in internal functions, use safer buffer building functions like `snprintf()` or structured memory copy.

---

### 8. **Missing Bounds Check in Array Access**
**File:** `src/array.c:323`
**Function:** `afc_array_item()`

```c
if (item >= am->num_items)
    return NULL;
return (am->mem[am->current_pos = item]);
```

**Issue:** The check is `>=` which is correct, but there's no check that `item` is non-negative. If `item` is a very large unsigned value wrapping from negative, it could pass this check.

**Impact:** Out-of-bounds memory access
**Recommendation:** Add explicit check: `if (item >= am->num_items || item < 0)` (if using signed) or ensure callers never pass wrapped values.

---

### 9. **Unsafe atoi() on Potentially NULL Pointers**
**File:** `src/pop3.c:285-286`
**Function:** `afc_pop3_stat()`

```c
p->tot_messages = atoi((s = afc_string_list_item(p->sn, 1)));
p->tot_size = atoi((s = afc_string_list_item(p->sn, 2)));
```

**Issue:** If `afc_string_list_item()` returns NULL (item not found), passing NULL to `atoi()` is undefined behavior. While the code assigns to `s` to avoid compiler warnings, it doesn't check if `s` is NULL.

**Impact:** Undefined behavior, potential segmentation fault
**Recommendation:** Check `s != NULL` before calling `atoi()`, or use safer parsing like `strtol()` with error checking.

---

### 10. **String Length Calculation Uses Unsafe Macro**
**File:** `src/string.c:205-208`
**Function:** `afc_string_max()`

```c
unsigned long afc_string_max(const char *str)
{
    return (str ? ((unsigned long)(*((unsigned long *)(str - sizeof(unsigned long) * 2)) - 1)) : 0L);
}
```

**Issue:** Directly dereferences memory before the string pointer without any bounds checking. If called with a non-AFC string (regular C string), this reads uninitialized/invalid memory.

**Impact:** Undefined behavior, potential segmentation fault, information disclosure
**Recommendation:** Add magic number validation before dereferencing metadata, or clearly document that mixing AFC and C strings in this way is forbidden.

---

### 11. **SMTP Response Buffer Not Validated for Size**
**File:** `src/smtp.c:321`
**Function:** `_afc_smtp_get_response()`

```c
afc_string_copy(smtp->buf, smtp->ic->buf, ALL);
```

**Issue:** Copies entire inet_client buffer into smtp buffer without checking if smtp->buf is large enough. If server sends response larger than smtp->buf can hold, data is silently truncated (per `afc_string_copy` bounds checking), but this could cause protocol errors.

**Impact:** Protocol parsing errors, potential security bypass if truncation creates valid-looking response codes
**Recommendation:** Check buffer size and return error if response is too large, or dynamically resize buffer.

---

### 12. **Race Condition in Hash Table Sorting**
**File:** `src/hash.c:253-254`
**Function:** `afc_hash_find()`

```c
if (hm->am->is_sorted == FALSE)
    afc_array_sort(hm->am, afc_hash_internal_sort);
```

**Issue:** In multi-threaded environment, two threads could both see `is_sorted == FALSE` and both call sort, causing corruption. No mutex protection on the hash table.

**Impact:** Data corruption, incorrect search results, possible segmentation fault
**Recommendation:** Add thread synchronization (mutex) around hash operations, or document that hash tables are not thread-safe.

---

## MEDIUM SEVERITY ISSUES

### 13. **Memory Leak in Error Paths**
**File:** `src/smtp.c:456-469`
**Function:** `_afc_smtp_auth_plain()`

**Issue:** If `afc_base64_encode()` fails or memory allocation fails after some allocations, not all allocated memory is freed. For example, if `encoded` allocation fails, `auth_str` and `b64` are already allocated.

**Impact:** Memory leak on error paths
**Recommendation:** Use TRY/EXCEPT/FINALLY pattern consistently, or ensure all error returns clean up.

---

### 14. **Insufficient Input Validation in HTTP Client**
**File:** `src/http_client.c`
**Function:** Various

**Issue:** HTTP client doesn't validate URL length or components before parsing. A maliciously crafted URL could cause buffer overflows in internal parsing functions.

**Impact:** Potential buffer overflow, denial of service
**Recommendation:** Add URL length checks and validate components before copying to fixed-size buffers.

---

### 15. **Integer Overflow in Buffer Size Calculation**
**File:** `src/smtp.c:435`
**Function:** `_afc_smtp_auth_plain()`

```c
auth_len = 1 + afc_string_len(smtp->username) + 1 + afc_string_len(smtp->password);
```

**Issue:** If username or password are extremely long (near SIZE_MAX), this addition could overflow, resulting in a small `auth_len` and subsequent buffer overflow.

**Impact:** Integer overflow leading to buffer overflow
**Recommendation:** Add overflow checks before allocation: `if (username_len > SIZE_MAX - password_len - 2) return error;`

---

### 16. **Dichotomous Search Can Loop Infinitely**
**File:** `src/hash.c:260-277`
**Function:** `afc_hash_find()`

**Issue:** If the array is corrupted or hash values are malformed, the dichotomous search could enter an infinite loop or access invalid memory. The termination condition relies on `min > max` which may not trigger if values wrap.

**Impact:** Infinite loop, denial of service, potential crash
**Recommendation:** Add iteration counter limit, validate array integrity before search.

---

### 17. **Missing Magic Number Validation**
**File:** Multiple files
**Functions:** Various internal functions

**Issue:** Many internal functions (prefixed with `_`) don't validate magic numbers before operating on structures. If called directly or after memory corruption, they operate on invalid data.

**Impact:** Undefined behavior, data corruption
**Recommendation:** Add magic number checks in internal functions as well, or clearly document preconditions.

---

### 18. **Base64 Encoder Modifies Constant Memory**
**File:** `src/base64.c:22`
**Global:** `static char eol[] = "\r\n";`

**Issue:** While `eol` is defined as static char array, it's passed to functions that might modify it. Though current code doesn't modify it, the pattern is unsafe.

**Impact:** Potential undefined behavior if code is modified
**Recommendation:** Declare as `const char eol[] = "\r\n";` to enforce immutability.

---

## LOW SEVERITY ISSUES

### 19. **Inconsistent Error Handling**
**File:** Multiple

**Issue:** Some functions return error codes, others return NULL, others call exit(). No consistent error handling strategy.

**Impact:** Difficult error handling for library users
**Recommendation:** Standardize on error code returns, document error conditions.

---

### 20. **Missing const Qualifiers**
**File:** Multiple

**Issue:** Many functions that don't modify their parameters lack `const` qualifiers. For example, `afc_string_comp()` should take `const char *`.

**Impact:** Lost optimization opportunities, unclear API semantics
**Recommendation:** Add `const` to all read-only parameters.

---

### 21. **Unsafe sprintf Usage**
**File:** `src/string.c` and others

**Issue:** While the code uses `afc_string_make()` which likely wraps `vsprintf()`, fixed-size buffers with sprintf-family functions are prone to overflow.

**Impact:** Potential buffer overflow in format string operations
**Recommendation:** Review all format string operations, use snprintf where appropriate.

---

### 22. **Resource Cleanup in Constructors**
**File:** Multiple

**Issue:** Some `_new()` functions don't properly clean up all resources on failure. For example, if allocation of the 3rd member fails, first two might not be freed.

**Impact:** Memory leak on construction failure
**Recommendation:** Use consistent TRY/EXCEPT/FINALLY pattern in all constructors.

---

### 23. **Platform-Specific Code Without Proper Abstraction**
**File:** `src/string.c:39-43`

```c
#ifdef MINGW
static const char dir_sep = '\\';
#else
static const char dir_sep = '/';
#endif
```

**Issue:** Platform detection at compile time may not match runtime environment (e.g., Cygwin on Windows).

**Impact:** Incorrect path handling on some platforms
**Recommendation:** Detect path separator at runtime or provide configuration option.

---

### 24. **No NULL Checks Before String Operations**
**File:** `src/smtp.c` and others

**Issue:** Many string operations assume pointers are non-NULL without explicit checks. While some functions do check, internal helper functions often don't.

**Impact:** Potential NULL pointer dereference
**Recommendation:** Add defensive NULL checks, especially in public API functions.

---

### 25. **Memory Tracker Missing Thread Safety**
**File:** `src/mem_tracker.c`

**Issue:** Memory tracker maintains global state without any mutex protection. In multi-threaded applications, this causes race conditions.

**Impact:** Incorrect memory tracking, possible crash
**Recommendation:** Add mutex protection to all memory tracker operations.

---

## SUMMARY STATISTICS

- **Total Issues Found:** 25
- **Critical Severity:** 6
- **High Severity:** 6
- **Medium Severity:** 6
- **Low Severity:** 7

---

## PRIORITIZED REMEDIATION RECOMMENDATIONS

### Immediate Action Required (Critical):
1. Fix exit() calls in base64.c - replace with error returns
2. Fix buffer overflow in inet_client.c:351
3. Fix unchecked realloc() in mem_tracker.c
4. Fix socket descriptor check in inet_client.c:257

### High Priority:
5. Add comprehensive bounds checking to SMTP auth functions
6. Fix NULL pointer handling in POP3 parsing
7. Add thread safety to hash table operations

### Medium Priority:
8. Audit all error paths for memory leaks
9. Add input validation to HTTP client URL parsing
10. Review all integer arithmetic for overflow potential

### Low Priority:
11. Standardize error handling across library
12. Add const qualifiers to improve type safety
13. Add thread safety to memory tracker

---

## TESTING RECOMMENDATIONS

1. **Fuzzing:** Use AFL or libFuzzer on network protocol parsers (SMTP, POP3, HTTP)
2. **Static Analysis:** Run Coverity, Clang Static Analyzer, or Cppcheck
3. **Dynamic Analysis:** Use AddressSanitizer, MemorySanitizer, UndefinedBehaviorSanitizer
4. **Thread Safety:** Use ThreadSanitizer to detect race conditions
5. **Valgrind:** Run all tests under Valgrind to detect memory errors

---

## NOTES

- This analysis prioritized application code over test files as requested
- Many issues stem from legacy C practices (pre-C99) and can be modernized
- The library would benefit from a comprehensive security audit
- Consider migrating to safer string handling (e.g., bstring library)
- Thread safety should be clearly documented for all public APIs

---

**Report Generated:** 2026-01-15
**Reviewer:** Claude Code (AI Assistant)
**Methodology:** Manual code review with focus on buffer overflows, memory safety, integer overflows, race conditions, and error handling
