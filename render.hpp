#pragma once

#include "camera.hpp"
#include "editor.hpp"
#include "entity.hpp"
#include "physics.hpp"

void render(
    const GameCamera* game_camera,
    const Entity* entities,
    int entity_count,
    const StaticColliders* colliders,
    const EditorState* editor
);
