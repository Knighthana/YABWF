/*
 * test_sanitize_log_string.c — Standalone test for sanitize_log_string
 *
 * CVE-2009-4496: prevent log injection via control characters.
 * See src/util.c:759-787.
 *
 * Compile:
 *   gcc -Wall -Wextra -std=c99 -o test_sanitize_log_string \
 *       test_sanitize_log_string.c
 *
 * Run:
 *   ./test_sanitize_log_string
 *   echo $?    # 0 = all passed
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ---------- copy of sanitize_log_string from src/util.c ---------- */
#define SANITIZE_BUF_SIZE 8192

static const char *sanitize_log_string(const char *str)
{
    static char buf[SANITIZE_BUF_SIZE];
    const char *p;
    char *q;
    size_t len;

    if (str == NULL)
        return "(null)";

    p = str;
    q = buf;
    len = 0;

    while (*p && len < sizeof(buf) - 1) {
        unsigned char c = (unsigned char) *p;
        if ((c <= 0x1F && c != 0x09) || c == 0x7F) {
            *q++ = '?';
            len++;
        } else {
            *q++ = *p;
            len++;
        }
        p++;
    }
    *q = '\0';
    return buf;
}
/* ---------------------------------------------------------------- */

static int failures = 0;

#define TEST(name, cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s: " fmt "\n", name, ##__VA_ARGS__); \
        failures++; \
    } else { \
        printf("PASS: %s\n", name); \
    } \
} while (0)

#define TEST_STR(name, input, expected) do { \
    const char *_got = sanitize_log_string(input); \
    TEST(name, strcmp(_got, expected) == 0, \
         "input=%s expected=%s got=%s", \
         (input ? input : "NULL"), expected, _got); \
} while (0)

int main(void)
{
    /* 1. Normal string — no modification */
    TEST_STR("normal ascii string", "GET /index.html HTTP/1.0",
             "GET /index.html HTTP/1.0");

    /* 2. Empty string */
    TEST_STR("empty string", "", "");

    /* 3. String containing tab (0x09) — should be preserved */
    TEST_STR("tab preserved", "GET\t/index.html",
             "GET\t/index.html");

    /* 4. NULL input → returns "(null)" */
    TEST_STR("NULL input", NULL, "(null)");

    /* 5. String with control char 0x00 (embedded NUL) */
    {
        char input[] = "bad\x00log";
        /* sanitize_log_string stops at NUL because of while(*p) */
        const char *got = sanitize_log_string(input);
        TEST("embedded NUL stops at NUL",
             strcmp(got, "bad") == 0,
             "expected=bad got=%s", got);
    }

    /* 6. String with control char 0x01 (SOH) → replaced with ? */
    {
        char input[] = "bad\x01log";
        TEST_STR("SOH replaced", input, "bad?log");
    }

    /* 7. String with control char 0x1F (US) → replaced with ? */
    {
        char input[] = "bad\x1flog";
        TEST_STR("US replaced", input, "bad?log");
    }

    /* 8. String with DEL 0x7F → replaced with ? */
    {
        char input[] = "bad\x7flog";
        TEST_STR("DEL replaced", input, "bad?log");
    }

    /* 9. All control chars in 0x01-0x08, 0x0A-0x1F (except tab) */
    {
        char input[256];
        char expected[256];
        int i, len = 0;
        for (i = 1; i <= 0x1F; i++) {
            if (i == 0x09) continue; /* tab preserved */
            input[len] = (char)i;
            expected[len] = '?';
            len++;
        }
        input[len] = '\0';
        expected[len] = '\0';
        TEST_STR("all control chars replaced except tab", input, expected);
    }

    /* 10. Mix of valid chars, tab, control chars, DEL */
    {
        /* Use explicit hex to avoid \x1fd being parsed as one escape */
        char input[] = {'a', '\t', 'b', 0x01, 'c', 0x1f, 'd', 0x7f, 'e', '\0'};
        TEST_STR("mixed valid/tab/ctrl/DEL", input, "a\tb?c?d?e");
    }

    /* 11. Long input exceeding 8191 bytes — should truncate safely */
    {
        size_t n = SANITIZE_BUF_SIZE + 100;  /* 8292 bytes */
        char *input = malloc(n + 1);
        char *expected = malloc(SANITIZE_BUF_SIZE);
        size_t i;
        if (!input || !expected) {
            fprintf(stderr, "FAIL: malloc failed\n");
            return 1;
        }
        memset(input, 'A', n);
        input[n] = '\0';
        memset(expected, 'A', SANITIZE_BUF_SIZE - 1);
        expected[SANITIZE_BUF_SIZE - 1] = '\0';

        const char *got = sanitize_log_string(input);
        int ok = (strlen(got) == SANITIZE_BUF_SIZE - 1);
        ok = ok && (memcmp(got, expected, SANITIZE_BUF_SIZE - 1) == 0);
        TEST("long input truncated safely", ok,
             "expected_len=%zu got_len=%zu",
             SANITIZE_BUF_SIZE - 1, strlen(got));
        free(input);
        free(expected);
    }

    /* 12. String right at 8191 bytes (no truncation needed) */
    {
        size_t n = SANITIZE_BUF_SIZE - 1;
        char *input = malloc(n + 1);
        if (!input) {
            fprintf(stderr, "FAIL: malloc failed\n");
            return 1;
        }
        memset(input, 'B', n);
        input[n] = '\0';
        const char *got = sanitize_log_string(input);
        int ok = (strlen(got) == n);
        ok = ok && (memcmp(got, input, n) == 0);
        TEST("exactly 8191 bytes preserved", ok,
             "expected_len=%zu got_len=%zu", n, strlen(got));
        free(input);
    }

    /* 13. Multiple calls — static buffer is overwritten each call.
     *     This is expected: the returned pointer must be used/consumed
     *     before the next call.  Here we verify the LAST return is correct. */
    {
        const char *r1 = sanitize_log_string("first");
        (void)r1; /* consumed before next call */
        const char *r2 = sanitize_log_string("second");
        /* r1 now points to "second" because the static buffer was reused */
        TEST("multiple calls: last value correct",
             strcmp(r2, "second") == 0,
             "r2=%s", r2);
    }

    /* 14. Printable chars: space, punctuation, digits, letters preserved */
    TEST_STR("printable ASCII", " !\"#$%&'()*+,-./0123456789:;<=>?@"
              "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~",
              " !\"#$%&'()*+,-./0123456789:;<=>?@"
              "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");

    /* 15. Newline 0x0A (LF) — control char, should be replaced */
    {
        char input[] = "line1\nline2";
        TEST_STR("LF replaced", input, "line1?line2");
    }

    /* 16. Carriage return 0x0D (CR) — control char, should be replaced */
    {
        char input[] = "line1\rline2";
        TEST_STR("CR replaced", input, "line1?line2");
    }

    printf("\n=== %s ===\n", failures ? "SOME TESTS FAILED" : "ALL TESTS PASSED");
    return failures ? 1 : 0;
}
