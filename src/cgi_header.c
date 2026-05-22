/*
 *  Boa, an http server
 *  cgi_header.c - cgi header parsing and control
 *  Copyright (C) 1997-2003 Jon Nelson <jnelson@boa.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "boa.h"

#ifdef CGI_STRIP_PREFIX
/* CGI strip prefix globals */
int cgi_strip_prefix = 0;           /* CGIStripPrefix: 0=off, 1=on */
char **cgi_strip_tokens = NULL;     /* dynamic token array */
int cgi_strip_token_count = 0;      /* number of tokens */

/* Built-in default CGI header tokens */
static const char *cgi_strip_builtin[] = {
    "Status:",
    "Location:",
    "Content-Type:",
    "Set-Cookie:",
    "Content-Length:",
    "Cache-Control:",
    "Connection:",
    "WWW-Authenticate:",
    "Expires:",
    "Pragma:",
    "Content-Encoding:",
    "Content-Language:",
    "Content-Disposition:",
    "Last-Modified:",
    "ETag:",
    "Vary:",
    "Allow:",
    NULL
};

void cgi_strip_add_token(const char *token)
{
    cgi_strip_tokens = realloc(cgi_strip_tokens,
                               (cgi_strip_token_count + 1) * sizeof(char *));
    if (cgi_strip_tokens == NULL) {
        DIE("memory allocation failure in cgi_strip_add_token");
    }
    cgi_strip_tokens[cgi_strip_token_count] = strdup(token);
    if (cgi_strip_tokens[cgi_strip_token_count] == NULL) {
        DIE("memory allocation failure in cgi_strip_add_token");
    }
    cgi_strip_token_count++;
}

/*
 * Check if a string starts with a valid wildcard HTTP token pattern:
 *   ^[A-Za-z][A-Za-z0-9-]*:
 * Matches custom / extension headers like "X-Custom-Header:", "Sec-WebSocket-Key:", etc.
 */
static int matches_wildcard_pattern(const char *line)
{
    const char *p = line;

    /* First character must be alpha */
    if (!((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z')))
        return 0;
    p++;

    /* Subsequent characters: alphanumeric or hyphen */
    while (*p && (isalnum((unsigned char)*p) || *p == '-'))
        p++;

    /* Must be followed by ": " */
    return (*p == ':' && *(p + 1) == ' ');
}

void cgi_strip_init(void)
{
    int i;

    /*
     * NOTE: Memory overhead is ~1-2 KB for the builtin token array
     * (17 tokens × ~20 bytes avg = ~340 bytes for strings plus
     *  pointer array).  Once built, runtime scan overhead is zero —
     *  only pointer dereferences and strncasecmp calls.
     */
    for (i = 0; cgi_strip_builtin[i] != NULL; i++) {
        cgi_strip_add_token(cgi_strip_builtin[i]);
    }
}

void cgi_strip_cleanup(void)
{
    int i;
    for (i = 0; i < cgi_strip_token_count; i++) {
        free(cgi_strip_tokens[i]);
    }
    free(cgi_strip_tokens);
    cgi_strip_tokens = NULL;
    cgi_strip_token_count = 0;
}

/*
 * Scan backward from pos to find the start of the line ending right
 * before pos.  Lines are delimited by '\n' or buffer start.  Trailing
 * '\r' / '\n' characters (line endings) are skipped before looking
 * for the delimiter.
 */
static char *find_line_start(char *buf, char *pos)
{
    char *p;

    if (pos <= buf)
        return buf;

    /* Skip past any trailing CR/LF at the end of the line */
    p = pos;
    while (p > buf && (*(p - 1) == '\n' || *(p - 1) == '\r'))
        p--;

    if (p == buf)
        return buf;

    /* Now find the last '\n' before the content */
    p = p - 1;
    while (p > buf && *p != '\n')
        p--;

    return (*p == '\n') ? p + 1 : buf;
}

/*
 * Strip garbage prefix from CGI output before valid HTTP headers.
 * Returns pointer to the start of valid header content.
 * If nothing to strip, returns original buf.
 */
char *strip_cgi_prefix(char *buf)
{
    char *anchor, *pos;

    if (!cgi_strip_prefix)
        return buf;

    /*
     * Locate \n\n, \n\r\n, or \r\n\r\n — check \n\r\n first (consistent
     * with process_cgi_header), then \n\n, then \r\n\r\n as fallback.
     */
    anchor = strstr(buf, "\n\r\n");
    if (anchor == NULL) {
        anchor = strstr(buf, "\n\n");
    } else {
        /* \n\r\n found; also check if \n\n appears earlier */
        char *p = strstr(buf, "\n\n");
        if (p && p < anchor)
            anchor = p;
    }
    if (anchor == NULL) {
        anchor = strstr(buf, "\r\n\r\n");
        if (anchor == NULL)
            return buf;         /* let existing logic report 502 */
    }

    pos = anchor;               /* pos points to first char of separator */

    while (1) {
        char *line_start;
        int i;
        int found_match = 0;

        line_start = find_line_start(buf, pos);

        /* Check if this line starts with a whitelist token */
        for (i = 0; i < cgi_strip_token_count; i++) {
            size_t token_len = strlen(cgi_strip_tokens[i]);
            if (strncasecmp(line_start, cgi_strip_tokens[i],
                            token_len) == 0) {
                found_match = 1;
                break;
            }
        }

        /* If no builtin token matched, try the wildcard pattern */
        if (!found_match && matches_wildcard_pattern(line_start)) {
            found_match = 1;
        }

        if (found_match) {
            /* This line is a valid header line */
            if (line_start == buf) {
                /* Reached beginning and all lines matched — nothing to strip */
                return buf;
            }
            /*
             * Continue scanning backwards.
             * Note: 'pos = line_start' is equivalent to 'pos = line_start - 1'
             * from the design document — find_line_start() skips trailing CR/LF
             * before searching for the previous '\n', so both produce the same
             * previous-line start pointer.
             */
            pos = line_start;
            continue;
        }

        /* Line does not start with a whitelist token */
        {
            char *last_token = NULL;
            char *strip_boundary;

            /* Search for the last whitelist token within [line_start, pos) */
            for (i = 0; i < cgi_strip_token_count; i++) {
                size_t token_len = strlen(cgi_strip_tokens[i]);
                char *scan;

                for (scan = line_start; scan + token_len <= pos; scan++) {
                    if (strncasecmp(scan, cgi_strip_tokens[i],
                                    token_len) == 0) {
                        last_token = scan;
                    }
                }
            }

            /* Also search for wildcard pattern matches within the line */
            {
                char *scan;
                for (scan = line_start; scan + 2 <= pos; scan++) {
                    if (matches_wildcard_pattern(scan)) {
                        last_token = scan;
                    }
                }
            }

            if (last_token != NULL) {
                /* Token found within the line — header starts at token pos */
                strip_boundary = last_token;
            } else {
                /* No token found — header starts at the beginning of the
                 * previously validated line (which is line_start if this is
                 * the first iteration, or the last valid line otherwise) */
                strip_boundary = pos;
            }

            /* Log the stripped content */
            if (strip_boundary > buf) {
                char strip_log[LOG_SANITIZE_BUF_SIZE];
                size_t strip_len = strip_boundary - buf;
                size_t copy_len = (strip_len < sizeof(strip_log) - 1)
                    ? strip_len : sizeof(strip_log) - 1;

                memcpy(strip_log, buf, copy_len);
                strip_log[copy_len] = '\0';

                /*
                 * Write strip notice to CGI log (cgi_log_fd).
                 * Fall back to stderr if CGI log is not configured (fd == 0).
                 */
                if (cgi_log_fd) {
                    char logmsg[LOG_SANITIZE_BUF_SIZE + 64];
                    int n = snprintf(logmsg, sizeof(logmsg),
                                     "[CGI STRIP] stripped %zu byte(s): \"%s\"\n",
                                     strip_len,
                                     sanitize_log_string(strip_log));
                    if (n > 0) {
                        size_t write_len = ((size_t) n < sizeof(logmsg))
                            ? (size_t) n : sizeof(logmsg) - 1;
                        if (write(cgi_log_fd, logmsg, write_len) < 0) {
                            /* write failure — silently ignore; CGI log is
                             * best-effort and a failed write should not
                             * disrupt request processing */
                        }
                    }
                } else {
                    fprintf(stderr,
                            "[CGI STRIP] stripped %zu byte(s): \"%s\"\n",
                            strip_len, sanitize_log_string(strip_log));
                }
            }

            return strip_boundary;
        }
    }
}
#endif

/* process_cgi_header

* returns 0 -=> error or HEAD, close down.
* returns 1 -=> done processing
* leaves req->cgi_status as WRITE
*/

/*
 The server MUST also resolve any conflicts between header fields returned by
 the script and header fields that it would otherwise send itself.

 ...

 At least one CGI-Field MUST be supplied, but no CGI field name may be used
 more than once in a response. If a body is supplied, then a
 "Content-type" header field MUST be supplied by the script,
 otherwise the script MUST send a "Location" or "Status" header
 field. If a Location CGI-Field is returned, then the script
 MUST NOT supply any HTTP-Fields.
 */

/* TODO:
 We still need to cycle through the data before the end of the headers,
 line-by-line, and check for any problems with the CGI
 outputting overriding http responses, etc...
 */

int process_cgi_header(request * req)
{
    char *buf;
    char *c;

    if (req->cgi_status != CGI_DONE)
        req->cgi_status = CGI_BUFFER;

    buf = req->header_line;

#ifdef CGI_STRIP_PREFIX
    if (cgi_strip_prefix && req->cgi_type != NPH) {
        char *stripped = strip_cgi_prefix(buf);
        if (stripped != buf) {
            /* garbage was stripped and logged; adjust header_line */
            req->header_line = stripped;
            buf = stripped;
        }
    }
#endif

    c = strstr(buf, "\n\r\n");
    if (c == NULL) {
        c = strstr(buf, "\n\n");
        if (c == NULL) {
            log_error_doc(req);
            fputs("cgi_header: unable to find LFLF\n", stderr);
#ifdef FASCIST_LOGGING
            log_error_time();
            fprintf(stderr, "\"%s\"\n", buf);
#endif
            send_r_bad_gateway(req);
            return 0;
        }
    }
    if (req->http_version == HTTP09) {
        if (*(c + 1) == '\r')
            req->header_line = c + 2;
        else
            req->header_line = c + 1;
        return 1;
    }
    if (!strncasecmp(buf, "Status: ", 8)) {
        req->header_line--;
        memcpy(req->header_line, "HTTP/1.0 ", 9);
    } else if (!strncasecmp(buf, "Location: ", 10)) { /* got a location header */
#ifdef FASCIST_LOGGING

        log_error_time();
        fprintf(stderr, "%s:%d - found Location header \"%s\"\n",
                __FILE__, __LINE__, buf + 10);
#endif


        if (buf[10] == '/') {   /* absolute path (potential directory traversal) */
            log_error_doc(req);
            fprintf(stderr,
                    "SECURITY: Location header with absolute path rejected: "
                    "\"%s\" — only http:// and https:// URLs are allowed\n",
                    buf + 10);
            send_r_bad_request(req);
        } else if (strncasecmp(buf + 10, "http://", 7) == 0 ||
                   strncasecmp(buf + 10, "https://", 8) == 0) {  /* valid URL */
            char *c2;
            c2 = strchr(buf + 10, '\n');
            /* c2 cannot ever equal NULL here because we already have found one */

            --c2;
            while (*c2 == '\r')
                --c2;
            ++c2;
            /* c2 now points to a '\r' or the '\n' */
            *c2++ = '\0';       /* end header */

            /* first next header, or is at req->header_end */
            while ((*c2 == '\n' || *c2 == '\r') && c2 < req->header_end)
                ++c2;
            if (c2 == req->header_end)
                send_r_moved_temp(req, buf + 10, "");
            else
                send_r_moved_temp(req, buf + 10, c2);
        } else {                /* non-URL Location value (rejected) */
            log_error_doc(req);
            fprintf(stderr,
                    "SECURITY: Location header with non-URL value rejected: "
                    "\"%s\" — only http:// and https:// URLs are allowed\n",
                    buf + 10);
            send_r_bad_request(req);
        }
        req->status = DONE;
        return 1;
    } else {                    /* not location and not status */
        char *dest;
        unsigned int howmuch;
        send_r_request_ok(req); /* does not terminate */
        /* got to do special things because
           a) we have a single buffer divided into 2 pieces
           b) we need to merge those pieces
           Easiest way is to memmove the cgi data backward until
           it touches the buffered data, then reset the cgi data pointers
         */
        dest = req->buffer + req->buffer_end;
        if (req->method == M_HEAD) {
            if (*(c + 1) == '\r')
                req->header_end = c + 2;
            else
                req->header_end = c + 1;
            req->cgi_status = CGI_DONE;
        }
        howmuch = req->header_end - req->header_line;

        if (dest + howmuch > req->buffer + BUFFER_SIZE) {
            /* big problem */
            log_error_doc(req);
            fprintf(stderr, "Too much data to move! Aborting! %s %d\n",
                    __FILE__, __LINE__);
            /* reset buffer pointers because we already called
               send_r_request_ok... */
            req->buffer_start = req->buffer_end = 0;
            send_r_error(req);
            return 0;
        }
        memmove(dest, req->header_line, howmuch);
        req->buffer_end += howmuch;
        req->header_line = req->buffer + req->buffer_end;
        req->header_end = req->header_line;
        req_flush(req);
        if (req->method == M_HEAD)
            return 0;
    }
    return 1;
}
