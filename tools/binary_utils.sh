#!/bin/sh

print_u8() {
    printf "\\$(printf %o $(($1&0xFF)))"
}

print_u16() {
    printf "\\$(printf %o $(($1&0xFF)))\\$(printf %o $((($1 >> 8)&0xFF)))"
}

print_u32() {
    printf "\\$(printf %o $(($1&0xFF)))\\$(printf %o $((($1 >> 8)&0xFF)))\\$(printf %o $((($1 >> 16)&0xFF)))\\$(printf %o $((($1 >> 24)&0xFF)))"
}

write_u8() {
    print_u8 $1 | dd if=/dev/stdin of=$3 conv=notrunc bs=1 seek=$2 status=none
}

write_u16() {
    print_u16 $1 | dd if=/dev/stdin of=$3 conv=notrunc bs=1 seek=$2 status=none
}

write_u32() {
    print_u32 $1 | dd if=/dev/stdin of=$3 conv=notrunc bs=1 seek=$2 status=none
}