#!/usr/bin/env bash
# test_cve_2022_45956.sh — Integration test for CVE-2022-45956
#
# CVE-2022-45956: HEAD method bypasses Allow/Deny access control.
# Tests that HEAD requests to denied paths return 403.
#
# Prerequisites:
#   - Boa binary built at src/boa with --enable-access-control
#   - curl or wget available
#   - bash, mktemp
#
# Usage:
#   ./test/integration/test_cve_2022_45956.sh
#   echo $?    # 0 = all passed (or skipped)
#
# If boa is not compiled with --enable-access-control, the test
# SKIPs and returns 0.

set -o errexit
set -o nounset
set -o pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BOA_BIN="$PROJECT_ROOT/src/boa"
TEST_NAME="CVE-2022-45956 HEAD access control bypass"
PASS=0
FAIL=1

# Find an HTTP client
HTT=""
for cmd in curl wget; do
    if command -v "$cmd" >/dev/null 2>&1; then
        HTT="$cmd"
        break
    fi
done
if [ -z "$HTT" ]; then
    echo "FAIL: $TEST_NAME — no HTTP client (curl/wget) found"
    exit $FAIL
fi

# Check boa binary
if [ ! -x "$BOA_BIN" ]; then
    echo "FAIL: $TEST_NAME — boa binary not found at $BOA_BIN"
    exit $FAIL
fi

# Check if access control is enabled.  When boa is built without
# --enable-access-control, the binary contains a diagnostic string.
# Use grep directly on the binary (no pipe) to avoid SIGPIPE exit 141
# that would interfere with the if condition under pipefail.
if grep -q "doesn't support access controls" "$BOA_BIN" 2>/dev/null; then
    echo "SKIP: $TEST_NAME — boa not compiled with --enable-access-control"
    exit $PASS
fi

# Create temp workspace
WORKDIR=$(mktemp -d "/tmp/boa_cve_45956.XXXXXX")
if [ -z "$WORKDIR" ]; then
    echo "FAIL: $TEST_NAME — failed to create temp directory"
    exit $FAIL
fi

DOC_ROOT="$WORKDIR/www"
mkdir -p "$DOC_ROOT/allowed" "$DOC_ROOT/denied"

# Determine a free port
PORT=$(python3 -c "import socket; s=socket.socket(); s.bind(('',0)); print(s.getsockname()[1]); s.close()" 2>/dev/null)
if [ -z "$PORT" ]; then
    echo "FAIL: $TEST_NAME — python3 not available, cannot determine free port"
    exit $FAIL
fi

# ---- Create boa.conf ----
cat > "$WORKDIR/boa.conf" << BOACONF
Port $PORT
User $(whoami)
Group $(id -gn)
ServerRoot $WORKDIR
DocumentRoot $DOC_ROOT
ErrorLog $WORKDIR/error_log
AccessLog $WORKDIR/access_log
DirectoryMaker /usr/bin/true
MimeTypes /dev/null
DefaultType text/plain
Allow /allowed/
Deny /denied/
BOACONF

# ---- Create test files ----
echo "allowed content" > "$DOC_ROOT/allowed/test.txt"
echo "secret content" > "$DOC_ROOT/denied/secret.txt"

# ---- Helper: clean up on exit ----
cleanup() {
    if [ -n "${BOA_PID:-}" ]; then
        kill "$BOA_PID" 2>/dev/null || true
        wait "$BOA_PID" 2>/dev/null || true
    fi
    rm -rf "$WORKDIR"
}
trap cleanup EXIT

# ---- Start boa ----
cd "$WORKDIR"
echo "--- Starting boa on port $PORT (PID tracking mode) ---"
"$BOA_BIN" -c "$WORKDIR" -d &
BOA_PID=$!
echo "  boa PID: $BOA_PID"

# Wait for boa to start and listen
sleep 2
if ! kill -0 "$BOA_PID" 2>/dev/null; then
    echo "FAIL: $TEST_NAME — boa failed to start"
    cat "$WORKDIR/error_log" 2>/dev/null || true
    exit $FAIL
fi

# Verify it's actually listening
if ! ss -tln 2>/dev/null | grep -q ":$PORT "; then
    # Maybe ss not available, try a quick connection test
    if ! curl -s --max-time 2 "http://127.0.0.1:$PORT/" >/dev/null 2>&1; then
        echo "FAIL: $TEST_NAME — boa started but not listening on port $PORT"
        kill "$BOA_PID" 2>/dev/null || true
        cat "$WORKDIR/error_log" 2>/dev/null || true
        exit $FAIL
    fi
fi

# ---- Helper: send HEAD request and get HTTP response code ----
http_head_code() {
    local url="$1"
    case "$HTT" in
        curl)
            curl -I -s --max-time 5 -o /dev/null -w '%{http_code}' "$url" 2>/dev/null || echo "000"
            ;;
        wget)
            # wget --spider prints headers to stderr; parse HTTP response code
            local result
            result=$(wget --spider -S --timeout=5 "$url" 2>&1) || true
            echo "$result" | grep -o 'HTTP/[0-9.]* [0-9]*' | tail -1 | awk '{print $2}'
            ;;
    esac
}

# ---- Test 1: HEAD to allowed path -> expect 200 ----
echo ""
echo "--- Test 1: HEAD to /allowed/test.txt (expect 200) ---"
HTTP_CODE=$(http_head_code "http://127.0.0.1:$PORT/allowed/test.txt")
echo "  HTTP code: $HTTP_CODE"

if [ "$HTTP_CODE" = "200" ]; then
    echo "  PASS: HEAD /allowed/test.txt returned 200"
else
    echo "  FAIL: HEAD /allowed/test.txt returned $HTTP_CODE (expected 200)"
    exit $FAIL
fi

# ---- Test 2: HEAD to denied path -> expect 403 ----
echo ""
echo "--- Test 2: HEAD to /denied/secret.txt (expect 403) ---"
HTTP_CODE=$(http_head_code "http://127.0.0.1:$PORT/denied/secret.txt")
echo "  HTTP code: $HTTP_CODE"

if [ "$HTTP_CODE" = "403" ]; then
    echo "  PASS: HEAD /denied/secret.txt returned 403"
else
    echo "  FAIL: HEAD /denied/secret.txt returned $HTTP_CODE (expected 403)"
    exit $FAIL
fi

# ---- All tests passed ----
echo ""
echo "=== $TEST_NAME: ALL CHECKS PASSED ==="
exit $PASS
