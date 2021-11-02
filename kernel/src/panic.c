#include "panic.h"
#include <vga/vga.h>
#include <kfmt.h>

void kpanic(char* message, int code) {
    vga_set_color(0b01001111);
    kprintf("PANIC (%d): %s", code, message);
    while(1);
}