cmake_minimum_required(VERSION 3.13)
project(hadron NONE)

add_subdirectory("boot")

# TODO: Fix this not being run if stage1.img or stage2.img is changed

add_custom_command(OUTPUT hadron.img
    COMMAND sh ../tools/create_boot_image.sh hadron.img boot/stage1/stage1.bin boot/stage2/stage2.bin
    DEPENDS stage1 stage2
    COMMENT "Creating Hadron binary image"
)
add_custom_target(hadron ALL DEPENDS hadron.img)