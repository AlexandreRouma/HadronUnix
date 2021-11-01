#include "test_driver.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

vfs_vnode_t* test_driver_walk(vfs_vnode_t* parent, char* name) {
    test_driver_t* driver = parent->driver->ctx;
    test_file_t* tfile = parent->ctx;

    if (strcmp(name, "bruh") && strcmp(name, "bruh_file")) { return NULL; }

    if (!strcmp(name, "bruh")) {
        
    }
}

vfs_link_t* test_driver_readdir(vfs_vnode_t* dir) {
    test_driver_t* driver = dir->driver->ctx;
    test_file_t* tfile = dir->ctx;
    
    bool bruh_found = false;
    bool bruh_file_found = false;
    vfs_link_t* ln = dir->child;
    while (ln) {
        if (!strcmp(ln->name, "bruh")) { bruh_found = true; }
        if (!strcmp(ln->name, "bruh_file")) { bruh_file_found = true; }
        ln = ln->next;
    }

    vfs_link_t* rln = NULL;

    if (!bruh_found) {
        test_file_t* bruh = malloc(sizeof(test_file_t));
        bruh->type = TYPE_DIRECTORY;
        bruh->name = malloc(strlen("bruh")+1);
        strcpy(bruh->name, "bruh");

        vfs_vnode_t* vn = malloc(sizeof(vfs_vnode_t));
        vn->ctx = bruh;
        vn->driver = &driver->vfsdriver;
        vn->child = NULL;
        vn->mount = NULL;
        vn->refcount = 1;
        vn->stat = (vfs_stat_t) {
            .flags = VFS_FLAG_DIRECTORY,
            .permissions = 0x1FF,
            .uid = 0,
            .gid = 0,
            .create_time = 0,
            .modified_time = 0,
            .size = 0,
        };

        vfs_link_t* ln = vfs_new_link("bruh", vn);
        vfs_append_link(&rln, ln);
    }

    if (!bruh_file_found) {
        test_file_t* bruh_file = malloc(sizeof(test_file_t));
        bruh_file->type = TYPE_FILE;
        bruh_file->name = malloc(strlen("bruh_file")+1);
        strcpy(bruh_file->name, "bruh_file");

        vfs_vnode_t* vn = malloc(sizeof(vfs_vnode_t));
        vn->ctx = bruh_file;
        vn->driver = &driver->vfsdriver;
        vn->child = NULL;
        vn->mount = NULL;
        vn->refcount = 1;
        vn->stat = (vfs_stat_t) {
            .flags = 0,
            .permissions = 0x1FF,
            .uid = 0,
            .gid = 0,
            .create_time = 0,
            .modified_time = 0,
            .size = 69,
        };

        vfs_link_t* ln = vfs_new_link("bruh_file", vn);
        vfs_append_link(&rln, ln);
    }

    return rln;
}

int test_driver_unlink(vfs_vnode_t* parent, char* name) {
    test_driver_t* driver = parent->driver->ctx;
    test_file_t* tfile = parent->ctx;
    
}

vfs_file_t* test_driver_open(vfs_vnode_t* vnode) {
    test_driver_t* driver = vnode->driver->ctx;
    test_file_t* tfile = vnode->ctx;
    
}

int test_driver_read(vfs_file_t* file, uint8_t* buf, int len) {
    test_driver_t* driver = file->vnode->driver->ctx;
    test_file_t* tfile = file->vnode->ctx;
    
}

int test_driver_write(vfs_file_t* file, uint8_t* buf, int len) {
    test_driver_t* driver = file->vnode->driver->ctx;
    test_file_t* tfile = file->vnode->ctx;
    
}

int test_driver_seek(vfs_file_t* file, int pos) {
    test_driver_t* driver = file->vnode->driver->ctx;
    test_file_t* tfile = file->vnode->ctx;
    
}

int test_driver_tell(vfs_file_t* file) {
    test_driver_t* driver = file->vnode->driver->ctx;
    test_file_t* tfile = file->vnode->ctx;
    
}

int test_driver_close(vfs_file_t* file) {
    test_driver_t* driver = file->vnode->driver->ctx;
    test_file_t* tfile = file->vnode->ctx;
    
}

test_driver_t* test_driver_create() {
    test_driver_t* driver = malloc(sizeof(test_driver_t));

    test_file_t* rootfile = malloc(sizeof(test_file_t));

    rootfile->type = TYPE_DIRECTORY;
    rootfile->name = NULL;

    driver->vfsdriver.walk = test_driver_walk;
    driver->vfsdriver.readdir = test_driver_readdir;
    driver->vfsdriver.unlink = test_driver_unlink;
    driver->vfsdriver.open = test_driver_open;
    driver->vfsdriver.read = test_driver_read;
    driver->vfsdriver.write = test_driver_write;
    driver->vfsdriver.seek = test_driver_seek;
    driver->vfsdriver.tell = test_driver_tell;
    driver->vfsdriver.close = test_driver_close;

    driver->root = malloc(sizeof(vfs_vnode_t));
    driver->root->ctx = rootfile;
    driver->root->driver = &driver->vfsdriver;
    driver->root->child = NULL;
    driver->root->mount = NULL;
    driver->root->refcount = 1;
    driver->root->stat = (vfs_stat_t) {
        .flags = VFS_FLAG_DIRECTORY,
        .permissions = 0x1FF,
        .uid = 0,
        .gid = 0,
        .create_time = 0,
        .modified_time = 0,
        .size = 0,
    };

    return driver;
}

void test_driver_destroy(test_driver_t* driver) {
    free(driver);
}