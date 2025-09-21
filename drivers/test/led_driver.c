#include "hardware/pio.h"
#include "hardware/structs/pio.h"
#include "led.pio.h"

#define PIO_CLOCK_DIVIDER 1.0f
#define LED_PIN 25

PIO pio = pio1;
int sm = 0;

static inline void led_program_init(PIO pio, uint sm, uint offset, uint led_pin)
{
    // Configure sideset pin (WR)
    pio_sm_set_consecutive_pindirs(pio, sm, led_pin, 1, true);

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