#include "pico/stdlib.h"
#include <stdio.h>
#include "drivers/st7789_8080_driver.h"
#include "drivers/test/button_driver.h"

InputState mouse_position;

int main()
{
    stdio_init_all();
    //button_init();
    st7789_8080_init();

    mouse_position.vertical = NUM_ROWS / 2;
    mouse_position.horizontal = NUM_COLS / 2;

    uint8_t color_shift = 0;

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    gpio_put(25, 1);
    int led_on = 1;
    
    while (true)
    {
        // Draw animation

        clear_framebuffer(color_shift, color_shift + 100, color_shift + 200);
        //clear_framebuffer(255, 255, 255);

        // Move mouse
        /*
        InputState input = get_input_state();
        mouse_position.horizontal += input.horizontal;
        mouse_position.vertical += input.vertical;
        if (mouse_position.vertical >= NUM_ROWS) mouse_position.vertical = NUM_ROWS - 1;
        else if (mouse_position.vertical < 0) mouse_position.vertical = 0;
        if (mouse_position.horizontal >= NUM_COLS) mouse_position.horizontal = NUM_COLS - 1;
        else if (mouse_position.horizontal < 0) mouse_position.horizontal = 0;

        write_pixel_to_framebuffer(mouse_position.vertical, mouse_position.horizontal, 0, 0, 0);
        */

        draw_framebuffer();

        color_shift++;

        //static uint8_t counter = 0;
        //if ((counter += 7) % 9 == 0) printf("Colorshift: %d\n", color_shift);

        /*
        led_on = led_on ? 0 : 1;
        gpio_put(25, led_on);
        sleep_ms(500);
        */
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