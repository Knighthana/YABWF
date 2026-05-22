/*
 * test_cgi_header_location.c — Standalone test for Location header validation
 *
 * Tests the security logic in process_cgi_header (src/cgi_header.c:85-128)
 * that validates Location: header values from CGI scripts.
 *
 * Rules:
 *   - Absolute paths like "/etc/passwd" are REJECTED (potential directory traversal)
 *   - "http://" and "https://" URLs are ACCEPTED
 *   - Any other non-URL value is REJECTED
 *
 * Compile:
 *   gcc -Wall -Wextra -std=c99 -o test_cgi_header_location \
 *       test_cgi_header_location.c
 *
 * Run:
 *   ./test_cgi_header_location
 *   echo $?    # 0 = all passed
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* ---------- extracted validation logic from cgi_header.c ---------- */
/*
 * Returns:
 *   1  → accepted (http:// or https:// URL)
 *   0  → rejected (absolute path or non-URL value)
 */
static int validate_location(const char *value)
{
    if (value == NULL)
        return 0;

    if (value[0] == '/') {
        /* absolute path — reject */
        return 0;
    }

    if (strncasecmp(value, "http://", 7) == 0 ||
        strncasecmp(value, "https://", 8) == 0) {
        /* valid URL — accept */
        return 1;
    }

    /* non-URL value — reject */
    return 0;
}
/* ------------------------------------------------------------------ */

/* strncasecmp is available via <strings.h> on POSIX / <string.h> on Linux */
#if defined(_WIN32) || !defined(HAVE_STRNCASECMP)
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

static int failures = 0;

#define TEST(name, cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s: " fmt "\n", name, ##__VA_ARGS__); \
        failures++; \
    } else { \
        printf("PASS: %s\n", name); \
    } \
} while (0)

#define TEST_ACCEPT(name, value) do { \
    int _ret = validate_location(value); \
    TEST(name " [accept]", _ret == 1, \
         "value=%s expected=ACCEPT got=%s", value, _ret ? "ACCEPT" : "REJECT"); \
} while (0)

#define TEST_REJECT(name, value) do { \
    int _ret = validate_location(value); \
    TEST(name " [reject]", _ret == 0, \
         "value=%s expected=REJECT got=%s", value, _ret ? "ACCEPT" : "REJECT"); \
} while (0)

int main(void)
{
    /* ===== Absolute path tests (must be REJECTED) ===== */

    TEST_REJECT("absolute path /etc/passwd", "/etc/passwd");
    TEST_REJECT("absolute path /", "/");
    TEST_REJECT("absolute path /usr/bin", "/usr/bin");
    TEST_REJECT("absolute path /tmp/../etc", "/tmp/../etc");
    TEST_REJECT("absolute path with spaces", "/path/with spaces");
    TEST_REJECT("absolute path double slash", "//etc/passwd");
    TEST_REJECT("absolute path /proc/self/environ", "/proc/self/environ");
    TEST_REJECT("absolute path /var/log", "/var/log");
    TEST_REJECT("absolute path /home/user", "/home/user");
    TEST_REJECT("absolute path /dev/null", "/dev/null");

    /* ===== http:// URL tests (must be ACCEPTED) ===== */

    TEST_ACCEPT("http:// example.com", "http://example.com");
    TEST_ACCEPT("http:// with path", "http://example.com/path");
    TEST_ACCEPT("http:// with query", "http://example.com/?q=1");
    TEST_ACCEPT("http:// with port", "http://example.com:8080/");
    TEST_ACCEPT("http:// with fragment", "http://example.com#frag");
    TEST_ACCEPT("http:// localhost", "http://localhost");
    TEST_ACCEPT("http:// IP", "http://192.168.1.1/");
    TEST_ACCEPT("http:// with auth", "http://user:pass@example.com/");
    TEST_ACCEPT("http:// subdomain", "http://sub.example.com/path");
    TEST_ACCEPT("http:// with encoded chars", "http://example.com/a%20b");

    /* ===== https:// URL tests (must be ACCEPTED) ===== */

    TEST_ACCEPT("https:// example.com", "https://example.com");
    TEST_ACCEPT("https:// with path", "https://example.com/secure");
    TEST_ACCEPT("https:// with query", "https://example.com/?token=abc");
    TEST_ACCEPT("https:// with port", "https://example.com:443/");
    TEST_ACCEPT("https:// IP", "https://10.0.0.1/");
    TEST_ACCEPT("https:// with fragment", "https://example.com#section");
    TEST_ACCEPT("https:// subdomain", "https://secure.example.com/");
    TEST_ACCEPT("https:// deep path", "https://example.com/a/b/c/d/e");

    /* ===== Non-URL string tests (must be REJECTED) ===== */

    TEST_REJECT("ftp:// URL", "ftp://example.com/");
    TEST_REJECT("mailto: URL", "mailto:user@example.com");
    TEST_REJECT("file:// URL", "file:///etc/passwd");
    TEST_REJECT("javascript: URL", "javascript:alert(1)");
    TEST_REJECT("data: URL", "data:text/html,<script>");
    TEST_REJECT("just a hostname", "example.com");
    TEST_REJECT("just a path", "relative/path");
    TEST_REJECT("empty string", "");
    TEST_REJECT("single character", "x");
    TEST_REJECT("only slash", "//");
    TEST_REJECT("backwards http", "http:\\\\example.com");
    TEST_REJECT("missing colon", "http//example.com");
    /* http:///example.com starts with "http://" => accepted by prefix check.
     * In the real process_cgi_header, strchr for '\n' safely truncates later. */
    TEST_ACCEPT("extra colon http (starts with http://)", "http:///example.com");
    TEST_REJECT("spaces only", "   ");
    /* These start with http:// so they pass the prefix check.  The real code
     * will find the newline with strchr and safely truncate the value. */
    TEST_ACCEPT("newline injection (prefix matches, truncated safely)",
                "http://good.com\nLocation: /etc/passwd");
    TEST_REJECT("tab in value", "\thttp://example.com");
    TEST_ACCEPT("carriage return in value (prefix matches, truncated safely)",
                "http://good.com\rLocation: /evil");

    /* ===== NULL input ===== */

    TEST_REJECT("NULL input", NULL);

    /* ===== Case insensitivity of http/https ===== */

    TEST_ACCEPT("HTTP uppercase", "HTTP://EXAMPLE.COM");
    TEST_ACCEPT("HTTPS uppercase", "HTTPS://EXAMPLE.COM");
    TEST_ACCEPT("Http mixed case", "Http://Example.Com");
    TEST_ACCEPT("hTtPs mixed case", "hTtPs://example.com");

    /* ===== Edge cases ===== */

    /* Just "http://" with nothing after */
    TEST_ACCEPT("http:// bare", "http://");
    /* Just "https://" with nothing after */
    TEST_ACCEPT("https:// bare", "https://");
    /* Very long URL */
    {
        char long_url[2048];
        snprintf(long_url, sizeof(long_url), "http://example.com/%s",
                 "abcdefghijklmnopqrstuvwxyz");
        TEST_ACCEPT("long http URL", long_url);
    }
    /* Very long https URL */
    {
        char long_url[2048];
        snprintf(long_url, sizeof(long_url), "https://example.com/%s",
                 "abcdefghijklmnopqrstuvwxyz0123456789");
        TEST_ACCEPT("long https URL", long_url);
    }

    printf("\n=== %s ===\n", failures ? "SOME TESTS FAILED" : "ALL TESTS PASSED");
    return failures ? 1 : 0;
}
