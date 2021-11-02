#pragma once

#include <stdarg.h>
#include <stddef.h>

extern int (*kfmt_out)(char *str);

int format(int (*emitter)(char *str, void *ctx, size_t offset), void *ctx, char *fmt, va_list ap);

int print_emitter(char *str, void *ctx, size_t offset);
int sprintf_emitter(char *str, void *ctx, size_t offset);

int kprintf(char *fmt, ...);
int sprintf(char *str, char *fmt, ...);