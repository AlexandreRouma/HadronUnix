#pragma once
#include <stdint.h>

struct vfs_driver {
    vfs_vnode_t* (*walk)(vfs_vnode_t* vnode, char* name);
    vfs_link_t* (*readdir)(vfs_vnode_t* vnode);
    vfs_vnode_t* (*touch)(vfs_vnode_t* vnode, char* name, vfs_stat_t stat);
    int (*unlink)(vfs_vnode_t* vnode, char* name);
    void (*destroy)(vfs_vnode_t* vnode);
    void* ctx;
};
typedef struct vfs_driver vfs_driver_t;

struct vfs_stat {
    uint16_t permissions;
    uint16_t uid;
    uint16_t gid;
    uint64_t create_time;
    uint64_t modified_time;
    uint64_t size;
};
typedef struct vfs_stat vfs_stat_t;

enum vfs_type {
    VFS_TYPE_FILE,
    VFS_TYPE_DIRECTORY,
    VFS_TYPE_SYMLINK,
    VFS_TYPE_MOUNTPOINT
};
typedef enum vfs_type vfs_type_t;

struct vfs_vnode_list {
    char* name;
    vfs_vnode_t* vnode;
    vfs_link_t* next;
};
typedef struct vfs_vnode_list vfs_link_t;

struct vfs_vnode {
    vfs_type_t flags;
    vfs_stat_t stat;
    int refcount;
    vfs_driver_t* driver;
    void* vnode_ctx;
    vfs_link_t* children;
};
typedef struct vfs_vnode vfs_vnode_t;

struct vfs_file {
    vfs_vnode_t* vnode;
    void* file_ctx;
};
typedef struct vfs_file vfs_file_t;

// Driver forwarded
vfs_vnode_t* vfs_walk(vfs_vnode_t* vnode, char* name);
vfs_link_t* vfs_readdir(vfs_vnode_t* vnode);
vfs_vnode_t* vfs_touch(vfs_vnode_t* vnode, char* name, vfs_stat_t stat);
int vfs_unlink(vfs_vnode_t* vnode, char* name);
void vfs_decrement_and_destroy(vfs_vnode_t* vnode);

// VFS stuff
vfs_vnode_t* vfs_reach(vfs_vnode_t* vnode, char* path);

// VFS internal
void vfs_update_children(vfs_vnode_t* vnode, vfs_link_t* children);
vfs_link_t* vfs_search_links_by_name(vfs_link_t* link, char* name, vfs_link_t** preceding);
void vfs_free_link(vfs_link_t* link);
