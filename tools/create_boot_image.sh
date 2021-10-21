#!/bin/sh

set -e

# Save arguments
OUTPUT_IMG=$1
STAGE1_BIN=$2
STAGE2_BIN=$3
BOOTFS_IMG=$4

# Get needed variables
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

# Calculate sizes
STAGE2_BIN_SIZE_BYTES=$(stat $STAGE2_BIN -c %s)
STAGE2_BIN_SIZE_SECTORS=$(((($STAGE2_BIN_SIZE_BYTES - 1) / 512) + 1))

OUTPUT_IMG_SIZE_SECTORS=$((1 + $STAGE2_BIN_SIZE_SECTORS))

# Include utilities
. $SCRIPT_DIR/binary_utils.sh

# Create blank image
dd if=/dev/zero of=$OUTPUT_IMG bs=512 count=$OUTPUT_IMG_SIZE_SECTORS status=none

# Write boot sector
dd if=$STAGE1_BIN of=$OUTPUT_IMG conv=notrunc bs=512 count=1 status=none

# Write stage 2
dd if=$STAGE2_BIN of=$OUTPUT_IMG conv=notrunc bs=512 seek=1 status=none

# # Write partition table
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