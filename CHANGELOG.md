# Change Log

## 14 Jan 2011

### FIX: CGIManager

**Status:** STABLE

- Fix parsing of GET params with escaped chars.

## 06 Apr 2007

### FIX: RegExp

**Status:** STABLE

- `afc_regexp_replace()` no longer returns an empty string if the
  given regex pattern is not found on the source string.

### ADD: Array / List / StringList

**Status:** STABLE

- Added: `afc_*_add_tail()`, `afc_*_add_head()`, and `afc_*_insert()`
  to add items at the end, head, and current position respectively.

### ENH: 64bit Compatibility

**Status:** STABLE

- AFC should work without problems on 64-bit machines.

## 02 Jun 2005

### ENH: Hash

**Status:** STABLE

- `afc_hash_del()` now returns the next node data and not
  a Hash Data structure.

### UPD: Dictionary

**Status:** STABLE

- `afc_dictionary_del()` has been updated to reflect Hash
  changes.
