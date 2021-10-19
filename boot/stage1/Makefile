NASM=nasm
BUILD=../../build/boot/stage1

.PHONY: build_dir clean

all: $(BUILD)/stage1.bin

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/stage1.bin: main.asm drive.asm gdt.asm memory.asm screen.asm strings.asm $(BUILD)
	$(NASM) $< -f bin -o $(BUILD)/stage1.bin

clean:
	rm -f $(BUILD)/*