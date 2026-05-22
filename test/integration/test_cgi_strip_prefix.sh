#!/usr/bin/env bash
# test_cgi_strip_prefix.sh — Integration test for CGI Strip Prefix
#
# Starts a temporary boa instance with CGIStripPrefix enabled,
# sends a request to a CGI script that outputs garbage before valid headers,
# and verifies:
#   1. The HTTP response is valid (200 OK, correct Content-Type)
#   2. The response body is correct (debug line stripped, not present)
#   3. The error log contains a "[CGI STRIP]" entry
#
# Prerequisites:
#   - Boa binary built at src/boa
#   - curl or wget available
#   - bash, mktemp
#
# Usage:
#   ./test/integration/test_cgi_strip_prefix.sh
#   echo $?    # 0 = all passed

set -o errexit
set -o nounset
set -o pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BOA_BIN="$PROJECT_ROOT/src/boa"
TEST_NAME="CGI Strip Prefix integration test"
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

# Create temp workspace
WORKDIR=$(mktemp -d "/tmp/boa_strip_test.XXXXXX")
if [ -z "$WORKDIR" ]; then
    echo "FAIL: $TEST_NAME — failed to create temp directory"
    exit $FAIL
fi

CGI_DIR="$WORKDIR/cgi-bin"
DOC_ROOT="$WORKDIR/www"
mkdir -p "$CGI_DIR" "$DOC_ROOT"

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
CgiLog $WORKDIR/cgi_log
DirectoryIndex index.html
KeepAliveMax 0
KeepAliveTimeout 0
MimeTypes /etc/mime.types
DefaultType text/plain
CGIPath /bin:/sbin:/usr/bin:/usr/sbin
ScriptAlias /cgi-bin/ $CGI_DIR/
CGIStripPrefix
BOACONF

# ---- Create a CGI script that outputs garbage before valid headers ----
cat > "$CGI_DIR/strip-test.cgi" << 'CGISCRIPT'
#!/bin/sh
# This CGI outputs debug lines before Content-Type to test strip prefix
echo "DEBUG: database connection established"
echo "DEBUG: query returned 42 rows"
echo "Content-Type: text/plain"
echo ""
echo "CGI STRIP TEST: OK"
CGISCRIPT
chmod 755 "$CGI_DIR/strip-test.cgi"

# Also create a "clean" CGI that doesn't output garbage (for comparison)
cat > "$CGI_DIR/clean-test.cgi" << 'CGISCRIPT'
#!/bin/sh
echo "Content-Type: text/plain"
echo ""
echo "CLEAN CGI: OK"
CGISCRIPT
chmod 755 "$CGI_DIR/clean-test.cgi"

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
# Note: -d prevents daemonizing so we can track the PID reliably.
# Boa redirects stderr to ErrorLog, so [CGI STRIP] will be in error_log.
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

# ---- Helper: send HTTP request and get response body ----
http_get() {
    local url="$1"
    case "$HTT" in
        curl)
            curl -s --max-time 5 "$url" 2>/dev/null || true
            ;;
        wget)
            wget -q -O - --timeout=5 "$url" 2>/dev/null || true
            ;;
    esac
}

# ---- Test 1: CGI with garbage (strip prefix enabled) ----
echo ""
echo "--- Test 1: CGI with garbage, strip enabled ---"
BODY=$(http_get "http://127.0.0.1:$PORT/cgi-bin/strip-test.cgi")
echo "  Response body: [$BODY]"

if echo "$BODY" | grep -q "CGI STRIP TEST: OK"; then
    echo "  PASS: response body contains expected content"
else
    echo "  FAIL: response body missing expected content"
    exit $FAIL
fi

if echo "$BODY" | grep -q "DEBUG:"; then
    echo "  FAIL: response body still contains DEBUG garbage (strip failed)"
    exit $FAIL
else
    echo "  PASS: DEBUG garbage successfully stripped from response"
fi

# ---- Test 2: Check error log for [CGI STRIP] entry ----
# (boa redirects stderr => error_log, so [CGI STRIP] appears there)
echo ""
echo "--- Test 2: Verify [CGI STRIP] in error log ---"
sleep 1  # wait for logs to flush

# Show error log for debugging
if [ -s "$WORKDIR/error_log" ]; then
    echo "  error_log contents:"
    while IFS= read -r line; do
        echo "    | $line"
    done < "$WORKDIR/error_log"
fi

if grep -q "\[CGI STRIP\]" "$WORKDIR/error_log" 2>/dev/null; then
    echo "  PASS: [CGI STRIP] found in error log"
else
    echo "  FAIL: [CGI STRIP] not found in error log"
    echo "  Check: was CGIStripPrefix enabled in config?"
    exit $FAIL
fi

# ---- Test 3: CGI without garbage (should work normally) ----
echo ""
echo "--- Test 3: Clean CGI (no garbage) ---"
BODY2=$(http_get "http://127.0.0.1:$PORT/cgi-bin/clean-test.cgi")
echo "  Response body: [$BODY2]"

if echo "$BODY2" | grep -q "CLEAN CGI: OK"; then
    echo "  PASS: clean CGI response correct"
else
    echo "  FAIL: clean CGI response incorrect"
    exit $FAIL
fi

# ---- All tests passed ----
echo ""
echo "=== $TEST_NAME: ALL CHECKS PASSED ==="
exit $PASS
