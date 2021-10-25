#pragma once
#include <fat32/fat32.h>
#include <stdint.h>

#define ELF_LOADER_CONSEC_PHDRS   4

int elf_loader_load(fat32_t* fat, fat32_entry_t* file, uint64_t* entry, uint64_t* lowest_addr, uint64_t* highest_addr);