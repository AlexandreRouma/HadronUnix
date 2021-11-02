#include "memmap.h"
#include <string.h>
#include <stdbool.h>

#ifdef MEMMAP_DEBUG
#include <inttypes.h>
#endif

memmap_t k_mem_map;

void memmap_init() {
    k_mem_map.entry_count = 0;
}

void memmap_insert(uint64_t id, memmap_entry_t entry) {
    if (k_mem_map.entry_count-id > 0) {
        memmove(&k_mem_map.map[id+1], &k_mem_map.map[id], (k_mem_map.entry_count-id) * sizeof(memmap_entry_t));
    }
    k_mem_map.map[id] = entry;
    k_mem_map.entry_count++;
}

void memmap_remove(uint64_t first, uint64_t last) {
    if (k_mem_map.entry_count-last-1 > 0) {
        memmove(&k_mem_map.map[first], &k_mem_map.map[last+1], (k_mem_map.entry_count-last-1) * sizeof(memmap_entry_t));   
    }
    k_mem_map.entry_count -= last - first + 1;
}

void memmap_merge(uint64_t first, uint64_t last) {
    memmap_entry_t fentry = k_mem_map.map[first];
    memmap_entry_t lentry = k_mem_map.map[last];
    memmap_remove(first, last);
    memmap_insert(first, (memmap_entry_t) {
        .base = fentry.base,
        .size = (lentry.base + lentry.size) - fentry.base,
        .type = fentry.type
    });
}

void memmap_split(uint64_t id, uint64_t offset, uint64_t size) {
    memmap_entry_t entry = k_mem_map.map[id];

    // If the offset is null, and the size is equal, remove the entry
    if (!offset && size == entry.size) {
        memmap_remove(id, id);
        return;
    }

    // if the offset is null, add size to the base
    if (!offset) {
        k_mem_map.map[id].base += size;
        k_mem_map.map[id].size -= size;
        return;
    }
    
    // If the end of the entry is the same as the end of the split, cut the end
    if (offset + size == entry.size) {
        k_mem_map.map[id].size -= size;
        return;
    }

    // Otherwise, create a hole in the middle of the entry
    k_mem_map.map[id].size = offset;
    memmap_insert(id+1, (memmap_entry_t) {
        .base = entry.base + offset + size,
        .size = entry.size - offset - size,
        .type = entry.type
    });
}

void memmap_cut_bottom(uint64_t id, uint64_t count) {
    // If cutting the entire entry, remove it from the list instread
    memmap_entry_t entry = k_mem_map.map[id];
    if (entry.size == count) {
        memmap_remove(id, id);
        return;
    }

    // Modify the base and size
    k_mem_map.map[id].base += count;
    k_mem_map.map[id].size -= count;
}

void memmap_cut_top(uint64_t id, uint64_t count) {
    // If cutting the entire entry, remove it from the list instread
    memmap_entry_t entry = k_mem_map.map[id];
    if (entry.size == count) {
        memmap_remove(id, id);
        return;
    }

    // Modify the size
    k_mem_map.map[id].size -= count;
}

uint8_t memmap_get_address_info(uint64_t addr, uint64_t* in, uint64_t* before, uint64_t* after) {
    uint8_t ret = 0;
    memmap_entry_t entry;

    // Find entry that contains the address
    for (uint64_t i = 0; i < k_mem_map.entry_count; i++) {
        entry = k_mem_map.map[i];
        if (addr >= entry.base && addr < entry.base + entry.size) {
            *in = i;
            ret |= MEMMAP_INFO_FLAGS_IN;
            break;
        }
    }

    // Find entry after the address
    for (uint64_t i = 0; i < k_mem_map.entry_count; i++) {
        entry = k_mem_map.map[i];
        if (entry.base > addr) {
            *after = i;
            ret |= MEMMAP_INFO_FLAGS_AFTER;
            break;
        }
    }

    // Find entry before the address
    for (uint64_t i = k_mem_map.entry_count; i > 0; i--) {
        entry = k_mem_map.map[i-1];
        if (entry.base < addr && addr >= entry.base + entry.size) {
            *before = i-1;
            ret |= MEMMAP_INFO_FLAGS_BEFORE;
            break;
        }
    }

    return ret;
}

void memmap_define(uint64_t base, uint64_t size, memmap_region_type_t type) {
    memmap_entry_t nentry = (memmap_entry_t) {
        .base = base,
        .size = size,
        .type = type
    };
    
    // If this is the only block just insert it at index 0
    if (!k_mem_map.entry_count) {
        memmap_insert(0, nentry);
        return;
    }

    // Get info about the bottom and top address of the block
    uint64_t bin, bbefore, bafter;
    uint64_t tin, tbefore, tafter;
    uint8_t binfo = memmap_get_address_info(base, &bin, &bbefore, &bafter);
    uint8_t tinfo = memmap_get_address_info(base + size - 1, &tin, &tbefore, &tafter);

    // If the new block is outside of all entries
    if (!(binfo & MEMMAP_INFO_FLAGS_IN) && !(tinfo & MEMMAP_INFO_FLAGS_IN)) {
        // If the new entry covers one or multiple entries completely, remove them
        if ((binfo & MEMMAP_INFO_FLAGS_AFTER) && (tinfo & MEMMAP_INFO_FLAGS_BEFORE) && tbefore >= bafter) {
            memmap_remove(bafter, tbefore);
            memmap_insert(bafter, nentry);
            return;
        }

        // Otherwise, insert normally
        if (binfo & MEMMAP_INFO_FLAGS_AFTER) {
            memmap_insert(bafter, nentry);
        }
        else {
            memmap_insert(bbefore + 1, nentry);
        }
        return;
    }

    // If the new block is all insize one entry
    if ((binfo & MEMMAP_INFO_FLAGS_IN) && (tinfo & MEMMAP_INFO_FLAGS_IN) && bin == tin) {
        memmap_entry_t entry = k_mem_map.map[bin];
        
        // If the base and size are indentical, just redifine the type
        if (entry.base == base && entry.size == size) {
            k_mem_map.map[bin].type = type;
            return;
        }

        // If the new entry replaces only the end
        if (entry.base + entry.size == base + size) {
            memmap_cut_top(bin, size);
            memmap_insert(bin + 1, nentry);
            return;
        }

        // If the new entry replaces only the start
        if (entry.base == base) {
            memmap_cut_bottom(bin, size);
            memmap_insert(bin, nentry);
            return;
        }

        // Otherwise, insert it in the middle
        memmap_split(bin, base - entry.base, size);
        memmap_insert(bin + 1, nentry);
        return;
    }

    // If both the top and bottom of the block are in diffentent entries
    if ((binfo & MEMMAP_INFO_FLAGS_IN) && (tinfo & MEMMAP_INFO_FLAGS_IN)) {
        memmap_entry_t bentry = k_mem_map.map[bin];
        memmap_entry_t tentry = k_mem_map.map[tin];

        // If the new block cover both entries completely, replace all
        if (bentry.base == base && tentry.base + tentry.size == base + size) {
            memmap_remove(bin, tin);
            memmap_insert(bin, nentry);
            return;
        }

        // If the new block covers entirely the bottom entry
        if (bentry.base == base) {
            // Clip the bottom of the top entry, remove the intermediates and insert new
            memmap_cut_bottom(tin, (base + size) - tentry.base);
            memmap_remove(bin, tin-1);
            memmap_insert(bin, nentry);
            return;
        }

        // If the new block covers entirely the top entry
        if (base + size == tentry.base + tentry.size) {
            memmap_cut_top(bin, bentry.size - (base - bentry.base));
            memmap_remove(bin + 1, tin);
            memmap_insert(bin + 1, nentry);
            return;
        }

        // Otherwise, the new block covers partially the top and bottom entries
        memmap_cut_bottom(tin, (base + size) - tentry.base);
        memmap_cut_top(bin, bentry.size - (base - bentry.base));

        // If there are intermediate entries, remove them
        if (bin + 1 < tin) {
            memmap_remove(bin + 1, tin - 1);
        }

        // Insert the new entry
        memmap_insert(bin + 1, nentry);
        return;
    }

    // If lower part is inside an entry and the upper isn't
    if (binfo & MEMMAP_INFO_FLAGS_IN) {
        memmap_entry_t entry = k_mem_map.map[bin];

        // If there are entries between the beginning entry and the end, remove them
        if (tbefore != bin) {
            memmap_remove(bafter + 1, tbefore);
        }
        
        // If the entry is completely relplaced
        if (entry.base == base) {
            k_mem_map.map[bin].size = size;
            k_mem_map.map[bin].type = type;
            return;
        }

        // Otherwise, cut the top of the entry and insert the new entry
        memmap_cut_top(bin, entry.size - (base - entry.base));
        memmap_insert(bin + 1, nentry);
        return;
    }

    // If the upper part is inside an entry, but the top isn't
    if (tinfo & MEMMAP_INFO_FLAGS_IN) {
        memmap_entry_t entry = k_mem_map.map[tin];

        // If there are entries between the beginning and end entry, remove them
        if (bafter != tin) {
            memmap_remove(bafter, tin - 1);
        }
        
        // If the entry is completely relplaced
        if (entry.base + entry.size == base + size) {
            k_mem_map.map[bafter].base = base;
            k_mem_map.map[bafter].size = size;
            k_mem_map.map[bafter].type = type;
            return;
        }

        // Otherwise, cut the top of the entry and insert the new entry
        memmap_cut_bottom(bafter, (base + size) - entry.base);
        memmap_insert(bafter, nentry);
        return;
    }
}

void memmap_agregate() {
    uint64_t i = 0;
    uint64_t count = 0;
    while (i < k_mem_map.entry_count) {
        // Count coninuous identical type entries
        for (count = 1; (i + count) < k_mem_map.entry_count &&
                        k_mem_map.map[i].type == k_mem_map.map[i + count].type &&
                        k_mem_map.map[i + count - 1].base + k_mem_map.map[i + count - 1].size == k_mem_map.map[i + count].base; count++);

        // If there are more than 1 continuous entries, merge them
        if (count > 1) {
            memmap_merge(i, i + count - 1);
        }

        i++;
    }
}

#ifdef MEMMAP_DEBUG
void memmap_dump() {
    printf("=================== MEMMAP ===================\n");
    for (int i = 0; i < k_mem_map.entry_count; i++) {
        printf("[%02d]: BASE=0x%08" PRIX64 " SIZE=0x%08" PRIX64 " TYPE=%d\n", i, k_mem_map.map[i].base, k_mem_map.map[i].size, k_mem_map.map[i].type);
    }
}
#endif