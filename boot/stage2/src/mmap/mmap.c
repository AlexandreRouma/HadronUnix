#include "mmap.h"
#include <realmode/realmode.h>

int mmap_get(mmap_entry_t* entries) {
    // Set the initial state of the registers
    uint32_t buf_addr = (uint32_t)&asm_mmap_buf;
    uint32_t eax = ((buf_addr / 16) << 16) | (buf_addr % 16);
    uint32_t ebx = 0;

    // Call BIOS until EBX is NULL
    int count = 0;
    ebx = realmode_call(asm_mmap_get, eax, ebx, 0, MMAP_BIOS_MAGIC);
    while (ebx) {
        entries[count++] = asm_mmap_buf;
        ebx = realmode_call(asm_mmap_get, eax, ebx, 0, MMAP_BIOS_MAGIC);
    }
    
    return count;
}