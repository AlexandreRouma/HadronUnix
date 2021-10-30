#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void* memcpy(void* dstptr, void* srcptr, uint64_t size);
void* memset(void* bufptr, uint8_t value, uint64_t size);
size_t strlen(char* str);
int strcmp(char* str1, char* str2);
int strmatch(char* str1, char* str2);
void itoa(int n, char* buf, int buflen);