#pragma once

#include "raylib.h"

struct InputState {
    float move_forward;
    float move_right;
    float move_up;
    Vector2 mouse_delta;
    Vector3 editor_adjustment;
    bool jump_pressed;
    bool sprint_down;
    bool primary_pressed;
    bool primary_down;
    bool primary_released;
    bool secondary_pressed;
    bool recall_pressed;
    bool editor_toggle_pressed;
    bool editor_add_pressed;
    bool editor_delete_pressed;
    bool editor_move_mode_pressed;
    bool editor_resize_mode_pressed;
    bool editor_save_pressed;
    bool editor_reload_pressed;
};

void input_sample(InputState* input);
