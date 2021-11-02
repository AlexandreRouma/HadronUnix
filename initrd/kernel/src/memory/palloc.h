#pragma once
#include <stdint.h>
#include <stdbool.h>

// 2^16 page buddy = 256MB
#define PALLOC_MAX_BUDDY_ORDER  15

void palloc_init(uint64_t min_addr, uint64_t max_addr, bool allow_mapping);
void palloc_map_buddies();
void* palloc_alloc(uint64_t count);
void palloc_free(void* page, uint64_t count);