#include <vga_basic/vga.h>
#include <interrupt/pic.h>
#include <interrupt/idt.h>

void stage2_main() {
    // Initialize VGA driver
    vga_init(80, 25);

    // Initialize the PIC
    pic_init(0x20, 0x28);

    // Intialize and load the IDT
    idt_init();
    idt_set_gate(0, 0x4269, 8, IDT_GATE_TYPE_TRAP, IDT_GATE_SIZE_32BIT, 0);

    vga_print("Load my boot");
}