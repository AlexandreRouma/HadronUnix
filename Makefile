BUILD=build

all: hadron.img

.PHONY: clean boot kernel initrd

hadron.img: boot kernel boot.ini initrd
	$(SHELL) tools/create_boot_image.sh $(BUILD)/hadron.img $(BUILD)/boot/stage1/stage1.bin $(BUILD)/boot/stage2/stage2.bin $(BUILD)/kernel/kernel boot.ini $(BUILD)/initrd

boot:
	$(MAKE) -C boot

kernel:
	$(MAKE) -C kernel

hddimage: hadron.img
	rm -rf $(BUILD)/hdd.img
	dd if=/dev/zero of=$(BUILD)/hdd.img bs=512 count=147456
	dd if=build/hadron.img of=$(BUILD)/hdd.img conv=notrunc

initrd:
	cd initrd && tar -cvf ../$(BUILD)/initrd *

clean:
	$(MAKE) -C boot clean
	$(MAKE) -C kernel clean
	rm -f $(BUILD)/hadron.img