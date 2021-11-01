#include "kfmt.h"
#include <stdint.h>
#include <stddef.h>

#include <string.h>

int safegard(char *str) {
    (void)str;
    return 0;
}

int (*kfmt_out)(char *str) = safegard;

int print_emitter(char *str, void *ctx, size_t offset) {
    (void)ctx;
    (void)offset;
    return kfmt_out(str);
}

int sprintf_emitter(char *str, void *ctx, size_t offset) {
    strcpy((char*)ctx + offset, str);
    return strlen(str);
}

int shortest_paddout(void *ptr) {
    size_t t = (size_t)ptr;
    int i;
    for (i = 0; t; i++) {
        t >>= 1;
    }
    return (i-1)/4 + 1;
}

int format(int (*emitter)(char *str, void *ctx, size_t offset), void *ctx, char *fmt, va_list ap) {
    size_t count = 0;
    size_t len = strlen(fmt);
    size_t off = 0;
    char escaped = 0;

    while (len > off) {
        char buf[4096];
        char *f = buf;
        strncpy(buf, fmt + off, 4095);
        off += 4096;

        char *it = f;
        char longcount = 0;
        int padcount = 0;

        for (; *it; it++) {
            if (!escaped && *it == '%') {
                char tmp = *it;
                *it = '\0';
                count += emitter(f, ctx, count);
                *it = tmp;
                f = it+1;

                escaped = 1;
                continue;
            }
            if (!escaped)
                continue;

            switch (*it) {
            case 'l':
                longcount++;
                continue;
            case '0':
                padcount = -1;
                continue;
            case 's':
                count += emitter(va_arg(ap, char *), ctx, count);
                break;
            case 'c':
                char c[2];
                c[0] = *it;
                c[1] = '\0';
                count += emitter(c, ctx, count);
                break;
            case '#':
                count += emitter("0x", ctx, count);
                continue;
            case 'p':
                count += emitter("0x", ctx, count);
                /* FALLTHROUGH */
            case 'x':
                char hex[17];
                void *ptr = va_arg(ap, void*);
                itohex((size_t)ptr, hex, padcount ? padcount : shortest_paddout(ptr));
                padcount = 0;
                count += emitter(hex, ctx, count);
                break;
            case 'u':
                // TODO: handle unsigned ints correctly
                /* FALLTHROUGH */
            case 'd':
                char num[64];

                // This is an example of how this could be done properly
                // TODO: do properly with a good itoa function
                switch (longcount) {
                case 0:
                    itoa_p(va_arg(ap, int32_t), num, 63, padcount);
                    break;
                default:
                    itoa_p(va_arg(ap, int64_t), num, 63, padcount);
                }
                count += emitter(num, ctx, count);
                break;
            default:
                // If padding specification mode set, compute it
                if (padcount == -1) {
                    // Set to zero
                    padcount = 0;

                    // Quickly convert next numbers into integer
                    char *beg = it;
                    for (; *(it) && *(it) >= '0' && *(it) <= '9'; it++);
                    char *end = --it;
                    for (size_t factor = 1; it >= beg; factor *= 10, it--)
                        padcount += (*it - '0') * factor;
                    it = end;
                    continue;
                }
            }
            f = it + 1;
            longcount = 0;
            escaped = 0;
        }
        count += emitter(f, ctx, count);
    }

    return count;
}

int kprintf(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = format(print_emitter, NULL, fmt, ap);
    va_end(ap);
    return ret;
}

int sprintf(char *str, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = format(sprintf_emitter, str, fmt, ap);
    str[ret] = '\0';
    va_end(ap);
    return ret;
}