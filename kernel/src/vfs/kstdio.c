#include "kstdio.h"
#include <string.h>

vfs_file_t* kstdout;
vfs_file_t* kstderr;

vfs_file_t* kfopen(char* path) {
    if (path[0] == '/') { path++; }
    int pathlen = strlen(path);
    if (!pathlen) { return 0; }

    char buf[512];

    vfs_vnode_t* vn = vfs_root;

    // Nagivate path
    while (pathlen > 0) {
        // Find next seperator
        int sep;
        for (sep = 0; sep < pathlen; sep++) {
            if (path[sep] == '/') { break; }
        }

        if (!sep) {
            pathlen -= 1;
            path += 1;
            continue;
        }

        memcpy(buf, path, sep);
        buf[sep] = 0;

        vn = vfs_walk(vn, buf);
        if (!vn) { return NULL; }
        
        pathlen -= sep + 1;
        path += sep + 1;
    }

    return vfs_open(vn);
}

int kfread(void* buf, int size, int nmemb, vfs_file_t* stream) {
    return vfs_read(stream, buf, size);
}

int kfwrite(void* buf, int size, int nmemb, vfs_file_t* stream) {
    return vfs_write(stream, buf, size);
}

int kfseek(vfs_file_t* stream, int offset, int origin) {
    if (origin == SEEK_SET) {
        return vfs_seek(stream, offset);
    }
    else if (origin == SEEK_CUR) {
        int cur = vfs_tell(stream);
        return vfs_seek(stream, cur + offset);
    }
    else if (origin == SEEK_END) {
        // TODO: Needs different behavior for read and write
        return vfs_seek(stream, stream->vnode->stat.size - 1);
    }
    return -1;
}

int kftell(vfs_file_t* stream) {
    return vfs_tell(stream);
}

void kfclose(vfs_file_t* file) {
    vfs_close(file);
}