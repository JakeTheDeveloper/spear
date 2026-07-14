#include "physics.hpp"

#include <cmath>

void physics_initialize(PhysicsWorld* physics) {
    *physics = {};

    StaticColliders* colliders = &physics->static_colliders;
    colliders->count = 6;
    colliders->position[0] = { 0.0f, 1.5f, -15.0f };
    colliders->half_size[0] = { 15.0f, 1.5f, 0.5f };
    colliders->position[1] = { 0.0f, 1.5f, 15.0f };
    colliders->half_size[1] = { 15.0f, 1.5f, 0.5f };
    colliders->position[2] = { -15.0f, 1.5f, 0.0f };
    colliders->half_size[2] = { 0.5f, 1.5f, 15.0f };
    colliders->position[3] = { 15.0f, 1.5f, 0.0f };
    colliders->half_size[3] = { 0.5f, 1.5f, 15.0f };
    colliders->position[4] = { 0.0f, 1.0f, -4.0f };
    colliders->half_size[4] = { 2.0f, 1.0f, 2.0f };
    colliders->position[5] = { -6.0f, 1.25f, 3.0f };
    colliders->half_size[5] = { 1.0f, 1.25f, 3.0f };
}

void physics_update(PhysicsWorld* physics, Entity* entities, int entity_count, float delta_time) {
    constexpr float COLLISION_EPSILON = 0.0001f;
    constexpr float GRAVITY = 20.0f;

    for (int entity_index = 0; entity_index < entity_count; entity_index++) {
        Entity* entity = &entities[entity_index];
        if (!entity->active) {
            continue;
        }

        PhysicsBody* body = &entity->physics;
        float radius = body->radius;
        Vector3 position = body->position;

        body->velocity.y -= GRAVITY * delta_time;
        position.y += body->velocity.y * delta_time;
        if (position.y < 0.0f) {
            position.y = 0.0f;
            body->velocity.y = 0.0f;
        }

        float previous_x = position.x;
        position.x += body->velocity.x * delta_time;
        for (int i = 0; i < physics->static_colliders.count; i++) {
            Vector3 collider_position = physics->static_colliders.position[i];
            Vector3 half_size = physics->static_colliders.half_size[i];
            float collider_min_x = collider_position.x - half_size.x;
            float collider_max_x = collider_position.x + half_size.x;
            bool overlaps_z = position.z + radius > collider_position.z - half_size.z &&
                position.z - radius < collider_position.z + half_size.z;

            if (!overlaps_z) {
                continue;
            }

            if (body->velocity.x > 0.0f &&
                previous_x + radius <= collider_min_x + COLLISION_EPSILON &&
                position.x + radius > collider_min_x) {
                position.x = collider_min_x - radius;
                body->velocity.x = 0.0f;
            } else if (body->velocity.x < 0.0f &&
                previous_x - radius >= collider_max_x - COLLISION_EPSILON &&
                position.x - radius < collider_max_x) {
                position.x = collider_max_x + radius;
                body->velocity.x = 0.0f;
            }
        }

        float previous_z = position.z;
        position.z += body->velocity.z * delta_time;
        for (int i = 0; i < physics->static_colliders.count; i++) {
            Vector3 collider_position = physics->static_colliders.position[i];
            Vector3 half_size = physics->static_colliders.half_size[i];
            float collider_min_z = collider_position.z - half_size.z;
            float collider_max_z = collider_position.z + half_size.z;
            bool overlaps_x = position.x + radius > collider_position.x - half_size.x &&
                position.x - radius < collider_position.x + half_size.x;

            if (!overlaps_x) {
                continue;
            }

            if (body->velocity.z > 0.0f &&
                previous_z + radius <= collider_min_z + COLLISION_EPSILON &&
                position.z + radius > collider_min_z) {
                position.z = collider_min_z - radius;
                body->velocity.z = 0.0f;
            } else if (body->velocity.z < 0.0f &&
                previous_z - radius >= collider_max_z - COLLISION_EPSILON &&
                position.z - radius < collider_max_z) {
                position.z = collider_max_z + radius;
                body->velocity.z = 0.0f;
            }
        }

        body->position = position;
    }
}

bool physics_linecast_static(const PhysicsWorld* physics, Vector3 start, Vector3 end, Vector3* hit_position) {
    Vector3 delta = { end.x - start.x, end.y - start.y, end.z - start.z };
    float length = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    if (length <= 0.0f) {
        return false;
    }

    Ray ray = {
        .position = start,
        .direction = { delta.x / length, delta.y / length, delta.z / length }
    };
    bool hit = false;
    float nearest_distance = length;

    for (int i = 0; i < physics->static_colliders.count; i++) {
        Vector3 position = physics->static_colliders.position[i];
        Vector3 half_size = physics->static_colliders.half_size[i];
        BoundingBox bounds = {
            .min = { position.x - half_size.x, position.y - half_size.y, position.z - half_size.z },
            .max = { position.x + half_size.x, position.y + half_size.y, position.z + half_size.z }
        };
        RayCollision collision = GetRayCollisionBox(ray, bounds);

        if (collision.hit && collision.distance <= nearest_distance) {
            nearest_distance = collision.distance;
            *hit_position = collision.point;
            hit = true;
        }
    }

    return hit;
}
