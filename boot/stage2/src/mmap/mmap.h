#pragma once
#include <stdint.h>
#include <bios_mmap.h>

int mmap_get(mmap_entry_t* entries);

extern mmap_entry_t asm_mmap_buf;
extern uint32_t asm_mmap_get();