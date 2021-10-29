#include <stdio.h>
#include <memmap.h>

#define B_COUNT 8000000
#define B_ORDER 15

int main() {
    memmap_init();

    memmap_define(0x00000, 0x00500, MEMMAP_REGION_TYPE_BIOS);
    memmap_define(0x80000, 0x20000, MEMMAP_REGION_TYPE_BIOS);

    memmap_dump();

    return 0;
}