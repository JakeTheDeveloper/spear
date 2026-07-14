#include "camera.hpp"
#include "gameplay.hpp"
#include "input.hpp"
#include "physics.hpp"
#include "render.hpp"

#include "raylib.h"

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1600, 900, "gone fishin");
    DisableCursor();

    PhysicsWorld physics = {};
    GameplayState gameplay = {};
    GameCamera game_camera = {};
    physics_initialize(&physics);
    gameplay_initialize(&gameplay);
    camera_initialize(&game_camera);

    Entity* player = &gameplay.entities[gameplay.player_entity_index];
    Vector3 player_position = player->physics.position;
    game_camera.position = { player_position.x, player_position.y + 1.7f, player_position.z };

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();
        if (delta_time > 0.05f) delta_time = 0.05f;

        InputState input = {};
        input_sample(&input);
        camera_update(&game_camera, &input, delta_time);
        gameplay_update(&gameplay, &input, &game_camera, &physics, delta_time);

        physics_update(&physics, gameplay.entities, gameplay.entity_count, delta_time);
        player_position = player->physics.position;
        game_camera.position = { player_position.x, player_position.y + 1.7f, player_position.z };
        render(&game_camera, player, &physics);
    }

    CloseWindow();
    return 0;
}
