#include "palloc.h"
#include "buddy_alloc.h"
#include "memmap.h"
#include "paging.h"
#include <stddef.h>
#include <vga_basic/vga.h>

bool PALLOC_DEBUG = false;

void palloc_init(uint64_t min_addr, uint64_t max_addr, bool allow_mapping) {
    for (int i = 0; i < k_mem_map.entry_count; i++) {
        memmap_entry_t entry = k_mem_map.map[i];

        // If not a free region of memory, ignore
        if (entry.type != MEMMAP_REGION_TYPE_FREE) { continue; }

        // Check min address
        if (entry.base < min_addr) { continue; }

        // Calculate region pages
        uint64_t first_page = entry.base ? (((entry.base - 1) >> 12) + 1) << 12 : 0;
        if (first_page >= entry.base + entry.size) { continue; }
        uint64_t page_count = ((entry.base + entry.size) - first_page) >> 12;

        // Check that we have enough space for allocation
        uint64_t bud_page_count = ((buddy_get_size(page_count, PALLOC_MAX_BUDDY_ORDER) - 1) >> 12) + 1;
        if (bud_page_count >= page_count) { continue; }

        // Initialize buddy
        // TODO: Figure out how to map it after the first init (or when paging is enabled, it'll break)
        if (allow_mapping) {
            paging_map_multiple(NULL, first_page, first_page, PAGING_ENTRY_FLAG_RW, bud_page_count, false);
        }
        buddy_t* bud = (buddy_t*)first_page;
        void* ctx = (void*)(first_page + (bud_page_count << 12));
        buddy_create(bud, ctx, page_count - bud_page_count, PALLOC_MAX_BUDDY_ORDER);

        // Mark the area as allocatable
        k_mem_map.map[i].type = MEMMAP_REGION_TYPE_ALLOCATABLE;
    }
}

void palloc_map_buddies() {
    for (int i = 0; i < k_mem_map.entry_count; i++) {
        memmap_entry_t entry = k_mem_map.map[i];

        // If not an allocatable region of memory, ignore
        if (entry.type != MEMMAP_REGION_TYPE_ALLOCATABLE) { continue; }

        // Map the buddy memory
        uint64_t first_page = entry.base ? (((entry.base - 1) >> 12) + 1) << 12 : 0;
        uint64_t page_count = ((entry.base + entry.size) - first_page) >> 12;
        uint64_t bud_page_count = ((buddy_get_size(page_count, PALLOC_MAX_BUDDY_ORDER) - 1) >> 12) + 1;
        paging_map_multiple(k_pml4, first_page, first_page, PAGING_ENTRY_FLAG_RW, bud_page_count, false);
    }
}

void* palloc_alloc(uint64_t count) {
    for (int i = 0; i < k_mem_map.entry_count; i++) {
        memmap_entry_t entry = k_mem_map.map[i];

        // If not an allocatable region of memory, ignore
        if (entry.type != MEMMAP_REGION_TYPE_ALLOCATABLE) { continue; }

        // Try to allocate, return if successful
        buddy_t* bud = (buddy_t*)(entry.base ? (((entry.base - 1) >> 12) + 1) << 12 : 0);
        uint64_t idx = buddy_find(bud, count);
        if (idx == BUDDY_ALLOC_FAILED) { continue; }

        // Mark area as allocated
        buddy_alloc(bud, idx, count);

        if (PALLOC_DEBUG) {
            vga_print("Allocated pages: ");
            char test[32];
            itoa(count, test, 31);
            vga_println(test);
        }

        // Return pointer
        return (void*)((idx << 12) + (uint64_t)bud->data_start);
    }

    return BUDDY_ALLOC_FAILED;
}

void palloc_free(void* page, uint64_t count) {
    uint64_t page_addr = (uint64_t)page;
    for (int i = 0; i < k_mem_map.entry_count; i++) {
        memmap_entry_t entry = k_mem_map.map[i];

        // If not an allocatable region of memory, ignore
        if (entry.type != MEMMAP_REGION_TYPE_ALLOCATABLE) { continue; }

        // Check if the page comes from this area
        if (page_addr < entry.base || page_addr >= entry.base + entry.size) { continue; }

        if (PALLOC_DEBUG) {
            vga_print("Freed pages: ");
            char test[32];
            itoa(count, test, 31);
            vga_println(test);
        }

        // Free
        buddy_t* bud = (buddy_t*)(entry.base ? (((entry.base - 1) >> 12) + 1) << 12 : 0);
        buddy_free(bud, (page_addr - (uint64_t)bud->data_start) >> 12, count);
    }
}