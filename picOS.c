#include "pico/stdlib.h"
#include <stdio.h>
#include "drivers/st7789_8080_driver.h"
#include "drivers/test/button_driver.h"
#include "drivers/test/led_driver.h"

#include "led_blink.pio.h"

InputState mouse_position;

int main()
{
    stdio_init_all();

    # define LED_PIN 25
    PIO pio = pio0;
    uint sm = 0;

    // Load program into PIO memory
    uint offset = pio_add_program(pio, &led_blink_program);

    // Get default config
    pio_sm_config c = led_blink_program_get_default_config(offset);

    // Configure pins
    sm_config_set_out_pins(&c, LED_PIN, 1);
    pio_gpio_init(pio, LED_PIN);
    pio_sm_set_consecutive_pindirs(pio, sm, LED_PIN, 1, true); // output
    //sm_config_set_clkdiv(&c, 500000.0f);
    sm_config_set_out_shift(&c, false, false, 32);

    // Initialize and start state machine
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);

    while (1) {        
        pio_sm_put_blocking(pio, sm, 0xFFFFFFFF);
        sleep_ms(500);
        pio_sm_put_blocking(pio, sm, 0x00000000);
        sleep_ms(500);
        printf("toggled led");
    }

    //button_init();
    //st7789_8080_init();
    led_driver_init();

    //mouse_position.vertical = NUM_ROWS / 2;
    //mouse_position.horizontal = NUM_COLS / 2;

    //uint8_t color_shift = 0;
    
    while (true)
    {
        // Draw animation

        //clear_framebuffer(color_shift, color_shift + 100, color_shift + 200);
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

        //draw_framebuffer();

        //color_shift++;

        //static uint8_t counter = 0;
        //if ((counter += 7) % 9 == 0) printf("Colorshift: %d\n", color_shift);

        toggle_led();
        sleep_ms(500);
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