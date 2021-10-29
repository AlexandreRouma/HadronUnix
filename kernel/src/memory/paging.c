#include "paging.h"
#include "memmap.h"
#include <string.h>

page_entry_t k_pml4[4096]__attribute__((aligned(4096)));

void paging_init() {
    paging_init_pml4(k_pml4);
}

void paging_init_pml4(page_entry_t* pml4) {
    // Clear the table and map the last entry to the PML4
    memset(pml4, 0, sizeof(pml4));
    pml4[PAGING_ENTRY_COUNT-1] = (uint64_t)&pml4 | PAGING_ENTRY_FLAG_PRESENT | PAGING_ENTRY_FLAG_RW;
}

page_entry_t* paging_gen_access(uint64_t virt, paging_level_t level) {
    if (level == PAGING_LEVEL_PT) {
        return  (((uint64_t)0b1111111111111111111111111 << 39) |
                (virt >> 9)) & ~(uint64_t)0xFFF;
    }
    else if (level == PAGING_LEVEL_PD) {
        return  (((uint64_t)0b1111111111111111111111111 << 39) | 
                ((uint64_t)0b111111111 << 30) | (virt >> 18)) & ~(uint64_t)0xFFF;
    }
    else if (level == PAGING_LEVEL_PDP) {
        return  (((uint64_t)0b1111111111111111111111111 << 39) |
                ((uint64_t)0b111111111 << 30) | ((uint64_t)0b111111111 << 21) |
                (virt >> 27)) & ~(uint64_t)0xFFF;
    }
}

void paging_map(page_entry_t* pml4, uint64_t virt, uint64_t phy, uint64_t flags) {
    // Get PML4 entry
    page_entry_t pml4e = k_pml4[PAGING_PML4_IDX(virt)];

    // If no page directory is available for this address, create one
    if (!(pml4e & PAGING_ENTRY_FLAG_PRESENT)) {

    }
    
}

int paging_get_mapping(page_entry_t* pml4, uint64_t virt, uint64_t* phy, uint64_t* flags) {
    // Get PML4 entry and PDP address
    page_entry_t pml4e = pml4[PAGING_PML4_IDX(virt)];
    if (!(pml4e & PAGING_ENTRY_FLAG_PRESENT)) { return -1; }
    page_entry_t* pdp = pml4e & PAGING_ENTRY_ADDR_MASK;

    // Get PDP entry and PD address
    page_entry_t pdpe = pdp[PAGING_PDP_IDX(virt)];
    if (!(pdpe & PAGING_ENTRY_FLAG_PRESENT)) { return -1; }
    page_entry_t* pd = pdpe & PAGING_ENTRY_ADDR_MASK;

    // Get PD entry and PT address
    page_entry_t pde = pd[PAGING_PD_IDX(virt)];
    if (!(pde & PAGING_ENTRY_FLAG_PRESENT)) { return -1; }
    page_entry_t* pt = pde & PAGING_ENTRY_ADDR_MASK;
    
    // Get PT entry and physical address
    page_entry_t pte = pt[PAGING_PT_IDX(virt)];
    if (!(pte & PAGING_ENTRY_FLAG_PRESENT)) { return -1; }
    return pte & PAGING_ENTRY_ADDR_MASK;
}