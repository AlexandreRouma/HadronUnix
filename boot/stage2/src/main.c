#include <vga_basic/vga.h>
#include <drive/drive.h>

extern uint32_t my_shitty_realmode_function();

void panic(char* err, int error_code) {
    vga_new_line();
    vga_set_color(0b11110100);
    vga_print("PANIC (");
    char buf[16];
    itoa(error_code, buf, 15);
    vga_print(buf);
    vga_print("): ");
    vga_print(err);
    while (1);
}

void stage2_main(uint32_t boot_drive_index) {
    // Initialize VGA driver
    vga_init(80, 25);
    vga_println("[BOOT Stage2]");

    drive_info_t dinfo;
    int ret = drive_get_info(&dinfo, boot_drive_index);
    if (ret) {
        panic("Could not get drive geometry", ret);
        return;
    }

    ret = drive_read_sectors(&dinfo, 1, 1, (uint8_t*)0x100000);
    if (ret) {
        panic("Could not read sector", ret);
        return;
    }

    vga_println("Ready.");
}