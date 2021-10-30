#pragma once

#include <stddef.h>
#include <stdint.h>

#define VBE_INT_GET_CTRL 0x4F00
#define VBE_INT_GET_MODE 0x4F01
#define VBE_INT_SET_MODE 0x4F02

#define VBE_MODE_END 0xFFFF
#define VBE_SUPPORTED 0x004F

struct vbe_ctrl_info {
    char signature[4];
    uint16_t version;
    uint32_t oem;
    uint32_t capabilities;
    uint16_t video_modes_offset;
    uint16_t video_modes_segment;
    uint16_t software_rev;
    uint32_t vendor;
    uint32_t product_name;
    uint32_t product_rev;
    char reserved[222];
    char oem_data[256];
} __attribute__((packed));
typedef struct vbe_ctrl_info vbe_ctrl_info_t;

struct vbe_mode_info {
    uint16_t attributes;
    uint8_t window_a;
    uint8_t window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;
    uint16_t width;
    uint16_t height;
    uint8_t w_char;
    uint8_t y_char;
    uint8_t memplanes;
    uint8_t bpp;
    uint8_t banks;
    uint8_t memmodel;
    uint8_t banksize;
    uint8_t imgperpage;
    uint8_t reserved;
    uint8_t redmasksize;
    uint8_t redfieldpos;
    uint8_t greenmasksize;
    uint8_t greenfieldpos;
    uint8_t bluemasksize;
    uint8_t bluefieldpos;
    uint8_t reservedmasksize;
    uint8_t reservedfieldpos;
    uint8_t directcolorinfo;
    uint32_t phys;
    uint32_t offscreenoff;
    uint16_t offscreensize;
    uint8_t reserved2[206];
};
typedef struct vbe_mode_info vbe_mode_info_t;

void vbe_get_ctrl_info(vbe_ctrl_info_t *ctrl_info);
uint16_t vbe_get_mode(vbe_ctrl_info_t *ctrl_info, vbe_mode_info_t *mode_info, uint16_t modenum);
char vbe_set_mode(uint16_t mode);

extern uint32_t asm_vbe_call();

#define VBE_QUERY_MODE 0
#define VBE_QUERY_TRIPLET 1

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
typedef struct vbe_mode_query vbe_mode_query_t;

char vbe_mode_query_parse(vbe_mode_query_t *query, char *src);

uint16_t vbe_mode_search(vbe_ctrl_info_t *ctrl_info, vbe_mode_query_t *query, vbe_mode_info_t *modeinfo);