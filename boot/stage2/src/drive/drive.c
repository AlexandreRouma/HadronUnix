#include <drive/drive.h>
#include <realmode/realmode.h>
#include <vga_basic/vga.h>

int drive_get_info(drive_info_t* info, uint8_t id) {
    // Call BIOS with the drive ID
    uint32_t ret = realmode_call(asm_drive_get_info, 0, 0, 0, id);

    // Save the rest of the info and return with no error
    info->drive_id = id;
    info->head_count = (ret >> 8) & 0xFF;
    info->sector_per_track = ret & 0xFF;

    return (ret < 256) ? ret : 0;
}

void drive_lba_to_chs(drive_info_t* info, uint32_t lba, uint16_t* cylinder, uint8_t* head, uint8_t* sector) {
    uint32_t total_tracks = lba / info->sector_per_track;
    *sector = (lba % info->sector_per_track) + 1;
    *head = total_tracks % info->head_count;
    *cylinder = total_tracks / info->head_count;
}

int drive_read_sectors(drive_info_t* info, uint32_t lba, uint32_t count, uint8_t* buf) {
    uint32_t read = 0;
    uint16_t cyl;
    uint8_t head, sect;
    
    // Calculate ES (scratch buffer is aligned to 16bits so no BX needed)
    uint32_t es = (uint32_t)drive_scratch_buffer / 16;

    while (count) {
        // Get CHS address of the read operation
        drive_lba_to_chs(info, lba + read, &cyl, &head, &sect);

        // Calculate number of sectors to next cylinder
        uint32_t next_cyl_lba = (cyl + 1) * info->head_count * info->sector_per_track;
        uint32_t max_cyl_read = next_cyl_lba - (lba + read);

        // Calculate the number of bytes to read this time
        uint32_t to_read = DRIVE_BIOS_MAX_SECTORS;
        if (max_cyl_read < to_read) { to_read = max_cyl_read; }
        if (count < to_read) { to_read = count; }

        // Call BIOS
        uint32_t ecx = ((cyl & 0xFF) << 8) | ((cyl >> 2) & 0xC0) | (sect & 0b111111);
        uint32_t edx = (head << 8) | info->drive_id;
        uint32_t ret = realmode_call(asm_drive_read_sectors, to_read, es << 16, ecx, edx);
        if (ret) { return ret; }

        // Copy over to the actual buffer
        memcpy(&buf[read * DRIVE_BIOS_SECTOR_SIZE], drive_scratch_buffer, to_read * DRIVE_BIOS_SECTOR_SIZE);

        // Increment counters
        read += to_read;
        count -= to_read;
        es += 32;
    }

    return 0;
}