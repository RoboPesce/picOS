#include "hardware/pio.h"
#include "hardware/structs/pio.h"
#include "led.pio.h"

#define PIO_CLOCK_DIVIDER 1.0f
#define LED_PIN 25

static PIO pio = pio1;
static int sm = 0;

void led_driver_init()
{
    uint offset = pio_add_program(pio, &led_program);
    led_program_init(pio, sm, offset, LED_PIN, PIO_CLOCK_DIVIDER);
}

void toggle_led()
{
    static int led_on = 0;
    pio_sm_put_blocking(pio, sm, led_on);
    led_on = !led_on;
}