cmake_minimum_required(VERSION 3.13)

# Avoid CMAKE trying to create an executable
set(CMAKE_SYSTEM_NAME   Generic)

# Set compiler executables
set(CMAKE_C_COMPILER    "i686-elf-gcc")
set(CMAKE_ASM_COMPILER  "nasm")
set(CMAKE_C_LINKER      "i686-elf-ld")

# Setup NASM to build 32bit elf
set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf")

# Set compiler and linker arguments
set(CMAKE_C_FLAGS           "-ffreestanding -fno-exceptions -Wall -Wextra")
set(CMAKE_EXE_LINKER_FLAGS  "-T${CMAKE_CURRENT_LIST_DIR}/linker.ld -ffreestanding -nostdlib")

project(stage2_elf C ASM_NASM)

file(GLOB_RECURSE SRC "src/*.c" "src/*.asm")

add_executable(stage2_elf ${SRC})

add_custom_command(OUTPUT stage2.bin
    COMMAND i686-elf-objcopy -O binary stage2_elf stage2.bin
    DEPENDS stage2_elf
    COMMENT "Creating stage2 binary image"
)
add_custom_target(stage2 ALL DEPENDS stage2.bin)