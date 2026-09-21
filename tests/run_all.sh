#!/bin/sh
# Runs every test_* executable in this directory and reports the summary.
# Network-dependent tests may fail without internet access.

cd "$(dirname "$0")" || exit 1

failed=0
for t in test_*; do
	case "$t" in
	*.c | *.o | *.h | *.sh) continue ;;
	esac
	[ -x "$t" ] || continue
	printf '=== %s ===\n' "$t"
	if timeout 60 "./$t"; then
		echo "PASS: $t"
	else
		echo "FAIL: $t"
		failed=$((failed + 1))
	fi
done

echo
if [ "$failed" -eq 0 ]; then
	echo "All tests passed."
else
	echo "$failed test(s) failed."
fi
exit "$failed"
