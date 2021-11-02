#include <vga/vga.h>
#include <drive/drive.h>
#include <fat32/fat32.h>
#include <fat32/mbr.h>
#include <elf/elf_loader.h>
#include <bootopts.h>
#include <longmode/longmode.h>
#include <bootinfo.h>
#include <mmap/mmap.h>
#include <init.h>
#include <video/video.h>
#include <video/gfx_query.h>

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

void dump_mmap_entry(mmap_entry_t entry) {
    vga_println("--------------- ENTRY ---------------");
    char buf[16];

    itoa(entry.base, buf, 15);
    vga_print("base: ");
    vga_println(buf);

    itoa(entry.size, buf, 15);
    vga_print("size: ");
    vga_println(buf);

    itoa(entry.type, buf, 15);
    vga_print("type: ");
    vga_println(buf);
}

void stage2_main(uint32_t boot_drive_index) {
    // Enforce text mode
    vbe_set_mode(VBE_TEXT_MODE);
    
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
    int err = fat32_init(&fat, drive_read_handler, mbr->part0.lba_start, &dinfo);
    if (err) {
        panic("Error loading FAT32 boot partition", err);
    }

    // Get handle to FAT32 root
    fat32_entry_t root;
    fat32_root_dir(&fat, &root);

    // Open the config file
    fat32_entry_t bootini;
    err = fat32_walk(&fat, &root, "BOOT.INI", &bootini);
    if (err) {
        panic("Error opening boot.ini", err);
    }

    // Read the config file
    char bootconf[bootini.size + 1];
    fat32_read(&fat, &bootini, bootconf, bootini.size, 0);
    bootconf[bootini.size] = '\0';
    vga_print("\n");
    vga_print(bootconf);
    vga_print("\n");

    // Parse the config
    bootopts_t bootopts;
    bootopts_fill(&bootopts, bootconf);

    // Open the kernel
    fat32_entry_t kernel;
    err = fat32_walk(&fat, &root, bootopts.kernel, &kernel);
    if (err) {
        panic("Error opening kernel", err);
    }

    // Load ELF in memory
    uint32_t entry;
    uint32_t min_addr;
    uint32_t max_addr;
    err = elf_loader_load(&fat, &kernel, &entry, &min_addr, &max_addr);
    if (err) {
        panic("Could not load kernel ELF", err);
    }
    uint32_t kernel_max = max_addr;

    // Open the initrd
    fat32_entry_t initrd;
    err = fat32_walk(&fat, &root, bootopts.initrd, &initrd);
    if (err) {
        panic("Error opening initrd", err);
    }
    
    // Load the initrd
    uint8_t* initrd_buf = (uint8_t*)max_addr;
    fat32_read(&fat, &initrd, initrd_buf, initrd.size, 0);
    max_addr += initrd.size;

    // Load command line
    int cmdlen = strlen(bootopts.cmdline);
    char* cmdline_buf = (char*)max_addr;
    memset(cmdline_buf, 0, cmdlen+1);
    memcpy(cmdline_buf, bootopts.cmdline, cmdlen);
    max_addr += cmdlen + 1;

    // Load memory map
    mmap_entry_t* mmap = (mmap_entry_t*)max_addr;
    int mmap_entry_count = mmap_get(mmap);
    if (!mmap_entry_count) {
        panic("Could not get memory map", 0);
    }
    max_addr += mmap_entry_count * sizeof(mmap_entry_t);

    // Get VBE controller info
    vbe_ctrl_info_t ctrl_info;
    vbe_get_ctrl_info(&ctrl_info);

    // Change video mode (except if text mode specified)
    bool textmode = true;
    vbe_mode_info_t modeinfo;
    if (!strcmp(bootopts.gfx, "text")) {
        vbe_get_mode_info(&modeinfo, VBE_TEXT_MODE);
    }
    else {
        // Parse the mode query
        gfx_query_t query;
        err = gfx_query_parse(&query, bootopts.gfx);
        if (err) {
            panic("Invalid video query syntax", err);
        }

        // Find a mode according to the query
        uint16_t mode = vbe_mode_search(&ctrl_info, &query, &modeinfo);
        if (mode == VBE_MODE_END) {
            panic("Could not find video mode matching the query", err);
        }

        // Set the new video mode
        vbe_set_mode(mode);
        textmode = false;
    }

    // Create boot info struct
    bootinfo_t* binfo = (bootinfo_t*)max_addr;
    binfo->bootloader_base = 0x7C00;
    binfo->bootloader_size = (uint32_t)&stage2_size_detect - 0x7C00;
    binfo->kernel_base = min_addr;
    binfo->kernel_size = kernel_max - min_addr;
    binfo->initrd_addr = (uint32_t)initrd_buf;
    binfo->initrd_size = initrd.size;
    binfo->cmdline_addr = (uint32_t)cmdline_buf;
    binfo->cmdline_size = cmdlen + 1;
    binfo->mmap_addr = (uint32_t)mmap;
    binfo->mmap_entry_count = mmap_entry_count;
    
    // Write video mode info
    if (!textmode) {
        binfo->fbinfo = (fbinfo_t) {
            .type = FB_TYPE_GRAPHIC,
            .addr = modeinfo.framebuffer,
            .width = modeinfo.width,
            .height = modeinfo.height,
            .pitch = modeinfo.pitch,
            .depth = modeinfo.bpp,
            .red_mask = modeinfo.red_mask,
            .red_pos = modeinfo.red_position,
            .green_mask = modeinfo.green_mask,
            .green_pos = modeinfo.green_position,
            .blue_mask = modeinfo.blue_mask,
            .blue_pos = modeinfo.blue_position,
        };
    }
    else {
        binfo->fbinfo = (fbinfo_t) {
            .type = FB_TYPE_TEXT,
            .addr = modeinfo.framebuffer,
            .width = modeinfo.w_char,
            .height = modeinfo.y_char,
            .pitch = modeinfo.pitch
        };
    }

    max_addr += sizeof(bootinfo_t);

    vga_println("[BOOT] Ready");

    // Call 64bit kernel
    longmode_call(entry, (uint64_t)(uint32_t)binfo, 0, 0, 0, 0, 0);
}