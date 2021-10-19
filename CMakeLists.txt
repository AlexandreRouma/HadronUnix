cmake_minimum_required(VERSION 3.13)
project(hadron NONE)

add_subdirectory("boot")

add_custom_command(OUTPUT hadron.img
    COMMAND sh ../tools/create_boot_image.sh hadron.img boot/stage1/stage1.bin boot/stage2/stage2.bin
    DEPENDS stage1 stage2
    COMMENT "Creating Hadron binary image"
)
add_custom_target(hadron ALL DEPENDS hadron.img)