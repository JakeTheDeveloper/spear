#include "render.hpp"

#include "rlgl.h"

#include <cassert>

void render(
    const GameCamera* game_camera,
    const Entity* entities,
    int entity_count,
    const StaticColliders* colliders,
    const EditorState* editor
) {
    const Entity* player = nullptr;
    for (int i = 0; i < entity_count; i++) {
        if (entities[i].active && entities[i].controller.type == CONTROLLER_PLAYER) {
            player = &entities[i];
            break;
        }
    }
    assert(player);

    const Spear* spear = &player->spear;
    Camera3D camera = {};
    camera.position = game_camera->position;
    camera.target = {
        camera.position.x + game_camera->forward.x,
        camera.position.y + game_camera->forward.y,
        camera.position.z + game_camera->forward.z
    };
    camera.up = game_camera->up;
    camera.fovy = game_camera->vertical_fov;
    camera.projection = CAMERA_PERSPECTIVE;

    float melee_offset = 0.0f;
    if (spear->melee_time > 0.0f) {
        melee_offset = spear->melee_time < 0.1f
            ? spear->melee_time / 0.1f * 1.5f
            : (1.0f - (spear->melee_time - 0.1f) / 0.2f) * 1.5f;
    }
    float spear_forward_offset = 0.3f - spear->pullback * 1.4f + melee_offset;
    Vector3 spear_base = spear->launched
        ? spear->position
        : (Vector3){
            camera.position.x + game_camera->right.x * 0.45f + camera.up.x * 0.35f + game_camera->forward.x * spear_forward_offset,
            camera.position.y + game_camera->right.y * 0.45f + camera.up.y * 0.35f + game_camera->forward.y * spear_forward_offset,
            camera.position.z + game_camera->right.z * 0.45f + camera.up.z * 0.35f + game_camera->forward.z * spear_forward_offset
        };
    Vector3 spear_direction = spear->launched ? spear->direction : game_camera->forward;
    Vector3 spear_end = spear->launched
        ? (Vector3){
            spear_base.x + spear_direction.x * 2.6f,
            spear_base.y + spear_direction.y * 2.6f,
            spear_base.z + spear_direction.z * 2.6f
        }
        : (Vector3){
            spear_base.x + spear_direction.x * 2.6f + camera.up.x * 0.15f,
            spear_base.y + spear_direction.y * 2.6f + camera.up.y * 0.15f,
            spear_base.z + spear_direction.z * 2.6f + camera.up.z * 0.15f
        };

    BeginDrawing();
    ClearBackground({ 18, 20, 24, 255 });
    BeginMode3D(camera);

    rlPushMatrix();
    rlTranslatef(0.0f, 0.01f, 0.0f);
    DrawGrid(70, 1.0f);
    rlPopMatrix();
    if (!editor->active) {
        DrawCylinderEx(spear_base, spear_end, 0.035f, 0.025f, 12, { 126, 85, 50, 255 });
    }

    for (int i = 0; i < entity_count; i++) {
        const Entity* entity = &entities[i];
        if (!entity->active || entity == player) {
            continue;
        }

        Vector3 position = entity->physics.position;
        float width = entity->physics.collision_radius * 2.0f;
        float height = entity->physics.collision_height;
        position.y += height * 0.5f * (1.0f - entity->fall_amount) +
            entity->physics.collision_radius * entity->fall_amount;
        rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);
        rlRotatef(entity->fall_amount * 90.0f, 0.0f, 0.0f, 1.0f);
        DrawCubeV({ 0.0f, 0.0f, 0.0f }, { width, height, width }, { 170, 62, 58, 255 });
        if (!entity->dead) {
            DrawCubeWiresV({ 0.0f, 0.0f, 0.0f }, { width, height, width }, { 235, 126, 112, 255 });
        }
        rlPopMatrix();
    }

    for (int i = 0; i < colliders->count; i++) {
        Vector3 half_size = colliders->half_size[i];
        Vector3 size = { half_size.x * 2.0f, half_size.y * 2.0f, half_size.z * 2.0f };
        DrawCubeV(colliders->position[i], size, { 92, 112, 126, 255 });
        DrawCubeWiresV(colliders->position[i], size, { 139, 166, 181, 255 });
        if (editor->active && editor->selected_collider_index == i) {
            Vector3 highlight_size = {
                size.x + 0.05f,
                size.y + 0.05f,
                size.z + 0.05f
            };
            DrawCubeWiresV(colliders->position[i], highlight_size, YELLOW);
        }
    }

    EndMode3D();

    int center_x = GetScreenWidth() / 2;
    int center_y = GetScreenHeight() / 2;
    int crosshair_gap = spear->pullback > 0.0f
        ? (int)((1.0f - spear->pullback) * 24.0f)
        : 0;
    DrawLine(center_x - crosshair_gap - 6, center_y, center_x - crosshair_gap, center_y, RAYWHITE);
    DrawLine(center_x + crosshair_gap, center_y, center_x + crosshair_gap + 6, center_y, RAYWHITE);
    DrawLine(center_x, center_y - crosshair_gap - 6, center_x, center_y - crosshair_gap, RAYWHITE);
    DrawLine(center_x, center_y + crosshair_gap, center_x, center_y + crosshair_gap + 6, RAYWHITE);
    if (editor->active) {
        const char* mode_text = "CAMERA";
        if (editor->mode == EDITOR_MODE_MOVE) mode_text = "MOVE";
        if (editor->mode == EDITOR_MODE_RESIZE) mode_text = "RESIZE";

        DrawText(
            TextFormat(
                "EDITOR  |  MODE: %s  |  M move  |  R resize  |  Insert add  |  Delete remove  |  Ctrl+S save  |  Ctrl+L reload",
                mode_text
            ),
            20,
            20,
            20,
            YELLOW
        );
    } else {
        DrawText("WASD move  |  Shift sprint  |  Space/Del jump  |  LMB throw  |  RMB melee  |  Q recall  |  Esc quit", 20, 20, 20, LIGHTGRAY);
    }
    DrawFPS(20, 48);

    EndDrawing();
}
