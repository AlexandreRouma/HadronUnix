NASM=nasm
NASMFLAGS=-f elf

CC=i686-elf-gcc
CFLAGS=-ffreestanding -fno-exceptions -Wall -Wextra -Isrc -g -Og

LD=i686-elf-ld
LDFLAGS=-Tlinker.ld -nostdlib

OBJCOPY=i686-elf-objcopy
OBJCOPYFLAGS=-O binary

BUILD=../../build/boot/stage2

SRC_C=$(shell find src -name '*.c')
SRC_ASM=$(shell find src -name '*.asm')

OBJ_C=$(addprefix $(BUILD)/, $(addsuffix .o,$(SRC_C)))
OBJ_ASM=$(addprefix $(BUILD)/, $(addsuffix .o,$(SRC_ASM)))

all: $(BUILD)/stage2.bin

.PHONY: clean

$(BUILD)/stage2.bin: $(BUILD)/stage2.elf
	$(OBJCOPY) $(OBJCOPYFLAGS) $^ $@ && sh ../../tools/fill_stage2_header.sh $(BUILD)/stage2.bin

$(BUILD)/stage2.elf: $(OBJ_C) $(OBJ_ASM)
	$(LD) $(LDFLAGS) $^ -o $@

$(BUILD)/%.asm.o: %.asm
	mkdir -p $(@D) && $(NASM) $(NASMFLAGS) $^ -o $@

$(BUILD)/%.c.o: %.c
	mkdir -p $(@D) && $(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -rf $(BUILD)