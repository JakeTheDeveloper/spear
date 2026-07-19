#include "editor.hpp"

#include "camera.hpp"
#include "game.hpp"
#include "input.hpp"
#include "level.hpp"
#include "physics.hpp"

#include <cmath>

void editor_initialize(EditorState* editor) {
    *editor = {};
    editor->selected_collider_index = -1;
}

static void update_editor_camera(
    GameCamera* camera,
    const InputState* input,
    float delta_time
) {
    constexpr float MOVE_SPEED = 16.0f;
    constexpr float FAST_MOVE_SPEED = 48.0f;

    Vector3 movement = {
        camera->forward.x * input->move_forward +
            camera->right.x * input->move_right,
        camera->forward.y * input->move_forward + input->move_up,
        camera->forward.z * input->move_forward +
            camera->right.z * input->move_right
    };
    float movement_length = std::sqrt(
        movement.x * movement.x +
        movement.y * movement.y +
        movement.z * movement.z
    );

    if (movement_length <= 0.0f) {
        return;
    }

    float speed = input->sprint_down ? FAST_MOVE_SPEED : MOVE_SPEED;
    camera->position.x += movement.x / movement_length * speed * delta_time;
    camera->position.y += movement.y / movement_length * speed * delta_time;
    camera->position.z += movement.z / movement_length * speed * delta_time;
}

static void select_collider(
    EditorState* editor,
    const GameCamera* camera,
    const StaticColliders* colliders
) {
    constexpr float SELECT_DISTANCE = 1000.0f;

    Vector3 end = {
        camera->position.x + camera->forward.x * SELECT_DISTANCE,
        camera->position.y + camera->forward.y * SELECT_DISTANCE,
        camera->position.z + camera->forward.z * SELECT_DISTANCE
    };
    LineColliderIntersection intersection =
        physics_line_intersects_colliders(
            colliders,
            camera->position,
            end
        );

    editor->selected_collider_index = intersection.did_intersect
        ? intersection.collider_index
        : -1;
}

static void add_collider(Game* game) {
    constexpr float SELECT_DISTANCE = 1000.0f;
    constexpr float PLACE_DISTANCE = 5.0f;
    constexpr Vector3 HALF_SIZE = { 0.5f, 0.5f, 0.5f };

    StaticColliders* colliders = &game->static_colliders;
    if (colliders->count >= MAX_STATIC_COLLIDERS) {
        return;
    }

    GameCamera* camera = &game->camera;
    Vector3 end = {
        camera->position.x + camera->forward.x * SELECT_DISTANCE,
        camera->position.y + camera->forward.y * SELECT_DISTANCE,
        camera->position.z + camera->forward.z * SELECT_DISTANCE
    };
    LineColliderIntersection intersection =
        physics_line_intersects_colliders(
            colliders,
            camera->position,
            end
        );

    Vector3 position = {
        camera->position.x + camera->forward.x * PLACE_DISTANCE,
        camera->position.y + camera->forward.y * PLACE_DISTANCE,
        camera->position.z + camera->forward.z * PLACE_DISTANCE
    };

    if (intersection.did_intersect) {
        position = {
            intersection.position.x + intersection.normal.x * HALF_SIZE.x,
            intersection.position.y + intersection.normal.y * HALF_SIZE.y,
            intersection.position.z + intersection.normal.z * HALF_SIZE.z
        };
    }

    int collider_index = colliders->count++;
    colliders->position[collider_index] = position;
    colliders->half_size[collider_index] = HALF_SIZE;
    game->editor.selected_collider_index = collider_index;
}

static void delete_selected_collider(Game* game) {
    EditorState* editor = &game->editor;
    StaticColliders* colliders = &game->static_colliders;
    int selected_index = editor->selected_collider_index;

    if (selected_index < 0 || selected_index >= colliders->count) {
        editor->selected_collider_index = -1;
        return;
    }

    for (int i = selected_index; i < colliders->count - 1; i++) {
        colliders->position[i] = colliders->position[i + 1];
        colliders->half_size[i] = colliders->half_size[i + 1];
    }

    colliders->count--;
    editor->selected_collider_index = -1;
}

static void transform_selected_collider(
    Game* game,
    const InputState* input
) {
    EditorState* editor = &game->editor;
    StaticColliders* colliders = &game->static_colliders;
    int selected_index = editor->selected_collider_index;

    if (selected_index < 0 || selected_index >= colliders->count) {
        editor->selected_collider_index = -1;
        return;
    }

    float step = input->sprint_down ? 1.0f : 0.25f;
    Vector3 adjustment = {
        input->editor_adjustment.x * step,
        input->editor_adjustment.y * step,
        input->editor_adjustment.z * step
    };

    if (editor->mode == EDITOR_MODE_MOVE) {
        Vector3* position = &colliders->position[selected_index];
        position->x += adjustment.x;
        position->y += adjustment.y;
        position->z += adjustment.z;
    } else if (editor->mode == EDITOR_MODE_RESIZE) {
        Vector3* half_size = &colliders->half_size[selected_index];
        half_size->x += adjustment.x;
        half_size->y += adjustment.y;
        half_size->z += adjustment.z;

        if (half_size->x < 0.25f) half_size->x = 0.25f;
        if (half_size->y < 0.25f) half_size->y = 0.25f;
        if (half_size->z < 0.25f) half_size->z = 0.25f;
    }
}

void editor_update(
    Game* game,
    const InputState* input,
    float delta_time
) {
    InputState editor_input = *input;
    if (input->editor_save_pressed || input->editor_reload_pressed) {
        editor_input.move_forward = 0.0f;
        editor_input.move_right = 0.0f;
        editor_input.move_up = 0.0f;
        editor_input.editor_adjustment = {};
    }
    input = &editor_input;

    EditorState* editor = &game->editor;
    if (input->editor_move_mode_pressed) {
        editor->mode = editor->mode == EDITOR_MODE_MOVE
            ? EDITOR_MODE_CAMERA
            : EDITOR_MODE_MOVE;
    }
    if (input->editor_resize_mode_pressed) {
        editor->mode = editor->mode == EDITOR_MODE_RESIZE
            ? EDITOR_MODE_CAMERA
            : EDITOR_MODE_RESIZE;
    }

    InputState camera_input = *input;
    camera_input.sprint_down = false;
    camera_update(&game->camera, &camera_input, delta_time);

    if (editor->mode == EDITOR_MODE_CAMERA) {
        update_editor_camera(
            &game->camera,
            input,
            delta_time
        );
    } else {
        transform_selected_collider(game, input);
    }

    if (input->editor_add_pressed) {
        add_collider(game);
    }

    if (input->editor_delete_pressed) {
        delete_selected_collider(game);
    }

    if (input->primary_pressed) {
        select_collider(
            &game->editor,
            &game->camera,
            &game->static_colliders
        );
    }

    if (input->editor_save_pressed) {
        level_save(game, game->level_path);
    }

    if (input->editor_reload_pressed &&
        level_load(game, game->level_path)) {
        game->editor.selected_collider_index = -1;
    }
}
