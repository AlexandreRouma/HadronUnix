#include <interrupt/pic.h>
#include <sys/io.h>

void pic_init(uint8_t master_offset, uint8_t slave_offset) {
    // Start initializing PICs (ICW4 will be needed)
    outb(PIC_MASTER, PIC_ICW1_INIT | PIC_ICW1_IC4);
    outb(PIC_SLAVE, PIC_ICW1_INIT | PIC_ICW1_IC4);

    // Set offset for both PICs (will be floored to a multiple of 8)
    outb(PIC_MASTER | PIC_A0, master_offset);
    outb(PIC_SLAVE | PIC_A0, slave_offset);

    // Tell the master pic it has a slave on IR2
    outb(PIC_MASTER | PIC_A0, PIC_ICW3_M_S2);

    // Tell the slave PIC it's connected to IR2 on the master
    outb(PIC_SLAVE | PIC_A0, 2);

    // Set PICs to 8086 mode
    outb(PIC_MASTER | PIC_A0, PIC_ICW4_UPM);
    outb(PIC_SLAVE | PIC_A0, PIC_ICW4_UPM);

    // Mask all IRQs
    pic_set_irq_mask(PIC_MASTER, 0xFF);
    pic_set_irq_mask(PIC_SLAVE, 0xFF);
}

uint8_t pic_get_irq_mask(uint8_t pic) {
    return inb(pic | PIC_A0);
}

void pic_set_irq_mask(uint8_t pic, uint8_t mask) {
    outb(pic | PIC_A0, mask);
}

bool pic_irq_is_masked(uint8_t pic, uint8_t irq) {
    return pic_get_irq_mask(pic) & (1 << irq);
}

void pic_mask_irq(uint8_t pic, uint8_t irq) {
    pic_set_irq_mask(pic, pic_get_irq_mask(pic) | (1 << irq));
}

void pic_umask_irq(uint8_t pic, uint8_t irq) {
    pic_set_irq_mask(pic, pic_get_irq_mask(pic) | ~(1 << irq));
}

void pic_send_eoi(uint8_t pic) {
    outb(pic, PIC_OCW2_EOI);
}