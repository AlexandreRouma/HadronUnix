#pragma once
#include <stdint.h>
#include <stdbool.h>

#define GDT_FLAG_CONFORMING (1 << 2)
#define GDT_FLAG_CODE       (1 << 3)
#define GDT_FLAG_NON_SYS    (1 << 4)
#define GDT_FLAG_PRESENT    (1 << 7)
#define GDT_FLAG_64BIT      (1 << 13)
#define GDT_FLAG_32BIT      (1 << 14)

#define GDT_SYS_TYPE_LDT        2
#define GDT_SYS_TYPE_AVL_TSS    9
#define GDT_SYS_TYPE_BUSY_TSS   11

#define GDT_ENTRY_COUNT     256

#define GDT_INVALID         0xFFFF

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint16_t flags;
    uint8_t base_high;
}__attribute__((packed));
typedef struct gdt_entry gdt_entry_t;

struct gdtr {
    uint16_t limit;
    uint64_t base;
}__attribute__((packed));
typedef struct gdtr gdtr_t;

extern uint16_t k_gdt_cs;
extern uint16_t k_gdt_ds;

void gdt_init();
void gdt_load(uint16_t ds);
uint16_t gdt_define_entry(int id, uint16_t flags, uint8_t dpl);
uint16_t gdt_define_system_entry(int id, uint64_t base, uint64_t limit, uint16_t flags, uint8_t dpl);
void gdt_clear_entry(int id);
void gdt_clear_system_entry(int id);
int gdt_empty(bool system);

extern void asm_gdt_load(uint64_t cs);