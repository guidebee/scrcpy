#include "str_util.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
# include <windows.h>
# include <tchar.h>
#endif

#include <SDL2/SDL_stdinc.h>

#include "config.h"

size_t
xstrncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n - 1 && src[i] != '\0'; ++i)
        dest[i] = src[i];
    if (n)
        dest[i] = '\0';
    return src[i] == '\0' ? i : n;
}

size_t
xstrjoin(char *dst, const char *const tokens[], char sep, size_t n) {
    const char *const *remaining = tokens;
    const char *token = *remaining++;
    size_t i = 0;
    while (token) {
        if (i) {
            dst[i++] = sep;
            if (i == n)
                goto truncated;
        }
        size_t w = xstrncpy(dst + i, token, n - i);
        if (w >= n - i)
            goto truncated;
        i += w;
        token = *remaining++;
    }
    return i;

    truncated:
    dst[n - 1] = '\0';
    return n;
}

char *
strquote(const char *src) {
    size_t len = strlen(src);
    char *quoted = SDL_malloc(len + 3);
    if (!quoted) {
        return NULL;
    }
    memcpy(&quoted[1], src, len);
    quoted[0] = '"';
    quoted[len + 1] = '"';
    quoted[len + 2] = '\0';
    return quoted;
}

bool
parse_integer(const char *s, long *out) {
    char *endptr;
    if (*s == '\0') {
        return false;
    }
    errno = 0;
    long value = strtol(s, &endptr, 0);
    if (errno == ERANGE) {
        return false;
    }
    if (*endptr != '\0') {
        return false;
    }

    *out = value;
    return true;
}

bool
parse_integer_with_suffix(const char *s, long *out) {
    char *endptr;
    if (*s == '\0') {
        return false;
    }
    errno = 0;
    long value = strtol(s, &endptr, 0);
    if (errno == ERANGE) {
        return false;
    }
    int mul = 1;
    if (*endptr != '\0') {
        if (s == endptr) {
            return false;
        }
        if ((*endptr == 'M' || *endptr == 'm') && endptr[1] == '\0') {
            mul = 1000000;
        } else if ((*endptr == 'K' || *endptr == 'k') && endptr[1] == '\0') {
            mul = 1000;
        } else {
            return false;
        }
    }

    if ((value < 0 && LONG_MIN / mul > value) ||
        (value > 0 && LONG_MAX / mul < value)) {
        return false;
    }

    *out = value * mul;
    return true;
}

size_t
utf8_truncation_index(const char *utf8, size_t max_len) {
    size_t len = strlen(utf8);
    if (len <= max_len) {
        return len;
    }
    len = max_len;
    // see UTF-8 encoding <https://en.wikipedia.org/wiki/UTF-8#Description>
    while ((utf8[len] & 0x80) != 0 && (utf8[len] & 0xc0) != 0xc0) {
        // the next byte is not the start of a new UTF-8 codepoint
        // so if we would cut there, the character would be truncated
        len--;
    }
    return len;
}


// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

#ifdef _WIN32

wchar_t *
utf8_to_wide_char(const char *utf8) {
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    if (!len) {
        return NULL;
    }

    wchar_t *wide = SDL_malloc(len * sizeof(wchar_t));
    if (!wide) {
        return NULL;
    }

    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wide, len);
    return wide;
}

char *
utf8_from_wide_char(const wchar_t *ws) {
    int len = WideCharToMultiByte(CP_UTF8, 0, ws, -1, NULL, 0, NULL, NULL);
    if (!len) {
        return NULL;
    }

    char *utf8 = SDL_malloc(len);
    if (!utf8) {
        return NULL;
    }

    WideCharToMultiByte(CP_UTF8, 0, ws, -1, utf8, len, NULL, NULL);
    return utf8;
}

#endif
