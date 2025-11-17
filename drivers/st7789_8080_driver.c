#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/structs/pio.h"
#include "hardware/dma.h"
#include "st7789_8080_driver.h"
#include "st7789_8080.pio.h"

#define PIO_CLOCK_DIVIDER 2.0f
#define POST_COMMAND_REST_MS 20 // Minimum: 5 ms. Raise in case of instability

#define DATA_PIN_BASE 0 // Starting GPIO pin for data pins (GP0-GP7)
#define DATA_PIN_COUNT 8 // Number of data pins
#define DC_PIN 8 // Data/Command pin
#define BL_PIN 9 // Backlight control pin
#define WR_PIN 10 // Write pin
#define CS_PIN 11 // Chip Select pin
#define RST_PIN 12 // Reset pin
// Note: RD pin (read) is not used, tied to VCC

#define FRAMEBUFFER_SIZE_BYTES (NUM_PIXELS * 2)

uint16_t framebuffer[NUM_PIXELS]; // For 16-bit RGB565

PIO pio = pio0;
int sm = 0;
uint dma_chan;

void write_command(uint8_t cmd);
void write_data_single(uint8_t data);
void write_data(const uint8_t *data, size_t len);

uint16_t rgb888_to_565(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    uint16_t endian = ((color >> 8) & 0x00FF) | ((color << 8) & 0xFF00); // The masks should be redundant, but we want to be sure
    return endian;
    //return color;
}

void st7789_8080_init() 
{
    // Initialize PIO
    static bool has_pio_init = false;
    if (!has_pio_init)
    {
        uint offset = pio_add_program(pio, &st7789_8080_program);
        st7789_8080_program_init(pio, sm, offset, DATA_PIN_BASE, DATA_PIN_COUNT + 1 /* Include DC */, WR_PIN, PIO_CLOCK_DIVIDER);
        has_pio_init = true;
    }

    gpio_init(BL_PIN);
    gpio_set_dir(BL_PIN, GPIO_OUT);
    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_init(RST_PIN);
    gpio_set_dir(RST_PIN, GPIO_OUT);

    gpio_put(CS_PIN, 1);
    
    // Reset sequence. Min pulse duration: 10 us
    int reset_pulse_duration = 25;
    gpio_put(RST_PIN, 0);
    sleep_us(reset_pulse_duration);
    gpio_put(RST_PIN, 1);
    sleep_us(reset_pulse_duration * 5);

    // Keep chip select on for remainder of session
    gpio_put(CS_PIN, 0);

    write_command(0x01); // Software reset
    sleep_ms(150); // Initialized to sleep in mode, must sleep 120 ms after resetting in sleep in

    write_command(0x11); // Sleep out (exit low power mode)
    sleep_ms(10); // Must sleep 5 ms after sleep out before sending further commands

    write_command(0x3A); // COLMOD, set color data format
    write_data_single(0x55); // Use 16 bit RGB565 format

    write_command(0x21); // INVON, turn on screen inversion. This may depend on your screen.
                         // If your colors are inverted, remove this command or use 0x20 (INVOFF).

    // No need to configure MADCTL, defaults are sufficient. But we do it just to be safe.
    write_command(0x36); // MADCTL
    write_data_single(0x00); // Bit 3 = 0: RGB order

    // No need to configure CASET/RASET as they default to the whole screen. But we do it just to be safe.
    // Future todo: only draw updated surface?
    write_command(0x2A); // CASET
    write_data_single(0x00);
    write_data_single(0x00);
    write_data_single(0x00);
    write_data_single(0xEF);
    write_command(0x2B); // RASET
    write_data_single(0x00);
    write_data_single(0x00);
    write_data_single(0x01);
    write_data_single(0x3F);

    write_command(0x29); // Display on

    gpio_put(BL_PIN, 1); // Backlight on
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

void write_command(uint8_t cmd)
{
    uint32_t cmd_ext = cmd;
    pio_sm_put_blocking(pio, sm, cmd_ext);
}

void write_data_single(uint8_t data) 
{
    uint32_t data_ext = (1 << 8) | ((uint32_t) data); // Set 9th pin (DC) to 1
    pio_sm_put_blocking(pio, sm, data_ext);
}

void write_data(const uint8_t *data, size_t len)
{
    // TESTING START

    for (size_t i = 0; i < len; ++i) 
    {
        uint32_t data_ext = (1 << 8) | ((uint32_t) data[i]); // Set 9th pin (DC) to 1
        pio_sm_put_blocking(pio, sm, data_ext);
    }

    return;

    // TESTING END

    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);

    dma_channel_configure(
        dma_chan,
        &c,
        &pio->txf[sm], // dest = PIO FIFO
        data,
        len,
        true // start immediately
    );

    // Optionally: wait for completion
    dma_channel_wait_for_finish_blocking(dma_chan);
}
