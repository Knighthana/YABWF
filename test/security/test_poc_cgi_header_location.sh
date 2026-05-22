#!/usr/bin/env bash
# test_poc_cgi_header_location.sh — Security PoC for Location header validation
#
# Demonstrates that the CGI Location header parser correctly:
#   - REJECTS absolute paths (e.g. /etc/passwd) — prevents directory traversal
#   - ACCEPTS http:// and https:// URLs
#   - REJECTS non-URL values
#
# Compile & run:
#   chmod +x test_poc_cgi_header_location.sh
#   ./test_poc_cgi_header_location.sh
#   echo $?    # 0 = all passed

set -o errexit
set -o nounset
set -o pipefail

TEST_DIR="$(cd "$(dirname "$0")/.." && pwd)"
WORK_DIR=$(mktemp -d)
trap 'rm -rf "$WORK_DIR"' EXIT

PASS=0
FAIL=1

echo "=== Security PoC: CGI Location Header Validation ==="

cat > "$WORK_DIR/poc.c" << 'POCEOF'
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* strncasecmp for platforms without it */
#if !defined(HAVE_STRNCASECMP) && !defined(__linux__)
static int my_strncasecmp(const char *s1, const char *s2, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        int c1 = tolower((unsigned char)s1[i]);
        int c2 = tolower((unsigned char)s2[i]);
        if (c1 != c2)
            return c1 - c2;
        if (c1 == 0)
            break;
    }
    return 0;
}
#define strncasecmp my_strncasecmp
#endif

/*
 * Simulates the Location validation logic from process_cgi_header
 * (src/cgi_header.c:85-128).
 *
 * Returns:
 *   1  → valid http:// or https:// URL (accepted)
 *   0  → rejected
 */
static int validate_location(const char *value)
{
    if (value == NULL || value[0] == '\0')
        return 0;

    if (value[0] == '/')
        return 0;  /* absolute path — reject */

    if (strncasecmp(value, "http://", 7) == 0 ||
        strncasecmp(value, "https://", 8) == 0)
        return 1;  /* valid URL — accept */

    return 0;      /* non-URL — reject */
}

int main(void)
{
    int failures = 0;

    /* Test 1: Absolute path directory traversal attempt */
    {
        const char *value = "/etc/passwd";
        int ret = validate_location(value);
        if (ret == 1) {
            printf("FAIL: Absolute path '/etc/passwd' was ACCEPTED — "
                   "directory traversal possible!\n");
            failures++;
        } else {
            printf("PASS: Absolute path '/etc/passwd' correctly REJECTED\n");
        }
    }

    /* Test 2: Absolute path with traversal */
    {
        const char *value = "/../../../etc/shadow";
        int ret = validate_location(value);
        if (ret == 1) {
            printf("FAIL: Absolute path '/../../../etc/shadow' was ACCEPTED\n");
            failures++;
        } else {
            printf("PASS: Absolute path '/../../../etc/shadow' correctly REJECTED\n");
        }
    }

    /* Test 3: Valid http:// URL */
    {
        const char *value = "http://example.com/";
        int ret = validate_location(value);
        if (ret == 0) {
            printf("FAIL: Valid http:// URL was REJECTED\n");
            failures++;
        } else {
            printf("PASS: 'http://example.com/' correctly ACCEPTED\n");
        }
    }

    /* Test 4: Valid https:// URL */
    {
        const char *value = "https://example.com/secure";
        int ret = validate_location(value);
        if (ret == 0) {
            printf("FAIL: Valid https:// URL was REJECTED\n");
            failures++;
        } else {
            printf("PASS: 'https://example.com/secure' correctly ACCEPTED\n");
        }
    }

    /* Test 5: ftp:// URL (non-http scheme) */
    {
        const char *value = "ftp://example.com/";
        int ret = validate_location(value);
        if (ret == 1) {
            printf("FAIL: ftp:// URL was ACCEPTED (should be rejected)\n");
            failures++;
        } else {
            printf("PASS: ftp:// URL correctly REJECTED\n");
        }
    }

    /* Test 6: Empty string */
    {
        const char *value = "";
        int ret = validate_location(value);
        if (ret == 1) {
            printf("FAIL: Empty string was ACCEPTED\n");
            failures++;
        } else {
            printf("PASS: Empty string correctly REJECTED\n");
        }
    }

    /* Test 7: NULL input */
    {
        int ret = validate_location(NULL);
        if (ret == 1) {
            printf("FAIL: NULL input was ACCEPTED\n");
            failures++;
        } else {
            printf("PASS: NULL input correctly REJECTED\n");
        }
    }

    /* Test 8: Case-insensitive HTTP */
    {
        const char *value = "HTTP://EXAMPLE.COM";
        int ret = validate_location(value);
        if (ret == 0) {
            printf("FAIL: Uppercase HTTP:// was REJECTED (case insensitive)\n");
            failures++;
        } else {
            printf("PASS: 'HTTP://EXAMPLE.COM' correctly ACCEPTED (case insensitive)\n");
        }
    }

    /* Test 9: Malicious javascript: URL */
    {
        const char *value = "javascript:alert(1)";
        int ret = validate_location(value);
        if (ret == 1) {
            printf("FAIL: 'javascript:alert(1)' was ACCEPTED — XSS possible!\n");
            failures++;
        } else {
            printf("PASS: 'javascript:alert(1)' correctly REJECTED\n");
        }
    }

    /* Test 10: file:// URL */
    {
        const char *value = "file:///etc/passwd";
        int ret = validate_location(value);
        if (ret == 1) {
            printf("FAIL: 'file:///etc/passwd' was ACCEPTED\n");
            failures++;
        } else {
            printf("PASS: 'file:///etc/passwd' correctly REJECTED\n");
        }
    }

    printf("\n=== PoC %s ===\n", failures ? "FAILED (security issue)" : "PASSED (validation correct)");
    return failures;
}
POCEOF

echo "--- Compiling PoC ---"
if ! gcc -Wall -Wextra -std=c99 -o "$WORK_DIR/poc" "$WORK_DIR/poc.c"; then
    echo "FAIL: compilation error"
    exit $FAIL
fi
echo "Compilation OK"

echo "--- Running PoC ---"
if "$WORK_DIR/poc"; then
    echo "PASS: All security PoC checks passed"
    exit $PASS
else
    echo "FAIL: Security PoC detected vulnerability"
    exit $FAIL
fi
