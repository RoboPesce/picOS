#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "st7789_driver.h"

#define SPI_FREQUENCY_MH 30 // 10 is definitely stable, but somewhat slow. 30 seems to be rather stable. I think the pi pico maxes out at 40
#define POST_COMMAND_REST_MS 20 // Minimum: 5 ms. Raise in case of instability

#define DATA_PIN_BASE 0 // Starting GPIO pin for data pins (GP0-GP7)
#define DATA_PIN_COUNT 8 // Number of data pins
#define BL_PIN 8 // Backlight control pin
#define RST_PIN 9 // Reset pin
#define  DC_PIN 10 // Data/Command pin
#define  CS_PIN 11 // Chip Select pin

#define SCL_PIN 18 // SPI0 SCK
#define SDA_PIN 19 // SPI0 TX
#define RST_PIN 20
#define  DC_PIN 21
#define  CS_PIN 22

#define FRAMEBUFFER_SIZE_BYTES (NUM_PIXELS * 2)

uint16_t framebuffer[NUM_PIXELS]; // For 16-bit RGB565

void st7789_init() 
{
    // Initialize SPI
    spi_init(spi0, SPI_FREQUENCY_MH * 1000 * 1000);

    gpio_set_function(SCL_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SDA_PIN, GPIO_FUNC_SPI);

    // Set up RST, DC, and CS as GPIO outputs
    gpio_init(RST_PIN);
    gpio_set_dir(RST_PIN, GPIO_OUT);
    gpio_init(DC_PIN);
    gpio_set_dir(DC_PIN, GPIO_OUT);
    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_put(CS_PIN, 1); // Deassert CS initially (note that it's active low)
    
    // Reset sequence
    gpio_put(RST_PIN, 0);
    sleep_ms(50);
    gpio_put(RST_PIN, 1);
    sleep_ms(50);

    write_command(0x01); // Software reset
    sleep_ms(POST_COMMAND_REST_MS);

    write_command(0x11); // Sleep out (exit low power mode)
    sleep_ms(POST_COMMAND_REST_MS);

    write_command(0x29); // Display on

    write_command(0x3A); // COLMOD
    write_data_single(0x05); // Use 16 bit RGB565 format

    write_command(0x21); // INVON, for testing

    // No need to configure CASET/RASET as they default to the whole screen.
    // Future todo: only draw updated surface?

    // No need to configure MADCTL, defaults are sufficient
    write_command(0x36); // MADCTL
    write_data_single(0x00); // Bit 3 = 0: RGB order
}