# AFC Library - Comprehensive API Documentation

AFC (Advanced Foundation Classes) is a C library providing foundation classes for application development. This document serves as a comprehensive reference for using all AFC features.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Core System](#core-system)
3. [String Handling](#string-handling)
4. [Data Structures](#data-structures)
5. [Networking](#networking)
6. [Utilities](#utilities)
7. [Web Development](#web-development)
8. [Database Access](#database-access)
9. [Plugin System](#plugin-system)
10. [Threading](#threading)
11. [Error Handling](#error-handling)

---

## Getting Started

### Including AFC

```c
#include <afc/afc.h>
```

### Basic Program Structure

Every AFC program must create and destroy the base AFC object:

```c
#include <afc/afc.h>

int main(void) {
    AFC *afc = afc_new();

    // Your code here

    afc_delete(afc);
    return 0;
}
```

### Compiling and Linking

```bash
# Get compiler flags
CFLAGS=$(afc-config --cflags)

# Get linker flags
LIBS=$(afc-config --libs)

# Compile
gcc -o myprogram myprogram.c $CFLAGS $LIBS
```

### API Conventions

- **Object creation**: `afc_<class>_new()`
- **Object deletion**: `afc_<class>_delete(<obj>)`
- **Clear/reset**: `afc_<class>_clear(<obj>)`
- **Operations**: `afc_<class>_<action>(<obj>, ...)`
- **Tag-based configuration**: `afc_<class>_set_tags(<obj>, TAG1, val1, ..., AFC_TAG_END)`
- **Internal functions**: Prefixed with `_` (e.g., `_afc_string_internal_resize()`)

### Constants

- `TRUE` / `FALSE` - Boolean values
- `AFC_TAG_END` (0xDEADBEEF) - Terminates tag lists
- `ALL` (~0L) - Maximum value for lengths/counts

---

## Core System

### AFC Base Object

The foundation object that manages memory tracking and logging.

```c
// Create/destroy
AFC *afc = afc_new();
afc_delete(afc);
afc_clear(afc);  // Reset to initial state

// Configure logging
afc_set_tags(afc,
    AFC_TAG_LOG_LEVEL, (void *)AFC_LOG_WARNING,
    AFC_TAG_LOG_EXIT_CRITICAL, (void *)TRUE,
    AFC_TAG_DEBUG_LEVEL, (void *)AFC_DEBUG_STANDARD,
    AFC_TAG_SHOW_MALLOCS, (void *)TRUE,
    AFC_TAG_SHOW_FREES, (void *)TRUE,
    AFC_TAG_OUTPUT_FILE, file_handle,
    AFC_TAG_END);
```

**Log Levels**:
- `AFC_LOG_MESSAGE` (0) - Informational messages
- `AFC_LOG_NOTICE` - Notable events
- `AFC_LOG_WARNING` - Warning conditions
- `AFC_LOG_ERROR` - Error conditions
- `AFC_LOG_CRITICAL` - Critical errors (can exit)

**Debug Levels**:
- `AFC_DEBUG_NONE` - No debug output
- `AFC_DEBUG_IMPORTANT` - Only important messages
- `AFC_DEBUG_STANDARD` - Standard debug info
- `AFC_DEBUG_VERBOSE` - Verbose output
- `AFC_DEBUG_EVERYTHING` - All possible output

### Memory Management

AFC provides tracked memory allocation:

```c
// Allocate memory (tracked)
void *ptr = afc_malloc(size);

// Reallocate memory
ptr = afc_realloc(ptr, new_size);

// Free memory
afc_free(ptr);

// Enable memory tracking
MemTracker *tracker = afc_track_mallocs(afc);
```

### Error Codes

Common error codes:
- `AFC_ERR_NO_ERROR` / `AFC_NO_ERR` (0) - Success
- `AFC_ERR_NO_MEMORY` - Memory allocation failed
- `AFC_ERR_NULL_POINTER` - NULL pointer passed
- `AFC_ERR_INVALID_POINTER` - Invalid pointer type
- `AFC_ERR_UNSUPPORTED_TAG` - Unknown tag passed

### Exception Handling

AFC provides C-style exception handling macros:

```c
int my_function(void) {
    TRY(int)  // Declare return type

    if (error_condition) {
        RAISE(AFC_LOG_ERROR, AFC_ERR_NO_MEMORY, "description", "info");
        // or RAISE_FAST(error_code, "info");
    }

    RETURN(AFC_NO_ERR);

    EXCEPT
    // Handle exceptions

    FINALLY
    // Cleanup code (always runs)

    ENDTRY
}
```

**Exception Macros**:
- `TRY(type)` - Begin try block with return type
- `RAISE(level, err, descr, info)` - Raise exception with logging
- `RAISE_FAST(err, info)` - Raise exception quickly
- `RAISE_RC(level, err, descr, info, rc)` - Raise with specific return code
- `RETURN(value)` - Return from function (triggers FINALLY)
- `EXCEPT` - Exception handler block
- `FINALLY` - Cleanup block (always executes)
- `ENDTRY` - End exception handling
- `IS_EXCEPTION()` - Check if in exception state
- `EXCEPTION_TYPE()` - Get exception error code

---

## String Handling

### String Class

Dynamic strings with bounds checking and JavaScript String API compatibility.

```c
// Create/destroy
char *str = afc_string_new(100);  // Initial size
afc_string_delete(str);

// Duplicate
char *copy = afc_string_dup(original);

// Get string info
unsigned long max = afc_string_max(str);   // Max capacity
unsigned long len = afc_string_len(str);   // Current length
afc_string_reset_len(str);                 // Recalculate length

// Basic operations
afc_string_copy(dest, src, ALL);           // Copy
afc_string_add(dest, src, ALL);            // Append
afc_string_clear(dest);                    // Clear to empty

// Printf-style formatting
afc_string_make(dest, "Value: %d", value);

// Resize operations
afc_string_resize_copy(&dest, src);        // Resize and copy
afc_string_resize_add(&dest, src);         // Resize and append

// Case conversion
afc_string_upper(str);                     // To uppercase
afc_string_lower(str);                     // To lowercase

// Trimming
afc_string_trim(str);                      // Trim both ends
afc_string_trim_start(str);                // Trim leading
afc_string_trim_end(str);                  // Trim trailing

// Substring operations
afc_string_mid(dest, src, from, count);    // Extract middle
afc_string_left(dest, src, count);         // Left portion
afc_string_right(dest, src, count);        // Right portion
afc_string_slice(dest, src, begin, end);   // Slice (JS-style)

// Searching
char *pos = afc_string_instr(str, match, start);  // Find substring
long idx = afc_string_index_of(str, search, from);     // Index of (JS-style)
long idx = afc_string_last_index_of(str, search, from); // Last index

// Comparison
signed long cmp = afc_string_comp(s1, s2, len);  // Compare

// Pattern matching
int matches = afc_string_pattern_match(str, pattern, nocase);

// Replacement
afc_string_replace(dest, str, pattern, replacement);
afc_string_replace_all(dest, str, pattern, replacement);

// JS-style API
char c = afc_string_char_at(str, index);           // Character at index
int code = afc_string_char_code_at(str, index);    // Char code at index
BOOL has = afc_string_includes(str, match, pos);   // Contains substring
BOOL starts = afc_string_starts_with(str, search, pos);
BOOL ends = afc_string_ends_with(str, search, len);
afc_string_repeat(dest, str, count);               // Repeat string
afc_string_pad_start(dest, str, targetLen, pad);   // Pad start
afc_string_pad_end(dest, str, targetLen, pad);     // Pad end
char *ch = afc_string_from_char_code(code);        // Char from code

// Path operations
char *dir = afc_string_dirname(path);      // Directory name
char *base = afc_string_basename(path);    // Base filename

// File operations
char *line = afc_string_fget(str, file);   // Read line from file
char *tmp = afc_string_temp("/tmp");       // Generate temp filename

// Encoding
char *latin1 = afc_string_utf8_to_latin1(utf8_str);

// Hashing
unsigned long hash = afc_string_hash(str, turbulence);

// Number conversion
afc_string_radix(dest, number, radix);     // Number to string
```

### StringList Class

A specialized list for managing strings.

```c
// Create/destroy
StringList *sn = afc_string_list_new();
afc_string_list_delete(sn);
afc_string_list_clear(sn);

// Configure
afc_string_list_set_tags(sn,
    AFC_STRING_LIST_TAG_DISCARD_ZERO_LEN, (void *)TRUE,
    AFC_STRING_LIST_TAG_ESCAPE_CHAR, (void *)'\\',
    AFC_TAG_END);

// Add strings
afc_string_list_add(sn, "text", AFC_STRING_LIST_ADD_TAIL);
afc_string_list_add_tail(sn, "text");
afc_string_list_add_head(sn, "text");
afc_string_list_insert(sn, "text");  // At current position

// Navigation
char *s = afc_string_list_first(sn);
s = afc_string_list_next(sn);
s = afc_string_list_prev(sn);
s = afc_string_list_last(sn);
s = afc_string_list_item(sn, index);
s = afc_string_list_obj(sn);           // Current item

// Position tracking
afc_string_list_before_first(sn);      // Move before first
unsigned long pos = afc_string_list_pos(sn);
unsigned long len = afc_string_list_len(sn);
BOOL empty = afc_string_list_is_empty(sn);

// Modification
afc_string_list_change(sn, "new text");
char *removed = afc_string_list_del(sn);

// Sorting
afc_string_list_sort(sn, ascending, case_sensitive, inverted);

// Searching (Linux only)
char *found = afc_string_list_search(sn, pattern, nocase, from_start);

// Splitting
afc_string_list_split(sn, "a,b,c", ",");  // Split by delimiter

// Cloning
StringList *clone = afc_string_list_clone(sn);

// Stack operations
afc_string_list_push(sn);              // Save position
afc_string_list_pop(sn, restore);      // Restore position
afc_string_list_clear_stack(sn);
```

---

## Data Structures

### Array Class

Dynamic array with custom sorting support.

```c
// Create/destroy
Array *am = afc_array_new();
afc_array_delete(am);
afc_array_clear(am);

// Initialize with capacity
afc_array_init(am, 1000);

// Add items
afc_array_add(am, item, AFC_ARRAY_ADD_TAIL);
afc_array_add_tail(am, item);
afc_array_add_head(am, item);
afc_array_insert(am, item);  // At current position

// Navigation
void *item = afc_array_first(am);
item = afc_array_next(am);
item = afc_array_prev(am);
item = afc_array_last(am);
item = afc_array_item(am, index);
item = afc_array_obj(am);              // Current item

// Position
afc_array_before_first(am);
unsigned long pos = afc_array_pos(am);
unsigned long len = afc_array_len(am);
BOOL empty = afc_array_is_empty(am);
BOOL first = afc_array_is_first(am);
BOOL last = afc_array_is_last(am);

// Deletion
void *removed = afc_array_del(am);     // Remove current

// Sorting
int my_compare(const void *a, const void *b) {
    // Use AFC_ARRAY_SORT_ELEMENT(type, ptr) to extract values
    int val1 = AFC_ARRAY_SORT_ELEMENT(int, a);
    int val2 = AFC_ARRAY_SORT_ELEMENT(int, b);
    return val1 - val2;
}
afc_array_sort(am, my_compare);

// Custom sort function
afc_array_set_custom_sort(am, custom_qsort_func);

// Clear function for items
afc_array_set_clear_func(am, my_clear_func);

// Iteration
int callback(Array *am, int pos, void *v, void *info) {
    // Process item
    return 0;  // Return non-zero to stop
}
afc_array_for_each(am, callback, user_info);
```

### List Class (Node Master)

Generic doubly-linked list with sorting and stack operations.

```c
// Create/destroy
List *nm = afc_list_new();
afc_list_delete(nm);
afc_list_clear(nm);

// Add items
afc_list_add(nm, item, AFC_LIST_ADD_TAIL);
afc_list_add_tail(nm, item);
afc_list_add_head(nm, item);
afc_list_insert(nm, item);  // At current position

// Navigation
void *item = afc_list_first(nm);
item = afc_list_next(nm);
item = afc_list_prev(nm);
item = afc_list_last(nm);
item = afc_list_item(nm, index);
item = afc_list_obj(nm);               // Current item

// Position tracking
afc_list_before_first(nm);
unsigned long pos = afc_list_pos(nm);
unsigned long len = afc_list_len(nm);
BOOL empty = afc_list_is_empty(nm);
BOOL first = afc_list_is_first(nm);
BOOL last = afc_list_is_last(nm);

// Modification
void *removed = afc_list_del(nm);
void *old = afc_list_change(nm, new_item);
afc_list_change_numerical_pos(nm, new_pos);

// Stack operations (8 levels max)
afc_list_push(nm);                     // Save position
item = afc_list_pop(nm, restore);      // Restore position
afc_list_clear_stack(nm);

// Sorting
long compare(void *a, void *b, void *info) {
    return strcmp((char *)a, (char *)b);
}
afc_list_sort(nm, compare, user_info);
afc_list_fast_sort(nm, compare, user_info);
afc_list_ultra_sort(nm, qsort_style_compare);

// Array representation
struct Node **arr = afc_list_create_array(nm);
afc_list_free_array(nm);

// Iteration
long callback(List *nm, void *item, void *info) {
    return 0;  // Return non-zero to stop
}
afc_list_for_each(nm, callback, user_info);
```

### Hash Class

Hash table with dichotomous search for fast lookups.

```c
// Create/destroy
Hash *hm = afc_hash_new();
afc_hash_delete(hm);
afc_hash_clear(hm);

// Add/find
unsigned long hash_value = afc_string_hash(key, 0);
afc_hash_add(hm, hash_value, data);
void *data = afc_hash_find(hm, hash_value);

// Navigation
void *item = afc_hash_first(hm);
item = afc_hash_next(hm);
item = afc_hash_prev(hm);
item = afc_hash_last(hm);
afc_hash_before_first(hm);

// Access internal data
HashData *hd = afc_hash_item(hm, index);
// hd->hash_value, hd->data

// Deletion
void *removed = afc_hash_del(hm);

// Status
int len = afc_hash_len(hm);
BOOL empty = afc_hash_is_empty(hm);
BOOL first = afc_hash_is_first(hm);
BOOL last = afc_hash_is_last(hm);

// Clear function
int clear_func(Hash *hm, void *data) {
    afc_free(data);
    return 0;
}
afc_hash_set_clear_func(hm, clear_func);

// Iteration
afc_hash_for_each(hm, callback, user_info);
```

### Dictionary Class

Key-value store similar to Python dictionaries.

```c
// Create/destroy
Dictionary *dict = afc_dictionary_new();
afc_dictionary_delete(dict);
afc_dictionary_clear(dict);

// Set/get values
afc_dictionary_set(dict, "key", value);
void *val = afc_dictionary_get(dict, "key");
void *val = afc_dictionary_get_default(dict, "key", default_value);

// Check existence
BOOL exists = afc_dictionary_has_key(dict, "key");

// Find key by value
char *key = afc_dictionary_find_key(dict, value);

// Navigation
void *item = afc_dictionary_first(dict);
item = afc_dictionary_next(dict);
item = afc_dictionary_prev(dict);
afc_dictionary_before_first(dict);

// Current item access
void *val = afc_dictionary_obj(dict);
char *key = afc_dictionary_get_key(dict);

// Deletion
void *removed = afc_dictionary_del(dict);      // Remove current
afc_dictionary_del_item(dict, "key");          // Remove by key

// Count
int count = afc_dictionary_len(dict);

// Clear function
afc_dictionary_set_clear_func(dict, my_clear_func);

// Iteration
int callback(Dictionary *dict, int pos, void *val, void *info) {
    char *key = afc_dictionary_get_key(dict);
    return 0;
}
afc_dictionary_for_each(dict, callback, user_info);
```

### BinTree Class (Binary Tree)

Binary search tree implementation.

```c
// Create/destroy
BinTree *bt = afc_bin_tree_new();
afc_bin_tree_delete(bt);
afc_bin_tree_clear(bt);

// Set comparison function
int compare(void *val1, void *val2) {
    return strcmp((char *)val1, (char *)val2);
}
afc_bin_tree_set_compare_func(bt, compare);

// Set clear function
void clear_func(void *key, void *val) {
    afc_free(key);
    afc_free(val);
}
afc_bin_tree_set_clear_func(bt, clear_func);

// Insert/get/delete
afc_bin_tree_insert(bt, key, value);
void *val = afc_bin_tree_get(bt, key);
afc_bin_tree_del(bt, key);

// Status
BOOL empty = afc_bin_tree_is_empty(bt);

// Traversal
int visitor(BinTree *bt, BinTreeNode *node) {
    // node->key, node->val
    return 0;  // Return non-zero to stop
}
afc_bin_tree_traverse(bt, AFC_BIN_TREE_MODE_INORDER, visitor);
// Modes: AFC_BIN_TREE_MODE_INORDER, AFC_BIN_TREE_MODE_PREORDER, AFC_BIN_TREE_MODE_POSTORDER
```

### AVLTree Class (Self-Balancing Binary Tree)

AVL tree for balanced insertions.

```c
// Create/destroy
AVLTree *avl = afc_avl_tree_new();
afc_avl_tree_delete(avl);
afc_avl_tree_clear(avl);

// Set comparison function
avl->comp = my_compare_func;

// Set clear function
afc_avl_tree_set_clear_func(avl, clear_func);

// Insert/get
afc_avl_tree_insert(avl, key, value);
void *val = afc_avl_tree_get(avl, key);

// Find nodes
AVLNode *node = afc_avl_tree_find_node(avl, key);
AVLNode *min = afc_avl_tree_find_node_min(avl);
AVLNode *max = afc_avl_tree_find_node_max(avl);
```

### BTree Class

B-tree implementation with file persistence.

```c
// Create/destroy
BTree *btr = afc_btree_new();
afc_btree_delete(btr);
afc_btree_clear(btr);

// Initialize with order and callbacks
BOOL lower_than(void *a, void *b) { return a < b; }
BOOL equal(void *a, void *b) { return a == b; }
// Additional callbacks for serialization...

afc_btree_init(btr, order, lower_than, equal,
    create_key, write_node, read_key, read_node);

// Operations
afc_btree_add(btr, entry);
void *found = afc_btree_find(btr, entry);
afc_btree_del(btr, entry);

// File persistence
afc_btree_write(btr, "index.btree", "data.btree");
afc_btree_read(btr, "index.btree", "data.btree");

// Clear function
afc_btree_set_clear_func(btr, clear_func);
```

### Tree Class (Generic Tree)

N-ary tree structure.

```c
// Create/destroy
Tree *tree = afc_tree_new();
afc_tree_delete(tree);
afc_tree_clear(tree);

// Status
BOOL empty = afc_tree_is_empty(tree);

// Insert root
TreeNode *root = afc_tree_insert(tree, value);

// Insert children/siblings
TreeNode *child = afc_subtree_insert_child(parent, value);
TreeNode *sibling = afc_subtree_insert_sibling(brother, value);

// Delete subtree
afc_subtree_delete(node);

// Traversal
int visitor(TreeNode *node) {
    void *val = node->val;
    Tree *tree = AS_TREE(node);
    return 0;  // Return non-zero to stop
}
afc_tree_traverse(tree, AFC_TREE_MODE_LEVEL, visitor);
afc_subtree_traverse(node, AFC_TREE_MODE_PREORDER, visitor);
// Modes: AFC_TREE_MODE_LEVEL, AFC_TREE_MODE_PREORDER, AFC_TREE_MODE_POSTORDER
```

### CircularList Class

Circular doubly-linked list with fixed capacity.

```c
// Create/destroy
CircularList *cl = afc_circular_list_new();
afc_circular_list_delete(cl);
afc_circular_list_clear(cl);

// Initialize with max elements
afc_circular_list_init(cl, max_elements);

// Add items (automatically wraps)
afc_circular_list_add(cl, data);

// Navigation (circular)
void *item = afc_circular_list_next(cl);
item = afc_circular_list_prev(cl);
item = afc_circular_list_obj(cl);  // Current item

// Remove current
void *removed = afc_circular_list_del(cl);

// Clear function
afc_circular_list_set_clear_func(cl, clear_func);
```

---

## Networking

**Note**: Networking classes are Linux-only (not available with MinGW).

### InetClient Class

TCP/IP client with SSL/TLS support.

```c
// Create/destroy
InetClient *ic = afc_inet_client_new();
afc_inet_client_delete(ic);
afc_inet_client_clear(ic);

// Configure SSL/TLS
afc_inet_client_set_tags(ic,
    AFC_INET_CLIENT_TAG_USE_SSL, (void *)TRUE,     // Enable SSL from start
    AFC_INET_CLIENT_TAG_TIMEOUT, (void *)30,       // Timeout in seconds
    AFC_TAG_END);

// Connect
afc_inet_client_open(ic, "example.com", 443);

// Upgrade existing connection to TLS (STARTTLS)
afc_inet_client_start_tls(ic);

// Enable SSL on existing socket
afc_inet_client_enable_ssl(ic);

// Send/receive
afc_inet_client_send(ic, data, length);
int byte = afc_inet_client_get(ic);  // Get single byte

// Get file descriptor for advanced operations
FILE *fd = afc_inet_client_get_file(ic);

// Resolve hostname
struct hostent *host = afc_inet_client_resolve(ic, "example.com");

// Close
afc_inet_client_close(ic);
```

**Error Codes**:
- `AFC_INET_CLIENT_ERR_SOCKET` - Socket creation failed
- `AFC_INET_CLIENT_ERR_RESOLVE` - DNS resolution failed
- `AFC_INET_CLIENT_ERR_HOST_UNKNOWN` - Unknown host
- `AFC_INET_CLIENT_ERR_CONNECT` - Connection failed
- `AFC_INET_CLIENT_ERR_RECEIVE` - Receive error
- `AFC_INET_CLIENT_ERR_SEND` - Send error
- `AFC_INET_CLIENT_ERR_SSL_*` - SSL-related errors

### InetServer Class

TCP/IP server with connection management.

```c
// Create/destroy
InetServer *is = afc_inet_server_new();
afc_inet_server_delete(is);
afc_inet_server_clear(is);

// Callbacks
int on_connect(InetServer *is, InetConnData *conn) {
    printf("Client connected: fd=%d\n", conn->fd);
    return 0;
}

int on_receive(InetServer *is, InetConnData *conn) {
    char *data = conn->buf;
    // Process received data
    afc_inet_server_send(is, conn, "Response");
    return 0;
}

int on_close(InetServer *is, InetConnData *conn) {
    printf("Client disconnected\n");
    return 0;
}

// Set callbacks
is->cb_connect = on_connect;
is->cb_receive = on_receive;
is->cb_close = on_close;
is->data = user_data;  // Custom data

// Create server
afc_inet_server_create(is, 8080);

// Main loop
while (1) {
    afc_inet_server_wait(is);     // Wait for events
    afc_inet_server_process(is);  // Process events
}

// Send to specific client
afc_inet_server_send(is, conn_data, message);

// Close client connection
afc_inet_server_close_conn(is, conn_data);

// Shutdown server
afc_inet_server_close(is);
```

### SMTP Class

SMTP client with TLS/SSL and authentication support.

```c
// Create/destroy
SMTP *smtp = afc_smtp_new();
afc_smtp_delete(smtp);
afc_smtp_clear(smtp);

// Configure for Amazon SES with STARTTLS (port 587)
afc_smtp_set_tags(smtp,
    AFC_SMTP_TAG_HOST, "email-smtp.us-east-1.amazonaws.com",
    AFC_SMTP_TAG_PORT, "587",
    AFC_SMTP_TAG_USERNAME, "your-smtp-username",
    AFC_SMTP_TAG_PASSWORD, "your-smtp-password",
    AFC_SMTP_TAG_USE_TLS, (void *)TRUE,           // STARTTLS (port 587)
    AFC_SMTP_TAG_AUTH_METHOD, (void *)AFC_SMTP_AUTH_LOGIN,
    AFC_TAG_END);

// Alternative: Direct SSL (port 465)
afc_smtp_set_tag(smtp, AFC_SMTP_TAG_USE_SSL, (void *)TRUE);

// Connect and authenticate
afc_smtp_connect(smtp);
afc_smtp_authenticate(smtp);

// Send email (simple)
afc_smtp_send_simple(smtp,
    "sender@example.com",
    "recipient@example.com",
    "Subject Line",
    "Email body content");

// Send email (advanced - raw message)
afc_smtp_send(smtp, raw_email_message);

// Disconnect
afc_smtp_quit(smtp);
```

**Authentication Methods**:
- `AFC_SMTP_AUTH_NONE` - No authentication
- `AFC_SMTP_AUTH_PLAIN` - AUTH PLAIN
- `AFC_SMTP_AUTH_LOGIN` - AUTH LOGIN

### POP3 Class

POP3 email client.

```c
// Create/destroy
POP3 *pop3 = afc_pop3_new();
afc_pop3_delete(pop3);
afc_pop3_clear(pop3);

// Configure
afc_pop3_set_tags(pop3,
    AFC_POP3_TAG_HOST, "mail.example.com",
    AFC_POP3_TAG_PORT, "110",
    AFC_POP3_TAG_LOGIN, "username",
    AFC_POP3_TAG_PASSWD, "password",
    AFC_TAG_END);

// Connect and login
afc_pop3_connect(pop3);
afc_pop3_login(pop3);

// Get mailbox status
afc_pop3_stat(pop3);
int total = pop3->tot_messages;
int size = pop3->tot_size;

// Get message list
afc_pop3_get_list(pop3);
// Access via pop3->msg hash

// Retrieve message
afc_pop3_retr(pop3, msg_number);
// Message content in pop3->sn StringList

// Get message headers
afc_pop3_top(pop3, msg_number, lines);

// Delete message
afc_pop3_dele(pop3, msg_number);

// Other commands
afc_pop3_noop(pop3);  // Keep connection alive
afc_pop3_rset(pop3);  // Unmark deleted messages

// Disconnect
afc_pop3_quit(pop3);
```

### HttpClient Class

HTTP client with redirect support.

```c
// Create/destroy
HttpClient *hc = afc_http_client_new();
afc_http_client_delete(hc);
afc_http_client_clear(hc);

// Configure
afc_http_client_set_tags(hc,
    AFC_HTTP_CLIENT_TAG_TIMEOUT, (void *)30,
    AFC_HTTP_CLIENT_TAG_FOLLOW_REDIRECTS, (void *)TRUE,
    AFC_HTTP_CLIENT_TAG_MAX_REDIRECTS, (void *)5,
    AFC_HTTP_CLIENT_TAG_USE_SSL, (void *)TRUE,
    AFC_TAG_END);

// Set request headers
afc_http_client_set_header(hc, "User-Agent", "MyApp/1.0");
afc_http_client_set_header(hc, "Accept", "application/json");

// HTTP methods
afc_http_client_get(hc, "https://api.example.com/data");
afc_http_client_post(hc, url, body, body_len);
afc_http_client_put(hc, url, body, body_len);
afc_http_client_patch(hc, url, body, body_len);
afc_http_client_delete_url(hc, url);
afc_http_client_head(hc, url);
afc_http_client_options(hc, url);

// Generic request
afc_http_client_request(hc, "CUSTOM", url, body, body_len);

// Get response
int status = afc_http_client_get_status_code(hc);
char *status_msg = afc_http_client_get_status_message(hc);
char *body = afc_http_client_get_response_body(hc);
int body_len = afc_http_client_get_response_body_len(hc);

// Get response headers
Dictionary *headers = afc_http_client_get_response_headers(hc);
char *content_type = afc_http_client_get_response_header(hc, "Content-Type");

// Clear request headers
afc_http_client_clear_headers(hc);

// Close connection
afc_http_client_close(hc);
```

### FtpClient Class

FTP client.

```c
// Create/destroy
FtpClient *fc = afc_ftp_client_new();
afc_ftp_client_delete(fc);
afc_ftp_client_clear(fc);

// Properties
int code = fc->last_code;        // Last response code
char *answer = fc->last_answer;  // Last response
BOOL passive = fc->pasv;         // Passive mode flag

// Callback types for data transfer
typedef int (*afc_ftp_client_retr_callback)(u_char *data, int len, void *param);
typedef int (*afc_ftp_client_store_callback)(u_char *data, int *len, void *param);
```

---

## Utilities

### MD5 Class

MD5 hash computation.

```c
// Create/destroy
MD5 *md5 = afc_md5_new();
afc_md5_delete(md5);
afc_md5_clear(md5);

// Hash data
unsigned char data[] = "Hello, World!";
afc_md5_update(md5, data, strlen(data));

// Get result
const char *hash = afc_md5_digest(md5);  // Returns hex string

// Hash file
afc_md5_encode_file(md5, "/path/to/file");
const char *file_hash = afc_md5_digest(md5);
```

### Base64 Class

Base64 encoding/decoding.

```c
// Create/destroy
Base64 *b64 = afc_base64_new();
afc_base64_delete(b64);

// Memory-to-memory encoding
char *input = "Hello, World!";
char *output = afc_malloc(1024);

afc_base64_set_tag(b64, AFC_BASE64_TAG_MEM_IN, input);
afc_base64_set_tag(b64, AFC_BASE64_TAG_MEM_IN_SIZE, (void *)strlen(input));
afc_base64_set_tag(b64, AFC_BASE64_TAG_MEM_OUT, output);
afc_base64_set_tag(b64, AFC_BASE64_TAG_MEM_OUT_SIZE, (void *)1024);

afc_base64_encode(b64, AFC_TAG_END);

// Memory-to-memory decoding
afc_base64_decode(b64, AFC_TAG_END);

// File-to-file operations
afc_base64_encode(b64,
    AFC_BASE64_TAG_FILE_IN, "input.bin",
    AFC_BASE64_TAG_FILE_OUT, "output.b64",
    AFC_TAG_END);

afc_base64_decode(b64,
    AFC_BASE64_TAG_FILE_IN, "output.b64",
    AFC_BASE64_TAG_FILE_OUT, "decoded.bin",
    AFC_TAG_END);

// Write results to file
afc_base64_fwrite(b64, "output.txt", AFC_BASE64_OUT);
```

### DateHandler Class

Date validation and arithmetic.

```c
// Create/destroy
DateHandler *dh = afc_date_handler_new();
afc_date_handler_delete(dh);
afc_date_handler_clear(dh);

// Set date
afc_date_handler_set(dh, 2025, 1, 15);
afc_date_handler_set_today(dh);
afc_date_handler_set_julian(dh, julian_day);

// Validate date
BOOL valid = afc_date_handler_is_valid(dh, year, month, day);

// Get properties
int dow = afc_date_handler_get_day_of_week(dh);  // 0=Sunday
long julian = afc_date_handler_get_julian(dh);

// Arithmetic
afc_date_handler_add_days(dh, 30);   // Add 30 days
afc_date_handler_add_days(dh, -7);   // Subtract 7 days

// Format to string
char buf[32];
afc_date_handler_to_string(dh, buf, AFC_DATE_HANDLER_MODE_FULL);
// Modes:
// AFC_DATE_HANDLER_MODE_FULL
// AFC_DATE_HANDLER_MODE_YYYYMMDD
// AFC_DATE_HANDLER_MODE_MMDDYYYY
// AFC_DATE_HANDLER_MODE_DDMMYYYY
// AFC_DATE_HANDLER_MODE_TEXT
```

### RegExp Class

Regular expressions with PCRE.

```c
// Create/destroy
RegExp *re = afc_regexp_new();
afc_regexp_delete(re);
afc_regexp_clear(re);

// Set options
afc_regexp_set_options(re,
    AFC_REGEXP_OPT_NOCASE |        // Case insensitive
    AFC_REGEXP_OPT_MULTILINE);     // Multiline mode

// Options:
// AFC_REGEXP_OPT_NOCASE - Case insensitive
// AFC_REGEXP_OPT_DOLLAR_END - $ matches only end
// AFC_REGEXP_OPT_DOT_NEWLINE - . matches newlines
// AFC_REGEXP_OPT_EXTENDED - Ignore whitespace
// AFC_REGEXP_OPT_MULTILINE - Multiline mode

// Compile pattern
afc_regexp_compile(re, "\\b(\\w+)@(\\w+\\.\\w+)\\b");

// Match
int result = afc_regexp_match(re, "Email: test@example.com", 0);
if (result == AFC_NO_ERR) {
    int num_matches = re->matches;

    // Get matched substrings
    char sub[256];
    afc_regexp_get_sub_string(re, sub, 0);  // Full match
    afc_regexp_get_sub_string(re, sub, 1);  // First group
    afc_regexp_get_sub_string(re, sub, 2);  // Second group

    // Get positions
    RegExpPos pos;
    afc_regexp_get_pos(re, 0, &pos);
    // pos.start, pos.end
}

// Replace
afc_regexp_set_buffer(re, 8192);  // Set output buffer size
afc_regexp_replace(re, dest, source, pattern, replacement, replace_all);

// Compute replace size (for buffer allocation)
int size = afc_regexp_compute_replace_size(re, str, pattern, replacement, TRUE);
```

### ReadArgs Class

Command-line and string parsing.

```c
// Create/destroy
ReadArgs *ra = afc_readargs_new();
afc_readargs_delete(ra);
afc_readargs_clear(ra);

// Parse template string
// Template format: NAME/A,SIZE/K/N,FORCE/S,FILES/M
// /A = Required, /K = Keyword required, /N = Numeric, /S = Switch, /M = Multi
afc_readargs_parse(ra, "NAME/A,SIZE/K/N,FORCE/S", "myfile SIZE=100 FORCE");

// Parse command line
afc_readargs_parse_cmd_line(ra, "INPUT/A,OUTPUT/A,VERBOSE/S", argc, argv);

// Get values
char *name = afc_readargs_get_by_name(ra, "NAME");
void *val = afc_readargs_get_by_pos(ra, 0);

// For numeric values, cast appropriately
long size = (long)afc_readargs_get_by_name(ra, "SIZE");

// For switches, check for non-NULL
BOOL force = afc_readargs_get_by_name(ra, "FORCE") != NULL;
```

**Template Modifiers**:
- `/A` - Required argument
- `/K` - Keyword must be present
- `/N` - Numeric value
- `/S` - Switch (boolean flag)
- `/M` - Multiple values

### FileOperations Class

File system operations (mostly Linux-only).

```c
// Create/destroy
FileOperations *fo = afc_fileops_new();
afc_fileops_delete(fo);
afc_fileops_clear(fo);

// Configure (Linux only)
afc_fileops_set_tags(fo,
    AFC_FILEOPS_TAG_BLOCK_CHOWN, (void *)FALSE,
    AFC_FILEOPS_TAG_BLOCK_CHMOD, (void *)TRUE,
    AFC_FILEOPS_TAG_BLOCK_UTIME, (void *)TRUE,
    AFC_TAG_END);

// Check existence
BOOL exists = afc_fileops_exists(fo, "/path/to/file");
int type = afc_fileops_exists_full(fo, path, name);  // Linux only

// Delete
afc_fileops_del(fo, "/path/to/file");

// Linux-only operations
afc_fileops_mkdir(fo, "/path/to/dir");
afc_fileops_copy(fo, source, dest);
afc_fileops_move(fo, source, dest);
afc_fileops_link(fo, source, dest);
afc_fileops_chmod(fo, path, mode);
afc_fileops_chown(fo, path, uid, gid);
afc_fileops_utime(fo, path, &utimbuf);

// Get last error
int err = fo->last_error;
```

### DirMaster Class (Linux only)

Directory scanning and file information.

```c
// Create/destroy
DirMaster *dm = afc_dirmaster_new();
afc_dirmaster_delete(dm);
afc_dirmaster_clear(dm);

// Configure
afc_dirmaster_set_tags(dm,
    AFC_DIRMASTER_TAG_DATE_FORMAT, (void *)DATEFORMAT_DD_MM_YYYY,
    AFC_DIRMASTER_TAG_SIZE_FORMAT, (void *)SIZEFORMAT_HUMAN,
    AFC_DIRMASTER_TAG_SIZE_DECIMALS, (void *)2,
    AFC_DIRMASTER_TAG_CONV_DATE_MODIFY, (void *)TRUE,
    AFC_DIRMASTER_TAG_CONV_USER, (void *)TRUE,
    AFC_DIRMASTER_TAG_CONV_GROUP, (void *)TRUE,
    AFC_DIRMASTER_TAG_CONV_MODE, (void *)TRUE,
    AFC_TAG_END);

// Scan directory
afc_dirmaster_scan_dir(dm, "/home/user");

// Navigation
FileInfo *fi = afc_dirmaster_first(dm);
while (fi) {
    printf("%s (%s) - %s\n", fi->name, fi->csize, fi->cmodify);
    fi = afc_dirmaster_next(dm);
}

// Access FileInfo fields
// fi->name      - Filename
// fi->cmode     - Mode string (e.g., "-rwxr-xr-x")
// fi->cuser     - User name
// fi->cgroup    - Group name
// fi->caccess   - Access date string
// fi->cmodify   - Modify date string
// fi->cchange   - Change date string
// fi->csize     - Size string
// fi->size      - Size in bytes
// fi->kind      - FINFO_KIND_FILE, FINFO_KIND_DIR, FINFO_KIND_LINK
// fi->hidden    - Is hidden file
// fi->selected  - Selection flag
// fi->st        - Raw stat structure

// Random access
fi = afc_dirmaster_item(dm, index);

// Search
fi = afc_dirmaster_search(dm, "filename", TRUE);  // nocase=TRUE

// Sort
afc_dirmaster_sort(dm,
    AFC_DIRMASTER_TAG_SORT_FIELD, (void *)FINFO_NAME,
    AFC_DIRMASTER_TAG_SORT_CASE_INSENSITIVE, (void *)TRUE,
    AFC_DIRMASTER_TAG_SORT_INVERTED, (void *)FALSE,
    AFC_TAG_END);

// Get parent directory
char parent[PATH_MAX];
afc_dirmaster_get_parent(dm, parent);

// Add custom item
afc_dirmaster_add_item(dm, path, name, stat_struct);

// Delete current item
afc_dirmaster_del(dm);

// Status
BOOL empty = afc_dirmaster_is_empty(dm);
unsigned long count = afc_dirmaster_len(dm);
unsigned long pos = afc_dirmaster_pos(dm);
```

---

## Web Development

### CGIManager Class

CGI application helper for web forms and cookies.

```c
// Create/destroy
CGIManager *cgi = afc_cgi_manager_new();
afc_cgi_manager_delete(cgi);
afc_cgi_manager_clear(cgi);

// Configure
afc_cgi_manager_set_tag(cgi, AFC_CGI_MANAGER_TAG_HANDLE_COOKIES, (void *)TRUE);

// Parse request
afc_cgi_manager_get_data(cgi);

// Check request method
if (cgi->method == AFC_CGI_MANAGER_METHOD_GET) {
    // Handle GET
} else if (cgi->method == AFC_CGI_MANAGER_METHOD_POST) {
    // Handle POST
}

// Get form values
char *name = afc_cgi_manager_get_val(cgi, "username");
char *email = afc_cgi_manager_get_val(cgi, "email");

// Cookie handling
afc_cgi_manager_set_cookie_domain(cgi, ".example.com");
afc_cgi_manager_set_cookie_path(cgi, "/");
afc_cgi_manager_set_cookie_expire(cgi, 3600);  // Seconds

afc_cgi_manager_set_cookie(cgi, "session_id", "abc123");
char *session = afc_cgi_manager_get_cookie(cgi, "session_id");

// Set content type
afc_cgi_manager_set_content_type(cgi, "text/html; charset=utf-8");

// Set default headers
afc_cgi_manager_set_default_headers(cgi);

// Write headers
afc_cgi_manager_write_header(cgi);

// Get headers as string (for debugging)
char header_buf[4096];
afc_cgi_manager_get_header_str(cgi, header_buf);

// Debug dump
afc_cgi_manager_debug_dump(cgi);

// Access request charset
char *charset = cgi->charset;
```

---

## Database Access

### DBIManager Class (Linux only)

Database abstraction layer with plugin drivers.

```c
// Create/destroy
DBIManager *dbi = afc_dbi_manager_new();
afc_dbi_manager_delete(dbi);
afc_dbi_manager_clear(dbi);

// Create database instance
DynamicClass *dc = afc_dbi_manager_new_instance(dbi, "mysql", "mysql.so");
// or
DynamicClass *dc = afc_dbi_manager_new_instance(dbi, "postgresql", "postgresql.so");

// Initialize
DBI_INIT(dc);

// Connect
DBI_CONNECT(dc, "localhost", "mydb", "user", "password");

// Execute query
DBI_QUERY(dc, "SELECT * FROM users WHERE id = 1");

// Get result info
unsigned long num_cols = DBI_NUM_COLS(dc);
unsigned long num_rows = DBI_NUM_ROWS(dc);

// Fetch rows
while (DBI_FETCH(dc) == AFC_NO_ERR) {
    // Access row data via dynamic class variables
    char *name = DB_GETV_P(dc, "name");
    unsigned long id = DB_GETV_N(dc, "id");
}

// Free result
DBI_FREE(dc);

// Close connection
DBI_CLOSE(dc);

// Delete instance
afc_dbi_manager_delete_instance(dbi, dc);
```

**Available Drivers**:
- `mysql.so` - MySQL driver
- `postgresql.so` - PostgreSQL driver

---

## Plugin System

### DynamicClass Class (Linux only)

Plugin client interface.

```c
// Create/destroy
DynamicClass *dc = afc_dynamic_class_new();
afc_dynamic_class_delete(dc);
afc_dynamic_class_clear(dc);

// Add methods
int my_method(DynamicClass *dc) {
    // Access arguments via dc->args Array
    char *arg1 = afc_array_item(dc->args, 0);

    // Set result
    dc->result = "success";
    dc->result_type = AFC_DYNAMIC_CLASS_RESULT_TYPE_STRING;

    return AFC_NO_ERR;
}

afc_dynamic_class_add_method(dc, "my_method", "param1,param2", my_method);

// Execute method
afc_dynamic_class_execute(dc, "my_method", "arg1", "arg2");

// Find method
DynamicClassMethodData *method = afc_dynamic_class_find_method(dc, "my_method");

// Set/get variables
afc_dynamic_class_set_var(dc, AFC_DYNAMIC_CLASS_VAR_KIND_STRING, "name", "value");
afc_dynamic_class_set_var(dc, AFC_DYNAMIC_CLASS_VAR_KIND_NUM, "count", (void *)42);
afc_dynamic_class_set_var(dc, AFC_DYNAMIC_CLASS_VAR_KIND_POINTER, "data", ptr);

char *name = afc_dynamic_class_get_var(dc, "name");

// Use info pointer
dc->info = my_data;
```

### DynamicClassMaster Class (Linux only)

Plugin host and loader.

```c
// Create/destroy
DynamicClassMaster *dcm = afc_dynamic_class_master_new();
afc_dynamic_class_master_delete(dcm);
afc_dynamic_class_master_clear(dcm);

// Configure
afc_dynamic_class_master_set_tags(dcm,
    AFC_DYNAMIC_CLASS_MASTER_TAG_INFO, user_data,
    AFC_DYNAMIC_CLASS_MASTER_TAG_CHECK_PARAMS, (void *)TRUE,
    AFC_TAG_END);

// Load plugin from shared library
afc_dynamic_class_master_load(dcm, "my_plugin", "/path/to/plugin.so");

// Check if class exists
BOOL exists = afc_dynamic_class_master_has_class(dcm, "my_plugin");

// Get plugin info
char *name = afc_dynamic_class_master_get_info(dcm, "my_plugin",
    AFC_DYNAMIC_CLASS_MASTER_INFO_NAME);
char *version = afc_dynamic_class_master_get_info(dcm, "my_plugin",
    AFC_DYNAMIC_CLASS_MASTER_INFO_VERSION);
char *author = afc_dynamic_class_master_get_info(dcm, "my_plugin",
    AFC_DYNAMIC_CLASS_MASTER_INFO_AUTHOR);

// Create instance
DynamicClass *instance = afc_dynamic_class_master_new_instance(dcm, "my_plugin");

// Use instance
afc_dynamic_class_execute(instance, "method_name", arg1, arg2);

// Delete instance
afc_dynamic_class_master_delete_instance(dcm, instance);

// Add plugin programmatically
afc_dynamic_class_master_add(dcm, "class_name", dl_handle,
    new_instance_func, del_instance_func, info_func);
```

---

## Threading

### Threader Class (Linux only)

Thread management with mutex support.

```c
// Create/destroy
Threader *th = afc_threader_new();
afc_threader_delete(th);
afc_threader_clear(th);

// Thread function
void *my_thread(void *arg) {
    ThreaderData *td = (ThreaderData *)arg;
    Threader *th = td->th;
    void *user_data = td->info;

    // Enable cancellation
    AFC_THREADER_CANCEL_ENABLE(td);
    AFC_THREADER_CANCEL_DEFERRED(td);  // or AFC_THREADER_CANCEL_ASYNC(td)

    // Lock a mutex
    afc_threader_thread_lock(td, "my_lock", TRUE);  // wait=TRUE

    // Critical section
    // ...

    // Unlock
    afc_threader_thread_unlock(td, "my_lock");

    return NULL;
}

// Add and start thread
afc_threader_add(th, "worker1", my_thread, user_data);
afc_threader_add(th, "worker2", my_thread, user_data);

// Wait for all threads
afc_threader_wait(th);

// Cancel specific thread
afc_threader_cancel(th, "worker1");
```

**Thread Macros**:
- `AFC_THREADER_CANCEL_ENABLE(td)` - Allow thread cancellation
- `AFC_THREADER_CANCEL_DISABLE(td)` - Prevent cancellation
- `AFC_THREADER_CANCEL_ASYNC(td)` - Cancel immediately
- `AFC_THREADER_CANCEL_DEFERRED(td)` - Cancel at cancellation points

---

## Error Handling

### Standard Error Codes

All AFC classes use consistent error codes:

```c
// Base errors (defined in base.h)
AFC_ERR_NO_ERROR        // 0 - Success
AFC_NO_ERR              // 0 - Alias for NO_ERROR
AFC_ERR_NO_MEMORY       // Memory allocation failed
AFC_ERR_NULL_POINTER    // NULL pointer passed
AFC_ERR_INVALID_POINTER // Invalid pointer type
AFC_ERR_UNSUPPORTED_TAG // Unknown configuration tag

// Class-specific errors use BASE + offset
// Example: AFC_INET_CLIENT_ERR_SOCKET = AFC_INET_CLIENT_BASE + 1
```

### Checking Errors

```c
int result = afc_some_function(obj, params);
if (result != AFC_NO_ERR) {
    // Handle error
    char *error_msg = __internal_afc_base->last_error;
    printf("Error: %s\n", error_msg);
}
```

### Using AFC_LOG Macros

```c
static const char class_name[] = "MyClass";

// Log with full details
AFC_LOG(AFC_LOG_ERROR, AFC_ERR_NO_MEMORY, "Memory allocation failed", "at line 42");

// Quick logging
AFC_LOG_FAST(AFC_ERR_NO_MEMORY);
AFC_LOG_FAST_INFO(AFC_ERR_NO_MEMORY, "additional info");

// Access last error
char *last_error = AFC_STR_ERROR();
```

---

## Platform Notes

### Linux-Only Features

The following features are not available on MinGW/Windows:

- **Networking**: InetClient, InetServer, SMTP, POP3, HttpClient, FtpClient
- **Database**: DBIManager, MySQL/PostgreSQL drivers
- **Plugin System**: DynamicClass, DynamicClassMaster
- **Threading**: Threader
- **File System**: DirMaster, most FileOperations functions
- **Parsing**: CmdParser
- **String**: Pattern matching (`afc_string_pattern_match`)

### MinGW Build

To build for Windows/MinGW:

```bash
make DEFINE=MINGW mingw_all
```

Available on MinGW:
- Core base system
- String handling
- Data structures (Array, List, Hash, Dictionary, Trees)
- Utilities (MD5, Base64, DateHandler)
- Basic file operations

---

## Complete Example

```c
#include <afc/afc.h>

int main(void) {
    // Initialize AFC
    AFC *afc = afc_new();

    // Create a dictionary
    Dictionary *config = afc_dictionary_new();
    afc_dictionary_set(config, "host", "localhost");
    afc_dictionary_set(config, "port", "8080");

    // Create a string list
    StringList *items = afc_string_list_new();
    afc_string_list_add_tail(items, "apple");
    afc_string_list_add_tail(items, "banana");
    afc_string_list_add_tail(items, "cherry");

    // Sort and iterate
    afc_string_list_sort(items, TRUE, TRUE, FALSE);

    char *item = afc_string_list_first(items);
    while (item) {
        printf("- %s\n", item);
        item = afc_string_list_next(items);
    }

    // Compute MD5
    MD5 *md5 = afc_md5_new();
    afc_md5_update(md5, (unsigned char *)"Hello", 5);
    printf("MD5: %s\n", afc_md5_digest(md5));

    // Date handling
    DateHandler *dh = afc_date_handler_new();
    afc_date_handler_set_today(dh);
    afc_date_handler_add_days(dh, 30);

    char date_str[32];
    afc_date_handler_to_string(dh, date_str, AFC_DATE_HANDLER_MODE_DDMMYYYY);
    printf("In 30 days: %s\n", date_str);

    // Cleanup
    afc_md5_delete(md5);
    afc_date_handler_delete(dh);
    afc_string_list_delete(items);
    afc_dictionary_delete(config);
    afc_delete(afc);

    return 0;
}
```

---

## Memory Management Best Practices

1. **Always pair new/delete**: Every `afc_<class>_new()` must have a corresponding `afc_<class>_delete()`

2. **Use clear functions**: For containers holding allocated data, set a clear function:
   ```c
   int my_clear(void *data) {
       afc_free(data);
       return 0;
   }
   afc_array_set_clear_func(am, my_clear);
   ```

3. **Enable memory tracking** during development:
   ```c
   afc_set_tags(afc,
       AFC_TAG_SHOW_MALLOCS, (void *)TRUE,
       AFC_TAG_SHOW_FREES, (void *)TRUE,
       AFC_TAG_END);
   ```

4. **Use afc_malloc/afc_free** instead of malloc/free for tracking support

---

## License

AFC is released under GNU LGPL v2.1. Commercial licenses are available from the author.
