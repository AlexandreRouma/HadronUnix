#!/bin/sh

set -e

# Save arguments
OUTPUT_IMG=$1
STAGE1_BIN=$2
STAGE2_BIN=$3
KERNEL_ELF=$4

# Config
BOOTFS_IMG=/tmp/hadron_bootfs.img
BOOTFS_SIZE=1024

# Get needed variables
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

# Create bootfs
dd if=/dev/zero of=$BOOTFS_IMG bs=512 count=$BOOTFS_SIZE status=none
mkfs.vfat -F32 $BOOTFS_IMG
mcopy -i $BOOTFS_IMG $KERNEL_ELF ::/KERNEL

# Calculate sizes
STAGE2_BIN_SIZE_BYTES=$(stat $STAGE2_BIN -c %s)
STAGE2_BIN_SIZE_SECTORS=$(((($STAGE2_BIN_SIZE_BYTES - 1) / 512) + 1))

BOOTFS_IMG_SIZE_BYTES=$(stat $BOOTFS_IMG -c %s)
BOOTFS_IMG_SIZE_SECTORS=$(((($BOOTFS_IMG_SIZE_BYTES - 1) / 512) + 1))
BOOTFS_IMG_OFFSET=$((1 + $STAGE2_BIN_SIZE_SECTORS))

OUTPUT_IMG_SIZE_SECTORS=$((1 + $STAGE2_BIN_SIZE_SECTORS + $BOOTFS_IMG_SIZE_SECTORS))

# Include utilities
. $SCRIPT_DIR/binary_utils.sh

# Create blank image
dd if=/dev/zero of=$OUTPUT_IMG bs=512 count=$OUTPUT_IMG_SIZE_SECTORS status=none

# Write boot sector
dd if=$STAGE1_BIN of=$OUTPUT_IMG conv=notrunc bs=512 count=1 status=none

# Write stage 2
dd if=$STAGE2_BIN of=$OUTPUT_IMG conv=notrunc bs=512 seek=1 status=none

# Write bootfs
dd if=$BOOTFS_IMG of=$OUTPUT_IMG conv=notrunc bs=512 seek=$BOOTFS_IMG_OFFSET status=none

# # Write partition table (TODO: make it an actual MBR and not a joke)
write_u32 $BOOTFS_IMG_OFFSET $((0x1BE + 0x08)) $OUTPUT_IMG

# write_u8 $((128)) $((0x1BE + 0x0)) $OUTPUT_IMG
# write_u8 $((0)) $((0x1BE + 0x1)) $OUTPUT_IMG
# write_u8 $((0)) $((0x1BE + 0x2)) $OUTPUT_IMG
# write_u8 $((1)) $((0x1BE + 0x3)) $OUTPUT_IMG
# write_u8 $((0)) $((0x1BE + 0x4)) $OUTPUT_IMG
# write_u8 $((0)) $((0x1BE + 0x5)) $OUTPUT_IMG
# write_u8 $((0)) $((0x1BE + 0x6)) $OUTPUT_IMG
# write_u8 $((3)) $((0x1BE + 0x7)) $OUTPUT_IMG
# write_u32 $((1)) $((0x1BE + 0x8)) $OUTPUT_IMG
# write_u32 $((3)) $((0x1BE + 0xC)) $OUTPUT_IMG