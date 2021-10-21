#!/bin/sh

# Get needed variables
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

# Calculate sizes
STAGE2_BIN_SIZE_BYTES=$(stat $1 -c %s)

# Include utilities
. $SCRIPT_DIR/binary_utils.sh

write_u32 $STAGE2_BIN_SIZE_BYTES 4 $1