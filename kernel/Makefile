NASM=nasm
NASMFLAGS=-f elf64

CC=x86_64-elf-gcc
CFLAGS=-ffreestanding -fno-exceptions -Wall -Wextra -Isrc -I../common

LD=x86_64-elf-ld
LDFLAGS=-Tlinker.ld -nostdlib -z max-page-size=4096

BUILD=../build/kernel

SRC_C=$(shell find src -name '*.c')
SRC_ASM=$(shell find src -name '*.asm')

OBJ_C=$(addprefix $(BUILD)/, $(addsuffix .o,$(SRC_C)))
OBJ_ASM=$(addprefix $(BUILD)/, $(addsuffix .o,$(SRC_ASM)))

all: $(BUILD)/kernel

.PHONY: clean

$(BUILD)/kernel: $(OBJ_C) $(OBJ_ASM)
	$(LD) $(LDFLAGS) $^ -o $@

$(BUILD)/%.asm.o: %.asm
	mkdir -p $(@D) && $(NASM) $(NASMFLAGS) $^ -o $@

$(BUILD)/%.c.o: %.c
	mkdir -p $(@D) && $(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -rf $(BUILD)