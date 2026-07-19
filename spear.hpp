#pragma once

#include "raylib.h"

struct Entity;
struct GameCamera;
struct StaticColliders;

struct Spear {
    Vector3 position;
    Vector3 direction;
    Vector3 velocity;
    Vector3 stuck_offset;
    Vector3 stuck_direction;
    float pullback;
    float flight_time;
    float return_time;
    float melee_time;
    int stuck_entity_index;
    bool launched;
    bool stuck;
    bool returning;
};

void spear_update(
    Entity* entity,
    Entity* entities,
    int entity_count,
    GameCamera* camera,
    const StaticColliders* colliders,
    float delta_time
);
