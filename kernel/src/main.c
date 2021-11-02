#include <bootinfo.h>
#include <vga/vga.h>
#include <memory/memmap.h>
#include <memory/palloc.h>
#include <bios_mmap.h>
#include <string.h>
#include <memory/paging.h>
#include <memory/halloc.h>
#include <gdt/gdt.h>
#include <interrupts/idt.h>
#include <interrupts/pic.h>
#include <kfmt.h>
#include <vfs/vfs.h>
#include <vfs/tarfs.h>
#include <panic.h>
#include <vfs/tmpfs.h>
#include <vfs/kstdio.h>
#include <vfs/devfs.h>

void dump_link(vfs_link_t* ln) {
    kprintf("%s %08d %s", (ln->vnode->stat.flags & VFS_FLAG_DIRECTORY) ? "DIR " : "FILE", ln->vnode->stat.size, ln->name);
}

void ls(vfs_vnode_t* vnode) {
    vfs_link_t* ln = vfs_readdir(vnode);
    while (ln) {
        kprintf("%s %08d %s\n", (ln->vnode->stat.flags & VFS_FLAG_DIRECTORY) ? "DIR " : "FILE", ln->vnode->stat.size, ln->name);
        ln = ln->next;
    }
}

void dir(vfs_vnode_t* vnode, int depth) {
    vfs_link_t* ln = vfs_readdir(vnode);
    while (ln) {
        for (int i = 0; i < depth-1; i++) { kprintf("    "); }
        if (depth >= 1) { kprintf(ln->next ? " |--" : " \x07--"); }
        dump_link(ln); kprintf("\n");
        if (ln->vnode->stat.flags & VFS_FLAG_DIRECTORY) {
            dir(ln->vnode, depth + 1);
        }
        ln = ln->next;
    }
}

int dev_read(uint8_t* buf, uint64_t len, void* ctx) {
    return 0;
}

int dev_write(uint8_t* buf, uint64_t len, void* ctx) {
    for (int i = 0; i < len; i++) {
        kprintf("%c", buf[i]);
    }
    return len;
}

int dev_ioctl(int call, void* in, void* out, void* ctx) {
    return -1;
}

void kmain(bootinfo_t* binfo) {
    vga_init(80, 25);
    kfmt_out = vga_print;

    // Print MOTD
    vga_set_color(0x6E);
    kprintf("  === Hadron Unix ===  \n");
    kprintf("   The back fell off  ");
    vga_set_color(0x0F);
    kprintf("\n\ncmdline: \"%s\"\n\n", (char*)(uint64_t)binfo->cmdline_addr);

    // Initialize the GDT
    gdt_init();
    k_gdt_cs = gdt_define_entry(gdt_empty(false), GDT_FLAG_CODE | GDT_FLAG_64BIT, 0);
    k_gdt_ds = gdt_define_entry(gdt_empty(false), GDT_FLAG_64BIT, 0);
    gdt_load(k_gdt_ds);

    // Initialize the IDT
    idt_init();
    idt_load();

    // Initialize the PIC
    pic_init(0x20, 0x28);

    // Initialize the memory map
    memmap_init();
    
    // Since the map can be unordered and items overlapping, define sections in a specific order
    mmap_entry_t* mmap = (mmap_entry_t*)(uint64_t)binfo->mmap_addr;
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type != MMAP_BIOS_ENTRY_TYPE_USABLE || !mmap[i].size) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_FREE);
    }
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type != MMAP_BIOS_ENTRY_TYPE_RSVD || !mmap[i].size) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_RESERVED);
    }
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type != MMAP_BIOS_ENTRY_TYPE_ACPI_REC || !mmap[i].size) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_ACPI);
    }
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type != MMAP_BIOS_ENTRY_TYPE_NVS || !mmap[i].size) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_ACPI);
    }
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type != MMAP_BIOS_ENTRY_TYPE_BAD_MEM || !mmap[i].size) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_DEAD);
    }

    // Define unknown types as dead
    for (uint64_t i = 0; i < binfo->mmap_entry_count; i++) {
        if (mmap[i].type == MMAP_BIOS_ENTRY_TYPE_USABLE) { continue; }
        if (mmap[i].type == MMAP_BIOS_ENTRY_TYPE_RSVD) { continue; }
        if (mmap[i].type == MMAP_BIOS_ENTRY_TYPE_ACPI_REC) { continue; }
        if (mmap[i].type == MMAP_BIOS_ENTRY_TYPE_NVS) { continue; }
        if (mmap[i].type == MMAP_BIOS_ENTRY_TYPE_BAD_MEM) { continue; }
        if (!mmap[i].size) { continue; }
        memmap_define(mmap[i].base, mmap[i].size, MEMMAP_REGION_TYPE_DEAD);
    }

    // Map the bios
    memmap_define(0x00000, 0x00500, MEMMAP_REGION_TYPE_BIOS);
    memmap_define(0x80000, 0x20000, MEMMAP_REGION_TYPE_BIOS);

    // Map elements loaded by the bootloader
    memmap_define(binfo->bootloader_base, binfo->bootloader_size, MEMMAP_REGION_TYPE_SOFTWARE);
    memmap_define(binfo->kernel_base, binfo->kernel_size, MEMMAP_REGION_TYPE_SOFTWARE);
    memmap_define(binfo->initrd_addr, binfo->initrd_size, MEMMAP_REGION_TYPE_SOFTWARE);
    memmap_define(binfo->cmdline_addr, binfo->kernel_size, MEMMAP_REGION_TYPE_SOFTWARE);
    memmap_define(binfo->mmap_addr, binfo->mmap_entry_count * sizeof(bootinfo_t), MEMMAP_REGION_TYPE_SOFTWARE);

    // Cleanup map
    memmap_agregate();

    // Initialize the physical allocator on available memory
    palloc_init(0, 0x3FFFFFFF, false);

    // Initialize paging
    paging_init();

    // Map all software and bios areas
    for (int i = 0; i < k_mem_map.entry_count; i++) {
        memmap_entry_t ent = k_mem_map.map[i];
        if (ent.type != MEMMAP_REGION_TYPE_SOFTWARE && ent.type != MEMMAP_REGION_TYPE_BIOS) { continue; }
        paging_map_multiple(k_pml4, ent.base, ent.base, PAGING_FLAG_RW, paging_area_size(ent.base, ent.size), false);
    }
    paging_map_multiple(k_pml4, 0xB8000, 0xB8000, PAGING_FLAG_RW, 80 * 25 * 2, false);
    palloc_map_buddies();

    // Enable paging
    paging_enable();

    // Initialize the physical allocator on the remaining memory
    palloc_init(0, 0xFFFFFFFFFFFFFFFF, true);

    // Initialize VFS
    vfs_init();

    // Initialize tarfs for the initrd
    tarfs_t* tarfs = tarfs_create((uint8_t*)binfo->initrd_addr, binfo->initrd_size);
    tmpfs_t* tmpfs = tmpfs_create();
    devfs_t* devfs = devfs_create();
    
    // Mount the initrd on /
    vfs_mount(vfs_root, tarfs->root);
    vfs_vnode_t* tmp = vfs_walk(vfs_root, "tmp");
    vfs_vnode_t* dev = vfs_walk(vfs_root, "dev");
    vfs_mount(tmp, tmpfs->root);
    vfs_mount(dev, devfs->root);

    devfs_chardev_t cdev;
    cdev.read = dev_read;
    cdev.write = dev_write;
    cdev.ioctl = dev_ioctl;
    cdev.ctx = NULL;
    devfs_bind_dev(devfs, DEVFS_TYPE_CHARDEV, &cdev, "tty0");

    vfs_vnode_t* hai = vfs_create(tmp, "hai", VFS_FLAG_DIRECTORY);
    vfs_create(hai, "hello", 0);

    vfs_file_t* f = kfopen("/tmp/hai/hello");
    kfwrite("hello!", 6, 1, f);
    kfclose(f);

    f = kfopen("/dev/tty0");
    kfwrite("This was written through kstdio and devfs!\n", 43, 1, f);
    kfclose(f);

    dir(vfs_root, 0);

    vfs_unmount(tmp);
    vfs_unmount(vfs_root);
    vfs_unmount(dev);

    devfs_destroy(devfs);
    tmpfs_destroy(tmpfs);
    tarfs_destroy(tarfs);
}