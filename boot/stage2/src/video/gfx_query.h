#pragma once
#include <stdint.h>

#define VBE_QUERY_MODE      0
#define VBE_QUERY_TRIPLET   1

struct vbe_mode_query {
    char type;
    union {
        uint16_t mode;
        struct {
            char wineq;
            char hineq;
            char dineq;
            uint16_t width;
            uint16_t height;
            uint16_t depth;
        } triplet;
    };
};
typedef struct vbe_mode_query gfx_query_t;

char gfx_query_parse(gfx_query_t *query, char *src);