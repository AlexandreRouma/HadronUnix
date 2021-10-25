NASM=nasm
NASMFLAGS=-f bin -Isrc

BUILD=../../build/boot/stage1

SRC_ASM=$(shell find src -name '*.asm')

OBJ_ASM=$(addprefix $(BUILD)/, $(addsuffix .o,$(SRC_ASM)))

all: $(BUILD)/stage1.bin

.PHONY: clean

$(BUILD)/stage1.bin: src/main.asm $(SRC_ASM)
	mkdir -p $(@D) && $(NASM) $(NASMFLAGS) $< -o $@

clean:
	rm -rf $(BUILD)