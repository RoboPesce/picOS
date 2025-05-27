#include "pico/stdlib.h"
#include "st7789_driver.h"

int main()
{
    st7789_init();

    clear_framebuffer(0, 0, 0);

    size_t shift = 0;
    
    while (true)
    {
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