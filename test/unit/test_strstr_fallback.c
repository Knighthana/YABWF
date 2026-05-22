/*
 * test_strstr_fallback.c — Standalone test for strstr() fallback
 *
 * Tests the strstr() implementation in extras/strutil.c, which serves
 * as a fallback for systems that lack the standard library function.
 *
 * NOTE: extras/strutil.c normally relies on the including file for
 * standard headers.  When compiled as a separate translation unit,
 * the needed headers (stddef.h, stdlib.h, string.h) are injected via
 * -include flags, and -Wno-implicit-function-declaration suppresses
 * the expected warnings for the implicit functions.
 *
 * Compile (from project root):
 *   gcc -Wall -Wextra -std=c99 -fno-builtin-strstr \
 *       -include stddef.h -include stdlib.h \
 *       -Wno-implicit-function-declaration \
 *       -DSTANDALONE_TEST -I./src \
 *       -o /tmp/test_strstr_fallback \
 *       extras/strutil.c test/unit/test_strstr_fallback.c
 *
 * Run:
 *   /tmp/test_strstr_fallback
 *   echo $?    # 0 = all passed
 */

/* STANDALONE_TEST compiles extras/strutil.c as a test binary.
 * The actual -DSTANDALONE_TEST flag is set by the build system. */
#ifndef STANDALONE_TEST
#define STANDALONE_TEST
#endif
#include <stdio.h>
#include <assert.h>

/* Declare strstr if we're testing the fallback.
 * The actual definition comes from extras/strutil.c when HAVE_STRSTR
 * is not defined.
 *
 * We deliberately do NOT include <string.h> here because the system
 * declares strstr as (const char *, const char *), which conflicts
 * with the fallback's (char *, char *) signature.  Our own declaration
 * below is compatible with the fallback. */
#ifndef HAVE_STRSTR
char *strstr(char *s1, char *s2);
#endif

int main(void)
{
    char *r;

    printf("=== strstr fallback unit tests ===\n\n");

    /* 1. Basic match at beginning */
    r = strstr("hello world", "hello");
    assert(r != NULL);
    assert(r[0] == 'h');
    assert(r[5] == ' ');
    printf("PASS: 1. basic match at beginning\n");

    /* 2. Basic match in middle */
    r = strstr("hello world", "world");
    assert(r != NULL);
    assert(r[0] == 'w');
    assert(r[5] == '\0');
    printf("PASS: 2. basic match in middle\n");

    /* 3. No match */
    assert(strstr("hello world", "xyz") == NULL);
    printf("PASS: 3. no match\n");

    /* 4. Empty needle matches everything */
    assert(strstr("hello", "") != NULL);
    printf("PASS: 4. empty needle\n");

    /* 5. Needle at end */
    r = strstr("abc", "c");
    assert(r != NULL && *r == 'c' && r[1] == '\0');
    printf("PASS: 5. needle at end\n");

    /* 6. Single character match */
    assert(strstr("a", "a") != NULL);
    printf("PASS: 6. single character match\n");

    /* 7. Needle longer than haystack */
    assert(strstr("ab", "abc") == NULL);
    printf("PASS: 7. needle longer than haystack\n");

    /* 8. Overlapping patterns — key CVE-2018-21028 fix test:
     * "aaaa" with "aa" should find at position 0 (first match) */
    r = strstr("aaaa", "aa");
    assert(r != NULL);
    assert(r - "aaaa" == 0);     /* must find first occurrence */
    printf("PASS: 8. overlapping pattern finds first match\n");

    /* 9. Exact match */
    r = strstr("abc", "abc");
    assert(r != NULL);
    assert(r[0] == 'a' && r[1] == 'b' && r[2] == 'c' && r[3] == '\0');
    printf("PASS: 9. exact match\n");

    /* 10. Multiple occurrences — returns first */
    r = strstr("ababab", "aba");
    assert(r != NULL);
    assert(r - "ababab" == 0);
    printf("PASS: 10. multiple occurrences returns first\n");

    /* 11. Match with special characters (not printable) */
    {
        char haystack[] = {'a', 'b', '\x01', 'c', '\0'};
        char needle[] = {'\x01', 'c', '\0'};
        r = strstr(haystack, needle);
        assert(r != NULL);
        assert(r - haystack == 2);
        printf("PASS: 11. match with special characters\n");
    }

    /* 12. Haystack and needle both single character */
    assert(strstr("z", "z") != NULL);
    printf("PASS: 12. both single character match\n");

    /* 13. Needle appears only at very end after a partial match */
    r = strstr("abcab", "ab");
    assert(r != NULL);
    assert(r - "abcab" == 0);    /* "ab" at position 0, not position 3 */
    printf("PASS: 13. first occurrence preferred over later\n");

    /* 14. Empty haystack, empty needle */
    r = strstr("", "");
    assert(r != NULL);
    assert(r[0] == '\0');
    printf("PASS: 14. empty haystack and empty needle\n");

    /* 15. Empty haystack, non-empty needle */
    assert(strstr("", "a") == NULL);
    printf("PASS: 15. empty haystack with non-empty needle\n");

    printf("\n=== ALL strstr fallback tests PASSED ===\n");
    return 0;
}
