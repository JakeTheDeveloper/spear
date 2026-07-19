#include "input.hpp"

static bool key_pressed_or_repeated(int key) {
    return IsKeyPressed(key) || IsKeyPressedRepeat(key);
}

void input_sample(InputState* input) {
    *input = {};

    if (IsKeyDown(KEY_W)) input->move_forward += 1.0f;
    if (IsKeyDown(KEY_S)) input->move_forward -= 1.0f;
    if (IsKeyDown(KEY_D)) input->move_right += 1.0f;
    if (IsKeyDown(KEY_A)) input->move_right -= 1.0f;
    if (IsKeyDown(KEY_SPACE)) input->move_up += 1.0f;
    if (IsKeyDown(KEY_E)) input->move_up -= 1.0f;

    if (key_pressed_or_repeated(KEY_D)) input->editor_adjustment.x += 1.0f;
    if (key_pressed_or_repeated(KEY_A)) input->editor_adjustment.x -= 1.0f;
    if (key_pressed_or_repeated(KEY_Q)) input->editor_adjustment.y += 1.0f;
    if (key_pressed_or_repeated(KEY_E)) input->editor_adjustment.y -= 1.0f;
    if (key_pressed_or_repeated(KEY_S)) input->editor_adjustment.z += 1.0f;
    if (key_pressed_or_repeated(KEY_W)) input->editor_adjustment.z -= 1.0f;

    input->mouse_delta = GetMouseDelta();
    input->jump_pressed = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_DELETE);
    input->sprint_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    input->primary_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    input->primary_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    input->primary_released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    input->secondary_pressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    input->recall_pressed = IsKeyPressed(KEY_Q);
    input->editor_toggle_pressed = IsKeyPressed(KEY_F1);
    input->editor_add_pressed = IsKeyPressed(KEY_INSERT);
    input->editor_delete_pressed = IsKeyPressed(KEY_DELETE);
    input->editor_move_mode_pressed = IsKeyPressed(KEY_M);
    input->editor_resize_mode_pressed = IsKeyPressed(KEY_R);

    bool control_down =
        IsKeyDown(KEY_LEFT_CONTROL) ||
        IsKeyDown(KEY_RIGHT_CONTROL);
    input->editor_save_pressed =
        control_down && IsKeyPressed(KEY_S);
    input->editor_reload_pressed =
        control_down && IsKeyPressed(KEY_L);
}
