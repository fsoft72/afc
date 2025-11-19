# JavaScript String API Reference

**Description:** This document outlines the properties and methods of the global `String` object in JavaScript.
**Context:** Strings are immutable primitives. All string methods return a new value; they do not modify the original string.

## 1. Static Methods

_Methods called directly on the String constructor._

### `String.fromCharCode(num1, ..., numN)`

- **Description:** Returns a string created from the specified sequence of UTF-16 code units.
- **Parameters:** A sequence of numbers (0–65535).
- **Returns:** `String`

### `String.fromCodePoint(num1, ..., numN)`

- **Description:** Returns a string created from the specified sequence of code points (handles full Unicode including astral symbols).
- **Parameters:** A sequence of code points.
- **Returns:** `String`

### `String.raw(template, ...substitutions)`

- **Description:** Returns a string created from a raw template string, allowing access to raw strings as they were written without processing escape sequences.
- **Parameters:** `template` (well-formed template call site object), `...substitutions`.
- **Returns:** `String`

---

## 2. Instance Properties

### `String.prototype.length`

- **Description:** Reflects the length of the string (number of UTF-16 code units).
- **Type:** `Number` (Read-only).

---

## 3. Instance Methods

_Methods called on a string instance (e.g., `str.method()`)._

### Character Access & Codes

#### `at(index)`

- **Description:** Returns the character at the specified index. Accepts negative integers to count back from the last string character.
- **Returns:** `String` | `undefined`

#### `charAt(index)`

- **Description:** Returns the character at the specified index.
- **Returns:** `String` (Empty string if out of range).

#### `charCodeAt(index)`

- **Description:** Returns an integer between 0 and 65535 representing the UTF-16 code unit at the given index.
- **Returns:** `Number` (NaN if out of range).

#### `codePointAt(index)`

- **Description:** Returns a non-negative integer that is the Unicode code point value at the given index.
- **Returns:** `Number` | `undefined`

### Checking & Searching

#### `endsWith(searchString, endPosition?)`

- **Description:** Determines whether a string ends with the characters of a specified string.
- **Parameters:** `searchString`, `endPosition` (defaults to length).
- **Returns:** `Boolean`

#### `includes(searchString, position?)`

- **Description:** Performs a case-sensitive search to determine whether one string may be found within another.
- **Parameters:** `searchString`, `position` (start index, defaults to 0).
- **Returns:** `Boolean`

#### `indexOf(searchString, position?)`

- **Description:** Returns the index of the first occurrence of the specified value, or -1 if not found.
- **Returns:** `Number`

#### `lastIndexOf(searchString, position?)`

- **Description:** Returns the index of the last occurrence of the specified value, searching backwards from `position`.
- **Returns:** `Number`

#### `startsWith(searchString, position?)`

- **Description:** Determines whether a string begins with the characters of a specified string.
- **Returns:** `Boolean`

### Pattern Matching (RegExp)

#### `match(regexp)`

- **Description:** Retrieves the result of matching a string against a regular expression.
- **Returns:** `Array` | `null`

#### `matchAll(regexp)`

- **Description:** Returns an iterator of all results matching a string against a regular expression (must have `/g` flag).
- **Returns:** `Iterator`

#### `search(regexp)`

- **Description:** Executes a search for a match between a regular expression and this String object.
- **Returns:** `Number` (Index of match or -1).

### Manipulation & Transformation (Returns New String)

#### `concat(str1, ..., strN)`

- **Description:** Concatenates the string arguments to the calling string.
- **Returns:** `String`

#### `padEnd(targetLength, padString?)`

- **Description:** Pads the current string with a given string (repeated, if needed) so that the resulting string reaches a given length. Padding is applied from the end.
- **Returns:** `String`

#### `padStart(targetLength, padString?)`

- **Description:** Pads the current string with a given string (repeated, if needed) so that the resulting string reaches a given length. Padding is applied from the start.
- **Returns:** `String`

#### `repeat(count)`

- **Description:** Returns a new string containing the specified number of copies of the given string.
- **Returns:** `String`

#### `replace(pattern, replacement)`

- **Description:** Returns a new string with one, some, or all matches of a `pattern` replaced by a `replacement`. `pattern` can be a string or RegExp.
- **Returns:** `String`

#### `replaceAll(pattern, replacement)`

- **Description:** Returns a new string with all matches of a `pattern` replaced by a `replacement`.
- **Returns:** `String`

#### `slice(indexStart, indexEnd?)`

- **Description:** Extracts a section of a string and returns it as a new string, without modifying the original string. Supports negative indices.
- **Returns:** `String`

#### `split(separator, limit?)`

- **Description:** Divides a string into an ordered list of substrings, puts these substrings into an array, and returns the array.
- **Returns:** `Array`

#### `substring(indexStart, indexEnd?)`

- **Description:** Returns the part of the string between the start and end indexes, or to the end of the string. Swaps arguments if `start > end`.
- **Returns:** `String`

#### `toLowerCase()`

- **Description:** Returns the calling string value converted to lower case.
- **Returns:** `String`

#### `toUpperCase()`

- **Description:** Returns the calling string value converted to upper case.
- **Returns:** `String`

#### `trim()`

- **Description:** Removes whitespace from both ends of a string.
- **Returns:** `String`

#### `trimEnd()` / `trimRight()`

- **Description:** Removes whitespace from the end of a string.
- **Returns:** `String`

#### `trimStart()` / `trimLeft()`

- **Description:** Removes whitespace from the beginning of a string.
- **Returns:** `String`

### Locale & Normalization

#### `localeCompare(compareString, locales?, options?)`

- **Description:** Returns a number indicating whether a reference string comes before, or after, or is the same as the given string in sort order.
- **Returns:** `Number` (Negative, 0, or Positive)

#### `normalize(form?)`

- **Description:** Returns the Unicode Normalization Form of the string (NFC, NFD, NFKC, or NFKD).
- **Returns:** `String`

#### `toLocaleLowerCase(locales?)`

- **Description:** Returns the calling string value converted to lower case, according to any locale-specific case mappings.
- **Returns:** `String`

#### `toLocaleUpperCase(locales?)`

- **Description:** Returns the calling string value converted to upper case, according to any locale-specific case mappings.
- **Returns:** `String`

### Unicode Well-Formedness (Newer Standards)

#### `isWellFormed()`

- **Description:** Returns a boolean indicating whether this string contains any lone surrogates.
- **Returns:** `Boolean`

#### `toWellFormed()`

- **Description:** Returns a string where all lone surrogates of this string are replaced with the Unicode replacement character U+FFFD.
- **Returns:** `String`

### Utility & Primitives

#### `toString()`

- **Description:** Returns a string representing the specified object.
- **Returns:** `String`

#### `valueOf()`

- **Description:** Returns the primitive value of a String object.
- **Returns:** `String`
