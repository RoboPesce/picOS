#include <stdio.h>
#include "pico/stdlib.h"
#include "drivers/st7789_8080_driver.h"
#include "drivers/test/button_driver.h"

#define ENABLE_HEARTBEAT
#define DO_ANIMATION

InputState mouse_position;

int main()
{
    stdio_init_all();

    //button_init();
    st7789_8080_init();
    //printf("st7789_8080_init called\n");

    //mouse_position.vertical = NUM_ROWS / 2;
    //mouse_position.horizontal = NUM_COLS / 2;

    #ifdef ENABLE_HEARTBEAT
    #define LED_PIN 25
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    #endif
    
    while (true)
    {
        //printf("main loop\n");
        // Draw animation

        #ifdef DO_ANIMATION
        static uint8_t color_shift = 0;
        clear_framebuffer(color_shift, color_shift + 100, color_shift + 200);
        color_shift++;
        //clear_framebuffer(255, 255, 255);
        #endif

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

        #ifdef DO_ANIMATION
        draw_framebuffer();
        #endif

        //static uint8_t counter = 0;
        //if ((counter += 7) % 9 == 0) printf("Colorshift: %d\n", color_shift);

        #ifdef ENABLE_HEARTBEAT
        static bool led_on = true;
        gpio_put(LED_PIN, led_on);
        led_on = !led_on;
        sleep_ms(2500);
        #endif
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