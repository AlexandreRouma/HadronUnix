#pragma once
#include <stdint.h>

enum fat32_error {
    FAT32_OK = 0,
    FAT32_INVALSIG,
    FAT32_INVALBPS,
    FAT32_INVALFATS,
    FAT32_NOENTRY,
    FAT32_NOTDIR,
    FAT32_EOF,
};
typedef enum fat32_error fat32_error_t;

#define FAT32_BYTES_PER_SECTOR_OFF 0x0B
#define FAT32_SECTORS_PER_CLUSTER_OFF 0x0D
#define FAT32_RESERVED_SECTORS_OFF 0x0E
#define FAT32_FATS_OFF 0x10
#define FAT32_SECTORS_PER_FAT_OFF 0x24
#define FAT32_ROOT_CLUSTER_OFF 0x2C
#define FAT32_SIGNATURE_OFF 0x1FE

#define FAT32_VOL_ID_SIGNATURE 0xAA55
#define FAT32_BYTES_PER_SECTOR 512
#define FAT32_FATS 2

#define FAT32_FAT_LAST_CLUSTER 0xFFFFFFFF

typedef int (*fat32_sector_reader_t)(uint32_t lba, uint32_t count, uint8_t *buf, void *ctx);

struct fat32 {
    uint32_t part_lba_begin;
    uint32_t fat_lba_begin;
    uint32_t clusters_lba_begin;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint32_t sectors_per_fat;
    uint32_t root_cluster;

    fat32_sector_reader_t read_sectors;
    void *ctx;
};
typedef struct fat32 fat32_t;

fat32_error_t fat32_init(fat32_t *fat, fat32_sector_reader_t read_sectors, uint32_t part_lba_begin, void *ctx);
uint32_t fat32_cluster_to_lba(fat32_t *fat, uint32_t cluster);
uint32_t fat32_next_cluster(fat32_t *fat, uint32_t cluster);

#define FAT32_FILENAME_MAX 11

#define FAT32_ATTRIBUTE_RO (1 << 0)
#define FAT32_ATTRIBUTE_HIDDEN (1 << 1)
#define FAT32_ATTRIBUTE_SYSTEM (1 << 2)
#define FAT32_ATTRIBUTE_VOL_ID (1 << 3)
#define FAT32_ATTRIBUTE_DIRECTORY (1 << 4)
#define FAT32_ATTRIBUTE_ARCHIVE (1 << 5)
#define FAT32_ATTRIBUTE_LFN (1 << 7)

#define FAT32_ENTRY_NAME_OFF 0x00
#define FAT32_ENTRY_ATTRIBUTE_OFF 0x0B
#define FAT32_ENTRY_FIRST_CLUSTER_HIGH_OFF 0x14
#define FAT32_ENTRY_FIRST_CLUSTER_LOW_OFF 0x1A
#define FAT32_ENTRY_SIZE_OFF 0x1C

struct fat32_entry {
    char filename[12];
    uint8_t attribute;
    uint32_t cluster;
    uint32_t size;
};
typedef struct fat32_entry fat32_entry_t;

enum fat32_entry_type {
    FAT32_NORMAL,
    FAT32_LFN,
    FAT32_UNUSED,
    FAT32_EOD
};
typedef enum fat32_entry_type fat32_entry_type_t;

fat32_entry_type_t fat32_parse_entry(uint8_t *addr, fat32_entry_t *ent);

void fat32_root_dir(fat32_t *fat, fat32_entry_t *root);
fat32_error_t fat32_walk(fat32_t *fat, fat32_entry_t *dir, char *name, fat32_entry_t *ent);
int fat32_read(fat32_t *fat, fat32_entry_t *file, void *buf, uint32_t count, uint32_t offset);
uint32_t fat32_skip_clusters(fat32_t *fat, uint32_t cluster, uint32_t count);