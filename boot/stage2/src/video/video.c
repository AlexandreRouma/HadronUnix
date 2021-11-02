#include "video.h"
#include <realmode/realmode.h>
#include <string.h>
#include <vga/vga.h>

void vbe_get_ctrl_info(vbe_ctrl_info_t* ctrl_info) {
    // Get address of ctrl_info in ES:DI
    uint32_t es = (uint32_t)ctrl_info / 16;
    uint32_t di = (uint32_t)ctrl_info % 16;
    realmode_call(asm_vbe_call, VBE_INT_GET_CTRL_INFO, (es << 16) | di, 0, 0);
}

void vbe_get_mode_info(vbe_mode_info_t* mode_info, uint16_t mode) {
    // Get address of mode_info in ES:DI
    uint32_t es = (uint32_t)mode_info / 16;
    uint32_t di = (uint32_t)mode_info % 16;
    realmode_call(asm_vbe_call, VBE_INT_GET_MODE_INFO, (es << 16) | di, mode, 0);
}

void vbe_dump_modes(vbe_ctrl_info_t* ctrl_info) {
    uint16_t* modes = (ctrl_info->video_modes_segment * 16) + ctrl_info->video_modes_offset;
    vbe_mode_info_t modeinfo;

    for(int i = 0; modes[i] != VBE_MODE_END; i++) {
        // Get mode number and information
        uint16_t mode = modes[i];
        vbe_get_mode_info(&modeinfo, mode);

        // Dump to terminal
        char buf[32];
        
        itoa(modeinfo.width, buf, 31);
        vga_print(buf);
        vga_print("x");
        itoa(modeinfo.height, buf, 31);
        vga_print(buf);
        vga_print("x");
        itoa(modeinfo.bpp, buf, 31);
        vga_print(buf);
        vga_print("\n");
    }
}

uint16_t vbe_mode_search(vbe_ctrl_info_t* ctrl_info, gfx_query_t* query, vbe_mode_info_t* modeinfo) {
    uint16_t* modes = (ctrl_info->video_modes_segment * 16) + ctrl_info->video_modes_offset;

    // Iterate through modes
    for(int i = 0; modes[i] != VBE_MODE_END; i++) {
        // Get mode number and information
        uint16_t mode = modes[i];
        vbe_get_mode_info(modeinfo, mode);

        // If query is of mode type and mode is correct, stop here
        if (query->type == VBE_QUERY_MODE) {
            if (query->mode == mode) { return mode; }
            continue;
        }

        // Otherwise, we are in find mode. Find how many conditions are satisfied ...
        uint8_t satis = 0;

        // ... for width
        switch (query->triplet.wineq) {
        case 0:
            satis += (modeinfo->width == query->triplet.width);
            break;
        case 1:
            satis += (modeinfo->width >= query->triplet.width);
            break;
        case -1:
            satis += (modeinfo->width <= query->triplet.width);
            break;
        }

        // ... for height
        switch (query->triplet.hineq) {
        case 0:
            satis += (modeinfo->height == query->triplet.height);
            break;
        case 1:
            satis += (modeinfo->height >= query->triplet.height);
            break;
        case -1:
            satis += (modeinfo->height <= query->triplet.height);
            break;
        }

        // ... for depth
        switch (query->triplet.dineq) {
        case 0:
            satis += (modeinfo->bpp == query->triplet.depth);
            break;
        case 1:
            satis += (modeinfo->bpp >= query->triplet.depth);
            break;
        case -1:
            satis += (modeinfo->bpp <= query->triplet.depth);
            break;
        }

        // and then, we return the mode if OK
        if (satis >= 3) { return mode; }
    }

    // Return mode end if nothing was found.
    return VBE_MODE_END;
}

char vbe_set_mode(uint16_t mode) {
    return realmode_call(asm_vbe_call, VBE_INT_SET_MODE, mode, 0, 0);
}