#pragma once
#include <stdint.h>

struct bootinfo {
    uint32_t bootloader_base;
    uint32_t bootloader_size;
    uint32_t kernel_base;
    uint32_t kernel_size;
    uint32_t initrd_addr;
    uint32_t initrd_size;
    uint32_t cmdline_addr;
    uint32_t cmdline_size;
    uint32_t mmap_addr;
    uint32_t mmap_entry_count;
};
typedef struct bootinfo bootinfo_t;