#pragma once

#include <vfs.h>

struct tmpfs {
    vfs_driver_t driver;
    vfs_vnode_t *root;
};
typedef struct tmpfs tmpfs_t;

// additional content, such as contents
struct tmpfs_inode {
    char *contents;
};
typedef struct tmpfs_inode tmpfs_inode_t;

struct tmpfs_file {
    int pos;
};
typedef struct tmpfs_file tmpfs_file_t;


tmpfs_t* tmpfs_create();
void tmpfs_destroy(tmpfs_t *tmpfs);

// Implementation
vfs_vnode_t* tmpfs_walk(vfs_vnode_t *parent, char *name);
vfs_link_t* tmpfs_readdir(vfs_vnode_t *dir);
int tmpfs_unlink(vfs_vnode_t *parent, char *name);

vfs_file_t* tmpfs_open(vfs_vnode_t *vnode);
vfs_vnode_t *tmpfs_createi(vfs_vnode_t* parent, char *name, uint16_t flags);
int tmpfs_read(vfs_file_t* file, uint8_t* buf, int len);
int tmpfs_write(vfs_file_t* file, uint8_t* buf, int len);
int tmpfs_seek(vfs_file_t* file, int pos);
int tmpfs_tell(vfs_file_t* file);
int tmpfs_close(vfs_file_t* file);

void tmpfs_driver_cleanup(vfs_driver_t* driver);
void tmpfs_ctx_cleanup(vfs_vnode_t* vnode);