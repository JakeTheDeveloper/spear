#include "camera.hpp"
#include "game.hpp"
#include "gameplay.hpp"
#include "input.hpp"
#include "level.hpp"
#include "physics.hpp"
#include "render.hpp"

#include "raylib.h"

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1600, 900, "gone fishin");
    DisableCursor();

    g_game.level_path = "levels/test.level";
    if (!level_load(&g_game, g_game.level_path)) {
        CloseWindow();
        return 1;
    }

    camera_initialize(&g_game.camera);
    editor_initialize(&g_game.editor);

    Entity* player = &g_game.gameplay.entities[g_game.gameplay.player_entity_index];
    g_game.camera.position = player->physics.position;
    g_game.camera.position.y += 1.7f;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();
        if (delta_time > 0.05f) delta_time = 0.05f;

        InputState input = {};
        input_sample(&input);

        if (input.editor_toggle_pressed) {
            g_game.editor.active = !g_game.editor.active;
        }

        if (g_game.editor.active) {
            editor_update(&g_game, &input, delta_time);
        } else {
            camera_update(&g_game.camera, &input, delta_time);
            gameplay_update(&g_game.gameplay, &input, &g_game.camera, &g_game.static_colliders, delta_time);

            physics_update(
                &g_game.static_colliders,
                g_game.gameplay.entities,
                g_game.gameplay.entity_count,
                delta_time
            );
            player =
                &g_game.gameplay.entities[g_game.gameplay.player_entity_index];
            g_game.camera.position = player->physics.position;
            g_game.camera.position.y += 1.7f;
        }

        render(
            &g_game.camera,
            g_game.gameplay.entities,
            g_game.gameplay.entity_count,
            &g_game.static_colliders,
            &g_game.editor
        );
    }

    CloseWindow();
    return 0;
}
