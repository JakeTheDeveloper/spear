#include "input.hpp"

void input_sample(InputState* input) {
    *input = {};

    if (IsKeyDown(KEY_W)) input->move_forward += 1.0f;
    if (IsKeyDown(KEY_S)) input->move_forward -= 1.0f;
    if (IsKeyDown(KEY_D)) input->move_right += 1.0f;
    if (IsKeyDown(KEY_A)) input->move_right -= 1.0f;

    input->mouse_delta = GetMouseDelta();
    input->jump_pressed = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_DELETE);
    input->sprint_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    input->primary_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    input->primary_released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    input->recall_pressed = IsKeyPressed(KEY_Q);
}
