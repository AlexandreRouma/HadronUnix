#include "gfx_query.h"
#include <stddef.h>

// Small decimal atoi function
// TODO: make it support hex for explicit mode selection
uint64_t atoi(char *src) {
    uint64_t res = 0;
    uint64_t len = strlen(src);
    for (int64_t i = len - 1, f = 1; i >= 0; i--, f *= 10) {
        res += (src[i] - '0') * f;
    }
    return res;
}

char gfx_query_parse(gfx_query_t *query, char *src) {
    char *type = src;
    char wineq = 0;
    char *width = NULL;
    char hineq = 0;
    char *height = NULL;
    char dineq = 0;
    char *depth = NULL;
    char *next = NULL;
    char ineq = 0;
    for (int i = 0; src[i]; i++) {
        if (src[i] == ':') {
            src[i] = '\0';
            i++;
            next = src + i;
        }
        else if (src[i] == 'x') {
            src[i] = '\0';
            ineq = 0;
            i++;
            next = src + i;
        }
        if (src[i] == '>' && src[i+1] && src[i+1] == '=') {
            ineq = 1;
            next = src + i + 2;
            i++;
        }
        else if (src[i] == '<' && src[i+1] && src[i+1] == '=') {
            ineq = -1;
            next = src + i + 2;
            i++;
        }

        if (next == NULL)
            continue;

        if (!width) {
            width = next;
            wineq = ineq;
        }
        else if (!height) {
            height = next;
            hineq = ineq;
        }
        else if (!depth) {
            depth = next;
            dineq = ineq;
        }

        next = NULL;
    }

    if (strcmp(type, "mode") == 0) {
        query->type = VBE_QUERY_MODE;
        if (!width)
            return 1;
        uint64_t m = atoi(width);
        query->mode = m;
        return 0;
    }
    else if (strcmp(type, "find") == 0) {
        query->type = VBE_QUERY_TRIPLET;
    }
    else {
        return 1;
    }

    if (!width || !height || !depth)
        return 1;

    query->triplet.wineq = wineq;
    query->triplet.hineq = hineq;
    query->triplet.dineq = dineq;
    query->triplet.width = atoi(width);
    query->triplet.height = atoi(height);
    query->triplet.depth = atoi(depth);
    return 0;
}