#pragma once
#include <stdint.h>

#define VFS_FLAG_DIRECTORY  (1 << 0)

struct vfs_driver {
    struct vfs_vnode* (*walk)(struct vfs_vnode* parent, char* name);
    struct vfs_link* (*readdir)(struct vfs_vnode* dir);
    int (*unlink)(struct vfs_vnode* parent, char* name);
    struct vfs_file* (*open)(struct vfs_vnode* vnode);
    struct vfs_vnode* (*create)(struct vfs_vnode* parent, char *name, uint16_t flags);
    int (*read)(struct vfs_file* file, uint8_t* buf, int len);
    int (*write)(struct vfs_file* file, uint8_t* buf, int len);
    int (*seek)(struct vfs_file* file, int pos);
    int (*tell)(struct vfs_file* file);
    int (*ioctl)(int call, void* in, void* out);
    int (*close)(struct vfs_file* file);
    void (*driver_cleanup)(struct vfs_driver* driver);
    void (*ctx_cleanup)(struct vfs_vnode* vnode);
    void* ctx;
    int refcount;
};
typedef struct vfs_driver vfs_driver_t;

struct vfs_stat {
    uint16_t flags;
    uint16_t permissions;
    uint16_t uid;
    uint16_t gid;
    uint64_t create_time;
    uint64_t modified_time;
    uint64_t size;
};
typedef struct vfs_stat vfs_stat_t;

struct vfs_vnode {
    vfs_stat_t stat;
    int refcount;
    struct vfs_link* child;
    struct vfs_vnode* mount;
    vfs_driver_t* driver;
    void* ctx;
};
typedef struct vfs_vnode vfs_vnode_t;

struct vfs_link {
    char* name;
    vfs_vnode_t* vnode;
    struct vfs_link* next;
};
typedef struct vfs_link vfs_link_t;

struct vfs_file {
    vfs_vnode_t* vnode;
    void* ctx;
};
typedef struct vfs_file vfs_file_t;

extern vfs_vnode_t* vfs_root;

// VFS API
void vfs_init();
vfs_vnode_t* vfs_walk(vfs_vnode_t* parent, char* name);
vfs_link_t* vfs_readdir(vfs_vnode_t* dir);
int vfs_unlink(vfs_vnode_t* parent, char* name);
int vfs_mount(vfs_vnode_t* mountpoint, vfs_vnode_t* root);
int vfs_unmount(vfs_vnode_t* mountpoint);

// VFS VNODE MANAGEMENT
vfs_vnode_t* vfs_ref(vfs_vnode_t *vnode);
void vfs_unref(vfs_vnode_t *vnode);

/* Do NOT (!) call these 2 directly :)) */
vfs_driver_t* vfs_driver_ref(vfs_driver_t *driver);
void vfs_driver_unref(vfs_driver_t *drive);

// VFS IO
vfs_file_t* vfs_open(vfs_vnode_t* vnode);
vfs_file_t* vfs_create(vfs_vnode_t* parent, char* name, uint16_t flags);
int vfs_read(vfs_file_t* file, uint8_t* buf, int len);
int vfs_write(vfs_file_t* file, uint8_t* buf, int len);
int vfs_seek(vfs_file_t* file, int pos);
int vfs_tell(vfs_file_t* file);
int vfs_close(vfs_file_t* file);

// VFS Internal
vfs_link_t* vfs_new_link(char* name, vfs_vnode_t* vnode);
void vfs_free_link(vfs_link_t* link);
void vfs_free_link_list(vfs_link_t* link);
void vfs_append_link(vfs_link_t** link, vfs_link_t* newlink);
void vfs_append_link_list(vfs_link_t** link, vfs_link_t* newlink);
vfs_link_t* vfs_get_child_by_name(vfs_vnode_t* parent, char* name, vfs_link_t** preceeding);