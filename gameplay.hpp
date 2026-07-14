#pragma once

#include "camera.hpp"
#include "entity.hpp"
#include "physics.hpp"

constexpr int MAX_ENTITIES = 64;

struct GameplayState {
    int entity_count;
    int player_entity_index;
    Entity entities[MAX_ENTITIES];
};

void gameplay_initialize(GameplayState* gameplay);
void gameplay_update(
    GameplayState* gameplay,
    const InputState* input,
    GameCamera* camera,
    const PhysicsWorld* physics,
    float delta_time
);
