#pragma once
#include <stdint.h>
#include "vfs.h"

struct devfs {
    vfs_driver_t driver;
    vfs_vnode_t* root;
};
typedef struct devfs devfs_t;

struct devfs_blockdev {
    int (*read)(uint8_t* buf, uint64_t offset, uint64_t len, void* ctx);
    int (*write)(uint8_t* buf, uint64_t offset, uint64_t len, void* ctx);
    int (*ioctl)(int call, void* in, void* out, void* ctx);
    void* ctx;
};
typedef struct devfs_blockdev devfs_blockdev_t;

struct devfs_chardev {
    int (*read)(uint8_t* buf, uint64_t len, void* ctx);
    int (*write)(uint8_t* buf, uint64_t len, void* ctx);
    int (*ioctl)(int call, void* in, void* out, void* ctx);
    void* ctx;
};
typedef struct devfs_chardev devfs_chardev_t;

enum devfs_dev_type {
    DEVFS_TYPE_BLOCKDEV,
    DEVFS_TYPE_CHARDEV
};
typedef enum devfs_dev_type devfs_dev_type_t;

struct devfs_dev {
    devfs_dev_type_t type;
    union {
        devfs_blockdev_t* blockdev;
        devfs_chardev_t* chardev;
    };
};
typedef struct devfs_dev devfs_dev_t;

devfs_t* devfs_create();
void devfs_destroy(devfs_t* devfs);

int devfs_gen_name(char* prefix, char* new_name);

int devfs_bind_blockdev(devfs_t* devfs, devfs_blockdev_t* blockdev, char* name);
int devfs_unbind_blockdev(devfs_t* devfs, char* name);
int devfs_bind_chardev(devfs_t* devfs, devfs_chardev_t* chardev, char* name);
int devfs_unbind_chardev(devfs_t* devfs, char* name);

// Driver implementation
struct vfs_vnode* devfs_walk(struct vfs_vnode* parent, char* name);
struct vfs_link* devfs_readdir(struct vfs_vnode* dir);
int devfs_unlink(struct vfs_vnode* parent, char* name);
struct vfs_file* devfs_open(struct vfs_vnode* vnode);
int devfs_read(struct vfs_file* file, uint8_t* buf, int len);
int devfs_write(struct vfs_file* file, uint8_t* buf, int len);
int devfs_seek(struct vfs_file* file, int pos);
int devfs_tell(struct vfs_file* file);
int devfs_close(struct vfs_file* file);