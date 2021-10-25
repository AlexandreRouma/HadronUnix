BUILD=build

all: hadron.img

.PHONY: clean boot kernel

hadron.img: boot kernel
	$(SHELL) tools/create_boot_image.sh $(BUILD)/hadron.img $(BUILD)/boot/stage1/stage1.bin $(BUILD)/boot/stage2/stage2.bin $(BUILD)/kernel/kernel

boot:
	$(MAKE) -C boot

kernel:
	$(MAKE) -C kernel

clean:
	$(MAKE) -C boot clean
	$(MAKE) -C kernel clean
	rm -f $(BUILD)/hadron.img