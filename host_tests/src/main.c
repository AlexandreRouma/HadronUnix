#include <stdio.h>
#include <memmap.h>

#define B_COUNT 50
#define B_ORDER 7

int main() {
    memmap_init();


    memmap_insert(0, (memmap_entry_t){
        .base = 0,
        .size = 0x10,
        .type = MEMMAP_REGION_TYPE_FREE
    });

    memmap_insert(1, (memmap_entry_t){
        .base = 0x10,
        .size = 0x10,
        .type = MEMMAP_REGION_TYPE_FREE
    });

    memmap_dump();

    memmap_split(0, 4, 4);

    memmap_dump();

    return 0;
}