#include "vfs.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

vfs_vnode_t* vfs_root = NULL;

void vfs_init() {
    vfs_root = malloc(sizeof(vfs_vnode_t));
    memset(vfs_root, 0, sizeof(vfs_vnode_t));
}

vfs_vnode_t* vfs_walk(vfs_vnode_t* parent, char* name) {
    if (parent->mount) { return vfs_walk(parent->mount, name); }

    // Check if link is already cached, if yes, return it
    vfs_link_t* ln = vfs_get_child_by_name(parent, name, NULL);
    if (ln) { return ln->vnode; }

    // Otherwise, request it from the driver and add it to the cache
    vfs_vnode_t* vn = parent->driver->walk(parent, name);

    // If the driver failed, return null
    if (!vn) { return NULL; }

    // Append to the list and return
    ln = vfs_new_link(name, vn);
    vfs_append_link(&parent->child, ln);

    return vn;
}

vfs_link_t* vfs_readdir(vfs_vnode_t* dir) {
    if (dir->mount) { return vfs_readdir(dir->mount); }
    vfs_link_t* ln = dir->driver->readdir(dir);
    vfs_append_link_list(&dir->child, ln);
    return dir->child;
}

int vfs_unlink(vfs_vnode_t* parent, char* name) {
    // Cannot unlink a mountpoint...
    if (parent->mount) { return -1; }

    // Call driver to unlink
    parent->driver->unlink(parent, name);

    vfs_link_t* preceding;
    vfs_link_t* ln = vfs_get_child_by_name(parent, name, &preceding);
    if (ln) {
        // Decrement refcount and free vnode if null
        ln->vnode->refcount--;
        if (ln->vnode->refcount <= 0) {
            free(ln->vnode);
        }
        
        // Remove link
        if (!preceding) {
            parent->child = ln->next;
        }
        else {
            preceding->next = ln->next;
        }
        vfs_free_link(ln);
    }

    return 0;
}

int vfs_mount(vfs_vnode_t* mountpoint, vfs_vnode_t* root) {
    if (mountpoint == root) { return -1; }
    if (mountpoint->mount) { return vfs_mount(mountpoint->mount, root); }
    mountpoint->mount = vfs_ref(root);
    return 0;
}

int vfs_unmount(vfs_vnode_t* mountpoint) {
    if (!mountpoint->mount) { return -1; }
    if (!mountpoint->mount->mount) {
        vfs_unref(mountpoint->mount);
        mountpoint->mount = NULL;
        return 0;
    }
    return vfs_unmount(mountpoint->mount);
}

vfs_file_t* vfs_open(vfs_vnode_t* vnode) {
    if (vnode->mount) { return vfs_open(vnode->mount); }
    return vnode->driver->open(vnode);
}

vfs_file_t* vfs_create(vfs_vnode_t* parent, char* name, uint16_t flags) {
    if (parent->mount) { return vfs_create(parent->mount, name, flags); }
    return parent->driver->create(parent, name, flags);
}

int vfs_read(vfs_file_t* file, uint8_t* buf, int len) {
    return file->vnode->driver->read(file, buf, len);
}

int vfs_write(vfs_file_t* file, uint8_t* buf, int len) {
    return file->vnode->driver->write(file, buf, len);
}

int vfs_seek(vfs_file_t* file, int pos) {
    return file->vnode->driver->seek(file, pos);
}

int vfs_tell(vfs_file_t* file) {
    return file->vnode->driver->tell(file);
}

int vfs_close(vfs_file_t* file) {
    return file->vnode->driver->close(file);
}

vfs_link_t* vfs_new_link(char* name, vfs_vnode_t* vnode) {
    vfs_link_t* ln = malloc(sizeof(vfs_link_t));
    int namelen = strlen(name);
    ln->name = malloc(namelen + 1);
    strcpy(ln->name, name);
    ln->vnode = vnode;
    ln->next = NULL;
    return ln;
}

void vfs_free_link(vfs_link_t* link) {
    free(link->name);
    free(link);
}

void vfs_free_link_list(vfs_link_t* link) {
    while (link) {
        vfs_link_t* next = link->next;
        vfs_free_link(link);
        link = next;
    }
}

void vfs_append_link(vfs_link_t** link, vfs_link_t* newlink) {
    vfs_link_t* old = *link;
    *link = newlink;
    newlink->next = old;
}

void vfs_append_link_list(vfs_link_t** link, vfs_link_t* newlink) {
    while (newlink) {
        vfs_append_link(link, newlink);
        newlink = newlink->next;
    }
}

vfs_link_t* vfs_get_child_by_name(vfs_vnode_t* parent, char* name, vfs_link_t** preceeding) {
    if (preceeding) { *preceeding = NULL; };
    vfs_link_t* ln = parent->child;
    while (ln) {
        if (!strcmp(ln->name, name)) { return ln; }
        if (preceeding) { *preceeding = ln; };
        ln = ln->next;
    }
    return NULL;
}

vfs_vnode_t* vfs_ref(vfs_vnode_t *vnode) {
    if (vnode->refcount <= 0 && vnode->driver) {
        vfs_driver_ref(vnode->driver);
    }

    vnode->refcount++;
    return vnode;
}

void vfs_unref(vfs_vnode_t *vnode) {
    vnode->refcount--;
    if (vnode->refcount > 0) { return; }
        

    vfs_driver_t *driver = vnode->driver;
    vnode->driver->ctx_cleanup(vnode);

    vfs_link_t *link = vnode->child;
    while (link) {
        vfs_link_t *tmp = link->next;
        vfs_unref(link->vnode);
        free(link->name);
        free(link);
        link = tmp;
    }

    free(vnode);

    if (driver) {
        vfs_driver_unref(driver);
    }
}

vfs_driver_t* vfs_driver_ref(vfs_driver_t *driver) {
    driver->refcount++;
    return driver;
}
void vfs_driver_unref(vfs_driver_t *driver) {
    driver->refcount--;
    if (driver->refcount > 0) { return; }
    driver->driver_cleanup(driver);
}