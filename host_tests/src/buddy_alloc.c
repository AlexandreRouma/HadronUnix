#include "buddy_alloc.h"
#include <string.h>

#ifdef BUDDY_ALLOC_DEBUG
#include <stdio.h>
#endif

uint16_t buddy_get_possible_order(uint64_t block_count, uint16_t max_order) {
    uint16_t order = 0;
    block_count >>= 1;
    while (block_count) {
        order++;
        block_count >>= 1;
    }
    return (order < max_order) ? order : max_order;
}

uint64_t buddy_get_count_at_order(uint64_t block_count, uint16_t order) {
    return block_count / (1 << order);
}

uint64_t buddy_get_size_at_order(uint64_t block_count, uint16_t order) {
    return ((buddy_get_count_at_order(block_count, order) - 1) / 8) + 1;
}

uint64_t buddy_get_size(uint64_t block_count, uint16_t max_order) {
    // Compute highest possible order
    uint16_t order = buddy_get_possible_order(block_count, max_order);

    // Add all lengths together
    uint64_t buddy_len = 0;
    for (uint16_t i = 0; i <= order; i++) {
        buddy_len += buddy_get_size_at_order(block_count, i);
    }

    return buddy_len + sizeof(buddy_t) + ((order+1) * sizeof(buddy_desc_t));
}

void buddy_create(buddy_t* buddy, uint64_t block_count, uint16_t max_order) {
    // Compute highest possible order
    uint16_t order = buddy_get_possible_order(block_count, max_order);

    // Write metadata
    buddy->block_count = block_count;
    buddy->order = order;

    // Calculate pointer and count of all buddies
    uint8_t* buddy_ptr = (uint8_t*)buddy + sizeof(buddy_t) + ((order+1) * sizeof(buddy_desc_t));
    uint64_t size;
    for (uint16_t i = 0; i <= order; i++) {
        buddy->buddies[i].ptr = buddy_ptr;
        buddy->buddies[i].count = buddy_get_count_at_order(block_count, i);
        size = buddy_get_size_at_order(block_count, i);
        memset(buddy_ptr, 0, size);
        buddy_ptr += size;
    }
}

uint64_t buddy_find(buddy_t* buddy, uint64_t count) {
    // Find required buddy order
    uint16_t req_order = 31;
    for (uint64_t i = 0; i < 31; i++) {
        if (count <= (1 << i)) {
            req_order = i;
            break;
        }
    }

    // If the required ordrer is not available, find continous sections at highest order
    if (req_order > buddy->order) {
        uint8_t* bud = buddy->buddies[buddy->order].ptr;
        uint64_t bcount = buddy->buddies[buddy->order].count;
        uint64_t nconseq = ((count - 1) / (1 << buddy->order)) + 1;
        uint64_t conseq = 0;
        for (uint64_t i = 0; i < bcount; i++) {
            // If it's free, count it, if not, restart counting
            if (!((bud[i/8] >> (i%8)) & 1)) {
                conseq++;
            }
            else {
                conseq = 0;
            }

            // If we got enough, return
            if (conseq >= nconseq) {
                return (i - nconseq + 1) << buddy->order;
            }
        }

        // Allocation failed
        return BUDDY_ALLOC_FAILED;
    }

    // Otherwise, find free buddy at order
    uint8_t* bud = buddy->buddies[req_order].ptr;
    uint64_t bcount = buddy->buddies[req_order].count;
    for (uint64_t i = 0; i < bcount; i++) {
        // If it's free, return it
        if (!((bud[i/8] >> (i%8)) & 1)) {
            return i << req_order;
        }
    }

    return BUDDY_ALLOC_FAILED;
}

void buddy_alloc(buddy_t* buddy, uint64_t offset, uint64_t count) {
    // Mark all orders used
    uint64_t first = offset;
    uint64_t last = offset + count - 1;
    for (int o = 0; o <= buddy->order; o++) {
        for (uint64_t i = first; i <= last; i++) {
            buddy->buddies[o].ptr[i/8] |= (1 << (i % 8));
        }
        first /= 2;
        last /= 2;
    }
}


void buddy_free(buddy_t* buddy, uint64_t offset, uint64_t count) {
    uint64_t first = offset;
    uint64_t last = offset + count - 1;

    // Free lowest order buddies
    for (uint64_t i = first; i <= last; i++) {
        buddy->buddies[0].ptr[i/8] &= ~(1 << (i % 8));
    }
    first /= 2;
    last /= 2;

    // Free higher order buddies
    uint8_t b1, b2;
    for (int o = 1; o <= buddy->order; o++) {
        for (uint64_t i = first; i <= last; i++) {
            b1 = (buddy->buddies[o-1].ptr[(i*2)/8] >> ((i*2)%8)) & 1;
            b2 = (buddy->buddies[o-1].ptr[((i*2)+1)/8] >> (((i*2)+1)%8)) & 1;
            if (b1 || b2) { continue; }
            buddy->buddies[o].ptr[i/8] &= ~(1 << (i % 8));
        }
        first /= 2;
        last /= 2;
    }
}

#ifdef BUDDY_ALLOC_DEBUG
void buddy_dump(buddy_t* buddy) {
    printf("=============== BUDDY ===============\n");

    for (int o = 0; o <= buddy->order; o++) {
        int count = buddy->buddies[o].count;
        uint8_t* bud = buddy->buddies[o].ptr;
        
        printf("%02d: ", o);
        for (int i = 0; i < count; i++) {
            printf(((bud[i/8] >> (i%8)) & 1) ? "#" : ".");

            for (int j = 0; j < (1 << o)-1; j++) {
                printf(" ");
            }
        }
        printf("\n");
    }
}
#endif