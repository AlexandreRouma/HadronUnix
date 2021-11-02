#include <stdio.h>
#include <stdlib.h>
#include "vfs.h"
#include "tarfs.h"
#include "devfs.h"
#include "tmpfs.h"
#include "kstdio.h"

uint8_t* read_to_buffer(char* path, int* len) {
    FILE* f = fopen(path, "rb");
    if (!f) { return NULL; }
    fseek(f, 0L, SEEK_END);
    int sz = ftell(f);
    rewind(f);
    uint8_t* buf = malloc(sz);
    fread(buf, 1, sz, f);
    fclose(f);
    *len = sz;
    return buf;
}

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
        for (int i = 0; i < depth; i++) { kprintf("    "); }
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
    return write(1, buf, len);
}

int dev_ioctl(int call, void* in, void* out, void* ctx) {
    return -1;
}

int main() {
    //int tarlen = 0;
    //uint8_t* tar = read_to_buffer("../../build/initrd", &tarlen);
    /*if (!tar) {
        printf("Could not open tar");
        return -1;
    }*/

    vfs_init();
    //tarfs_t* tarfs = tarfs_create(tar, tarlen);
    devfs_t* devfs = devfs_create();
    tmpfs_t* tmpfs = tmpfs_create();

    //vfs_mount(vfs_root, tarfs->root);
    vfs_mount(vfs_root, tmpfs->root);
    vfs_create(vfs_root, "dev", VFS_FLAG_DIRECTORY);
    vfs_create(vfs_root, "lmao", 0);
    vfs_vnode_t* devdir = vfs_walk(vfs_root, "dev");
    vfs_mount(devdir, devfs->root);

    devfs_chardev_t cdev;
    cdev.read = dev_read;
    cdev.write = dev_write;
    cdev.ioctl = dev_ioctl;
    cdev.ctx = NULL;
    devfs_bind_dev(devfs, DEVFS_TYPE_CHARDEV, &cdev, "tty0");

    devfs_chardev_t cdev1;
    cdev1.read = dev_read;
    cdev1.write = dev_write;
    cdev1.ioctl = dev_ioctl;
    cdev1.ctx = NULL;
    devfs_bind_dev(devfs, DEVFS_TYPE_CHARDEV, &cdev1, "tty1");

    devfs_chardev_t cdev2;
    cdev2.read = dev_read;
    cdev2.write = dev_write;
    cdev2.ioctl = dev_ioctl;
    cdev2.ctx = NULL;
    devfs_bind_dev(devfs, DEVFS_TYPE_CHARDEV, &cdev2, "tty2");

    devfs_chardev_t cdev3;
    cdev3.read = dev_read;
    cdev3.write = dev_write;
    cdev3.ioctl = dev_ioctl;
    cdev3.ctx = NULL;
    devfs_bind_dev(devfs, DEVFS_TYPE_CHARDEV, &cdev3, "tty3");

    /*dir(vfs_root, 0);

    // TEST
    vfs_vnode_t* tty0 = vfs_walk(devdir, "tty0");
    vfs_file_t* ttyf = vfs_open(tty0);
    vfs_write(ttyf, "Hello World!\n", strlen("Hello World!\n"));
    vfs_close(ttyf);*/

    // vfs_vnode_t *t = vfs_walk(devdir, "tty0");
    // vfs_file_t *f = vfs_open(t);
    // vfs_write(f, "Hello\n", 6);
    // vfs_close(f);

    vfs_file_t *f = kfopen("/lmao");
    kfwrite("Hello\n", 6, 1, f);
    kfclose(f);

    f = kfopen("/lmao");
    char buf[7];
    buf[6] = 0;
    kfread(buf, 6, 1, f);
    kfclose(f);

    dir(vfs_root, 0);

    printf("%s", buf);
    
    
    devfs_unbind_dev(devfs, "tty0");
    devfs_unbind_dev(devfs, "tty1");
    devfs_unbind_dev(devfs, "tty2");
    devfs_unbind_dev(devfs, "tty3");

    vfs_unmount(devdir);
    vfs_unmount(vfs_root);
    tmpfs_destroy(tmpfs);
    devfs_destroy(devfs);
    /*vfs_unmount(vfs_root);
    vfs_unmount(devdir);
    vfs_unmount(vfs_root);
    devfs_destroy(devfs);
    tarfs_destroy(tarfs);
    free(vfs_root);
    free(tar);*/

    return 0;
}