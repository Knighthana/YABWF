/*
 * Name: strstr and strdup
 *
 * These are the standard library utilities.  We define them here for
 * systems that don't have them.
 */

#ifndef HAVE_STRSTR
char *strstr(char *s1, char *s2)
{                               /* from libiberty */
    char *p;
    int len = strlen(s2);

    if (*s2 == '\0')            /* everything matches empty string */
        return s1;
    for (p = s1; (p = strchr(p, *s2)) != NULL; p++) {
        if (strncmp(p, s2, len) == 0)
            return (p);
    }
    return NULL;
}
#endif

#ifndef HAVE_STRDUP
char *strdup(char *s)
{
    char *retval;

    retval = (char *) malloc(strlen(s) + 1);
    if (retval == NULL) {
        perror("boa: out of memory in strdup");
        exit(1);
    }
    return strcpy(retval, s);
}
#endif

/*
 * strncasecmp: POSIX.1-2001.
 * Provided here for platforms that lack it.
 */
#ifndef HAVE_STRNCASECMP
#include <ctype.h>
int strncasecmp(const char *s1, const char *s2, size_t n)
{
    if (n == 0)
        return 0;
    while (n-- > 0) {
        int c1 = tolower((unsigned char)*s1++);
        int c2 = tolower((unsigned char)*s2++);
        if (c1 != c2)
            return c1 - c2;
        if (c1 == '\0')
            break;
    }
    return 0;
}
#endif
