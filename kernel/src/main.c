#include <bootinfo.h>
#include <vga_basic/vga.h>
#include <memory/memmap.h>
#include <memory/palloc.h>
#include <bios_mmap.h>
#include <string.h>
#include <memory/paging.h>
#include <memory/halloc.h>
#include <gdt/gdt.h>
#include <kfmt.h>

void dumphex(uint64_t n, int count) {
    char buf[32];
    itohex(n, buf, count);
    vga_print(buf);
}

void kmain(bootinfo_t* binfo) {
    vga_init(80, 25);
    kfmt_out = vga_print;

    // Print MOTD
    vga_set_color(0x6E);
    kprintf("  === Hadron Unix ===  \n");
    kprintf("   The back fell off  \n");
    vga_set_color(0x0F);
    kprintf("\n\ncmdline: \"%s\"\n", (char*)(uint64_t)binfo->cmdline_addr);

    // Initialize the GDT
    gdt_init();
    k_gdt_cs = gdt_define_entry(gdt_empty(false), GDT_FLAG_CODE | GDT_FLAG_64BIT, 0);
    k_gdt_ds = gdt_define_entry(gdt_empty(false), GDT_FLAG_64BIT, 0);
    gdt_load(k_gdt_ds);

    // Initialize the memory map
    memmap_init();
    
    // Since the map can be unordered and items overlapping, define sections in a specific order
    mmap_entry_t* mmap = (mmap_entry_t*)(uint64_t)binfo->mmap_addr;
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type != MMAP_BIOS_ENTRY_TYPE_USABLE || !mmap[i].size) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_FREE);
    }
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type != MMAP_BIOS_ENTRY_TYPE_RSVD || !mmap[i].size) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_RESERVED);
    }
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type != MMAP_BIOS_ENTRY_TYPE_ACPI_REC || !mmap[i].size) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_ACPI);
    }
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type != MMAP_BIOS_ENTRY_TYPE_NVS || !mmap[i].size) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_ACPI);
    }
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type != MMAP_BIOS_ENTRY_TYPE_BAD_MEM || !mmap[i].size) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_DEAD);
    }

    // Define unknown types as dead
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type == MMAP_BIOS_ENTRY_TYPE_USABLE) { continue; }
        if (mmap[i].type == MMAP_BIOS_ENTRY_TYPE_RSVD) { continue; }
        if (mmap[i].type == MMAP_BIOS_ENTRY_TYPE_ACPI_REC) { continue; }
        if (mmap[i].type == MMAP_BIOS_ENTRY_TYPE_NVS) { continue; }
        if (mmap[i].type == MMAP_BIOS_ENTRY_TYPE_BAD_MEM) { continue; }
        if (!mmap[i].size) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_DEAD);
    }

    // Map the bios
    memmap_define(0x00000, 0x00500, MEMMAP_REGION_TYPE_BIOS);
    memmap_define(0x80000, 0x20000, MEMMAP_REGION_TYPE_BIOS);

    // Map elements loaded by the bootloader
    memmap_define(binfo->bootloader_base, binfo->bootloader_size, MEMMAP_REGION_TYPE_SOFTWARE);
    memmap_define(binfo->kernel_base, binfo->kernel_size, MEMMAP_REGION_TYPE_SOFTWARE);
    memmap_define(binfo->initrd_addr, binfo->initrd_size, MEMMAP_REGION_TYPE_SOFTWARE);
    memmap_define(binfo->cmdline_addr, binfo->kernel_size, MEMMAP_REGION_TYPE_SOFTWARE);
    memmap_define(binfo->mmap_addr, binfo->mmap_entry_count * sizeof(bootinfo_t), MEMMAP_REGION_TYPE_SOFTWARE);

    // Cleanup map
    memmap_agregate();

    // Initialize the physical allocator on available memory
    palloc_init(0, 0x3FFFFFFF, false);

    // Initialize paging
    paging_init();

    // Map all software and bios areas
    for (int i = 0; i < k_mem_map.entry_count; i++) {
        memmap_entry_t ent = k_mem_map.map[i];
        if (ent.type != MEMMAP_REGION_TYPE_SOFTWARE && ent.type != MEMMAP_REGION_TYPE_BIOS) { continue; }
        paging_map_multiple(k_pml4, ent.base, ent.base, PAGING_FLAG_RW, paging_area_size(ent.base, ent.size), false);
    }
    paging_map_multiple(k_pml4, 0xB8000, 0xB8000, PAGING_FLAG_RW, 80 * 25 * 2, false);
    palloc_map_buddies();

    // Enable paging
    paging_enable();

    // Initialize the physical allocator on the remaining memory
    palloc_init(0, 0xFFFFFFFFFFFFFFFF, true);

    kprintf("\n\nW E   B E   P A G I N '\n");

    for (uint64_t i = 0; i < k_mem_map.entry_count; i++) {
        memmap_entry_t ent = k_mem_map.map[i];

        char type[16];
        if (ent.type == MEMMAP_REGION_TYPE_DEAD) {
            sprintf(type, "DEAD");
        }
        else if (ent.type == MEMMAP_REGION_TYPE_RESERVED) {
            sprintf(type, "RESERVED");
        }
        else if (ent.type == MEMMAP_REGION_TYPE_HARDWARE) {
            sprintf(type, "HARDWARE");
        }
        else if (ent.type == MEMMAP_REGION_TYPE_BIOS) {
            sprintf(type, "BIOS");
        }
        else if (ent.type == MEMMAP_REGION_TYPE_ACPI) {
            sprintf(type, "ACPI");
        }
        else if (ent.type == MEMMAP_REGION_TYPE_SOFTWARE) {
            sprintf(type, "SOFTWARE");
        }
        else if (ent.type == MEMMAP_REGION_TYPE_FREE) {
            sprintf(type, "FREE");
        }
        else if (ent.type == MEMMAP_REGION_TYPE_ALLOCATABLE) {
            vga_print(" TYPE=ALLOCATABLE\n");
        }
        else {
            sprintf(type, "%08x", ent.type);
        }

        kprintf("[%02x]: BASE=%#016x SIZE=%#016x TYPE=%s\n", i, ent.base, ent.size, type);
    }

    void* bruh = malloc(420);
    void* bruh2 = malloc(420);
    kprintf("bruh(%p) bruh2(%p)\n", bruh, bruh2);

    free(bruh);
    free(bruh2);

    bruh = malloc(420);
    bruh2 = malloc(420);
    kprintf("bruh(%p) bruh2(%p)\n", bruh, bruh2);
}