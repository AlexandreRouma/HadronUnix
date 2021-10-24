#include <interrupt/idt.h>

idt_register_t _K_IDTR_ __attribute__((aligned(8)));
idt_gate_descriptor_t _K_IDT_[IDT_ENTRY_COUNT] __attribute__((aligned(8)));

void idt_init() {
    // Setup IDTR
    _K_IDTR_.base = (uint32_t)&_K_IDT_;
    _K_IDTR_.limit = sizeof(_K_IDT_) - 1;

    // Clear all entries
    for (int i = 0; i < IDT_ENTRY_COUNT; i++) {
        idt_clear_gate(i);
    }

    // Load the IDT
    asm ("lidt %0" : : "m"(_K_IDTR_));
}

void idt_set_gate(uint8_t id, uint32_t offset, uint16_t segment, uint8_t gate_type, uint8_t gate_size, uint8_t dpl) {
    _K_IDT_[id].offset_low = offset & 0xFFFF;
    _K_IDT_[id].segment = segment;
    _K_IDT_[id]._rsvd0 = 0;
    _K_IDT_[id].gate_type = gate_type;
    _K_IDT_[id].gate_size = gate_size;
    _K_IDT_[id]._rsvd1 = 0;
    _K_IDT_[id].dpl = dpl;
    _K_IDT_[id].present = 1;
    _K_IDT_[id].offset_high = (offset >> 16) && 0xFFFF;
}

void idt_clear_gate(uint8_t id) {
    _K_IDT_[id].offset_low = 0;
    _K_IDT_[id].segment = 0;
    _K_IDT_[id]._rsvd0 = 0;
    _K_IDT_[id].gate_type = 0;
    _K_IDT_[id].gate_size = 0;
    _K_IDT_[id]._rsvd1 = 0;
    _K_IDT_[id].dpl = 0;
    _K_IDT_[id].present = 0;
    _K_IDT_[id].offset_high = 0;
}