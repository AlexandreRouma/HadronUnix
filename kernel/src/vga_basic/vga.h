#pragma once
#include <stdint.h>
#include <string.h>
#include <stdbool.h> 
#include <sys/io.h>

void vga_init(uint8_t w, uint8_t h);
void vga_set_color(char color);
void vga_putcar(char c, uint8_t x, uint8_t y);
int vga_print(char* str);
void vga_println(char* str);
void vga_set_cursor(uint8_t x, uint8_t y);
void vga_clear();
void vga_scroll_up(uint8_t n);
void vga_scroll_down(uint8_t n);
void vga_new_line();
void vga_show_cursor(uint8_t thickness);
void vga_hide_cursor();
void vga_ok(bool newLine);
void vga_warn(bool newLine);
void vga_failed(bool newLine);
uint8_t vga_get_width();
uint8_t vga_get_height();
uint8_t vga_get_cursor_x();
uint8_t vga_get_cursor_y();