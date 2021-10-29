#pragma once
#include <stdint.h>

#define PAGING_ENTRY_FLAG_PRESENT       (1 << 0)
#define PAGING_ENTRY_FLAG_RW            (1 << 1)
#define PAGING_ENTRY_FLAG_USER          (1 << 2)
#define PAGING_ENTRY_FLAG_WRITETHROUGH  (1 << 3)
#define PAGING_ENTRY_FLAG_NO_CACHE      (1 << 4)
#define PAGING_ENTRY_FLAG_ACCESSED      (1 << 5)
#define PAGING_ENTRY_FLAG_DIRTY         (1 << 6)
#define PAGING_ENTRY_FLAG_GLOBAL        (1 << 8)
#define PAGING_ENTRY_FLAG_NX            (1 << 63)

#define PAGING_ENTRY_ADDR_MASK  0xFFFFFFFFFF000

#define PAGING_ENTRY_COUNT      512

#define PAGING_PT_IDX(addr)     (((addr) & ((uint64_t)0b111111111 << 12)) >> 12)
#define PAGING_PD_IDX(addr)     (((addr) & ((uint64_t)0b111111111 << 21)) >> 21)
#define PAGING_PDP_IDX(addr)    (((addr) & ((uint64_t)0b111111111 << 30)) >> 30)
#define PAGING_PML4_IDX(addr)   (((addr) & ((uint64_t)0b111111111 << 39)) >> 39)

typedef uint64_t page_entry_t;

extern page_entry_t k_pml4[4096];

enum paging_level {
    PAGING_LEVEL_PT,
    PAGING_LEVEL_PD,
    PAGING_LEVEL_PDP,
    PAGING_LEVEL_PML4
};
typedef enum paging_level paging_level_t;

void paging_init();
void paging_init_pml4(page_entry_t* pml4);
page_entry_t* paging_gen_access(uint64_t virt, paging_level_t level);
void paging_map(page_entry_t* pml4, uint64_t virt, uint64_t phy, uint64_t flags);
int paging_get_mapping(page_entry_t* pml4, uint64_t virt, uint64_t* phy, uint64_t* flags);