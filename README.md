# AFC - Advanced Foundation Classes

[![License: LGPL v2.1](https://img.shields.io/badge/License-LGPL%20v2.1-blue.svg)](LICENSE)

**AFC** (Advanced Foundation Classes) is a comprehensive C library providing foundation classes for modern application development. Originally "Amiga Foundation Classes," AFC now targets Linux and open source platforms with a robust set of reusable components.

## ✨ What is AFC?

AFC is a suite of C classes designed to ease project development by providing standardized, well-tested building blocks for common programming tasks. Think of it as a foundational library that bridges the gap between raw C and higher-level languages, offering:

- **Dynamic string handling** with bounds checking
- **Data structures** (arrays, lists, trees, hash tables, dictionaries)
- **Networking** (TCP/IP clients/servers, HTTP, FTP, SMTP, POP3)
- **Database abstraction** layer (MySQL, PostgreSQL support)
- **Web development** tools (CGI manager, HTTP client)
- **File operations** and directory management
- **Threading** support
- **Memory tracking** for leak detection
- **Regular expressions** and text parsing

## 🚀 Quick Start

### Installation

#### Prerequisites

- GCC or compatible C compiler
- GNU Make
- OpenSSL development libraries (for networking features)

On Debian/Ubuntu:
```bash
sudo apt-get install build-essential libssl-dev
```

On Red Hat/Fedora:
```bash
sudo yum install gcc make openssl-devel
```

#### Build and Install

```bash
# Clone the repository
git clone https://github.com/fsoft72/afc.git
cd afc

# Build the library
make

# Install (requires root)
sudo make install
```

By default, AFC installs to `/usr/local/lib` with headers in `/usr/local/include/afc`.

### Your First AFC Program

```c
#include <afc/afc.h>

int main(void) {
    // Initialize AFC
    AFC *afc = afc_new();
    
    // Create a dynamic string
    char *str = afc_string_new(50);
    afc_string_copy(str, "Hello, AFC!", ALL);
    
    printf("String: %s\n", str);
    printf("Length: %lu\n", afc_string_len(str));
    
    // Cleanup
    afc_string_delete(str);
    afc_delete(afc);
    
    return 0;
}
```

Compile with:
```bash
gcc -o hello hello.c $(afc-config --cflags --libs)
```

## 💡 Key Features

### Standardized API

All AFC classes follow consistent naming conventions:
- `afc_<class>_new()` - Create objects
- `afc_<class>_delete()` - Destroy objects
- `afc_<class>_<action>()` - Perform operations
- `afc_<class>_set_tags()` - Configure with tag system

### Memory Safety

AFC includes built-in memory tracking to detect leaks:

```c
AFC *afc = afc_new();

// Enable memory tracking
afc_set_tags(afc, 
    AFC_TAG_SHOW_MALLOCS, (void *)TRUE,
    AFC_TAG_SHOW_FREES, (void *)TRUE,
    AFC_TAG_END);

// Your code here - all mallocs are tracked

afc_delete(afc);  // Reports any leaks
```

### Dynamic Strings

Powerful string operations with automatic memory management:

```c
char *str = afc_string_new(10);

afc_string_copy(str, "Hello", ALL);
afc_string_add(str, " World", ALL);
afc_string_upper(str);  // "HELLO WORLD"

// Format strings
afc_string_make(str, "Number: %d", 42);

// String manipulation
afc_string_trim(str);
afc_string_substr(str, 0, 5);

afc_string_delete(str);
```

### Data Structures

Rich collection of data structures:

```c
// Dynamic Arrays
Array *arr = afc_array_new();
afc_array_add(arr, my_data, AFC_ARRAY_ADD_TAIL);
void *item = afc_array_item(arr, index);

// Dictionaries (Hash Tables)
Dictionary *dict = afc_dictionary_new();
afc_dictionary_set(dict, "key", value);
void *val = afc_dictionary_get(dict, "key");

// Lists
NodeMaster *list = afc_nodemaster_new();
afc_nodemaster_add(list, data, AFC_NODEMASTER_ADD_TAIL);
afc_nodemaster_sort(list, compare_func);
```

### Networking Made Easy

```c
// HTTP Client
HttpClient *http = afc_http_client_new();
afc_http_client_get(http, "http://example.com/api/data");
char *response = afc_http_client_get_body(http);

// SMTP Email
Smtp *smtp = afc_smtp_new();
afc_smtp_connect(smtp, "smtp.example.com", 25);
afc_smtp_send_mail(smtp, "from@example.com", "to@example.com", 
                   "Subject", "Message body");

// TCP Server
InetServer *server = afc_inet_server_new();
afc_inet_server_bind(server, 8080);
afc_inet_server_listen(server, handle_connection);
```

### Database Abstraction

Work with databases through a unified API:

```c
// Connect to database
DBIManager *dbi = afc_dbi_manager_new();
afc_dbi_manager_connect(dbi, "mysql://localhost/mydb", "user", "pass");

// Execute queries
afc_dbi_manager_query(dbi, "SELECT * FROM users WHERE id = ?", user_id);

// Fetch results
while (afc_dbi_manager_fetch_row(dbi)) {
    char *name = afc_dbi_manager_get_field(dbi, "name");
    printf("User: %s\n", name);
}
```

### Tag-Based Configuration

Flexible parameter passing without breaking API compatibility:

```c
StringNode *list = afc_string_node_new();

afc_string_node_set_tags(list,
    AFC_STRING_NODE_TAG_DISCARD_ZERO_LEN, (void *)TRUE,
    AFC_STRING_NODE_TAG_CASE_SENSITIVE, (void *)FALSE,
    AFC_TAG_END);
```

### Exception Handling

C-style exception handling with cleanup guarantees:

```c
int my_function(void) {
    TRY(int)
    
    char *data = afc_malloc(size);
    if (!data) {
        RAISE(AFC_LOG_ERROR, AFC_ERR_NO_MEMORY, 
              "Failed to allocate", "allocation");
    }
    
    // Do work...
    
    RETURN(AFC_ERR_NO_ERROR);
    
    EXCEPT
        // Handle errors
    
    FINALLY
        // Always executed - cleanup here
        if (data) afc_free(data);
    
    ENDTRY
}
```

## 📚 Documentation

- **[Comprehensive API Documentation](ai/afc.md)** - Complete reference for all AFC classes
- **[Development Guidelines](ai/guidelines.md)** - Coding standards and best practices
- **[INSTALL](INSTALL)** - Detailed installation instructions
- **[CHANGES.md](CHANGES.md)** - Version history and changelog

## 🏗️ Available Classes

### Core
- **AFC** - Base object with logging and memory tracking
- **MemTracker** - Memory leak detection
- **Base64** - Base64 encoding/decoding

### Strings & Text
- **String** - Dynamic strings with bounds checking
- **StringNode** / **StringList** - Lists of strings
- **ReadArgs** - Template-based string parsing
- **RegExp** - Regular expression support

### Data Structures
- **Array** - Dynamic arrays
- **NodeMaster** / **List** - Linked lists
- **Dictionary** - Hash-based key-value store
- **Hash** - Generic hash tables
- **BTree**, **AVLTree**, **BinTree** - Tree structures
- **CircularList** - Circular buffers

### Networking (Linux only)
- **InetClient** / **InetServer** - TCP/IP networking
- **HttpClient** - HTTP client
- **FtpClient** - FTP client
- **Smtp** - SMTP email client
- **Pop3** - POP3 email client

### Database (Linux only)
- **DBIManager** - Database abstraction layer
- MySQL and PostgreSQL drivers

### File & System (Linux only)
- **FileOps** - File operations (copy, move, chmod, etc.)
- **DirMaster** - Directory scanning and management

### Web Development
- **CGIManager** - CGI application support

### Utilities
- **DateHandler** - Date manipulation
- **MD5** - MD5 hashing
- **Threader** - Multi-threading support (Linux only)
- **CmdParser** - Custom language parser (Linux only)

### Plugin System (Linux only)
- **DynamicClass** / **DynamicClassMaster** - Plugin architecture

## 🔧 Building Tests

AFC includes comprehensive tests for all modules:

```bash
# Build and run all tests for a module
cd src/test_area/string
make
./test_01

# Or from src directory
make string.test
```

## 🖥️ Platform Support

- **Linux**: Full feature support
- **MinGW/Windows**: Limited support (no networking, database, or threading)

## 📄 License

AFC is released under the **GNU Lesser General Public License v2.1** (LGPL v2.1).

This means you can:
- Use AFC in both open source and commercial projects
- Link against AFC libraries without licensing restrictions on your code
- Modify AFC itself (changes to AFC must remain LGPL)

See [LICENSE](LICENSE) for the complete license text.

## 🤝 Contributing

Contributions are welcome! Please:

1. Read the [Development Guidelines](ai/guidelines.md)
2. Follow existing code conventions
3. Write tests for new features
4. Keep changes minimal and focused
5. Update documentation for API changes

## 📧 Support

For questions, bug reports, or contributions, please open an issue on GitHub.

## 🌟 Why AFC?

- **Battle-tested**: Used in production applications
- **Well-documented**: Comprehensive documentation and examples
- **Memory-safe**: Built-in leak detection and bounds checking
- **Consistent API**: Easy to learn, predictable patterns
- **Zero dependencies**: Only standard C libraries (except OpenSSL for networking)
- **Portable**: Works on Linux and other Unix-like systems
- **LGPL licensed**: Use freely in commercial projects

---

**Start building better C applications today with AFC!** 🚀
