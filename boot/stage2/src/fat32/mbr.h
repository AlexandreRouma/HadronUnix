#pragma once
#include <stdint.h>

struct mbr_part_entry {
    uint32_t flemme0;
    uint32_t flemme1;
    uint32_t lba_start;
    uint32_t flemme2;
}__attribute__((packed));
typedef struct mbr_part_entry mbr_part_entry_t;

struct mbr {
    uint32_t disk_id;
    uint16_t _rsvd0;
    mbr_part_entry_t part0;
    mbr_part_entry_t part1;
    mbr_part_entry_t part2;
    mbr_part_entry_t part3;
}__attribute__((packed));
typedef struct mbr mbr_t;