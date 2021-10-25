#include <vga_basic/vga.h>
#include <drive/drive.h>
#include <fat32/fat32.h>
#include <fat32/mbr.h>
#include <elf/elf_loader.h>

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

int drive_read_handler(uint32_t lba, uint32_t count, uint8_t *buf, void *ctx) {
    drive_info_t* dinfo = ctx;
    return drive_read_sectors(dinfo, lba, count, buf);
}

void stage2_main(uint32_t boot_drive_index) {
    // Initialize VGA driver
    vga_init(80, 25);
    vga_println("[BOOT] Starting");

    // Gather drive geometry
    drive_info_t dinfo;
    int ret = drive_get_info(&dinfo, boot_drive_index);
    if (ret) {
        panic("Could not get drive geometry", ret);
        return;
    }

    // Parse MBR
    mbr_t* mbr = (mbr_t*)(0x7C00 + 0x1B8);

    // Load FAT32 FS
    fat32_t fat;
    fat32_entry_t root;
    int err = fat32_init(&fat, drive_read_handler, mbr->part0.lba_start, &dinfo);
    if (err) {
        panic("Error loading FAT32 boot partition", err);
    }
    fat32_root_dir(&fat, &root);

    // Open KERNEL on bootfs
    fat32_entry_t kernel;
    err = fat32_walk(&fat, &root, "KERNEL", &kernel);
    if (err) {
        panic("Error opening KERNEL on bootfs", err);
    }

    // Load ELF in memory
    uint64_t entry;
    uint64_t min_addr;
    uint64_t max_addr;
    err = elf_loader_load(&fat, &kernel, &entry, &min_addr, &max_addr);
    if (err) {
        panic("Could not load kernel ELF", err);
    }

    char buf[16];

    vga_print("Entry: ");
    itoa(entry, buf, 15);
    vga_println(buf);

    vga_print("Min Addr: ");
    itoa(min_addr, buf, 15);
    vga_println(buf);

    vga_print("Max Addr: ");
    itoa(max_addr, buf, 15);
    vga_println(buf);

    vga_println("[BOOT] Ready");

}