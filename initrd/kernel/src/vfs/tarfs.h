#pragma once
#include <vfs/vfs.h>

struct tarfs {
    vfs_driver_t driver;
    vfs_vnode_t* root;
};
typedef struct tarfs tarfs_t;

tarfs_t* tarfs_create(uint8_t* data, uint64_t len);
void tarfs_destroy(tarfs_t* tarfs);

char* tarfs_parse_header(char* header, vfs_stat_t* vnode);
uint64_t tarfs_parse_octal(char* octal, int maxlen);
int tarfs_get_path_elem_count(char* path);

void tarfs_free_link(vfs_link_t* link);

// Driver implementation
struct vfs_vnode* tarfs_walk(struct vfs_vnode* parent, char* name);
struct vfs_link* tarfs_readdir(struct vfs_vnode* dir);
int tarfs_unlink(struct vfs_vnode* parent, char* name);
struct vfs_file* tarfs_open(struct vfs_vnode* vnode);
int tarfs_read(struct vfs_file* file, uint8_t* buf, int len);
int tarfs_write(struct vfs_file* file, uint8_t* buf, int len);
int tarfs_seek(struct vfs_file* file, int pos);
int tarfs_tell(struct vfs_file* file);
int tarfs_close(struct vfs_file* file);