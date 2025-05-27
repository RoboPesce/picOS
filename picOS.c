#include "pico/stdlib.h"
#include "drivers/st7789_driver.h"
#include "drivers/button_driver.h"

InputState mouse_position;

int main()
{
    button_init();
    st7789_init();

    mouse_position.vertical = NUM_ROWS / 2;
    mouse_position.horizontal = NUM_COLS / 2;

    clear_framebuffer(0, 0, 0);

    size_t shift = 0;
    
    while (true)
    {
        // Draw animation

        for (size_t row = 0; row < NUM_ROWS; row++)
        {
            for (size_t col = 0; col < NUM_COLS; col++)
            {
                size_t shift_row = (row + shift) % NUM_ROWS;
                size_t shift_col = (col + shift) % NUM_COLS;

                uint8_t r = (shift_row * 255) / (NUM_ROWS - 1);
                uint8_t g = (shift_col * 255) / (NUM_COLS - 1);
                uint8_t b = ((shift_row + shift_col) * 255) / (NUM_ROWS + NUM_COLS - 2);
                write_pixel_to_framebuffer(row, col, r, g, b);
            }
        }

        // Move mouse

        InputState input = get_input_state();
        mouse_position.horizontal += input.horizontal;
        mouse_position.vertical += input.vertical;
        if (mouse_position.vertical >= NUM_ROWS) mouse_position.vertical = NUM_ROWS - 1;
        else if (mouse_position.vertical < 0) mouse_position.vertical = 0;
        if (mouse_position.horizontal >= NUM_COLS) mouse_position.horizontal = NUM_COLS - 1;
        else if (mouse_position.horizontal < 0) mouse_position.horizontal = 0;

        write_pixel_to_framebuffer(mouse_position.vertical, mouse_position.horizontal, 0, 0, 0);

        draw_framebuffer();

        shift += 5;
    }

    /*
    uint16_t row = 0; 
    uint16_t col = 0;

    while (true)
    {
        write_pixel_to_framebuffer(row, col, 0, 0, 0);

        row++;
        if (row >= 320)
        {
            row = 0;
            col++;
            if (col >= 240) col = 0;
        }

        draw_framebuffer();
    }
    */
}