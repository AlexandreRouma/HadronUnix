#include <bootinfo.h>
#include <vga_basic/vga.h>

void kmain(bootinfo_t* binfo) {
    vga_init(80, 25);

    vga_println("Hello from long mode!");
    vga_print("cmdline: \"");
    vga_print((char*)(uint64_t)binfo->cmdline_addr);
    vga_print("\"");
}