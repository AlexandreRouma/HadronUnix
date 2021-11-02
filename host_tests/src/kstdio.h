#pragma once
#include "vfs.h"

extern vfs_file_t* kstdout;
extern vfs_file_t* kstderr;

#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

vfs_file_t* kfopen(char* path);
int kfread(void* buf, int size, int nmemb, vfs_file_t* stream);
int kfwrite(void* buf, int size, int nmemb, vfs_file_t* stream);
int kfseek(vfs_file_t* stream, int offset, int origin);
int kftell(vfs_file_t* stream);
void kfclose(vfs_file_t* file);