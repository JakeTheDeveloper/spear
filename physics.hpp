#pragma once

#include "entity.hpp"

constexpr int MAX_STATIC_COLLIDERS = 128;

struct StaticColliders {
    int count;
    Vector3 position[MAX_STATIC_COLLIDERS];
    Vector3 half_size[MAX_STATIC_COLLIDERS];
};

struct LineColliderIntersection {
    bool did_intersect;
    int collider_index;
    Vector3 position;
    Vector3 normal;
};

void physics_update(const StaticColliders* colliders, Entity* entities, int entity_count, float delta_time);
LineColliderIntersection physics_line_intersects_colliders(
    const StaticColliders* colliders,
    Vector3 start,
    Vector3 end
);
bool physics_linecast_entities(
    const Entity* entities,
    int entity_count,
    const Entity* ignored_entity,
    Vector3 start,
    Vector3 end,
    int* hit_entity_index,
    Vector3* hit_position
);
