#pragma once

typedef struct InputState
{
    int vertical;
    int horizontal;
} InputState;

InputState get_input_state();

void button_init();