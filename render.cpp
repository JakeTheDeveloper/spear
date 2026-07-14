#include "render.hpp"

void render(const GameCamera* game_camera, const Entity* player, const PhysicsWorld* physics) {
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

    float spear_forward_offset = 0.3f - spear->pullback * 1.4f;
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

    DrawPlane({ 0.0f, -0.01f, 0.0f }, { 40.0f, 40.0f }, { 56, 62, 66, 255 });
    DrawGrid(40, 1.0f);
    DrawCylinderEx(spear_base, spear_end, 0.035f, 0.025f, 12, { 126, 85, 50, 255 });

    const StaticColliders* colliders = &physics->static_colliders;
    for (int i = 0; i < colliders->count; i++) {
        Vector3 half_size = colliders->half_size[i];
        Vector3 size = { half_size.x * 2.0f, half_size.y * 2.0f, half_size.z * 2.0f };
        DrawCubeV(colliders->position[i], size, { 92, 112, 126, 255 });
        DrawCubeWiresV(colliders->position[i], size, { 139, 166, 181, 255 });
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
    DrawText("WASD move  |  Shift sprint  |  Space/Del jump  |  LMB throw  |  Q recall  |  Esc quit", 20, 20, 20, LIGHTGRAY);
    DrawFPS(20, 48);

    EndDrawing();
}
