#include <vga_basic/vga.h>

void stage2_main() {
    // Initialize VGA driver
    vga_init(80, 25);

    vga_print("Doin yo mom");
    vga_ok(true);
}