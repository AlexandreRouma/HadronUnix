#include "mmap.h"
#include <realmode/realmode.h>
#include <vga/vga.h>

// addr = (seg * 16) + off

int mmap_get(mmap_entry_t* entries) {
    // Set the initial state of the registers
    uint32_t buf_addr = (uint32_t)&asm_mmap_buf;
    uint32_t eax = ((buf_addr / 16) << 16) | (buf_addr % 16);
    uint32_t ebx = 0;
    int count = 0;

    // Do the first initializing call
    ebx = realmode_call(asm_mmap_get, eax, ebx, 0, MMAP_BIOS_MAGIC);
    if (ebx == 0xFFFFFFFF) { return -1; }
    entries[count++] = asm_mmap_buf;
    
    // Call BIOS until end of list
    while (ebx) {
        ebx = realmode_call(asm_mmap_get, eax, ebx, 0, MMAP_BIOS_MAGIC);
        if (ebx == 0xFFFFFFFF) { break; }
        entries[count++] = asm_mmap_buf;
    }
    
    return count;
}