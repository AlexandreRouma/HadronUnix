#include "tarfs.h"
#include <string.h>
#include <kfmt.h>

tarfs_t* tarfs_create(uint8_t* data, uint64_t len) {
    tarfs_t* tarfs = malloc(sizeof(tarfs_t));

    // Fill driver function pointers
    tarfs->driver.walk = tarfs_walk;
    tarfs->driver.readdir = tarfs_readdir;
    tarfs->driver.unlink = tarfs_unlink;
    tarfs->driver.open = tarfs_open;
    tarfs->driver.read = tarfs_read;
    tarfs->driver.write = tarfs_write;
    tarfs->driver.seek = tarfs_seek;
    tarfs->driver.tell = tarfs_tell;
    tarfs->driver.close = tarfs_close;

    // Create root vnode
    tarfs->root = malloc(sizeof(vfs_vnode_t));
    tarfs->root->driver = &tarfs->driver;
    tarfs->root->stat.flags = VFS_FLAG_DIRECTORY;
    tarfs->root->stat.permissions = 0777;
    tarfs->root->stat.uid = 0;
    tarfs->root->stat.gid = 0;
    tarfs->root->stat.create_time = 0;
    tarfs->root->stat.modified_time = 0;
    tarfs->root->stat.size = 0;
    tarfs->root->refcount = 1;
    tarfs->root->child = NULL;
    tarfs->root->mount = NULL;

    char namebuf[128];
    char tempbuf[128];

    // Parse TAR file
    for (uint64_t i = 0; i < len;) {
        vfs_stat_t stat;
        char* path = tarfs_parse_header(&data[i], &stat);
        int pathlen = strlen(path);

        // If the name is null, we've reacher the end of the archive
        if (!pathlen) {
            break;
        }

        // Namebuf must contain the filename and parent must contain the parent vnode
        vfs_vnode_t* parent = tarfs->root;
        int path_count = tarfs_get_path_elem_count(path);
        for (int i = 0; i < path_count; i++) {
            // Find separator
            int sep;
            for (sep = 0; sep  < pathlen; sep++) {
                if (path[sep] == '/') { break; }
            }

            // Get name of path part
            memcpy(tempbuf, path, sep);
            tempbuf[sep] = 0;

            // Walk the path
            vfs_link_t* cln = vfs_get_child_by_name(parent, tempbuf, NULL);
            if (!cln) {
                kprintf("Could not find parent for %s\n", tempbuf);
                return NULL;
            }
            parent = cln->vnode;

            pathlen -= sep + 1;
            path += sep + 1;
        }

        // Get name of file
        strcpy(namebuf, path);
        int namebuflen = strlen(namebuf);
        if (namebuf[namebuflen-1] == '/') {
            namebuf[namebuflen-1] = 0;
        }

        // Create the vnode
        vfs_vnode_t* vn = malloc(sizeof(vfs_vnode_t));
        vn->stat = stat;
        vn->refcount = 1;
        vn->child = NULL;
        vn->mount = NULL;
        vn->driver = &tarfs->driver;
        vn->ctx = &data[i + 512];

        // Create and append the link
        vfs_link_t* ln = vfs_new_link(namebuf, vn);
        vfs_append_link(&parent->child, ln);
        
        // Skip to the next file entry
        uint64_t data_size = (((stat.size - 1) >> 9) + 1) << 9;
        i += 512 + data_size;
    }

    return tarfs;
}

void tarfs_destroy(tarfs_t* tarfs) {
    vfs_link_t* ln = tarfs->root->child;
    while (ln) {
        vfs_link_t* next = ln->next;
        tarfs_free_link(ln);
        ln = next;
    }

    free(tarfs->root);
    free(tarfs);
}

void tarfs_free_link(vfs_link_t* link) {
    vfs_link_t* ln = link->vnode->child;
    while (ln) {
        vfs_link_t* next = ln->next;
        tarfs_free_link(ln);
        ln = next;
    }

    free(link->vnode);
    free(link->name);
    free(link);
}

char* tarfs_parse_header(char* header, vfs_stat_t* stat) {
    char* name = &header[0];
    char* _mode = &header[100];
    char* _uid = &header[108];
    char* _gid = &header[116];
    char* _size = &header[124];
    char* _mod_time = &header[136];
    char* _type = &header[156];

    uint64_t mode = tarfs_parse_octal(_mode, 8);
    uint64_t uid = tarfs_parse_octal(_uid, 8);
    uint64_t gid = tarfs_parse_octal(_gid, 8);
    uint64_t size = tarfs_parse_octal(_size, 12);
    uint64_t mod_time = tarfs_parse_octal(_mod_time, 12);
    uint64_t type = tarfs_parse_octal(_type, 1);

    stat->flags = (type == 5) ? VFS_FLAG_DIRECTORY : 0;
    stat->permissions = mode;
    stat->uid = uid;
    stat->gid = gid;
    stat->create_time = mod_time;
    stat->modified_time = mod_time;
    stat->size = size;

    return name;
}

uint64_t tarfs_parse_octal(char* octal, int maxlen) {
    uint64_t n = 0;
    int len = strlen(octal);
    if (len > maxlen) { len = maxlen; }
    for (int i = len-1; i >= 0; i--) {
        char c = octal[i];
        if (c > '7' || c < '0') { break; }
        n |= (uint64_t)(c-'0') << ((len - i - 1) * 3);
    }
    return n;
}

int tarfs_get_path_elem_count(char* path) {
    int len = strlen(path);
    int count = 0;
    for (int i = 0; i < len-1; i++) {
        if (path[i] == '/') { count++; }
    }
    return count;
}

struct vfs_vnode* tarfs_walk(struct vfs_vnode* parent, char* name) {
    return NULL;
}

struct vfs_link* tarfs_readdir(struct vfs_vnode* dir) {
    return NULL;
}

int tarfs_unlink(struct vfs_vnode* parent, char* name) {
    return -1;
}

struct vfs_file* tarfs_open(struct vfs_vnode* vnode) {
    vfs_file_t* file = malloc(sizeof(vfs_file_t));
    file->vnode = vnode;
    file->ctx = malloc(sizeof(uint64_t));
    uint64_t* cur = file->ctx;
    *cur = 0;
    return file;
}

int tarfs_read(struct vfs_file* file, uint8_t* buf, int len) {
    uint64_t* cur = file->ctx;
    int to_read = len;
    int readable = file->vnode->stat.size - *cur;
    if (readable <= 0) { return 0; }
    if (to_read > readable) { to_read = readable; }
    memcpy(buf, file->vnode->ctx + *cur, to_read);
    *cur += to_read;
    return to_read;
}

int tarfs_write(struct vfs_file* file, uint8_t* buf, int len) {
    return -1;
}

int tarfs_seek(struct vfs_file* file, int pos) {
    uint64_t* cur = file->ctx;
    if (pos >= file->vnode->stat.size) { pos = file->vnode->stat.size - 1; }
    *cur = pos;
    return pos;
}

int tarfs_tell(struct vfs_file* file) {
    uint64_t* cur = file->ctx;
    return *cur;
}

int tarfs_close(struct vfs_file* file) {
    free(file->ctx);
    free(file);
}