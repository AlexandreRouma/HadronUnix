#pragma once
#include <fat32/fat32.h>
#include <stdint.h>

#define ELF_LOADER_CONSEC_PHDRS   4

int elf_loader_load(fat32_t* fat, fat32_entry_t* file, uint32_t* entry, uint32_t* lowest_addr, uint32_t* highest_addr);