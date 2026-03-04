# AFC Optimization Report

Comprehensive analysis of the AFC codebase for performance, memory, correctness, and security improvements.

Each item is tagged with priority and category:
- **Priority**: CRITICAL, HIGH, MEDIUM, LOW
- **Category**: `perf` (performance), `mem` (memory), `bug` (correctness), `sec` (security), `build` (build system)

---

## Table of Contents

1. [Critical Bugs (Fix First)](#1-critical-bugs-fix-first)
2. [Core & Base System](#2-core--base-system)
3. [String Module](#3-string-module)
4. [Data Structures (Array, List, Hash, Dictionary)](#4-data-structures)
5. [Tree Structures (BTree, BinTree, AVL, Tree, CircularList)](#5-tree-structures)
6. [Networking Stack](#6-networking-stack)
7. [Utilities (MD5, Base64, RegExp, etc.)](#7-utilities)
8. [Build System](#8-build-system)
9. [Summary: Top 20 Highest-Impact Changes](#9-summary-top-20-highest-impact-changes)

---

## 1. Critical Bugs (Fix First)

These are correctness/security issues that should be resolved before any optimization work.

### 1.1 `hash.c` / `dictionary.c` -- Hash collision causes data corruption [CRITICAL/bug]

`afc_dictionary_internal_find()` (`dictionary.c:786`) finds entries by hash value but never verifies the actual key matches. If two keys produce the same `afc_string_hash` value, lookups silently return the wrong entry and `afc_dictionary_set` overwrites the wrong data. **Fix**: after finding a hash match, compare the actual key strings.

### 1.2 `hash.c:227` -- Full sort on every insertion [CRITICAL/perf]

`afc_hash_add()` calls `afc_array_sort()` after every single insertion. Adding N items costs O(n^2 log n) total. Since Dictionary depends on Hash, this makes all dictionary bulk operations catastrophically slow. **Fix**: use sorted insertion (binary search + shift) or deferred sorting with a dirty flag.

### 1.3 `inet_client.c` -- FILE* bypasses SSL [CRITICAL/bug+sec]

`afc_inet_client_get_file()` returns `fdopen(sockfd)`. When SSL is enabled, `fgets()`/`fread()` on this FILE* read raw encrypted bytes, bypassing `SSL_read()`. This breaks:
- **HttpClient** (`http_client.c:989`) -- all HTTPS response parsing
- **POP3** (`pop3.c:471`) -- POP3S/STLS would read garbage
- **FTP** -- if TLS were ever added

**Fix**: implement a line-buffered reader that dispatches to `SSL_read()`/`recv()` internally, or use OpenSSL BIO objects.

### 1.4 `circular_list.c:393` -- Wrong allocation size [CRITICAL/bug]

`afc_circular_list_int_create_node()` allocates `sizeof(CircularList)` instead of `sizeof(CircularListNode)`. If sizes differ, this causes either buffer overflows or memory waste on every node.

### 1.5 `circular_list.c` -- Use-after-free on single-element deletion [CRITICAL/bug]

`afc_circular_list_del()` for a single-element list: after freeing the node, it still dereferences the freed pointer before setting `cl->pointer = NULL`.

### 1.6 `array.c:282` -- Out-of-bounds write before resize [CRITICAL/bug]

`afc_array_add()` writes `am->mem[am->num_items] = data` **before** checking if the array needs to grow. When `num_items == max_items`, this writes past the buffer. **Fix**: move the capacity check before the write.

### 1.7 `array.c:606` -- `memcpy` on overlapping memory [CRITICAL/bug]

`afc_array_del()` uses `memcpy()` to shift elements in the same array. Overlapping source/dest is undefined behavior. **Fix**: change to `memmove()`.

### 1.8 `avl_tree.c:122,129,141` -- Insert compares value instead of key [CRITICAL/bug]

`_afc_avl_tree_insert()` compares `val` against `node->key` instead of `key` against `node->key`. The tree ends up ordered by values, not keys.

### 1.9 `avl_tree.c:65` -- Dangling root pointer after clear [HIGH/bug]

`afc_avl_tree_clear()` frees all nodes but never sets `t->root = NULL`. Subsequent operations dereference freed memory.

### 1.10 `list.c:721` -- Clear function invoked on wrong node [HIGH/bug]

`afc_list_clear()` calls `func_clear(n->ln_Name)` where `n` is `w->ln_Succ`, not the node being removed. First node's data is never cleared; sentinel node's data gets cleared incorrectly. Causes memory leaks.

### 1.11 `inet_client.c` -- Partial sends not handled [HIGH/bug]

Both `send()` and `SSL_write()` can return fewer bytes than requested. The code checks only for `-1` but does not loop to send remaining data. On large payloads or loaded systems, data is silently truncated.

---

## 2. Core & Base System

### 2.1 `mem_tracker.c:255-271` -- Linear scan on every free [HIGH/perf]

`_memtrack_find()` does O(n) linear search across all tracked allocations. With thousands of allocations, every `free()` becomes O(n), making overall memory management O(n^2). **Fix**: use a hash table keyed on memory address for O(1) amortized lookup.

### 2.2 `mem_tracker.c:104-106` -- Unnecessary strdup on every allocation [MEDIUM/perf+mem]

Every tracked allocation does two `strdup()` calls for `__FILE__` and `__FUNCTION__`. These are static string literals with program lifetime -- store the raw pointers instead, saving 2 allocations + 2 frees per tracked allocation.

### 2.3 `base.c:797-801` -- Double fflush in `_afc_dprintf()` [LOW/perf]

Two consecutive `fflush(f)` calls. The second is redundant.

### 2.4 `base.c:153` -- malloc + memset instead of calloc [LOW/perf]

`afc_new()` uses `malloc` then `memset`. Using `calloc(1, sizeof(AFC))` is potentially faster (OS can provide pre-zeroed pages).

### 2.5 `base.c:676` -- Same malloc + memset pattern in `_afc_malloc()` [LOW/perf]

When the memory tracker is not active, `_afc_malloc()` calls `malloc()` then `memset()`. Use `calloc()` instead.

### 2.6 `base.c:128` -- Global singleton prevents thread safety [LOW/perf]

`__internal_afc_base` global pointer makes the library non-thread-safe and prevents multiple AFC contexts per process.

---

## 3. String Module

### 3.1 `string.c:316-318,1038-1040` -- Manual byte-copy loops [HIGH/perf]

`afc_string_copy()` and `afc_string_add()` copy one byte at a time in a for loop. Modern `memcpy()` uses SIMD and copies 16-64 bytes at a time. **Fix**: replace with `memcpy(dest, source, len)`.

### 3.2 Multiple redundant `strlen` calls [HIGH/perf]

Numerous functions compute `strlen` on the same string multiple times:
- `afc_string_dup()` (`string.c:889,897`) -- calls `strlen` then passes to `afc_string_copy` which calls `strlen` again
- `afc_string_utf8_to_latin1()` (`string.c:1294-1307`) -- calls `strlen` three times on the same input
- `afc_string_hash()` (`string.c:794`) -- calls `strlen` even when callers already know the length

**Fix**: compute length once and pass explicitly through internal APIs.

### 3.3 `string.c:725` -- `afc_string_radix()` is O(n^2) [MEDIUM/perf]

Builds result by repeatedly prepending characters (each prepend copies entire string). **Fix**: build digits in reverse order in a buffer, then reverse once.

### 3.4 `string.c:1380` -- Pattern match allocates for case-insensitive compare [MEDIUM/perf]

`afc_string_pattern_match()` with `nocase=true` allocates two strings, copies, converts to uppercase, calls `fnmatch`, then frees. **Fix**: use `fnmatch()` with `FNM_CASEFOLD` flag directly.

### 3.5 `string.c:1710,1768` -- Padding adds one char at a time [MEDIUM/perf]

`afc_string_pad_start()` and `afc_string_pad_end()` call `afc_string_add()` per character. Each call involves strlen + bounds check + copy. **Fix**: bulk-copy the pad string using `memcpy`.

### 3.6 `string.c:1553` -- `afc_string_repeat()` repeated adds [MEDIUM/perf]

Each repetition calls `afc_string_add()` which recomputes `strlen(source)` every time. **Fix**: compute length once, pre-allocate result, use `memcpy`.

### 3.7 `string.c:1158` -- Resize doubles but ignores actual need [LOW/mem]

`afc_string_resize_copy()` allocates `max * 2` but doesn't check if that's sufficient. **Fix**: allocate `max(max * 2, needed_size + margin)`.

### 3.8 `string.c:133` -- No power-of-2 alignment in allocation [LOW/mem]

Rounding up to the next power of 2 would reduce allocator fragmentation.

---

## 4. Data Structures

### Array (`array.c`)

#### 4.1 `array.c:821` -- Element-by-element shift instead of memmove [MEDIUM/perf]

`afc_array_internal_insert()` shifts elements one at a time in a for loop. **Fix**: use `memmove()`.

#### 4.2 `array.c:613` -- Del invalidates sort flag unnecessarily [LOW/perf]

Deleting from a sorted array doesn't break sort order, but `is_sorted` is set to FALSE, forcing unnecessary re-sorts.

#### 4.3 `array.h:45` -- Default 100 items too generous [LOW/mem]

Default of 100 slots (800 bytes) is wasteful for most uses. Consider 16 as default.

### List (`list.c`)

#### 4.4 `list.c:981` -- Default sort is O(n^2 log n) [HIGH/perf]

`afc_list_sort()` uses `afc_list_item()` (O(n) access) inside the partition loop. **Fix**: default to `afc_list_fast_sort()` which creates an array first, or `afc_list_ultra_sort()`.

#### 4.5 `list.c:505-527` -- Stack cleanup bug in `afc_list_del()` [HIGH/bug]

The loop at line 518 has underflow issues: when `sposcount` is 0 (unsigned char), `sposcount - 1` wraps to 255. Line 523 accesses `spos[t-1]` when `t=0`, causing underflow.

#### 4.6 `list.c:1090` -- Array creation includes sentinel node [MEDIUM/bug]

`afc_list_create_array()` includes the sentinel tail node in the array. Off-by-one that wastes a slot and may cause issues in callers.

#### 4.7 `list.c:1154` -- Clone doesn't copy `func_clear` [MEDIUM/bug]

Cloned list doesn't inherit the clear callback, causing memory leaks when the clone is deleted.

### Hash (`hash.c`)

#### 4.8 `hash.c:280` -- Binary search uses function calls [MEDIUM/perf]

`afc_hash_find()` calls `afc_array_item()` (with NULL/bounds checks) inside the binary search loop. **Fix**: access `hm->am->mem[pos]` directly.

#### 4.9 Hash is not a hash table [MEDIUM/perf]

Despite the name, this is a sorted array with binary search (O(log n) lookup). A proper hash table with buckets/open addressing would give O(1) average lookup.

#### 4.10 `hash.c:214` -- Small allocations per entry [LOW/mem]

Every entry allocates a separate 16-byte `HashData` struct. Consider a flat array or pool allocator.

### Dictionary (`dictionary.c`)

#### 4.11 Dictionary double-computes string length [MEDIUM/perf]

Both `afc_dictionary_set()` and `afc_dictionary_internal_find()` compute `strlen(key)` then pass to `afc_string_hash()` which calls `strlen()` again internally. **Fix**: pass pre-computed length through the hash function.

#### 4.12 Dictionary has 3 heap allocations per entry [MEDIUM/mem]

Each entry: HashData (allocated) -> DictionaryData (allocated) -> key copy (allocated). **Fix**: single allocation containing all three.

### StringList (`string_list.c`)

#### 4.13 `string_list.c:1073` -- Case-insensitive sort allocates per comparison [HIGH/perf]

The sort comparator allocates 2 strings, copies, uppercases, compares, frees -- per comparison. With O(n log n) comparisons, this creates an allocation storm. **Fix**: use `strcasecmp()`.

#### 4.14 `string_list.c:170` -- `strlen` called 3 times on same string [MEDIUM/perf]

`afc_string_list_add()` calls `strlen(s)` at lines 174 and 179, then `afc_string_dup` calls it again. **Fix**: compute once.

#### 4.15 `string_list.c:915` -- Split duplicates delimiters unnecessarily [LOW/mem]

`afc_string_list_split()` duplicates the delimiter string even though it's never modified.

#### 4.16 `string_list.c:946` -- O(n*d) delimiter search [LOW/perf]

For each position, iterates all delimiter characters. **Fix**: use a 256-byte lookup table for O(1) per character.

---

## 5. Tree Structures

### BTree (`btree.c`)

#### 5.1 `btree.c:483-505` -- Linear search in B-tree nodes [HIGH/perf]

Node-internal search uses reverse linear scan. Since entries are sorted, binary search would turn O(m) per node into O(log m). **Fix**: replace with binary search.

#### 5.2 `btree.c:691-711` -- Three allocations per node [MEDIUM/mem]

Each `BTreeNode` requires 3 separate `afc_malloc` calls (struct + entry array + branch array). **Fix**: single allocation with computed offsets.

#### 5.3 `btree.c` -- Two comparison callbacks instead of one [MEDIUM/perf]

Uses separate `lower_than` and `equal` functions instead of a single three-way comparator. Doubles function pointer calls in the search path.

#### 5.4 `btree.c:111` -- Fixed 1024-byte key buffer [LOW/mem]

Unconditionally allocates 1024 bytes regardless of key size. No bounds checking anywhere. **Fix**: dynamically size or make configurable.

#### 5.5 `btree.c:461` -- Recursive search could be iterative [LOW/perf]

`afc_btree_int_search_tree()` is tail-recursive. Convert to iterative loop to save stack frames.

### BinTree (`bin_tree.c`)

#### 5.6 No balancing on insert [MEDIUM/perf]

`afc_bin_tree_int_insert()` is a standard unbalanced BST insert. Sorted data degrades to O(n) per operation. Consider suggesting users use AVLTree instead.

#### 5.7 Traversals ignore callback return value [LOW/perf]

No way to short-circuit traversal early. All N nodes are always visited.

#### 5.8 Default comparison casts void* to long int [LOW/bug]

`afc_bin_tree_int_compare()` casts `void *` to `long int`. On platforms where sizes differ, this truncates values.

### AVL Tree (`avl_tree.c`)

#### 5.9 No deletion operation [MEDIUM/perf]

Only way to remove an element is to clear and rebuild the entire tree: O(n log n).

#### 5.10 No traversal API [LOW/perf]

Unlike BinTree, AVLTree provides no traversal function. Users must know internal struct layout.

#### 5.11 Double rotation could be optimized [LOW/perf]

Two sequential single rotations with redundant height recalculations. A combined 3-node rotation would be faster.

### Generic Tree (`tree.c`)

#### 5.12 `tree.c:178` -- Insert walks all root siblings [MEDIUM/perf]

`afc_tree_insert()` traverses all root-level siblings to find the last one. **Fix**: maintain a `last_root` pointer.

#### 5.13 `tree.c:91` -- No NULL check on allocation [MEDIUM/bug]

`afc_tree_int_node_new()` doesn't check if `afc_malloc` returned NULL. Segfaults on allocation failure.

#### 5.14 `tree.h:56` -- Unused `compare` function pointer [LOW/mem]

Set in `afc_tree_new()` but never called anywhere. Dead code.

---

## 6. Networking Stack

### InetClient (`inet_client.c`)

#### 6.1 `inet_client.c:298` -- Uses deprecated `gethostbyname()` [HIGH/perf+sec]

Blocking, not thread-safe, no IPv6 support. **Fix**: replace with `getaddrinfo()`.

#### 6.2 No TCP_NODELAY [MEDIUM/perf]

For request-response protocols, Nagle's algorithm adds up to 200ms latency per small send. **Fix**: set `TCP_NODELAY` after socket creation.

#### 6.3 `inet_client.c:550` -- No SSL certificate verification [HIGH/sec]

`SSL_CTX_set_verify()` is never called. Connections accept any certificate (self-signed, expired), enabling MITM attacks. **Fix**: enable `SSL_VERIFY_PEER` and load system CA certificates.

#### 6.4 No SNI (Server Name Indication) [MEDIUM/sec]

`SSL_set_tlsext_host_name()` is never called. Many modern servers require SNI. **Fix**: set SNI hostname before SSL handshake.

#### 6.5 `inet_client.c:545` -- SSL globals initialized per connection [LOW/perf]

`SSL_library_init()` etc. called on every `enable_ssl()`. Should be `pthread_once`.

#### 6.6 `inet_client.c:199-212` -- Socket leak on DNS failure [LOW/bug]

Socket created but not closed if `afc_inet_client_resolve()` fails.

#### 6.7 `inet_client.c:80` -- Small 1024-byte receive buffer [LOW/perf]

Forces many `recv()` syscalls for bulk data. **Fix**: increase to 4096-8192 bytes.

### InetServer (`inet_server.c`)

#### 6.8 `inet_server.c:244` -- select() scalability limit [MEDIUM/perf]

Hard limit of FD_SETSIZE (1024) fds, O(n) per call. **Fix**: use `epoll` on Linux.

#### 6.9 `inet_server.c:201` -- listen() backlog of 10 [MEDIUM/perf]

Extremely small. Under burst traffic, connections are dropped. **Fix**: use `SOMAXCONN`.

#### 6.10 `inet_server.c:288` -- Buffer overflow in recv [MEDIUM/bug]

`recv` fills buffer to max capacity, then `data->buf[nbytes] = '\0'` writes past the end. Need to reserve one byte for the null terminator.

#### 6.11 No SSL support in server [LOW/sec]

Server accepts only plaintext connections.

### HttpClient (`http_client.c`)

#### 6.12 `http_client.c:912-964` -- Multiple sends per request [HIGH/perf]

Each header line triggers a separate `send()`. With Nagle enabled, each may be a separate TCP segment. **Fix**: build entire request in one buffer and send once.

#### 6.13 `http_client.c:1166-1201` -- Response body reallocation [MEDIUM/perf]

Body accumulated via repeated `afc_string_add()` on a 4096-byte buffer. **Fix**: pre-allocate to Content-Length when known.

#### 6.14 No response header size limit [MEDIUM/sec]

`_afc_http_client_parse_headers()` reads headers in an unbounded loop. Malicious server can exhaust memory. **Fix**: impose max header size/count.

#### 6.15 `http_client.c:1179` -- `atoi` for Content-Length without validation [MEDIUM/sec]

Negative or non-numeric values cause undefined behavior. **Fix**: use `strtol` with validation.

#### 6.16 No `Connection: keep-alive` header management [LOW/perf]

No persistent connection handling despite rudimentary reuse logic.

### SMTP (`smtp.c`)

#### 6.17 `smtp.c:330-356` -- Multi-line response not fully parsed [HIGH/bug]

`_afc_smtp_get_response()` reads only one chunk. EHLO responses are multi-line (lines with `-` after code). Missing capabilities can cause wrong TLS/auth behavior.

#### 6.18 `smtp.c:866` -- Fixed 4096-byte message buffer [MEDIUM/bug]

`afc_smtp_send_simple()` truncates large emails silently.

### POP3 (`pop3.c`)

#### 6.19 `pop3.c:296` -- Password logged in debug output [HIGH/sec]

`afc_dprintf("Sending password: %s\n", p->passwd)` prints password in cleartext.

#### 6.20 `pop3.c:349` -- NULL pointer dereference on malformed response [MEDIUM/bug]

`strchr()` can return NULL; next line dereferences without checking.

#### 6.21 `pop3.c:352` -- POP3MsgData not zero-initialized [MEDIUM/bug]

Allocated with `afc_malloc` but fields not zeroed. Freeing garbage pointers on cleanup.

#### 6.22 No SSL/TLS support in POP3 [LOW/sec]

No STLS or POP3S support. Plaintext only.

### FTP Client (`ftp_client.c`)

#### 6.23 Per-command buffer allocation throughout [MEDIUM/perf]

Every FTP command allocates/frees 1024-byte `cmd` and `answer` strings. **Fix**: pre-allocate in FtpClient struct.

#### 6.24 TYPE command sent before every transfer [LOW/perf]

Transfer type persists per session but is re-sent every time. **Fix**: cache current type, send only on change.

#### 6.25 `ftp_client.c:647` -- Password logged in debug output [HIGH/sec]

Same issue as POP3.

#### 6.26 Active mode store/retrieve broken [MEDIUM/bug]

`afc_ftp_client_internal_store_active()` returns immediately without doing anything. Active mode retrieve has no data callback. Both are non-functional.

#### 6.27 `ftp_client.c:1140` -- Infinite loop risk in answer reading [LOW/bug]

No iteration limit or timeout. Malformed responses cause infinite loop.

---

## 7. Utilities

### MD5 (`md5.c`)

#### 7.1 `md5.c:281-285` -- Digest formatting: 16 iterations of sprintf + string add [MEDIUM/perf]

**Fix**: single `snprintf()` into a pre-sized 33-byte buffer.

#### 7.2 `md5.c:305` -- Dynamic allocation for 1024-byte read buffer [LOW/perf]

Stack variable would avoid malloc/free overhead.

#### 7.3 `md5.c:329` -- Unnecessary memset between fread calls [LOW/perf]

Buffer is overwritten by next fread; zeroing serves no purpose.

### Base64 (`base64.c`)

#### 7.4 `base64.c:294,372` -- Lookup tables rebuilt on every call [HIGH/perf]

Both encode and decode tables are reconstructed from scratch on every invocation. **Fix**: pre-compute as `static const` arrays.

### RegExp (`regexp.c`)

#### 7.5 `regexp.c:532-588` -- Replace creates a full RegExp per call [HIGH/perf]

`afc_regexp_internal_replace()` allocates an entire `RegExp` instance just to find `\1`-style backreferences. **Fix**: simple character scan instead.

#### 7.6 `regexp.c:401` -- Pattern recompiled on every replace [MEDIUM/perf]

`afc_regexp_replace()` always calls `afc_regexp_compile()`. **Fix**: cache compiled patterns.

#### 7.7 `regexp.c:242` -- strlen called twice in match [LOW/perf]

`afc_regexp_match()` computes `strlen(str)` at line 242 and again at line 252.

### DirMaster (`dirmaster.c`)

#### 7.8 `dirmaster.c:1218-1233` -- Sort comparison allocates 2 strings per compare [HIGH/perf]

Case-insensitive sort allocates two strings, copies, converts to uppercase, compares, frees -- per O(n log n) comparisons. **Fix**: use `strcasecmp()`.

#### 7.9 `dirmaster.c:837` -- Heap-allocated `struct stat` per file [LOW/mem]

Each FileInfo allocates a separate `struct stat`. **Fix**: embed directly in `FileInfo` struct.

### FileOps (`fileops.c`)

#### 7.10 `fileops.c:1079` -- File copy uses read()/write() [MEDIUM/perf]

Modern Linux supports `sendfile()` or `copy_file_range()` for zero-copy kernel-space transfers. **Fix**: use `copy_file_range()` with fallback to read/write.

#### 7.11 `fileops.h:24-26` -- Duplicate #include directives [LOW/build]

`<stdio.h>` and `<errno.h>` each included twice.

### Date Handler (`date_handler.c`)

#### 7.12 `date_handler.c:25-26` -- Arrays should be const [LOW/build]

`def_week_days` and `def_month_names` contain string literals but aren't declared `const`.

### CGI Manager (`cgi_manager.c`)

#### 7.13 `cgi_manager.c:295` -- Allocates/frees string on every get_val [MEDIUM/perf]

Each call does `afc_string_dup()` + `afc_string_upper()` + `afc_string_delete()`. **Fix**: convert in-place on a stack buffer for short keys.

#### 7.14 `cgi_manager.c:345` -- 16KB allocation even when headers already sent [LOW/mem]

`afc_cgi_manager_write_header()` allocates 16384 bytes before checking `headers_sent`.

### Threader (`threader.c`)

#### 7.15 `threader.c:547` -- Linear scan for mutex lookup [LOW/perf]

`afc_threader_internal_data_del_lock()` does O(n) scan for mutex pointer. **Fix**: use a hash set.

#### 7.16 `threader.c:569` -- Thread cancellation without join [LOW/bug]

`afc_threader_internal_remove_threads()` cancels threads but doesn't `pthread_join()`, potentially leaving zombie threads.

### ReadArgs (`readargs.c`)

#### 7.17 `readargs.c:326` -- Linear scan for field-by-name lookup [LOW/perf]

`afc_readargs_get_by_name()` iterates entire linked list with `strcasecmp()`. **Fix**: use dictionary for named fields.

#### 7.18 `readargs.c:522` -- Pointer-to-short cast chain [LOW/bug]

`(short)(int)(long)index(argv[t], ' ')` is undefined behavior. **Fix**: use `strchr() != NULL`.

---

## 8. Build System

### 8.1 `src/Makefile:24,37` -- `-O2` specified twice [LOW/build]

Base CFLAGS (line 24) already has `-O2`. The non-MINGW branch (line 37) adds it again.

### 8.2 Missing `-flto` (Link-Time Optimization) [MEDIUM/build]

Adding `-flto` to CFLAGS and link flags enables whole-program optimization across translation units. Particularly beneficial for a library with many small functions.

### 8.3 No header dependency tracking [MEDIUM/build]

No `.d` dependency files generated. Changing a header doesn't trigger recompilation of dependent `.c` files. **Fix**: add `-MMD -MP` to CFLAGS and `-include $(wildcard *.d)`.

### 8.4 Missing `-march=native` option [LOW/build]

For local builds, `-march=native` enables CPU-specific optimizations. Should be opt-in to preserve binary portability.

### 8.5 PCRE rebuilt unconditionally [LOW/build]

`libpcre` target runs `make` in pcre/ without checking if already up-to-date.

---

## 9. Summary: Top 20 Highest-Impact Changes

Ranked by estimated impact on real-world usage:

| # | File | Issue | Category | Priority |
|---|------|-------|----------|----------|
| 1 | `hash.c:227` | Full sort on every insertion -- makes Dictionary O(n^2 log n) for bulk inserts | perf | CRITICAL |
| 2 | `dictionary.c:786` | No hash collision resolution -- silent data corruption | bug | CRITICAL |
| 3 | `inet_client.c` | FILE* bypasses SSL -- breaks all HTTPS/TLS protocols | bug+sec | CRITICAL |
| 4 | `array.c:282` | Out-of-bounds write before resize check | bug | CRITICAL |
| 5 | `array.c:606` | memcpy on overlapping memory (undefined behavior) | bug | CRITICAL |
| 6 | `circular_list.c:393` | Wrong struct size in allocation | bug | CRITICAL |
| 7 | `avl_tree.c:122` | Insert compares value instead of key | bug | CRITICAL |
| 8 | `mem_tracker.c:255` | O(n) linear scan per free -- makes tracking O(n^2) | perf | HIGH |
| 9 | `string.c:316` | Manual byte-copy loops instead of memcpy | perf | HIGH |
| 10 | `string_list.c:1073` | Sort allocates 2 strings per O(n log n) comparisons | perf | HIGH |
| 11 | `dirmaster.c:1218` | Sort allocates 2 strings per comparison (same pattern) | perf | HIGH |
| 12 | `list.c:981` | Default sort is O(n^2 log n) due to O(n) element access | perf | HIGH |
| 13 | `inet_client.c:550` | No SSL certificate verification (MITM vulnerability) | sec | HIGH |
| 14 | `base64.c:294` | Lookup tables rebuilt on every encode/decode call | perf | HIGH |
| 15 | `regexp.c:532` | Full RegExp instance created for backreference substitution | perf | HIGH |
| 16 | `btree.c:483` | Linear search in B-tree nodes instead of binary search | perf | HIGH |
| 17 | `http_client.c:912` | Multiple TCP sends per HTTP request (one per header) | perf | HIGH |
| 18 | `list.c:721` | Clear function invoked on wrong node | bug | HIGH |
| 19 | `inet_client.c:298` | gethostbyname -- blocking, not thread-safe, no IPv6 | perf+sec | HIGH |
| 20 | `smtp.c:330` | Multi-line SMTP responses not fully parsed | bug | HIGH |
