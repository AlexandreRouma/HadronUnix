#pragma once
#include "vfs.h"

enum test_type {
    TYPE_FILE,
    TYPE_DIRECTORY
};
typedef enum test_type test_type_t;

struct test_file {
    test_type_t type;
    char* name;
};
typedef struct test_file test_file_t;


struct test_driver {
    vfs_driver_t vfsdriver;
    vfs_vnode_t* root;
};
typedef struct test_driver test_driver_t;

test_driver_t* test_driver_create();
void test_driver_destroy(test_driver_t* driver);