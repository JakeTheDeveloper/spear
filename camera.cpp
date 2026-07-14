#include "camera.hpp"

#include <cmath>

void camera_initialize(GameCamera* camera) {
    *camera = {};
    camera->forward = { 0.0f, 0.0f, -1.0f };
    camera->up = { 0.0f, 1.0f, 0.0f };
    camera->right = { 1.0f, 0.0f, 0.0f };
    camera->vertical_fov = 75.0f;
    camera->mouse_sensitivity = 0.0025f;
}

void camera_update(GameCamera* camera, const InputState* input, float delta_time) {
    constexpr float PITCH_LIMIT = 1.55334f;
    constexpr float MAX_LEAN = 0.052f;
    constexpr float SPEAR_CATCH_ROTATION = 0.045f;

    camera->yaw += input->mouse_delta.x * camera->mouse_sensitivity;
    camera->pitch -= input->mouse_delta.y * camera->mouse_sensitivity;
    if (camera->pitch > PITCH_LIMIT) camera->pitch = PITCH_LIMIT;
    if (camera->pitch < -PITCH_LIMIT) camera->pitch = -PITCH_LIMIT;

    float target_lean = input->sprint_down ? input->move_right * MAX_LEAN : 0.0f;
    float lean_blend = delta_time * 10.0f;
    if (lean_blend > 1.0f) lean_blend = 1.0f;
    camera->lean += (target_lean - camera->lean) * lean_blend;

    float spear_catch_yaw = 0.0f;
    if (camera->spear_catch_active) {
        camera->spear_catch_time += delta_time;

        if (camera->spear_catch_time < 0.1f) {
            spear_catch_yaw = SPEAR_CATCH_ROTATION * camera->spear_catch_time / 0.1f;
        } else if (camera->spear_catch_time < 0.35f) {
            spear_catch_yaw = SPEAR_CATCH_ROTATION * (1.0f - (camera->spear_catch_time - 0.1f) / 0.25f);
        } else {
            camera->spear_catch_active = false;
            camera->spear_catch_time = 0.0f;
        }
    }

    float view_yaw = camera->yaw + spear_catch_yaw;
    float yaw_sin = std::sin(view_yaw);
    float yaw_cos = std::cos(view_yaw);
    float pitch_sin = std::sin(camera->pitch);
    float pitch_cos = std::cos(camera->pitch);
    float lean_sin = std::sin(camera->lean);
    float lean_cos = std::cos(camera->lean);
    Vector3 unrolled_right = { yaw_cos, 0.0f, yaw_sin };
    Vector3 unrolled_up = { -yaw_sin * pitch_sin, pitch_cos, yaw_cos * pitch_sin };

    camera->forward = {
        pitch_cos * yaw_sin,
        pitch_sin,
        -pitch_cos * yaw_cos
    };
    camera->up = {
        unrolled_up.x * lean_cos + unrolled_right.x * lean_sin,
        unrolled_up.y * lean_cos + unrolled_right.y * lean_sin,
        unrolled_up.z * lean_cos + unrolled_right.z * lean_sin
    };
    camera->right = {
        unrolled_right.x * lean_cos - unrolled_up.x * lean_sin,
        unrolled_right.y * lean_cos - unrolled_up.y * lean_sin,
        unrolled_right.z * lean_cos - unrolled_up.z * lean_sin
    };
}

void camera_play_spear_catch(GameCamera* camera) {
    camera->spear_catch_time = 0.0f;
    camera->spear_catch_active = true;
}
