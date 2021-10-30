#pragma once
#include <stdint.h>
#include <stdbool.h>

#define PAGING_ENTRY_FLAG_PRESENT       (1 << 0)
#define PAGING_ENTRY_FLAG_RW            (1 << 1)
#define PAGING_ENTRY_FLAG_USER          (1 << 2)
#define PAGING_ENTRY_FLAG_WRITETHROUGH  (1 << 3)
#define PAGING_ENTRY_FLAG_NO_CACHE      (1 << 4)
#define PAGING_ENTRY_FLAG_ACCESSED      (1 << 5)
#define PAGING_ENTRY_FLAG_DIRTY         (1 << 6)
#define PAGING_ENTRY_FLAG_GLOBAL        (1 << 8)
#define PAGING_ENTRY_FLAG_NX            (1 << 63)

#define PAGING_ENTRY_ADDR_MASK  0xFFFFFFFFF000

#define PAGING_ENTRY_COUNT      512
#define PAGING_TABLE_SIZE       (PAGING_ENTRY_COUNT * sizeof(page_entry_t))

#define PAGING_PT_IDX(addr)     (((addr) >> 12) & 0b111111111)
#define PAGING_PD_IDX(addr)     (((addr) >> 21) & 0b111111111)
#define PAGING_PDP_IDX(addr)    (((addr) >> 30) & 0b111111111)
#define PAGING_PML4_IDX(addr)   (((addr) >> 39) & 0b111111111)

typedef uint64_t page_entry_t;

extern page_entry_t k_pml4[PAGING_ENTRY_COUNT];

enum paging_level {
    PAGING_LEVEL_PT,
    PAGING_LEVEL_PD,
    PAGING_LEVEL_PDP,
    PAGING_LEVEL_PML4
};
typedef enum paging_level paging_level_t;

void paging_init();
void paging_init_pml4(page_entry_t* pml4);
page_entry_t* paging_fractal_access(uint64_t virt, paging_level_t level);
uint64_t paging_increment_subcount(page_entry_t* entry);
uint64_t paging_decrement_subcount(page_entry_t* entry);
uint64_t paging_get_subcount(page_entry_t* entry);
uint64_t paging_set_subcount(page_entry_t* entry, uint64_t subcount);
uint64_t paging_area_size(uint64_t base, uint64_t size);
void paging_map(page_entry_t* pml4, uint64_t virt, uint64_t phy, uint64_t flags, bool map_tables);
void paging_map_multiple(page_entry_t* pml4, uint64_t virt, uint64_t phy, uint64_t flags, uint64_t count, bool map_tables);
void paging_unmap(page_entry_t* pml4, uint64_t virt, bool unmap_tables);
void paging_unmap_multiple(page_entry_t* pml4, uint64_t virt, uint64_t count, bool unmap_tables);
int paging_get_mapping(page_entry_t* pml4, uint64_t virt, uint64_t* phy, uint64_t* flags);
void paging_invalidate_tlb(uint64_t virt);
void paging_enable();

void asm_paging_load_cr3(uint64_t cr3);