#pragma once
#include <stdint.h>

#define MMAP_BIOS_MAGIC                 0x534D4150
#define MMAP_BIOS_ENTRY_TYPE_USABLE     1
#define MMAP_BIOS_ENTRY_TYPE_RSVD       2
#define MMAP_BIOS_ENTRY_TYPE_ACPI_REC   3
#define MMAP_BIOS_ENTRY_TYPE_NVS        4
#define MMAP_BIOS_ENTRY_TYPE_BAD_MEM    5

struct mmap_entry {
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t acpi_attr;
}__attribute__((packed));
typedef struct mmap_entry mmap_entry_t;