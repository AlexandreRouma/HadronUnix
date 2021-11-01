    kprintf("\nW E   B E   P A G I N '\n\n");

    // for (uint64_t i = 0; i < k_mem_map.entry_count; i++) {
    //     memmap_entry_t ent = k_mem_map.map[i];

    //     char type[16];
    //     if (ent.type == MEMMAP_REGION_TYPE_DEAD) {
    //         sprintf(type, "DEAD");
    //     }
    //     else if (ent.type == MEMMAP_REGION_TYPE_RESERVED) {
    //         sprintf(type, "RESERVED");
    //     }
    //     else if (ent.type == MEMMAP_REGION_TYPE_HARDWARE) {
    //         sprintf(type, "HARDWARE");
    //     }
    //     else if (ent.type == MEMMAP_REGION_TYPE_BIOS) {
    //         sprintf(type, "BIOS");
    //     }
    //     else if (ent.type == MEMMAP_REGION_TYPE_ACPI) {
    //         sprintf(type, "ACPI");
    //     }
    //     else if (ent.type == MEMMAP_REGION_TYPE_SOFTWARE) {
    //         sprintf(type, "SOFTWARE");
    //     }
    //     else if (ent.type == MEMMAP_REGION_TYPE_FREE) {
    //         sprintf(type, "FREE");
    //     }
    //     else if (ent.type == MEMMAP_REGION_TYPE_ALLOCATABLE) {
    //         sprintf(type, "ALLOCATABLE");
    //     }
    //     else {
    //         sprintf(type, "%08x", ent.type);
    //     }

    //     kprintf("[%02x]: BASE=%#016x SIZE=%#016x TYPE=%s\n", i, ent.base, ent.size, type);
    // }