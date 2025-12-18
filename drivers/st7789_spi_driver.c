#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "st7789_spi_driver.h"

#define SPI_FREQUENCY_MH 30 // 10 is definitely stable, but somewhat slow. 30 seems to be rather stable. I think the pi pico maxes out at 40
#define POST_COMMAND_REST_MS 20 // Minimum: 5 ms. Raise in case of instability

#define SCL_PIN 18 // SPI0 SCK
#define SDA_PIN 19 // SPI0 TX
#define RST_PIN 20
#define  DC_PIN 21
#define  CS_PIN 22

#define FRAMEBUFFER_SIZE_BYTES (NUM_PIXELS * 2)

uint16_t framebuffer[NUM_PIXELS]; // For 16-bit RGB565

uint16_t rgb888_to_565(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    uint16_t endian = ((color >> 8) & 0x00FF) | ((color << 8) & 0xFF00); // The masks should be redundant, but we want to be sure
    return endian;
}

void write_command(uint8_t cmd)
{
    gpio_put(DC_PIN, 0); // Command mode
    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi0, &cmd, 1);
    gpio_put(CS_PIN, 1);
}

void write_data_single(uint8_t data)
{
    gpio_put(DC_PIN, 1); // Data mode
    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi0, &data, 1);
    gpio_put(CS_PIN, 1);
}

void write_data(const uint8_t *data, size_t len)
{
    gpio_put(DC_PIN, 1); // Data mode
    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi0, data, len);
    gpio_put(CS_PIN, 1);
}

void st7789_spi_init() 
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

void draw_framebuffer()
{
    write_command(0x2C); // RAMWR
    write_data((const uint8_t *)framebuffer, FRAMEBUFFER_SIZE_BYTES);
}

void write_pixel_to_framebuffer(uint16_t row, uint16_t col, uint8_t r, uint8_t g, uint8_t b)
{
    size_t index = NUM_COLS * row + col;
    framebuffer[index] = rgb888_to_565(r, g, b);
}

void clear_framebuffer(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t clear_color = rgb888_to_565(r, g, b);
    for (size_t i = 0; i < NUM_PIXELS; i++) framebuffer[i] = clear_color;
}