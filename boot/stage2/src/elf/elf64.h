#pragma once
#include <stdint.h>

typedef uint64_t    Elf64_Addr;
typedef uint64_t    Elf64_Off;
typedef uint16_t    Elf64_Half;
typedef uint32_t    Elf64_Word;
typedef int32_t     Elf64_Sword;
typedef uint64_t    Elf64_Xword;
typedef int64_t     Elf64_Sxword;

struct Elf64_Ehdr {
    Elf64_Word magic;
    uint8_t file_class;
    uint8_t encoding;
    uint8_t version;
    uint8_t os_abi;
    uint8_t abi_version;
    uint8_t _pad0[7];
    Elf64_Half type;
    Elf64_Half machine;
    Elf64_Word obj_version;
    Elf64_Addr entry;
    Elf64_Off prog_hdr_offset;
    Elf64_Off section_hdr_offset;
    Elf64_Word flags;
    Elf64_Half elf_hdr_size;
    Elf64_Half prog_hdr_entry_size;
    Elf64_Half prog_hdr_entry_count;
    Elf64_Half section_hdr_entry_size;
    Elf64_Half section_hdr_entry_count;
    Elf64_Half section_hdr_str_tab_index;
}__attribute__((packed));
typedef struct Elf64_Ehdr Elf64_Ehdr_t;

#define ELF64_PROG_HEADER_TYPE_NULL     0
#define ELF64_PROG_HEADER_TYPE_LOAD     1
#define ELF64_PROG_HEADER_TYPE_DYNAMIC  2
#define ELF64_PROG_HEADER_TYPE_INTERP   3
#define ELF64_PROG_HEADER_TYPE_NOTE     4

struct Elf64_Phdr {
    Elf64_Word type;
    Elf64_Word flags;
    Elf64_Off offset;
    Elf64_Addr virt_addr;
    Elf64_Addr _rsvd0;
    Elf64_Xword size_file;
    Elf64_Xword size_mem;
    Elf64_Xword align;
}__attribute__((packed));
typedef struct Elf64_Phdr Elf64_Phdr_t;