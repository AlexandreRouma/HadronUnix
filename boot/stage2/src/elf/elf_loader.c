#include "elf_loader.h"
#include "elf64.h"
#include <vga_basic/vga.h>

int elf_loader_load(fat32_t* fat, fat32_entry_t* file, uint64_t* entry, uint64_t* lowest_addr, uint64_t* highest_addr) {
    // Load header
    Elf64_Ehdr_t hdr;
    fat32_read(fat, file, &hdr, sizeof(Elf64_Ehdr_t), 0);

    *entry = hdr.entry;

    // Read program header 4 entries at a time
    Elf64_Phdr_t phdr[ELF_LOADER_CONSEC_PHDRS];
    int phdr_count = hdr.prog_hdr_entry_count;
    int read = 0;
    *lowest_addr = 0xFFFFFFFF;
    *highest_addr = 0;
    while (phdr_count) {
        int to_read = ELF_LOADER_CONSEC_PHDRS;
        if (phdr_count < to_read) { to_read = phdr_count; }

        fat32_read(fat, file, phdr, sizeof(Elf64_Phdr_t) * ELF_LOADER_CONSEC_PHDRS, hdr.prog_hdr_offset + (read * sizeof(Elf64_Phdr_t)));

        for (int i = 0; i < to_read; i++) {
            if (phdr[i].type == ELF64_PROG_HEADER_TYPE_NULL) {
                // Nothing to do
            }
            else if (phdr[i].type == ELF64_PROG_HEADER_TYPE_LOAD) {
                // Load into memory
                fat32_read(fat, file, phdr[i].virt_addr, phdr[i].size_file, phdr[i].offset);

                // Check min and max addresses
                if (phdr[i].virt_addr < *lowest_addr) {
                    *lowest_addr = phdr[i].virt_addr;
                }
                if (phdr[i].virt_addr + phdr[i].size_mem > *highest_addr) {
                    *highest_addr = phdr[i].virt_addr + phdr[i].size_mem;
                }
            }
            else if (phdr[i].type == ELF64_PROG_HEADER_TYPE_DYNAMIC) {
                // Unsupported, return error
                return 1;
            }
            else if (phdr[i].type == ELF64_PROG_HEADER_TYPE_INTERP) {
                // Unsupported, return error
                return 1;
            }
            else if (phdr[i].type == ELF64_PROG_HEADER_TYPE_NOTE) {
                // Nothing to do
            }
            else {
                return 1;
            }
        }

        phdr_count -= to_read;
        read += to_read;
    }

    return 0;
}