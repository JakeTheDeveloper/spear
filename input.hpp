#pragma once

#include "raylib.h"

struct InputState {
    float move_forward;
    float move_right;
    Vector2 mouse_delta;
    bool jump_pressed;
    bool sprint_down;
    bool primary_down;
    bool primary_released;
    bool recall_pressed;
};

void input_sample(InputState* input);
