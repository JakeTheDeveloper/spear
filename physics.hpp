#pragma once

#include "entity.hpp"

constexpr int MAX_STATIC_COLLIDERS = 128;

struct StaticColliders {
    int count;
    Vector3 position[MAX_STATIC_COLLIDERS];
    Vector3 half_size[MAX_STATIC_COLLIDERS];
};

struct PhysicsWorld {
    StaticColliders static_colliders;
};

void physics_initialize(PhysicsWorld* physics);
void physics_update(PhysicsWorld* physics, Entity* entities, int entity_count, float delta_time);
bool physics_linecast_static(const PhysicsWorld* physics, Vector3 start, Vector3 end, Vector3* hit_position);
