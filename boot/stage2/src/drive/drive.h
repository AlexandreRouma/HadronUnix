#pragma once
#include <stdint.h>
#include <stdbool.h>

#define DRIVE_BIOS_MAX_SECTORS  127
#define DRIVE_BIOS_SECTOR_SIZE  512

struct drive_info {
    uint8_t drive_id;
    uint8_t head_count;
    uint8_t sector_per_track;
};
typedef struct drive_info drive_info_t;

int drive_get_info(drive_info_t* info, uint8_t id);
void drive_lba_to_chs(drive_info_t* info, uint32_t lba, uint16_t* cylinder, uint8_t* head, uint8_t* sector);
int drive_read_sectors(drive_info_t* info, uint32_t lba, uint32_t count, uint8_t* buf);

extern uint32_t asm_drive_get_info();
extern uint32_t asm_drive_read_sectors();
extern uint8_t drive_scratch_buffer[DRIVE_BIOS_MAX_SECTORS];