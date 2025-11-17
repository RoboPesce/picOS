#include <stdio.h>
#include "pico/stdlib.h"
#include "drivers/st7789_8080_driver.h"
#include "drivers/test/button_driver.h"

#define ENABLE_HEARTBEAT
//#define DO_ANIMATION
#define DO_STATIC_COLOR
//#define WATCHDOG_RESTART

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
    #define HEARTBEAT_INTERVAL_MS 2500
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    uint32_t last_toggle = to_ms_since_boot(get_absolute_time());
    static bool led_on = true;
    #endif

    uint32_t last_drew_fb = to_ms_since_boot(get_absolute_time());
    uint32_t last_changed = to_ms_since_boot(get_absolute_time());
    #ifdef WATCHDOG_RESTART
    uint32_t last_reset   = to_ms_since_boot(get_absolute_time());
    #endif
    
    while (true)
    {
        //printf("main loop\n");
        // Draw animation

        #ifdef DO_ANIMATION
        static uint8_t color_shift = 0;
        //clear_framebuffer(color_shift, color_shift + 100, color_shift + 200);
        color_shift++;

        uint16_t row = 0; 
        uint16_t col = 0;

        for (; row < 320; ++row)
        {
            for (; col < 240; ++col)
            {
                uint8_t r = ((row + color_shift) % 320) * 256 / 320;
                uint8_t g = ((row + col + color_shift) % (320 + 240)) * 256 / (320 + 240);
                uint8_t b = ((col + color_shift) % 240) * 256 / 240;

                write_pixel_to_framebuffer(row, col, r, g, b);
            }
        }
        #elif defined(DO_STATIC_COLOR)
        static Color color = {0, 255, 0}; 
        clear_framebuffer(color.r, color.g, color.b);
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

        uint32_t now = to_ms_since_boot(get_absolute_time());
        
        #if defined(DO_ANIMATION) || defined(DO_STATIC_COLOR)
        #define framerate 10
        if (now - last_drew_fb >= (1.0 / framerate) * 1000)
        {
            draw_framebuffer();
            last_drew_fb = now;
        }
        #endif

        #ifdef ENABLE_HEARTBEAT        
        if (now - last_toggle >= HEARTBEAT_INTERVAL_MS)
        {
            led_on = !led_on;
            gpio_put(LED_PIN, led_on);
            last_toggle = now;
        }
        #define CHANGE_COLOR_EVERY_MS 100
        if (now - last_changed >= CHANGE_COLOR_EVERY_MS)
        {
            color.r += 17;
            color.g -= 23;
            color.b += 31;
            last_changed = now;
        }
        #endif
        #ifdef WATCHDOG_RESTART
        if (now - last_reset >= 5000)
        {
            st7789_8080_init();
            last_reset = now;
        }
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