cmake_minimum_required(VERSION 3.13)

# Avoid CMAKE trying to create an executable
set(CMAKE_SYSTEM_NAME   Generic)

# Set compiler executables
set(CMAKE_ASM_COMPILER      "nasm")
set(CMAKE_ASM_NASM_LINKER   "i686-elf-ld")
set(CMAKE_ASM_NASM_LINK_EXECUTABLE "i686-elf-ld <CMAKE_ASM_NASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")

# Setup NASM to build 32bit elf
set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf")

# Set compiler and linker arguments
set(CMAKE_ASM_NASM_FLAGS           "-I${CMAKE_CURRENT_LIST_DIR}/src")
set(CMAKE_EXE_LINKER_FLAGS  "-T${CMAKE_CURRENT_LIST_DIR}/linker.ld -nostdlib")

project(stage1_elf ASM_NASM)

add_executable(stage1_elf "src/main.asm")

add_custom_command(OUTPUT stage1.bin
    COMMAND i686-elf-objcopy -O binary stage1_elf stage1.bin
    DEPENDS stage1_elf
    COMMENT "Creating stage1 binary image"
)
add_custom_target(stage1 ALL DEPENDS stage1.bin)