#pragma once
#include <stdint.h>
#include <stddef.h>

#define HALLOC_BLOCK 4096

struct alloc_header {
	uint32_t next; // next in freelist
	uint32_t bef;  // previous in freelist
	uint32_t size; // position of contiguous next (thus size)
	uint32_t prev; // position of contiguous prev
	char meta; // meta
};
typedef struct alloc_header alloc_header_t;

struct block_header {
	struct block_header *next;
	uint32_t rc;
	uint32_t size;
	uint32_t freelist;
};
typedef struct block_header block_header_t;

extern block_header_t *halloc_first;

void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
