#include "memmap.h"
#include <string.h>
#include <memory.h>
#include <stdbool.h>

#ifdef MEMMAP_DEBUG
#include <inttypes.h>
#endif

memmap_t k_mem_map;

void memmap_init() {
    k_mem_map.entry_count = 0;
}

void memmap_insert(uint64_t id, memmap_entry_t entry) {
    if (k_mem_map.entry_count-id > 0) {
        memmove(&k_mem_map.map[id+1], &k_mem_map.map[id], k_mem_map.entry_count-id);
    }
    k_mem_map.map[id] = entry;
    k_mem_map.entry_count++;
}

void memmap_remove(uint64_t first, uint64_t last) {
    if (k_mem_map.entry_count-last-1 > 0) {
        memmove(&k_mem_map.map[first], &k_mem_map.map[last+1], k_mem_map.entry_count-last-1);   
    }
    k_mem_map.entry_count -= last - first + 1;
}

void memmap_merge(uint64_t first, uint64_t last) {
    memmap_entry_t fentry = k_mem_map.map[first];
    memmap_entry_t lentry = k_mem_map.map[last];
    memmap_remove(first, last);
    memmap_insert(first, (memmap_entry_t){
        .base = fentry.base,
        .size = (lentry.base + lentry.size) - fentry.base,
        .type = fentry.type
    });
}

void memmap_split(uint64_t id, uint64_t offset, uint64_t size) {
    memmap_entry_t entry = k_mem_map.map[id];

    // If the offset is null, and the size is equal, remove the entry
    if (!offset && size == entry.size) {
        memmap_remove(id, id);
        return;
    }

    // if the offset is null, add size to the base
    if (!offset) {
        k_mem_map.map[id].base += size;
        k_mem_map.map[id].size -= size;
        return;
    }
    
    // If the end of the entry is the same as the end of the split, cut the end
    if (offset + size == entry.size) {
        k_mem_map.map[id].size -= size;
        return;
    }

    // Otherwise, create a hole in the middle of the entry
    k_mem_map.map[id].size = offset;
    memmap_insert(id+1, (memmap_entry_t){
        .base = entry.base + offset + size,
        .size = entry.size - offset - size,
        .type = entry.type
    });
}

uint8_t memmap_get_address_info(uint64_t addr, uint64_t* in, uint64_t* before, uint64_t* after) {
    memmap_entry_t entry;

    // Find entry that contains the address
    for (int i = 0; i < k_mem_map.entry_count; i++) {
        entry = k_mem_map.map[i];
        if (addr >= entry.base && addr < entry.base + entry.size) {
            *in = i;
            break;
        }
    }

    // Find entry after the address
    for (int i = 0; i < k_mem_map.entry_count; i++) {
        entry = k_mem_map.map[i];
        if (entry.base > addr) {
            *after = i;
            break;
        }
    }

    // Find entry before the address
    for (int i = 0; i < k_mem_map.entry_count; i++) {
         
    }
}

void memmap_define(uint64_t base, uint64_t size, memmap_region_type_t type) {

}

#ifdef MEMMAP_DEBUG
void memmap_dump() {
    printf("=================== MEMMAP ===================\n");
    for (int i = 0; i < k_mem_map.entry_count; i++) {
        printf("[%02d]: BASE=0x%08" PRIX64 " SIZE=0x%08" PRIX64 " TYPE=%d\n", i, k_mem_map.map[i].base, k_mem_map.map[i].size, k_mem_map.map[i].type);
    }
}
#endif