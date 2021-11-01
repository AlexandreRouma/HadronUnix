#include <stdio.h>
#include <stdlib.h>
#include "vfs.h"
#include "tarfs.h"

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

int main() {
    int tarlen = 0;
    uint8_t* tar = read_to_buffer("../../build/initrd", &tarlen);
    if (!tar) {
        printf("Could not open tar");
        return -1;
    }

    vfs_init();
    tarfs_t* tarfs = tarfs_create(tar, tarlen);

    vfs_mount(vfs_root, tarfs->root);

    vfs_unmount(vfs_root);

    tarfs_destroy(tarfs);

    free(vfs_root);
    free(tar);

    return 0;
}