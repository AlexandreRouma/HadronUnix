#include "gdt.h"
#include <string.h>

gdt_entry_t k_gdt[GDT_ENTRY_COUNT];
gdtr_t k_gdtr;

uint16_t k_gdt_cs = 0;
uint16_t k_gdt_ds = 0;

void gdt_init() {
    memset(k_gdt, 0, sizeof(k_gdt));
    k_gdtr.base = (uint64_t)&k_gdt[0];
    k_gdtr.limit = sizeof(k_gdt) - 1;
}

void gdt_load(uint16_t ds) {
    asm_gdt_load(ds);
}

uint16_t gdt_define_entry(int id, uint16_t flags, uint8_t dpl) {
    if (id <= 0 || id >= GDT_ENTRY_COUNT) { return GDT_INVALID; }
    gdt_entry_t* entry = &k_gdt[id];

    entry->flags = flags | GDT_FLAG_PRESENT | GDT_FLAG_NON_SYS | ((dpl & 0b11) << 5);
    entry->limit_low = 0;
    entry->base_low = 0;
    entry->base_mid = 0;
    entry->base_high = 0;

    return (uint16_t)id * 8;
}

uint16_t gdt_define_system_entry(int id, uint64_t base, uint64_t limit, uint16_t flags, uint8_t dpl) {
    // WARNING: This seems to create a 32bit segment (according to bochs). Might cause future issues
    if (id <= 0 || id >= GDT_ENTRY_COUNT - 1) { return GDT_INVALID; }
    gdt_entry_t* entry = &k_gdt[id];
    gdt_entry_t* next = &k_gdt[id+1];
    
    // Lower entry
    entry->limit_low = limit & 0xFFFF;
    entry->base_low = base & 0xFFFF;
    entry->base_mid = (base >> 16) & 0xFF;
    entry->flags = flags | GDT_FLAG_PRESENT | ((dpl & 0b11) << 5);
    entry->base_high = (base >> 24) & 0xFF;
    
    // Upper entry
    next->limit_low = (base >> 32) & 0xFFFF;
    next->base_low = (base >> 48) & 0xFFFF;;
    next->base_mid = 0;
    next->flags = 0;
    next->base_high = 0;

    return (uint16_t)id * 8;
}

void gdt_clear_entry(int id) {
    if (id <= 0 || id >= GDT_ENTRY_COUNT) { return -1; }
    gdt_entry_t* entry = &k_gdt[id];
    entry->flags = 0;
    entry->limit_low = 0;
    entry->base_low = 0;
    entry->base_mid = 0;
    entry->base_high = 0;
}

void gdt_clear_system_entry(int id) {
    if (id <= 0 || id >= GDT_ENTRY_COUNT - 1) { return -1; }
    gdt_entry_t* entry = &k_gdt[id];
    gdt_entry_t* next = &k_gdt[id+1];
    entry->flags = 0;
    entry->limit_low = 0;
    entry->base_low = 0;
    entry->base_mid = 0;
    entry->base_high = 0;
    next->flags = 0;
    next->limit_low = 0;
    next->base_low = 0;
    next->base_mid = 0;
    next->base_high = 0;
}

int gdt_empty(bool system) {
    int count = system ? (GDT_ENTRY_COUNT - 1) : GDT_ENTRY_COUNT;
    for (int i = 1; i < count; i++) {
        gdt_entry_t* entry = &k_gdt[i];
        gdt_entry_t* next = &k_gdt[i+1];

        if (entry->flags & GDT_FLAG_PRESENT) {
            // Skip one more entry if this it's a system segment
            if (!(entry->flags & GDT_FLAG_NON_SYS)) { i++; }
            continue;
        }

        // If we're searching for a system segment, check that the next one is free too
        if (system && (next->flags & GDT_FLAG_PRESENT)) { continue; }

        return i;
    }
    return -1;
}