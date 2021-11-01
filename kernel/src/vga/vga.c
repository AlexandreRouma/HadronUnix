#include "vga.h"

uint8_t vga_text_color = 0x0F;
uint16_t* vga_frame_buffer = (uint16_t*)0xB8000;
uint8_t vga_cursor_x = 0;
uint8_t vga_cursor_y = 0;
uint8_t vga_terminal_width = 0;
uint8_t vga_terminal_height = 0;
uint8_t vga_last_clear_color = 0x00;

void vga_init(uint8_t w, uint8_t h) {
    vga_terminal_width = w;
    vga_terminal_height = h;
    vga_clear();
}

void vga_set_color(char color) {
    vga_text_color = color;
}

void vga_putcar(char c, uint8_t x, uint8_t y) {
    vga_frame_buffer[x + (vga_terminal_width * y)] = c | (vga_text_color << 8);
}

int vga_print(char* str) {
    size_t len = strlen(str);
    for (uint32_t i = 0; i < len; i++) {
        if (str[i] == '\n') {
            vga_new_line();
        }
        else if (str[i] == '\r') {
            vga_cursor_x = 0;
        }
        else if (str[i] == '\f') {
            vga_clear();
        }
        else if (str[i] == '\b' && vga_cursor_x > 0) {
            vga_cursor_x--;
            vga_print(" ");
        }
        else if (vga_cursor_x == vga_terminal_width) {
            vga_new_line();
        }
        else {
            vga_frame_buffer[vga_cursor_x + (vga_terminal_width * vga_cursor_y)] = str[i] | (vga_text_color << 8);
            vga_cursor_x++;
        }
    }
    vga_set_cursor(vga_cursor_x, vga_cursor_y);
    return len;
}

void vga_set_cursor(uint8_t x, uint8_t y)  {
    vga_frame_buffer[x + (vga_terminal_width * y)] = (vga_frame_buffer[x + (vga_terminal_width * y)] & 0x00FF) | (vga_text_color << 8);
    vga_cursor_x = x;
    vga_cursor_y = y;
    uint16_t pos = x + (vga_terminal_width * y);
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void vga_clear() {
    for (uint8_t y = 0; y < vga_terminal_height; y++) {
        for (uint8_t x = 0; x < vga_terminal_width; x++) {
            vga_frame_buffer[x + (vga_terminal_width * y)] = ' ' | (vga_text_color << 8);
        }
    }
    vga_cursor_x = 0;
    vga_cursor_y = 0;
    vga_set_cursor(0, 0);
}

void vga_scroll_up(uint8_t n) {
    for (uint8_t i = 0; i < n; i++) {
        memcpy(&vga_frame_buffer[0], &vga_frame_buffer[vga_terminal_width], vga_terminal_width * (vga_terminal_height - 1) * 2);
        for (uint8_t x = 0; x < vga_terminal_width; x++) {
            vga_frame_buffer[x + ((vga_terminal_height - 1) * vga_terminal_width)] = ' ' | (vga_text_color << 8);
        }
    }
}

void vga_scroll_down(uint8_t n) {
    for (int i = 0; i < n; i++) {
        memcpy(&vga_frame_buffer[vga_terminal_width], &vga_frame_buffer[0], (vga_terminal_width * (vga_terminal_height - 1)) * 2);
        for (int x = 0; x < vga_terminal_width; x++) {
            vga_frame_buffer[x] = ' ' | (vga_text_color << 8);
        }
    }
}

void vga_new_line() {
    if (vga_cursor_y == 24) {
        vga_scroll_up(1);
        vga_cursor_x = 0;
    }
    else {
        vga_cursor_y++;
        vga_cursor_x = 0;
    }
}

void vga_show_cursor(uint8_t thickness) {

    outb(0x3D4, 0x09);
    uint8_t end = inb(0x3D5) & 0b00011111; // Get the max scanline
    uint8_t start = end - thickness;

    outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | start);
	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | end);
}

void vga_cursor() {
    outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

uint8_t vga_get_width() {
    return vga_terminal_width;
}

uint8_t vga_get_height() {
    return vga_terminal_height;
}

uint8_t vga_get_vga_cursor_x() {
    return vga_cursor_x;
}

uint8_t vga_get_vga_cursor_y() {
    return vga_cursor_y;
}