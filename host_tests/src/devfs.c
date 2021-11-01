#include "devfs.h"
#include <stddef.h>

devfs_t* devfs_create() {
    devfs_t* devfs = malloc(sizeof(devfs_t));

    // Fill driver function pointers
    devfs->driver.walk = devfs_walk;
    devfs->driver.readdir = devfs_readdir;
    devfs->driver.unlink = devfs_unlink;
    devfs->driver.open = devfs_open;
    devfs->driver.read = devfs_read;
    devfs->driver.write = devfs_write;
    devfs->driver.seek = devfs_seek;
    devfs->driver.tell = devfs_tell;
    devfs->driver.close = devfs_close;
    
    // Create root vnode
    devfs->root = malloc(sizeof(vfs_vnode_t));
    devfs->root->driver = &devfs->driver;
    devfs->root->stat.flags = VFS_FLAG_DIRECTORY;
    devfs->root->stat.permissions = 0777;
    devfs->root->stat.uid = 0;
    devfs->root->stat.gid = 0;
    devfs->root->stat.create_time = 0;
    devfs->root->stat.modified_time = 0;
    devfs->root->stat.size = 0;
    devfs->root->refcount = 1;
    devfs->root->child = NULL;
    devfs->root->mount = NULL;

    return devfs;
}

void devfs_destroy(devfs_t* devfs) {

}

int devfs_gen_name(char* prefix, char* new_name) {

}

int devfs_bind_blockdev(devfs_t* devfs, devfs_blockdev_t* blockdev, char* name) {
    // Check that a device with that name doesn't exist
    if (vfs_get_child_by_name(devfs->root, name, NULL)) { return -1; }
}

int devfs_unbind_blockdev(devfs_t* devfs, char* name) {

}

int devfs_bind_chardev(devfs_t* devfs, devfs_chardev_t* chardev, char* name) {

}

int devfs_unbind_chardev(devfs_t* devfs, char* name) {

}

struct vfs_vnode* devfs_walk(struct vfs_vnode* parent, char* name)  {

}

struct vfs_link* devfs_readdir(struct vfs_vnode* dir)  {

}

int devfs_unlink(struct vfs_vnode* parent, char* name) {

}

struct vfs_file* devfs_open(struct vfs_vnode* vnode) {

}

int devfs_read(struct vfs_file* file, uint8_t* buf, int len) {

}

int devfs_write(struct vfs_file* file, uint8_t* buf, int len) {

}

int devfs_seek(struct vfs_file* file, int pos) {

}

int devfs_tell(struct vfs_file* file) {

}

int devfs_close(struct vfs_file* file) {

}
