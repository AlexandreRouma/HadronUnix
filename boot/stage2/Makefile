NASM=nasm
CC=i686-elf-gcc
CFLAGS=-ffreestanding -fno-exceptions -Wall -Wextra -I./
LD=i686-elf-ld
OBJCOPY=i686-elf-objcopy
BUILD=../../build/boot/stage2

.PHONY: build_dir clean

all: $(BUILD)/stage2.bin

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/init.o: init.asm
	$(NASM) $< -f elf -o $@

$(BUILD)/main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/stage2.elf: $(BUILD)/init.o $(BUILD)/main.o
	$(LD) -Tlinker.ld $^ -o $@

$(BUILD)/stage2.bin: $(BUILD)/stage2.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f $(BUILD)/*