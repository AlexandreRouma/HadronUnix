#pragma once
#include <stdint.h>

enum {
    FB_TYPE_TEXT,
    FB_TYPE_GRAPHIC
};

struct fbinfo {
    uint8_t type;
    uint64_t addr;
    uint16_t width;
    uint16_t height;
    uint16_t pitch;
    uint16_t depth;
    uint8_t red_mask;
    uint8_t red_pos;
    uint8_t green_mask;
    uint8_t green_pos;
    uint8_t blue_mask;
    uint8_t blue_pos;
}__attribute__((packed));
typedef struct fbinfo fbinfo_t;

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
    fbinfo_t fbinfo;
}__attribute__((packed));
typedef struct bootinfo bootinfo_t;