#include "fat32.h"

fat32_error_t fat32_init(fat32_t *fat, fat32_sector_reader_t read_sectors, uint32_t part_lba_begin, void *ctx) {
	*fat = (fat32_t) {
		.part_lba_begin = part_lba_begin,
		.read_sectors = read_sectors,
		.ctx = ctx
	};

	uint8_t ws[FAT32_BYTES_PER_SECTOR];

	int ret = fat->read_sectors(fat->part_lba_begin, 1, ws, fat->ctx);
	if (ret) { return ret; }

	uint16_t signature = *(uint16_t*)(ws + FAT32_SIGNATURE_OFF);
	if (signature != FAT32_VOL_ID_SIGNATURE)  { return FAT32_INVALSIG; }

	uint16_t bps = *(uint16_t*)(ws + FAT32_BYTES_PER_SECTOR_OFF);
	if (bps != FAT32_BYTES_PER_SECTOR) { return FAT32_INVALBPS; }

	uint8_t fats = *(uint8_t*)(ws + FAT32_FATS_OFF);
	if (fats != FAT32_FATS) { return FAT32_INVALFATS; }

	fat->sectors_per_cluster = *(uint8_t*)(ws + FAT32_SECTORS_PER_CLUSTER_OFF);
	fat->reserved_sectors = *(uint16_t*)(ws + FAT32_RESERVED_SECTORS_OFF);
	fat->sectors_per_fat = *(uint32_t*)(ws + FAT32_SECTORS_PER_FAT_OFF);
	fat->root_cluster = *(uint32_t*)(ws + FAT32_ROOT_CLUSTER_OFF);

	fat->fat_lba_begin = fat->part_lba_begin + fat->reserved_sectors;
	fat->clusters_lba_begin = fat->fat_lba_begin + fat->sectors_per_fat * FAT32_FATS;
	
	return FAT32_OK;
}

uint32_t fat32_cluster_to_lba(fat32_t *fat, uint32_t cluster) {
	return fat->clusters_lba_begin + (cluster - 2) * fat->sectors_per_cluster;
}

uint32_t fat32_next_cluster(fat32_t *fat, uint32_t cluster) {
	uint32_t fatsector_off = cluster / 128;
	uint32_t fatsector = fat->fat_lba_begin + fatsector_off;

	uint8_t ws[FAT32_BYTES_PER_SECTOR];

	int ret = fat->read_sectors(fatsector, 1, ws, fat->ctx);
	if (ret) { return ret; }

	uint8_t fat_entry = cluster % 128;
	uint32_t next = ((uint32_t*)ws)[fat_entry];
	return next;
}

void fat32_root_dir(fat32_t *fat, fat32_entry_t *root) {
	*root = (fat32_entry_t) {
		.attribute = FAT32_ATTRIBUTE_DIRECTORY,
		.cluster = fat->root_cluster,
	};
}

fat32_entry_type_t fat32_parse_entry(uint8_t *addr, fat32_entry_t *ent) {
	if (addr[0] == 0) { return FAT32_EOD; }
	if (addr[0] == 0xE5) { return FAT32_UNUSED; }

	for (int i = FAT32_ENTRY_NAME_OFF; i < FAT32_ENTRY_ATTRIBUTE_OFF; i++) {
		ent->filename[i] = addr[i];
	}
	ent->filename[11] = '\0';

	ent->attribute = addr[FAT32_ENTRY_ATTRIBUTE_OFF];
	ent->cluster = ((*(uint16_t *)(addr + FAT32_ENTRY_FIRST_CLUSTER_HIGH_OFF)) << 16) | *(uint16_t *)(addr + FAT32_ENTRY_FIRST_CLUSTER_LOW_OFF);
	ent->size = *(uint32_t *)(addr + FAT32_ENTRY_SIZE_OFF);

	/* TODO: add LFN */

	return FAT32_NORMAL;
}

int streqfat(char *a, char *cstr) {
	int find = 0, cind = 0;
	char fatfmt[FAT32_FILENAME_MAX];
	while (find < FAT32_FILENAME_MAX) {
		if (cstr[cind] == '\0') {
			fatfmt[find++] = ' ';
			continue;
		}
		if (cstr[cind] == '.') {
			while (find < 8)
				fatfmt[find++] = ' ';
			cind++;
			continue;
		}

		fatfmt[find++] = cstr[cind++];
	}
	if (cstr[cind] != '\0') { return 0; }
		

	for (int i = 0; i < FAT32_FILENAME_MAX; i++) {
		if (a[i] != fatfmt[i]) { return 0; }
	}

	return 1;
}

fat32_error_t fat32_walk(fat32_t *fat, fat32_entry_t *dir, char *name, fat32_entry_t *ent) {
	if (!(dir->attribute & FAT32_ATTRIBUTE_DIRECTORY)) { return FAT32_NOTDIR; }

	uint8_t ws[FAT32_BYTES_PER_SECTOR];
	uint32_t cluster = dir->cluster;

	while (cluster != FAT32_FAT_LAST_CLUSTER) {
		int ret = fat->read_sectors(fat32_cluster_to_lba(fat, cluster), 1, ws, fat->ctx);
		if (ret) { return ret; }

		for (int i = 0; i < 16; i++) {
			uint8_t *entry = ws + i*32;
			fat32_entry_type_t type = fat32_parse_entry(entry, ent);
			if (type == FAT32_LFN || type == FAT32_UNUSED) { continue; }
			if (type == FAT32_EOD) { return FAT32_NOENTRY; }
			if (streqfat(ent->filename, name)) {
				return FAT32_OK;
			}
		}

		cluster = fat32_next_cluster(fat, cluster);
	}
	
	return FAT32_NOENTRY;
}

uint32_t fat32_skip_clusters(fat32_t *fat, uint32_t cluster, uint32_t count) {
	while (cluster != FAT32_FAT_LAST_CLUSTER && count > 0) {
		cluster = fat32_next_cluster(fat, cluster);
		count--;
	}
	return cluster;
}

int fat32_read(fat32_t *fat, fat32_entry_t *file, void *buf, uint32_t count, uint32_t offset) {
    if (offset > file->size) { return 0; }
        	
	if (count + offset > file->size) {
		count = file->size - offset;
	}

	uint32_t cluster = file->cluster;

	uint32_t wanted = offset / (FAT32_BYTES_PER_SECTOR * fat->sectors_per_cluster);
	cluster = fat32_skip_clusters(fat, cluster, wanted);

	uint8_t ws[FAT32_BYTES_PER_SECTOR * fat->sectors_per_cluster];

	uint32_t head = offset % (FAT32_BYTES_PER_SECTOR * fat->sectors_per_cluster);
	uint32_t index = 0;
	while (cluster != FAT32_FAT_LAST_CLUSTER && count > 0) {
		int ret = fat->read_sectors(fat32_cluster_to_lba(fat, cluster), fat->sectors_per_cluster, ws, fat->ctx);
		if (ret) { return ret; }

		for (uint32_t i = head; i < FAT32_BYTES_PER_SECTOR * fat->sectors_per_cluster && count > 0; i++, count--, index++) {
			((uint8_t*)buf)[index] = ws[i];
		}
		head = 0;

		cluster = fat32_next_cluster(fat, cluster);
	}

	return index;
}