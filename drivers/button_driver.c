#include "pico/stdlib.h"
#include "button_driver.h"

#define    UP_PIN 10
#define  DOWN_PIN 11
#define  LEFT_PIN 12
#define RIGHT_PIN 13

void button_init()
{
    gpio_init(UP_PIN);
    gpio_init(DOWN_PIN);
    gpio_init(LEFT_PIN);
    gpio_init(RIGHT_PIN);
    gpio_pull_up(UP_PIN); 
    gpio_pull_up(DOWN_PIN); 
    gpio_pull_up(LEFT_PIN); 
    gpio_pull_up(RIGHT_PIN); 
}

InputState get_input_state()
{
    InputState state;
    state.horizontal = 0;
    state.vertical = 0;
    if (!gpio_get(UP_PIN)) state.vertical--;
    if (!gpio_get(DOWN_PIN)) state.vertical++;
    if (!gpio_get(LEFT_PIN)) state.horizontal--;
    if (!gpio_get(RIGHT_PIN)) state.horizontal++;

    return state;
}