#include "paging.h"
#include "memmap.h"
#include "palloc.h"
#include <string.h>
#include <vga_basic/vga.h>

page_entry_t k_pml4[PAGING_ENTRY_COUNT]__attribute__((aligned(4096)));

void paging_init() {
    paging_init_pml4(k_pml4);
}

void paging_init_pml4(page_entry_t* pml4) {
    // Clear the table and map the last entry to the PML4
    memset(pml4, 0, PAGING_TABLE_SIZE);
    pml4[PAGING_ENTRY_COUNT-1] = (uint64_t)pml4 | PAGING_ENTRY_FLAG_PRESENT | PAGING_ENTRY_FLAG_RW;
}

page_entry_t* paging_fractal_access(uint64_t virt, paging_level_t level) {
    // Bits 63:52 arte all 1s to respect canonical address form
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
    else if (level == PAGING_LEVEL_PML4) {
        return  (((uint64_t)0b1111111111111111111111111 << 39) |
                ((uint64_t)0b111111111 << 30) | ((uint64_t)0b111111111 << 21) |
                ((uint64_t)0b111111111 << 12) | (virt >> 27)) & ~(uint64_t)0xFFF;
    }
}

uint64_t paging_increment_subcount(page_entry_t* entry) {
    uint64_t count = paging_get_subcount(entry) + 1;
    paging_set_subcount(entry, count);
    return count;
}

uint64_t paging_decrement_subcount(page_entry_t* entry) {
    uint64_t count = paging_get_subcount(entry) - 1;
    paging_set_subcount(entry, count);
    return count;
}

uint64_t paging_get_subcount(page_entry_t* entry) {
    return (*entry >> 52) & 0b111111111;
}

uint64_t paging_set_subcount(page_entry_t* entry, uint64_t subcount) {
    *entry &= ~((uint64_t)0b111111111 << 52);
    *entry |= (subcount & 0b111111111) << 52;
}

uint64_t paging_area_size(uint64_t base, uint64_t size) {
    uint64_t first = (base & PAGING_ENTRY_ADDR_MASK) >> 12;
    uint64_t last = ((base + size - 1) & PAGING_ENTRY_ADDR_MASK) >> 12;
    return last - first + 1;
}

void paging_map(page_entry_t* pml4, uint64_t virt, uint64_t phy, uint64_t flags, bool map_tables) {   
    bool frac = (pml4 == 0);
    page_entry_t* pdp = NULL;
    page_entry_t* pd = NULL;
    page_entry_t* pt = NULL;

    // Align addresses
    virt &=  PAGING_ENTRY_ADDR_MASK;
    phy &= PAGING_ENTRY_ADDR_MASK;

    // Get PML4 entry and PDP address
    if (!pml4) {
        pml4 = paging_fractal_access(virt, PAGING_LEVEL_PML4);
    }
    page_entry_t* pml4e = &pml4[PAGING_PML4_IDX(virt)];

    // If we have no PDP, allocate it
    if (!(*pml4e & PAGING_ENTRY_FLAG_PRESENT)) {
        // Allocate table
        pdp = palloc_alloc(1);

        // Add it to the parent table
        *pml4e = (uint64_t)pdp | PAGING_ENTRY_FLAG_PRESENT | PAGING_ENTRY_FLAG_RW | PAGING_ENTRY_FLAG_USER;
        
        // If in fractal mode, invalidate TLB, otherwise, map it for further access
        if (frac) {
            paging_invalidate_tlb(virt);
        }
        else if (map_tables) {
            paging_map(NULL, pdp, pdp, PAGING_ENTRY_FLAG_RW, false);
        }

        // Clear the new table
        memset(pdp, 0, PAGING_TABLE_SIZE);
    }

    // Get PDP entry and PD address
    if (!pdp) {
        pdp = frac ? paging_fractal_access(virt, PAGING_LEVEL_PDP) : (*pml4e & PAGING_ENTRY_ADDR_MASK);
    }
    page_entry_t* pdpe = &pdp[PAGING_PDP_IDX(virt)];

    // If we have no PDP, allocate it
    if (!(*pdpe & PAGING_ENTRY_FLAG_PRESENT)) {
        // Allocate table
        pd = palloc_alloc(1);

        // Add it to the parent table
        *pdpe = (uint64_t)pd | PAGING_ENTRY_FLAG_PRESENT | PAGING_ENTRY_FLAG_RW | PAGING_ENTRY_FLAG_USER;
        
        // If in fractal mode, invalidate TLB, otherwise, map it for further access
        if (frac) {
            paging_invalidate_tlb(virt);
        }
        else if (map_tables) {
            paging_map(NULL, pd, pd, PAGING_ENTRY_FLAG_RW, false);
        }

        // Clear the new table
        memset(pd, 0, PAGING_TABLE_SIZE);

        // Increment the number of entries
        paging_increment_subcount(pml4e);
    }

    // Get PD entry and PT address
    if (!pd) {
        pd = frac ? paging_fractal_access(virt, PAGING_LEVEL_PD) : (*pdpe & PAGING_ENTRY_ADDR_MASK);
    }
    page_entry_t* pde = &pd[PAGING_PD_IDX(virt)];

    // If we have no PDP, allocate it
    if (!(*pde & PAGING_ENTRY_FLAG_PRESENT)) {
        // Allocate table
        pt = palloc_alloc(1);

        // Add it to the parent table
        *pde = (uint64_t)pt | PAGING_ENTRY_FLAG_PRESENT | PAGING_ENTRY_FLAG_RW | PAGING_ENTRY_FLAG_USER;
        
        // If in fractal mode, invalidate TLB, otherwise, map it for further access
        if (frac) {
            paging_invalidate_tlb(virt);
        }
        else if (map_tables) {
            paging_map(NULL, pt, pt, PAGING_ENTRY_FLAG_RW, false);
        }

        // Clear the new table
        memset(pt, 0, PAGING_TABLE_SIZE);

        // Increment the number of entries
        paging_increment_subcount(pdpe);
    }

    // Get PT entry
    if (!pt) {
        pt = frac ? paging_fractal_access(virt, PAGING_LEVEL_PT) : (*pde & PAGING_ENTRY_ADDR_MASK);
    }
    page_entry_t* pte = &pt[PAGING_PT_IDX(virt)];

    // Increment the number of entries if the page wasn't mapped
    if (!(*pte & PAGING_ENTRY_FLAG_PRESENT)) {
        paging_increment_subcount(pde);
    }

    // Map address and invalidate the TLB
    *pte = flags | PAGING_ENTRY_FLAG_PRESENT | phy;
    paging_invalidate_tlb(virt);

    return;
}

void paging_map_multiple(page_entry_t* pml4, uint64_t virt, uint64_t phy, uint64_t flags, uint64_t count, bool map_tables) {
    for (uint64_t i = 0; i < count; i++) {
        paging_map(pml4, virt, phy, flags, map_tables);
        virt += 4096;
        phy += 4096;
    }
}

void paging_unmap(page_entry_t* pml4, uint64_t virt, bool unmap_tables) {
    bool frac = (pml4 == 0);
    page_entry_t* pdp = NULL;
    page_entry_t* pd = NULL;
    page_entry_t* pt = NULL;

    // Align addresses
    virt &=  PAGING_ENTRY_ADDR_MASK;

    // Get PML4 entry and PDP address
    if (!pml4) {
        pml4 = paging_fractal_access(virt, PAGING_LEVEL_PML4);
    }
    page_entry_t* pml4e = &pml4[PAGING_PML4_IDX(virt)];

    // Abort if table not mapped
    if (!(*pml4e & PAGING_ENTRY_FLAG_PRESENT)) {
        return;
    }

    // Get PDP entry and PD address
    if (!pdp) {
        pdp = frac ? paging_fractal_access(virt, PAGING_LEVEL_PDP) : (*pml4e & PAGING_ENTRY_ADDR_MASK);
    }
    page_entry_t* pdpe = &pdp[PAGING_PDP_IDX(virt)];

    // Abort if table not mapped
    if (!(*pdpe & PAGING_ENTRY_FLAG_PRESENT)) {
        return;
    }

    // Get PD entry and PT address
    if (!pd) {
        pd = frac ? paging_fractal_access(virt, PAGING_LEVEL_PD) : (*pdpe & PAGING_ENTRY_ADDR_MASK);
    }
    page_entry_t* pde = &pd[PAGING_PD_IDX(virt)];

    // Abort if table not mapped
    if (!(*pde & PAGING_ENTRY_FLAG_PRESENT)) {
        return;
    }

    // Get PT entry
    if (!pt) {
        pt = frac ? paging_fractal_access(virt, PAGING_LEVEL_PT) : (*pde & PAGING_ENTRY_ADDR_MASK);
    }
    page_entry_t* pte = &pt[PAGING_PT_IDX(virt)];

    // Abort if table not mapped
    if (!(*pte & PAGING_ENTRY_FLAG_PRESENT)) {
        return;
    }

    // Unmap page
    *pte = 0;

    // Decremement entry count and remove table if empty
    uint64_t ptec = paging_decrement_subcount(pde);
    if (!ptec) {
        palloc_free(pt, 1);
        if (unmap_tables) {
            paging_unmap(NULL, pt, false);
        }

        // Remove table from page directory
        *pde = 0;

        // Decremement entry count and remove table if empty
        uint64_t pdec = paging_decrement_subcount(pdpe);
        if (!pdec) {
            palloc_free(pd, 1);
            if (unmap_tables) {
                paging_unmap(NULL, pd, false);
            }

            // Remove page directory from page directory pointer
            *pdpe = 0;

            // Decremement entry count and remove table if empty
            uint64_t pdpec = paging_decrement_subcount(pml4);
            if (!pdpec) {
                palloc_free(pdp, 1);
                if (unmap_tables) {
                    paging_unmap(NULL, pdp, false);
                }

                // Remove page directory pointer from PML4
                *pml4e = 0;
            }
        }
    }

    paging_invalidate_tlb(virt);

    return;
}

void paging_unmap_multiple(page_entry_t* pml4, uint64_t virt, uint64_t count, bool unmap_tables) {
    for (uint64_t i = 0; i < count; i++) {
        paging_unmap(pml4, virt, unmap_tables);
        virt += 4096;
    }
}

int paging_get_mapping(page_entry_t* pml4, uint64_t virt, uint64_t* phy, uint64_t* flags) {
    // Use fractal mapping to get current table if no PML4 given
    if (!pml4) {
        // Get PML4 entry and PDP address
        pml4 = paging_fractal_access(virt, PAGING_LEVEL_PML4);
        page_entry_t pml4e = pml4[PAGING_PML4_IDX(virt)];
        if (!(pml4e & PAGING_ENTRY_FLAG_PRESENT)) { return -1; }

        // Get PDP entry and PD address
        page_entry_t* pdp = paging_fractal_access(virt, PAGING_LEVEL_PDP);
        page_entry_t pdpe = pdp[PAGING_PDP_IDX(virt)];
        if (!(pdpe & PAGING_ENTRY_FLAG_PRESENT)) { return -1; }

        // Get PD entry and PT address
        page_entry_t* pd = paging_fractal_access(virt, PAGING_LEVEL_PD);
        page_entry_t pde = pd[PAGING_PD_IDX(virt)];
        if (!(pde & PAGING_ENTRY_FLAG_PRESENT)) { return -1; }

        // Get PT entry and physical address
        page_entry_t* pt = paging_fractal_access(virt, PAGING_LEVEL_PT);
        page_entry_t pte = pt[PAGING_PT_IDX(virt)];
        if (!(pte & PAGING_ENTRY_FLAG_PRESENT)) { return -1; }
        
        // TODO: Return info
    }
    
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
    
    // TODO: Return info
}

void paging_invalidate_tlb(uint64_t virt) {
    asm volatile("invlpg (%0)" ::"r" (virt) : "memory");
}

void paging_enable() {
    asm_paging_load_cr3(&k_pml4[0]);
}