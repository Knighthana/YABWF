#!/usr/bin/env bash
# test_verify_cross.sh — Integration test for construct.sh verify-cross
#
# Verifies that the cross-compilation verification target works.
# Runs `HOST=x86_64-linux-gnu ./construct.sh verify-cross` and checks
# that it exits with status 0.
#
# Prerequisites:
#   - autoconf, gcc, make, etc. installed
#   - run from project root
#
# Usage:
#   ./test/integration/test_verify_cross.sh
#   echo $?    # 0 = all passed

set -o errexit
set -o nounset
set -o pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TEST_NAME="verify-cross integration test"
PASS=0
FAIL=1

cd "$PROJECT_ROOT"

echo "=== $TEST_NAME ==="

# Pre-flight: check that we have the tools we need
for cmd in gcc make autoconf; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
        echo "FAIL: required command '$cmd' not found"
        exit $FAIL
    fi
done

# Run verify-cross
echo "--- Running: HOST=x86_64-linux-gnu ./construct.sh verify-cross ---"
if HOST=x86_64-linux-gnu ./construct.sh verify-cross; then
    echo "PASS: $TEST_NAME — verify-cross exited with status 0"
else
    rc=$?
    echo "FAIL: $TEST_NAME — verify-cross exited with status $rc"
    exit $FAIL
fi

# Verify that the expected binaries were produced
for bin in src/boa src/boa_indexer; do
    if [ -f "$bin" ]; then
        size=$(stat --format="%s" "$bin" 2>/dev/null)
        echo "  artifact present: $bin ($size bytes)"
    else
        echo "FAIL: $TEST_NAME — expected artifact $bin not found"
        exit $FAIL
    fi
done

echo "=== $TEST_NAME: ALL CHECKS PASSED ==="
exit $PASS
