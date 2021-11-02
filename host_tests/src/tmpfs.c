#include "tmpfs.h"

#include <stddef.h>
#include <stdlib.h>
#include <vfs.h>

vfs_driver_t driver = (vfs_driver_t) {
    .walk = tmpfs_walk,
    .readdir = tmpfs_readdir,
    .unlink = tmpfs_unlink,
    .open = tmpfs_open,
    .create = tmpfs_createi,
    .read = tmpfs_read,
    .write = tmpfs_write,
    .seek = tmpfs_seek,
    .tell = tmpfs_tell,
    .close = tmpfs_close,

    .driver_cleanup = tmpfs_driver_cleanup,
    .ctx_cleanup = tmpfs_ctx_cleanup,
};

tmpfs_t* tmpfs_create() {
    tmpfs_t *tmpfs = malloc(sizeof(tmpfs_t));

    tmpfs->driver = driver;
    tmpfs->driver.refcount = 0;
    tmpfs->driver.ctx = tmpfs;
    
    tmpfs->root = malloc(sizeof(vfs_vnode_t));
    *tmpfs->root = (vfs_vnode_t) {
        .driver = &tmpfs->driver,
        .stat = {
            .flags = VFS_FLAG_DIRECTORY,
            .permissions = 0777,
            .uid = 0,
            .gid = 0,
            .create_time = 0,
            .modified_time = 0,
            .size = 0,
        },
        .refcount = 0,
        .child = NULL,
        .mount = NULL
    };

    vfs_driver_ref(&tmpfs->driver);
    vfs_ref(tmpfs->root);

    return tmpfs;
}

void tmpfs_destroy(tmpfs_t *tmpfs) {
    vfs_unref(tmpfs->root);
    vfs_driver_unref(&tmpfs->driver);
}

void tmpfs_driver_cleanup(vfs_driver_t *tmpfs_driver) {
    free(tmpfs_driver->ctx);
}

void tmpfs_ctx_cleanup(vfs_vnode_t *vnode) {    
    tmpfs_inode_t *inode = vnode->ctx;
    if (inode == NULL)
        return;

    if (inode->contents)
        free(inode->contents);

    free(inode);
    vnode->ctx = NULL;
    vnode->stat.size = 0;
}

vfs_vnode_t* tmpfs_walk(vfs_vnode_t *parent, char *name) {
    return NULL;
}

vfs_link_t* tmpfs_readdir(vfs_vnode_t *dir) {
    return NULL;
}

int tmpfs_unlink(vfs_vnode_t *parent, char *name) {
    vfs_link_t *pre;
    vfs_link_t *ln = vfs_get_child_by_name(parent, name, &pre);
    if (ln == NULL)
        return -1;

    if (ln->vnode->child)
        return -1;

    if (pre)
        pre->next = ln->next;
    else
        parent->child = ln->next;

    vfs_unref(ln->vnode);

    free(ln->name);
    free(ln);
}

vfs_file_t* tmpfs_open(vfs_vnode_t *vnode) {
    vfs_file_t *file = malloc(sizeof(vfs_file_t));
    file->vnode = vfs_ref(vnode);
    tmpfs_file_t *tfile = malloc(sizeof(tmpfs_file_t));
    tfile->pos = 0;
    file->ctx = tfile;
    return file;
}

vfs_vnode_t *tmpfs_createi(vfs_vnode_t* parent, char *name, uint16_t flags) {
    vfs_vnode_t *new = malloc(sizeof(vfs_vnode_t));
    *new = (vfs_vnode_t) {
        .driver = parent->driver,
        .stat = {
            .flags = flags,
            .permissions = 0777,
            .uid = 0,
            .gid = 0,
            .create_time = 0,
            .modified_time = 0,
            .size = 0,
        },
        .refcount = 0,
        .child = NULL,
        .mount = NULL
    };
    tmpfs_inode_t *newinode = malloc(sizeof(tmpfs_inode_t));
    newinode->contents = NULL;
    new->ctx = newinode;

    vfs_link_t *newlink = vfs_new_link(name, new);
    vfs_ref(new);
    vfs_append_link(&parent->child, newlink);
}

int tmpfs_read(vfs_file_t* file, uint8_t* buf, int len) {
    tmpfs_inode_t *inode = file->vnode->ctx;
    tmpfs_file_t *tfile = file->ctx;

    if (inode->contents == NULL)
        return 0;

    int c;
    for (c = 0; c < len && tfile->pos + c < file->vnode->stat.size; c++) {
        buf[c] = inode->contents[c];
    }

    tfile->pos += c;

    return c;
}

int tmpfs_write(vfs_file_t* file, uint8_t* buf, int len) {
    tmpfs_inode_t *inode = file->vnode->ctx;
    tmpfs_file_t *tfile = file->ctx;

    if (inode->contents == NULL || len + tfile->pos > file->vnode->stat.size) {
        inode->contents = realloc(inode->contents, sizeof(char) * (tfile->pos + len));
        file->vnode->stat.size = len + tfile->pos;
    }
    
    int c;
    for (c = 0; c < len; c++) {
        inode->contents[tfile->pos + c] = buf[c];
    }

    tfile->pos += c;

    return c;
}

int tmpfs_seek(vfs_file_t* file, int pos) {
    return 0;
}
int tmpfs_tell(vfs_file_t* file) {
    return 0;
}

int tmpfs_close(vfs_file_t* file) {
    vfs_vnode_t *vn = file->vnode;
    free(file->ctx);
    free(file);
    vfs_unref(vn);
}