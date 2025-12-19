#pragma once

#include "structs/color.h"

#define NUM_ROWS 320
#define NUM_COLS 240
#define NUM_PIXELS (NUM_ROWS * NUM_COLS)

void st7789_init();

// Sends the framebuffer to the screen. Use cautiously
void draw_framebuffer();

void clear_framebuffer(uint8_t r, uint8_t g, uint8_t b);

void write_pixel_to_framebuffer(uint16_t row, uint16_t col, uint8_t r, uint8_t g, uint8_t b);