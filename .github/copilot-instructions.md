# GitHub Copilot Instructions for AFC

## Project Overview

AFC (Advanced Foundation Classes) is a C library providing foundation classes for application development. Originally "Amiga Foundation Classes," it now targets Linux and open source platforms.

**Important**: Before working on this repository, read the [Development Guidelines](../ai/guidelines.md) and [API Documentation](../ai/afc.md). For Claude Code users, also refer to [CLAUDE.md](../CLAUDE.md).

## Key Characteristics

- **Language**: Pure C (C89/C99)
- **Platform**: Linux primary, MinGW/Windows limited support
- **License**: GNU LGPL v2.1
- **Architecture**: Tag-based API system with standardized naming conventions

## Coding Standards

### Naming Conventions

- **Public APIs**: `afc_<class>_<action>` (e.g., `afc_string_copy`, `afc_array_add`)
- **Internal functions**: Prefix with `_` (e.g., `_afc_string_internal_resize`)
- **Constants**: UPPERCASE with underscores (e.g., `AFC_TAG_END`, `AFC_MAGIC`)
- **Structs**: PascalCase for typedef'd types (e.g., `String`, `Array`, `Dictionary`)

### API Patterns

All AFC classes follow these patterns:
- Object creation: `afc_<class>_new()`
- Object deletion: `afc_<class>_delete(<obj>)`
- Clear/reset: `afc_<class>_clear(<obj>)`
- Operations: `afc_<class>_<action>(<obj>, ...)`
- Configuration: `afc_<class>_set_tags(<obj>, TAG1, val1, ..., AFC_TAG_END)`

### Memory Management

- **Always use** `afc_malloc()` and `afc_free()` wrappers (not raw `malloc`/`free`)
- Memory tracking is built-in for leak detection
- Every `afc_<class>_new()` MUST have a corresponding `afc_<class>_delete()`
- Set clear functions for containers: `afc_<class>_set_clear_func(obj, func)`

### Error Handling

- Functions return error codes from `AFC_ERR_*` enum
- Success is `AFC_ERR_NO_ERROR` or `AFC_NO_ERR` (0)
- Use exception handling macros: `TRY(type)`, `RAISE()`, `RETURN()`, `EXCEPT`, `FINALLY`, `ENDTRY`
- Never silently swallow errors

### Magic Numbers

Each class validates objects using magic numbers (e.g., `AFC_MAGIC`, `AFC_STRING_MAGIC`). Always check magic numbers in functions that accept object pointers.

## Build System

### Standard Build Commands

```bash
# Build library
make

# Install (requires root)
make install

# Clean build
make clean

# Build MinGW version
make DEFINE=MINGW mingw_all

# Build C++ bindings
make cpp
make cpp_install
```

### Testing

Individual module tests are in `src/test_area/<module>/`:

```bash
# Build all tests for a module
cd src/test_area/string
make

# Build specific test
make test_01

# Run test
./test_01

# Build with leak detection
make leak
```

From `src/` directory:
```bash
make <class>.test  # e.g., make string.test, make array.test
```

## Platform Differences

### Linux-Only Features

These features are NOT available on MinGW/Windows:
- Networking (InetClient, InetServer, SMTP, POP3, HttpClient, FtpClient)
- Database (DBIManager, MySQL/PostgreSQL drivers)
- Plugin System (DynamicClass, DynamicClassMaster)
- Threading (Threader)
- File System (DirMaster, most FileOperations functions)
- Command Parser (CmdParser)

Use `#ifndef MINGW` preprocessor directives to guard Linux-only code.

## Dependencies

### Required

- **GCC or compatible C compiler**
- **GNU Make**
- **OpenSSL** (for SSL/TLS support in networking classes)
  ```bash
  # Debian/Ubuntu
  sudo apt-get install libssl-dev
  
  # Red Hat/Fedora
  sudo yum install openssl-devel
  ```

### Optional

- **MySQL development libraries** (for MySQL DBI driver)
- **PostgreSQL development libraries** (for PostgreSQL DBI driver)
- **pthread library** (for threading support)

## Code Style

- **Indentation**: Tabs (not spaces)
- **Braces**: K&R style (opening brace on same line)
- **Comments**: Only when necessary; prefer self-documenting code
- **Line endings**: Unix (LF)
- **File endings**: Always end files with a newline

## Important Conventions

### Tag System

Functions accept tag-based parameters terminated with `AFC_TAG_END`:

```c
afc_string_list_set_tags(sn,
    AFC_STRING_LIST_TAG_DISCARD_ZERO_LEN, (void *)TRUE,
    AFC_STRING_LIST_TAG_ESCAPE_CHAR, (void *)'\\',
    AFC_TAG_END);
```

### Boolean Values

Use `TRUE` and `FALSE` constants (defined in `base.h`), not `1` and `0`.

### Test Pattern

Many `.c` files have embedded test code:

```c
#ifdef TEST_CLASS
int main() {
    // Test code here
}
#endif
```

Build with `-DTEST_CLASS` flag to enable.

## Security Considerations

- **Never commit secrets** or credentials
- **Validate all input** from external sources (network, files, user input)
- **Check buffer boundaries** when using string operations
- **Use afc_malloc/afc_free** for memory tracking
- **Sanitize data** before passing to external systems
- **Close file descriptors and sockets** after use

## Contributing Guidelines

### Before Starting

1. Read [ai/guidelines.md](../ai/guidelines.md) for development philosophy
2. Study existing similar features in the codebase
3. Identify patterns and conventions used
4. Plan incrementally - small changes that compile and test

### Making Changes

1. **Learn from existing code**: Find similar implementations
2. **Use existing tools**: Don't introduce new dependencies without justification
3. **Test incrementally**: Build and test after each change
4. **Update documentation**: If changing public APIs, update relevant docs
5. **Never remove working tests**: Fix them instead
6. **Follow conventions**: Match the style of surrounding code

### Pull Requests

- Make minimal, surgical changes
- Include clear commit messages
- Update CHANGES.md for user-visible changes
- Ensure all tests pass before submitting
- Address code review feedback promptly

## Common Tasks

### Adding a New Class

1. Create `<class>.h` and `<class>.c` in `src/`
2. Define struct and magic number
3. Implement standard functions: `new`, `delete`, `clear`
4. Add to `src/Makefile`
5. Include in `src/afc.h`
6. Write tests in `src/test_area/<class>/`
7. Document in `ai/afc.md`

### Adding a New Function to Existing Class

1. Add prototype to `<class>.h`
2. Implement in `<class>.c`
3. Follow naming: `afc_<class>_<action>`
4. Document parameters and return values in comments
5. Add test case in `src/test_area/<class>/`

### Fixing a Bug

1. Write a test that reproduces the bug
2. Fix the bug with minimal changes
3. Verify the test now passes
4. Run all related tests to ensure no regression

## Resources

- **Main Documentation**: `ai/afc.md` - Complete API reference
- **Development Guidelines**: `ai/guidelines.md` - Philosophy and standards
- **Claude Instructions**: `CLAUDE.md` - Claude Code specific guidance
- **Changes**: `CHANGES.md` - User-visible changes log
- **Installation**: `INSTALL` - Build and installation instructions
- **License**: `LICENSE` - LGPL v2.1 license text

## Getting Help

- Check existing implementations in the codebase
- Review test files in `src/test_area/` for usage examples
- Consult `ai/afc.md` for API documentation
- Study `ai/guidelines.md` for development principles
