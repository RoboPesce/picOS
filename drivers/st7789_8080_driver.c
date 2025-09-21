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
#define BL_PIN 8 // Backlight control pin
#define WR_PIN 9 // Write pin
#define DC_PIN 10 // Data/Command pin
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

static inline void st7789_8080_program_init(PIO pio, uint sm, uint offset,
                                            uint data_pin_base, uint wr_pin);

uint16_t rgb888_to_565(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    uint16_t endian = ((color >> 8) & 0x00FF) | ((color << 8) & 0xFF00); // The masks should be redundant, but we want to be sure
    return endian;
}

void st7789_8080_init() 
{
    // Initialize PIO
    uint offset = pio_add_program(pio, &st7789_8080_program);
    st7789_8080_program_init(pio, sm, offset, DATA_PIN_BASE, WR_PIN);

    /*
    // Initialize pins
    for (int pin = DATA_PIN_BASE; pin < DATA_PIN_BASE + DATA_PIN_COUNT; ++pin) 
    {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
    }
    */

    gpio_init(BL_PIN);
    gpio_set_dir(BL_PIN, GPIO_OUT);
    gpio_init(DC_PIN);
    gpio_set_dir(DC_PIN, GPIO_OUT);
    /* Set by state machine
    gpio_init(WR_PIN);
    gpio_set_dir(WR_PIN, GPIO_OUT);
    gpio_init(CS_PIN);
    */
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_init(RST_PIN);
    gpio_set_dir(RST_PIN, GPIO_OUT);

    // Idle states
    gpio_put(BL_PIN, 1);
    gpio_put(CS_PIN, 1);
    //gpio_put(WR_PIN, 1);
    gpio_put(DC_PIN, 1);
    
    // Reset sequence. Min pulse duration: 10 us
    int reset_pulse_duration = 25;
    gpio_put(RST_PIN, 0);
    sleep_us(reset_pulse_duration);
    gpio_put(RST_PIN, 1);
    sleep_us(reset_pulse_duration);

    // Keep chip select on for remainder of session
    gpio_put(CS_PIN, 0);

    write_command(0x01); // Software reset
    sleep_ms(POST_COMMAND_REST_MS);

    write_command(0x11); // Sleep out (exit low power mode)
    sleep_ms(POST_COMMAND_REST_MS);

    write_command(0x29); // Display on

    write_command(0x3A); // COLMOD
    write_data_single(0x05); // Use 16 bit RGB565 format

    // write_command(0x21); // INVON, for testing

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

void write_command(uint8_t cmd)
{
    gpio_put(DC_PIN, 0);
    pio_sm_put_blocking(pio, sm, cmd);
}

void write_data_single(uint8_t data) 
{
    gpio_put(DC_PIN, 1);
    pio_sm_put_blocking(pio, sm, data);
}

void write_data(const uint8_t *data, size_t len)
{
    gpio_put(DC_PIN, 1);

    // TESTING START

    for (size_t i = 0; i < len; ++i) pio_sm_put_blocking(pio, sm, data[i]);

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

static inline void st7789_8080_program_init(PIO pio, uint sm, uint offset,
                                            uint data_pin_base, uint wr_pin)
{
    // Configure 8 consecutive pins for data
    pio_sm_set_consecutive_pindirs(pio, sm, data_pin_base, DATA_PIN_COUNT, true);

    // Configure sideset pin (WR)
    pio_sm_set_consecutive_pindirs(pio, sm, wr_pin, 1, true);

    pio_sm_config c = st7789_8080_program_get_default_config(offset);

    // Map "out pins" 8 data pins
    sm_config_set_out_pins(&c, data_pin_base, 8);

    // Map sideset pin
    sm_config_set_sideset_pins(&c, wr_pin);

    // Shift settings: push 8 bits from FIFO -> OSR at a time
    sm_config_set_out_shift(&c, false, true, 8);

    // Clock divider (controls PIO speed)
    sm_config_set_clkdiv(&c, PIO_CLOCK_DIVIDER);

    // Initialize SM with this config
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}