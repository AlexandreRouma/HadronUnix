#pragma once
#include <stdint.h>

struct idt_entry {
    uint16_t offset_low;
    uint16_t segment;
    uint16_t flags;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t _rsvd;
}__attribute__((packed));
typedef struct idt_entry idt_entry_t;

struct idtr {
    uint16_t limit;
    uint64_t base;
}__attribute__((packed));
typedef struct idtr idtr_t;

#define IDT_ENTRY_COUNT     256

#define IDT_FLAG_PRESENT    (1 << 15)

#define IDT_GATE_TYPE_CALL_GATE     12
#define IDT_GATE_TYPE_INT_GATE      14
#define IDT_GATE_TYPE_TRAP_GATE     15

void idt_init();
void idt_load();
void idt_set_entry(int id, void* handler, uint32_t segment, uint8_t type, uint8_t dpl);
void idt_clear_entry(int id);