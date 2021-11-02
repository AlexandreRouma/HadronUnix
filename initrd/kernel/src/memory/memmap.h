#pragma once
#include <stdint.h>

//#define MEMMAP_DEBUG

#define MEMMAP_MAX_ENTRIES      (4096 / sizeof(memmap_entry_t))

#define MEMMAP_INFO_FLAGS_IN        (1 << 0)
#define MEMMAP_INFO_FLAGS_BEFORE    (1 << 1)
#define MEMMAP_INFO_FLAGS_AFTER     (1 << 2)

enum memmap_region_type {
    MEMMAP_REGION_TYPE_DEAD,
    MEMMAP_REGION_TYPE_RESERVED,
    MEMMAP_REGION_TYPE_HARDWARE,
    MEMMAP_REGION_TYPE_BIOS,
    MEMMAP_REGION_TYPE_ACPI,
    MEMMAP_REGION_TYPE_SOFTWARE,
    MEMMAP_REGION_TYPE_ALLOCATABLE,
    MEMMAP_REGION_TYPE_FREE
};
typedef enum memmap_region_type memmap_region_type_t;

struct memmap_entry {
    uint64_t base;
    uint64_t size;
    memmap_region_type_t type;
};
typedef struct memmap_entry memmap_entry_t;

struct memmap {
    uint64_t entry_count;
    memmap_entry_t map[MEMMAP_MAX_ENTRIES];
};
typedef struct memmap memmap_t;

extern memmap_t k_mem_map;

void memmap_init();
void memmap_insert(uint64_t id, memmap_entry_t entry);
void memmap_remove(uint64_t first, uint64_t last);
void memmap_merge(uint64_t first, uint64_t last);
void memmap_split(uint64_t id, uint64_t offset, uint64_t size);
void memmap_cut_bottom(uint64_t id, uint64_t count);
void memmap_cut_top(uint64_t id, uint64_t count);

uint8_t memmap_get_address_info(uint64_t addr, uint64_t* in, uint64_t* before, uint64_t* after);
void memmap_define(uint64_t base, uint64_t size, memmap_region_type_t type);
void memmap_agregate();

#ifdef MEMMAP_DEBUG
void memmap_dump();
#endif
