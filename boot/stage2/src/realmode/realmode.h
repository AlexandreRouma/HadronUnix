#pragma once
#include <stdint.h>

extern uint32_t asm_realmode_enter(uint32_t cs, uint32_t ip, uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

uint32_t realmode_call(uint32_t (*func)(void), uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);