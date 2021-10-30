#include <bootinfo.h>
#include <vga_basic/vga.h>
#include <memory/memmap.h>
#include <memory/palloc.h>
#include <bios_mmap.h>
#include <string.h>
#include <memory/paging.h>
#include <memory/halloc.h>

void dumphex(uint64_t n, int count) {
    char buf[32];
    itohex(n, buf, count);
    vga_print(buf);
}

void kmain(bootinfo_t* binfo) {
    vga_init(80, 25);

    // Print MOTD
    vga_set_color(0x6E);
    vga_println("  === Hadron Unix ===  ");
    vga_print("   The back fell off  ");
    vga_set_color(0x0F);
    vga_print("\n\n");
    vga_print("cmdline: \"");
    vga_print((char*)(uint64_t)binfo->cmdline_addr);
    vga_print("\"");

    // Initialize the memory map
    memmap_init();
    
    // Since the map can be unordered and items overlapping, define sections in a specific order
    mmap_entry_t* mmap = (mmap_entry_t*)(uint64_t)binfo->mmap_addr;
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type != MMAP_BIOS_ENTRY_TYPE_USABLE) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_FREE);
    }
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type != MMAP_BIOS_ENTRY_TYPE_RSVD) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_RESERVED);
    }
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type != MMAP_BIOS_ENTRY_TYPE_ACPI_REC) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_ACPI);
    }
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type != MMAP_BIOS_ENTRY_TYPE_NVS) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_ACPI);
    }
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type != MMAP_BIOS_ENTRY_TYPE_BAD_MEM) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_DEAD);
    }

    // Define unknown types as dead
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type == MMAP_BIOS_ENTRY_TYPE_USABLE) { continue; }
        if (mmap[i].type == MMAP_BIOS_ENTRY_TYPE_RSVD) { continue; }
        if (mmap[i].type == MMAP_BIOS_ENTRY_TYPE_ACPI_REC) { continue; }
        if (mmap[i].type == MMAP_BIOS_ENTRY_TYPE_NVS) { continue; }
        if (mmap[i].type == MMAP_BIOS_ENTRY_TYPE_BAD_MEM) { continue; }
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

    vga_print("\n\n");

    for (uint64_t i = 0; i < k_mem_map.entry_count; i++) {
        memmap_entry_t ent = k_mem_map.map[i];
        vga_print("[");
        dumphex(i, 2);
        vga_print("]: BASE=0x");
        dumphex(ent.base, 16);
        vga_print(" SIZE=0x");
        dumphex(ent.size, 16);
        if (ent.type == MEMMAP_REGION_TYPE_DEAD) {
            vga_print(" TYPE=DEAD\n");
        }
        else if (ent.type == MEMMAP_REGION_TYPE_RESERVED) {
            vga_print(" TYPE=RESERVED\n");
        }
        else if (ent.type == MEMMAP_REGION_TYPE_HARDWARE) {
            vga_print(" TYPE=HARDWARE\n");
        }
        else if (ent.type == MEMMAP_REGION_TYPE_BIOS) {
            vga_print(" TYPE=BIOS\n");
        }
        else if (ent.type == MEMMAP_REGION_TYPE_ACPI) {
            vga_print(" TYPE=ACPI\n");
        }
        else if (ent.type == MEMMAP_REGION_TYPE_SOFTWARE) {
            vga_print(" TYPE=SOFTWARE\n");
        }
        else if (ent.type == MEMMAP_REGION_TYPE_FREE) {
            vga_print(" TYPE=FREE\n");
        }
        else {
            vga_print(" TYPE=");
            dumphex(ent.type, 8);
            vga_print("\n");
        }
    }

    // Initialize the physical allocator on available memory
    palloc_init(0, 0x3FFFFFFF, false);

    // Initialize paging
    paging_init();

    

    // Map all software and bios areas
    for (int i = 0; i < k_mem_map.entry_count; i++) {
        memmap_entry_t ent = k_mem_map.map[i];
        if (ent.type != MEMMAP_REGION_TYPE_SOFTWARE && ent.type != MEMMAP_REGION_TYPE_BIOS) { continue; }
        paging_map_multiple(k_pml4, ent.base, ent.base, PAGING_ENTRY_FLAG_RW, paging_area_size(ent.base, ent.size), false);
    }
    paging_map_multiple(k_pml4, 0xB8000, 0xB8000, PAGING_ENTRY_FLAG_RW, 80 * 25 * 2, false);
    palloc_map_buddies();

    // Enable paging
    paging_enable();

    vga_println("W E   B E   P A G I N '");

    PALLOC_DEBUG = true;

    void* bruh = malloc(420);
    void* bruh2 = malloc(420);
    dumphex(bruh, 16); vga_print("\n");
    dumphex(bruh2, 16);

    free(bruh);
    free(bruh2);

    bruh = malloc(420);
    bruh2 = malloc(420);
    dumphex(bruh, 16); vga_print("\n");
    dumphex(bruh2, 16);
}