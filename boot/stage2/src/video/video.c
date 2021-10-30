#include "video.h"

#include <realmode/realmode.h>
#include <string.h>

void vbe_get_ctrl_info(vbe_ctrl_info_t *ctrl_info) {
    // Get address of ctrl_info in ES:DI
    uint32_t es = (uint32_t)ctrl_info / 16;
    uint32_t di = (uint32_t)ctrl_info % 16;

    realmode_call(asm_vbe_call, VBE_INT_GET_CTRL, es << 16 | di, 0, 0);
}

uint16_t vbe_get_mode(vbe_ctrl_info_t *ctrl_info, vbe_mode_info_t *mode_info, uint16_t index) {
    uint16_t *modes = (uint16_t*)((ctrl_info->video_modes_segment * 16) + (ctrl_info->video_modes_offset));

    // Select mode based on index
    uint16_t mode = modes[index];

    // If the mode is the array end, return VBE_MODE_END
    if (mode == VBE_MODE_END)
        return mode;


    // ES:DI contains the address of the mode_info struct
    uint32_t es = (uint32_t)mode_info / 16;
    uint32_t di = (uint32_t)mode_info % 16;
    uint16_t ax = realmode_call(asm_vbe_call, VBE_INT_GET_MODE, es << 16 | di, mode, 0);

    // If not supported, return MODE_END_TOO
    if (ax != VBE_SUPPORTED)
        return VBE_MODE_END;
    return mode;
}

// Small decimal atoi function
// TODO: make it support hex for explicit mode selection
uint64_t atoi(char *src) {
    uint64_t res = 0;
    uint64_t len = strlen(src);
    for (int64_t i = len - 1, f = 1; i >= 0; i--, f *= 10) {
        res += (src[i] - '0') * f;
    }
    return res;
}

char vbe_mode_query_parse(vbe_mode_query_t *query, char *src) {
    char *type = src;
    char wineq = 0;
    char *width = NULL;
    char hineq = 0;
    char *height = NULL;
    char dineq = 0;
    char *depth = NULL;
    char *next = NULL;
    char ineq = 0;
    for (int i = 0; src[i]; i++) {
        if (src[i] == ':') {
            src[i] = '\0';
            i++;
            next = src + i;
        }
        else if (src[i] == 'x') {
            src[i] = '\0';
            ineq = 0;
            i++;
            next = src + i;
        }
        if (src[i] == '>' && src[i+1] && src[i+1] == '=') {
            ineq = 1;
            next = src + i + 2;
            i++;
        }
        else if (src[i] == '<' && src[i+1] && src[i+1] == '=') {
            ineq = -1;
            next = src + i + 2;
            i++;
        }

        if (next == NULL)
            continue;

        if (!width) {
            width = next;
            wineq = ineq;
        }
        else if (!height) {
            height = next;
            hineq = ineq;
        }
        else if (!depth) {
            depth = next;
            dineq = ineq;
        }

        next = NULL;
    }

    if (strcmp(type, "mode") == 0) {
        query->type = VBE_QUERY_MODE;
        if (!width)
            return 1;
        uint64_t m = atoi(width);
        query->mode = m;
        return 0;
    }
    else if (strcmp(type, "find") == 0) {
        query->type = VBE_QUERY_TRIPLET;
    }
    else {
        return 1;
    }

    if (!width || !height || !depth)
        return 1;

    query->triplet.wineq = wineq;
    query->triplet.hineq = hineq;
    query->triplet.dineq = dineq;
    query->triplet.width = atoi(width);
    query->triplet.height = atoi(height);
    query->triplet.depth = atoi(depth);
    return 0;
}

uint16_t vbe_mode_search(vbe_ctrl_info_t *ctrl_info, vbe_mode_query_t *query, vbe_mode_info_t *modeinfo) {
    uint16_t index;
    uint16_t modenum;

    // Iterate through types
    for(index = 0; (modenum = vbe_get_mode(ctrl_info, modeinfo, index)) != VBE_MODE_END; index++) {

        // If query is of mode type and mode is correct, stop here
        if (query->type == VBE_QUERY_MODE && query->mode == modenum) {
            return modenum;
        }

        // Otherwise, we are in find mode. Find how many conditions are satisfied ...
        char satis = 0;

        // ... for width
        switch (query->triplet.wineq) {
        case 0:
            satis += (modeinfo->width == query->triplet.width) ? 1 : 0;
            break;
        case 1:
            satis += (modeinfo->width >= query->triplet.width) ? 1 : 0;
            break;
        case -1:
            satis += (modeinfo->width <= query->triplet.width) ? 1 : 0;
            break;
        }

        // ... for height
        switch (query->triplet.hineq) {
        case 0:
            satis += (modeinfo->height == query->triplet.height) ? 1 : 0;
            break;
        case 1:
            satis += (modeinfo->height >= query->triplet.height) ? 1 : 0;
            break;
        case -1:
            satis += (modeinfo->height <= query->triplet.height) ? 1 : 0;
            break;
        }

        // ... for depth
        switch (query->triplet.dineq) {
        case 0:
            satis += (modeinfo->bpp == query->triplet.depth) ? 1 : 0;
            break;
        case 1:
            satis += (modeinfo->bpp >= query->triplet.depth) ? 1 : 0;
            break;
        case -1:
            satis += (modeinfo->bpp <= query->triplet.depth) ? 1 : 0;
            break;
        }

        // and then, we return the modenum if OK
        if (satis >= 3)
            return modenum;
    }

    // Return mode end if nothing was found.
    return VBE_MODE_END;
}

char vbe_set_mode(uint16_t mode) {
    return realmode_call(asm_vbe_call, VBE_INT_SET_MODE, mode, 0, 0);
}