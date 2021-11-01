#include "vfs.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

vfs_vnode_t* vfs_walk(vfs_vnode_t* vnode, char* name) {
    // Check if a vnode is cached
}

vfs_link_t* vfs_readdir(vfs_vnode_t* vnode) {
    vfs_link_t* ln = vnode->driver->readdir(vnode);

    
}

vfs_vnode_t* vfs_touch(vfs_vnode_t* vnode, char* name, vfs_stat_t stat) {
    
}

int vfs_unlink(vfs_vnode_t* vnode, char* name) {
    int err = vnode->driver->unlink(vnode, name);
    if (err) { return err; }
    
    vfs_link_t* preceding;
    vfs_link_t* ln = vfs_search_links_by_name(vnode->children, name, &preceding);
    if (!ln) { return 0; }

    if (preceding) {
        preceding->next = ln->next;
    }
    else {
        vnode->children = ln->next;
    }

    vfs_decrement_and_destroy(ln->vnode);
    vfs_free_link(ln);
}

vfs_vnode_t* vfs_reach(vfs_vnode_t* vnode, char* path) {
    
}

void vfs_decrement_and_destroy(vfs_vnode_t* vnode) {
    if (--vnode->refcount <= 0) {
        vnode->driver->destroy(vnode);
        free(vnode);
    }
}

// void vfs_update_children(vfs_vnode_t* vnode, vfs_link_t* children) {
//     while (children) {
//         // Check if already a children, if yes, update stat
//         vfs_link_t* ln = vfs_search_links_by_name(vnode->children, children->name, NULL);
//         if (ln) {
//             // Free element and go to next element
//             vfs_link_t* next = children->next;
//             vfs_free_element(children);
//             children = next;
//             continue;
//         }

//         // If not found, add it to the list
//         vfs_link_t* next = children->next;
//         vfs_link_t* first = vnode->children;
//         vnode->children = children;
//         children->next = first;
//         children = next;
//     }
// }

vfs_link_t* vfs_search_links_by_name(vfs_link_t* link, char* name, vfs_link_t** preceding) {
    if (preceding) { *preceding = NULL; };
    while (link) {
        if (!strcmp(name, link->name)) {
            return link;
        }
        if (preceding) { *preceding = link; };
        link = link->next;
    }
    return NULL;
}

void vfs_free_link(vfs_link_t* link) {
    free(link->name);
    free(link);
}