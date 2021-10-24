#pragma once
#include <stdint.h>

#define IDT_GATE_TYPE_TASK  0b101
#define IDT_GATE_TYPE_INT   0b110
#define IDT_GATE_TYPE_TRAP  0b111

#define IDT_GATE_SIZE_16BIT 0
#define IDT_GATE_SIZE_32BIT 1

#define IDT_ENTRY_COUNT 255
#define IDT_ENTRY_SIZE  sizeof(idt_gate_descriptor_t)

struct idt_gate_descriptor {
    uint16_t offset_low;
    uint16_t segment;
    uint8_t _rsvd0;
    uint8_t gate_type:3;
    uint8_t gate_size:1;
    uint8_t _rsvd1:1;
    uint8_t dpl:2;
    uint8_t present:1;
    uint16_t offset_high;
}__attribute__((packed));
typedef struct idt_gate_descriptor idt_gate_descriptor_t;

struct idt_register {
    uint16_t limit;
    uint32_t base;
}__attribute__((packed));
typedef struct idt_register idt_register_t;

void idt_init();
void idt_set_gate(uint8_t id, uint32_t offset, uint16_t segment, uint8_t gate_type, uint8_t gate_size, uint8_t dpl);
void idt_clear_gate(uint8_t id);