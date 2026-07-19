#pragma once

struct Game;
struct InputState;

enum EditorMode {
    EDITOR_MODE_CAMERA,
    EDITOR_MODE_MOVE,
    EDITOR_MODE_RESIZE
};

struct EditorState {
    int selected_collider_index;
    EditorMode mode;
    bool active;
};

void editor_initialize(EditorState* editor);
void editor_update(
    Game* game,
    const InputState* input,
    float delta_time
);
