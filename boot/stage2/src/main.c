#include <vga_basic/vga.h>
#include <interrupt/pic.h>

void stage2_main() {
    // Initialize VGA driver
    vga_init(80, 25);

    // Initialize the PIC
    pic_init(0x20, 0x28);

    vga_print("Load my boot");
}