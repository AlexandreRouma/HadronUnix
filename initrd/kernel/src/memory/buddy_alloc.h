#pragma once
#include <stdint.h>

//#define BUDDY_ALLOC_DEBUG

#define BUDDY_ALLOC_FAILED      (~(uint64_t)0)

struct buddy_desc {
    uint8_t* ptr;
    uint64_t count;
};
typedef struct buddy_desc buddy_desc_t;

struct buddy {
    void* data_start;
    uint64_t block_count;
    uint16_t order;
    buddy_desc_t buddies[];
};
typedef struct buddy buddy_t;

// Prediction functions
uint16_t buddy_get_possible_order(uint64_t block_count, uint16_t max_order);
uint64_t buddy_get_count_at_order(uint64_t block_count, uint16_t order);
uint64_t buddy_get_size_at_order(uint64_t block_count, uint16_t order);
uint64_t buddy_get_size(uint64_t block_count, uint16_t max_order);

void buddy_create(buddy_t* buddy, void* data_start, uint64_t block_count, uint16_t max_order);
uint64_t buddy_find(buddy_t* buddy, uint64_t count);
void buddy_alloc(buddy_t* buddy, uint64_t offset, uint64_t count);
void buddy_free(buddy_t* buddy, uint64_t offset, uint64_t count);

#ifdef BUDDY_ALLOC_DEBUG
void buddy_dump(buddy_t* buddy);
#endif