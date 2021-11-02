#pragma once
#include <stddef.h>
#include <stdint.h>
#include "gfx_query.h"

#define VBE_TEXT_MODE           0x03

#define VBE_INT_GET_CTRL_INFO   0x4F00
#define VBE_INT_GET_MODE_INFO   0x4F01
#define VBE_INT_SET_MODE        0x4F02

#define VBE_MODE_END            0xFFFF
#define VBE_SUPPORTED           0x004F

struct vbe_ctrl_info {
    char signature[4];
    uint16_t version;
    uint16_t oem_ptr_offset;
    uint16_t oem_ptr_segment;
    uint8_t capabilities[4];
    uint16_t video_modes_offset;
    uint16_t video_modes_segment;
    uint16_t total_mem;
    uint16_t software_rev;
    uint32_t vendor;
    uint32_t product_name;
    uint32_t product_rev;
    char reserved[222];
    char oem_data[256];
}__attribute__((packed));
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
    uint8_t planes;
    uint8_t bpp;
    uint8_t banks;
    uint8_t memory_model;
    uint8_t bank_size;
    uint8_t image_pages;
    uint8_t reserved0;
 
    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;
 
    uint32_t framebuffer;
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t _rsvd0[206];
}__attribute__((packed));
typedef struct vbe_mode_info vbe_mode_info_t;

void vbe_get_ctrl_info(vbe_ctrl_info_t* ctrl_info);
void vbe_get_mode_info(vbe_mode_info_t* mode_info, uint16_t mode);
void vbe_dump_modes(vbe_ctrl_info_t* ctrl_info);
uint16_t vbe_mode_search(vbe_ctrl_info_t* ctrl_info, gfx_query_t *query, vbe_mode_info_t* modeinfo);
char vbe_set_mode(uint16_t mode);

extern uint32_t asm_vbe_call();

