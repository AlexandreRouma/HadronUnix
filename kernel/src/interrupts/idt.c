#include "idt.h"
#include "string.h"

idt_entry_t k_idt[IDT_ENTRY_COUNT];
idtr_t k_idtr;

void idt_init() {
    memset(k_idt, 0, sizeof(k_idt));
    k_idtr.base = (uint64_t)&k_idt[0];
    k_idtr.limit = sizeof(k_idt) - 1;
}

void idt_load() {
    asm ("lidt %0" : : "m"(k_idtr));
}

void idt_set_entry(int id, void* handler, uint32_t segment, uint8_t type, uint8_t dpl) {
    idt_entry_t* entry = &k_idt[id];
    uint64_t offset = (uint64_t)handler;
    entry->offset_low = offset & 0xFFFF;
    entry->segment = segment;
    entry->flags = ((uint16_t)type << 8) | ((uint16_t)dpl << 13) | IDT_FLAG_PRESENT;
    entry->offset_mid = (offset >> 16) & 0xFFFF;
    entry->offset_high = (offset >> 32) & 0xFFFFFFFF;
}

void idt_clear_entry(int id) {
    idt_entry_t* entry = &k_idt[id];
    entry->offset_low = 0;
    entry->segment = 0;
    entry->flags = 0;
    entry->offset_mid = 0;
    entry->offset_high = 0;
}