#include "devfs.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

devfs_t* devfs_create() {
    devfs_t* devfs = malloc(sizeof(devfs_t));

    // Fill driver function pointers
    devfs->driver = (vfs_driver_t) {
        .walk = devfs_walk,
        .readdir = devfs_readdir,
        .unlink = devfs_unlink,
        .open = devfs_open,
        .read = devfs_read,
        .write = devfs_write,
        .seek = devfs_seek,
        .tell = devfs_tell,
        .close = devfs_close,

        .driver_cleanup = devfs_driver_cleanup,
        .ctx_cleanup = devfs_ctx_cleanup,
        .refcount = 0,
        .ctx = devfs
    };
    
    // Create root vnode
    devfs->root = malloc(sizeof(vfs_vnode_t));
    devfs->root->driver = &devfs->driver;
    devfs->root->stat = (vfs_stat_t) {
        .flags = VFS_FLAG_DIRECTORY,
        .permissions = 0777,
        .uid = 0,
        .gid = 0,
        .create_time = 0,
        .modified_time = 0,
        .size = 0,
    };
    devfs->root->ctx = NULL;
    devfs->root->refcount = 0;
    devfs->root->child = NULL;
    devfs->root->mount = NULL;

    vfs_driver_ref(devfs);
    vfs_ref(devfs->root);

    return devfs;
}

void devfs_destroy(devfs_t* devfs) {
    vfs_unref(devfs->root);
    vfs_driver_unref(devfs);
}

void devfs_driver_cleanup(vfs_driver_t *driver) {
    free(driver->ctx);
}

void devfs_ctx_cleanup(vfs_vnode_t *vnode) {
    if (vnode->ctx == NULL) { return; }

    // TODO: check this function

    free(vnode->ctx);
    vnode->ctx = NULL;
    vnode->stat.size = 0;
}

int devfs_gen_name(devfs_t* devfs, char* prefix, char* new_name) {
    int count = 0;
    vfs_link_t* ln = devfs->root->child;
    while (ln) {
        if (devfs_has_prefix(ln->name, prefix)) { count++; }
        ln = ln->next;
    }

    int prelen = strlen(prefix);
    strcpy(new_name, prefix);
    
    // TODO: IMPLEMENT
    // itoa() on &new_name[prelen]
    
    return -1;
}

bool devfs_has_prefix(char* str, char* prefix) {
    int lenstr = strlen(str);
    int lenpre = strlen(prefix);
    if (lenpre > lenstr) { return false; }
    for (int i = 0; i < lenpre; i++) {
        if (str[i] != prefix[i]) { return false; }
    }
    return true;
}

int devfs_bind_dev(devfs_t* devfs, devfs_dev_type_t type, void* new_dev, char* name) {
    // Check that a device with that name doesn't exist
    if (vfs_get_child_by_name(devfs->root, name, NULL)) { return -1; }

    // Create devuce context
    devfs_dev_t* dev = malloc(sizeof(devfs_dev_t));
    dev->type = type;
    dev->dev = new_dev;

    // Create new vnode
    vfs_vnode_t* vn = malloc(sizeof(vfs_vnode_t));
    vn->driver = &devfs->driver;
    vn->stat = (vfs_stat_t) {
        .flags = 0,
        .permissions = 0777,
        .uid = 0,
        .gid = 0,
        .create_time = 0,
        .modified_time = 0,
        .size = (type == DEVFS_TYPE_BLOCKDEV) ? ((devfs_blockdev_t*)new_dev)->size : 0
    };
    vn->refcount = 0;
    vn->child = NULL;
    vn->mount = NULL;
    vn->ctx = dev;

    // Create and append link
    vfs_link_t* ln = vfs_new_link(name, vn);
    vfs_ref(vn);
    vfs_append_link(&devfs->root->child, ln);
}

int devfs_unbind_dev(devfs_t* devfs, char* name) {
    // Get link
    vfs_link_t* preceding;
    vfs_link_t* ln = vfs_get_child_by_name(devfs->root, name, &preceding);
    if (!ln) { return -1; }

    // Free the device
    vfs_unref(ln->vnode);
    
    // Remove the link from the list
    if (preceding) {
        preceding->next = ln->next;
    }
    else {
        devfs->root->child = ln->next;
    }
    vfs_free_link(ln);
}

struct vfs_vnode* devfs_walk(struct vfs_vnode* parent, char* name)  {
    return NULL;
}

struct vfs_link* devfs_readdir(struct vfs_vnode* dir)  {
    return NULL;
}

int devfs_unlink(struct vfs_vnode* parent, char* name) {
    return -1;
}

struct vfs_file* devfs_open(struct vfs_vnode* vnode) {
    vfs_file_t* file = malloc(sizeof(vfs_file_t));
    file->vnode = vfs_ref(vnode);
    file->ctx = malloc(sizeof(uint64_t));
    uint64_t* cur = file->ctx;
    *cur = 0;
    return file;
}

int devfs_read(struct vfs_file* file, uint8_t* buf, int len) {
    uint64_t* cur = file->ctx;
    devfs_dev_t* dev = file->vnode->ctx;

    if (dev->type == DEVFS_TYPE_BLOCKDEV) {
        devfs_blockdev_t* bdev = dev->dev;
        int to_read = len;
        int readable = file->vnode->stat.size - *cur;
        if (readable <= 0) { return 0; }
        if (to_read > readable) { to_read = readable; }
        to_read = bdev->read(buf, *cur, to_read, bdev->ctx);
        if (to_read > 0) { *cur += to_read; }
        return to_read;
    }
    else if (dev->type == DEVFS_TYPE_CHARDEV) {
        devfs_chardev_t* cdev = dev->dev;
        return cdev->read(buf, len, cdev->ctx);
    }

    return -1;
}

int devfs_write(struct vfs_file* file, uint8_t* buf, int len) {
    uint64_t* cur = file->ctx;
    devfs_dev_t* dev = file->vnode->ctx;
    
    if (dev->type == DEVFS_TYPE_BLOCKDEV) {
        devfs_blockdev_t* bdev = dev->dev;
        int to_write = len;
        int writeable = file->vnode->stat.size - *cur;
        if (writeable <= 0) { return 0; }
        if (to_write > writeable) { to_write = writeable; }
        to_write = bdev->write(buf, *cur, to_write, bdev->ctx);
        if (to_write > 0) { *cur += to_write; }
        return to_write;
    }
    else if (dev->type == DEVFS_TYPE_CHARDEV) {
        devfs_chardev_t* cdev = dev->dev;
        return cdev->write(buf, len, cdev->ctx);
    }

    return -1;
}

int devfs_seek(struct vfs_file* file, int pos) {
    uint64_t* cur = file->ctx;
    devfs_dev_t* dev = file->vnode->ctx;

    if (dev->type == DEVFS_TYPE_CHARDEV) { return -1; }

    if (pos >= file->vnode->stat.size) { pos = file->vnode->stat.size - 1; }
    *cur = pos;
    return pos;
}

int devfs_tell(struct vfs_file* file) {
    uint64_t* cur = file->ctx;devfs_dev_t* dev = file->vnode->ctx;

    if (dev->type == DEVFS_TYPE_CHARDEV) { return 0; }

    return *cur;
}

int devfs_close(struct vfs_file* file) {
    vfs_vnode_t *vn = file->vnode;
    free(file->ctx);
    free(file);
    vfs_unref(vn);
}