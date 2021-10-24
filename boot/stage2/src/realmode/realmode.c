#include <realmode/realmode.h>

uint32_t realmode_call(uint32_t (*func)(void), uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    uint32_t ptr = (uint32_t)func;
    return asm_realmode_enter(ptr / 16, ptr % 16, eax, ebx, ecx, edx);
}