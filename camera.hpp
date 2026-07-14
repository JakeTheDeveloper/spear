#pragma once

#include "input.hpp"

struct GameCamera {
    Vector3 position;
    Vector3 forward;
    Vector3 up;
    Vector3 right;
    float yaw;
    float pitch;
    float lean;
    float vertical_fov;
    float mouse_sensitivity;
    float spear_catch_time;
    bool spear_catch_active;
};

void camera_initialize(GameCamera* camera);
void camera_update(GameCamera* camera, const InputState* input, float delta_time);
void camera_play_spear_catch(GameCamera* camera);
