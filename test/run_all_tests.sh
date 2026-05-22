#!/usr/bin/env bash
# run_all_tests.sh — Run all infrastructure tests for YABWF
#
# Usage:
#   ./test/run_all_tests.sh
#   echo $?    # 0 = all passed

set -o errexit
set -o nounset
set -o pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PASS=0
FAIL=1
ANY_FAIL=0

cd "$PROJECT_ROOT"

echo "============================================"
echo "  YABWF Infrastructure Phase Test Suite"
echo "============================================"
echo ""

# ============================================================
# 1. Unit test: sanitize_log_string
# ============================================================
echo "--- [1/5] Unit Test: sanitize_log_string ---"
if gcc -Wall -Wextra -std=c99 -o /tmp/test_sanitize_log_string \
    test/unit/test_sanitize_log_string.c 2>&1 &&
   /tmp/test_sanitize_log_string; then
    echo ">>> PASS: sanitize_log_string unit test"
else
    echo ">>> FAIL: sanitize_log_string unit test"
    ANY_FAIL=1
fi
echo ""

# ============================================================
# 2. Unit test: CGI header Location validation
# ============================================================
echo "--- [2/5] Unit Test: CGI Header Location ---"
if gcc -Wall -Wextra -std=c99 -o /tmp/test_cgi_header_location \
    test/unit/test_cgi_header_location.c 2>&1 &&
   /tmp/test_cgi_header_location; then
    echo ">>> PASS: CGI header Location unit test"
else
    echo ">>> FAIL: CGI header Location unit test"
    ANY_FAIL=1
fi
echo ""

# ============================================================
# 3. Integration test: construct.sh verify-cross
# ============================================================
echo "--- [3/5] Integration Test: construct.sh verify-cross ---"
if HOST=x86_64-linux-gnu ./test/integration/test_verify_cross.sh 2>&1; then
    echo ">>> PASS: verify-cross integration test"
else
    echo ">>> FAIL: verify-cross integration test"
    ANY_FAIL=1
fi
echo ""

# ============================================================
# 4. Security PoC: CVE-2009-4496 log injection
# ============================================================
echo "--- [4/5] Security PoC: CVE-2009-4496 ---"
if ./test/security/test_poc_sanitize_log_string.sh 2>&1; then
    echo ">>> PASS: CVE-2009-4496 security PoC"
else
    echo ">>> FAIL: CVE-2009-4496 security PoC"
    ANY_FAIL=1
fi
echo ""

# ============================================================
# 5. Security PoC: CGI Location header validation
# ============================================================
echo "--- [5/7] Security PoC: CGI Location Header ---"
if ./test/security/test_poc_cgi_header_location.sh 2>&1; then
    echo ">>> PASS: CGI Location security PoC"
else
    echo ">>> FAIL: CGI Location security PoC"
    ANY_FAIL=1
fi
echo ""

# ============================================================
# 6. Unit test: CGI Strip Prefix
# ============================================================
echo "--- [6/7] Unit Test: CGI Strip Prefix ---"
if gcc -Wall -Wextra -std=c99 -DSTANDALONE_TEST -o /tmp/test_cgi_strip_prefix \
    test/unit/test_cgi_strip_prefix.c 2>&1 &&
   /tmp/test_cgi_strip_prefix; then
    echo ">>> PASS: CGI Strip Prefix unit test"
else
    echo ">>> FAIL: CGI Strip Prefix unit test"
    ANY_FAIL=1
fi
echo ""

# ============================================================
# 7. Integration test: CGI Strip Prefix
# ============================================================
echo "--- [7/7] Integration Test: CGI Strip Prefix ---"
if ./test/integration/test_cgi_strip_prefix.sh 2>&1; then
    echo ">>> PASS: CGI Strip Prefix integration test"
else
    echo ">>> FAIL: CGI Strip Prefix integration test"
    ANY_FAIL=1
fi
echo ""

# ============================================================
# Summary
# ============================================================
echo "============================================"
if [ "$ANY_FAIL" -eq 0 ]; then
    echo "  ALL INFRASTRUCTURE TESTS PASSED"
    exit $PASS
else
    echo "  SOME INFRASTRUCTURE TESTS FAILED"
    exit $FAIL
fi
echo "============================================"
