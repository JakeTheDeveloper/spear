#include "physics.hpp"

#include <cmath>

void physics_update(const StaticColliders* colliders, Entity* entities, int entity_count, float delta_time) {
    constexpr float COLLISION_EPSILON = 0.0001f;
    constexpr float GRAVITY = 20.0f;

    for (int entity_index = 0; entity_index < entity_count; entity_index++) {
        Entity* entity = &entities[entity_index];
        if (!entity->active) {
            continue;
        }

        PhysicsBody* body = &entity->physics;
        float radius = body->collision_radius;
        Vector3 position = body->position;

        body->grounded = false;
        if (!entity->stuck_to_wall) {
            body->velocity.y -= GRAVITY * delta_time;
        } else {
            body->velocity.y = 0.0f;
        }

        float previous_x = position.x;
        position.x += body->velocity.x * delta_time;
        for (int i = 0; i < colliders->count; i++) {
            Vector3 collider_position = colliders->position[i];
            Vector3 half_size = colliders->half_size[i];
            float collider_min_x = collider_position.x - half_size.x;
            float collider_max_x = collider_position.x + half_size.x;
            bool overlaps_z = position.z + radius > collider_position.z - half_size.z &&
                position.z - radius < collider_position.z + half_size.z;
            bool overlaps_y = position.y + body->collision_height > collider_position.y - half_size.y &&
                position.y < collider_position.y + half_size.y;

            if (!overlaps_z || !overlaps_y) {
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
        for (int i = 0; i < colliders->count; i++) {
            Vector3 collider_position = colliders->position[i];
            Vector3 half_size = colliders->half_size[i];
            float collider_min_z = collider_position.z - half_size.z;
            float collider_max_z = collider_position.z + half_size.z;
            bool overlaps_x = position.x + radius > collider_position.x - half_size.x &&
                position.x - radius < collider_position.x + half_size.x;
            bool overlaps_y = position.y + body->collision_height > collider_position.y - half_size.y &&
                position.y < collider_position.y + half_size.y;

            if (!overlaps_x || !overlaps_y) {
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

        if (!entity->stuck_to_wall) {
            float previous_y = position.y;
            float vertical_velocity = body->velocity.y;
            position.y += vertical_velocity * delta_time;

            for (int i = 0; i < colliders->count; i++) {
                Vector3 collider_position = colliders->position[i];
                Vector3 half_size = colliders->half_size[i];
                bool overlaps_x = position.x + radius > collider_position.x - half_size.x &&
                    position.x - radius < collider_position.x + half_size.x;
                bool overlaps_z = position.z + radius > collider_position.z - half_size.z &&
                    position.z - radius < collider_position.z + half_size.z;

                if (!overlaps_x || !overlaps_z) {
                    continue;
                }

                float collider_min_y = collider_position.y - half_size.y;
                float collider_max_y = collider_position.y + half_size.y;

                if (vertical_velocity > 0.0f &&
                    previous_y + body->collision_height <= collider_min_y + COLLISION_EPSILON &&
                    position.y + body->collision_height > collider_min_y) {
                    position.y = collider_min_y - body->collision_height;
                    body->velocity.y = 0.0f;
                } else if (vertical_velocity < 0.0f &&
                    previous_y >= collider_max_y - COLLISION_EPSILON &&
                    position.y < collider_max_y) {
                    position.y = collider_max_y;
                    body->velocity.y = 0.0f;
                    body->grounded = true;
                }
            }
        }

        body->position = position;
    }
}

LineColliderIntersection physics_line_intersects_colliders(
    const StaticColliders* colliders,
    Vector3 start,
    Vector3 end
) {
    LineColliderIntersection result = {};
    result.collider_index = -1;

    Vector3 delta = { end.x - start.x, end.y - start.y, end.z - start.z };
    float length = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    if (length <= 0.0f) {
        return result;
    }

    Ray ray = {
        .position = start,
        .direction = { delta.x / length, delta.y / length, delta.z / length }
    };
    float nearest_distance = length;

    for (int i = 0; i < colliders->count; i++) {
        Vector3 position = colliders->position[i];
        Vector3 half_size = colliders->half_size[i];
        BoundingBox bounds = {
            .min = { position.x - half_size.x, position.y - half_size.y, position.z - half_size.z },
            .max = { position.x + half_size.x, position.y + half_size.y, position.z + half_size.z }
        };
        RayCollision collision = GetRayCollisionBox(ray, bounds);

        if (collision.hit && collision.distance <= nearest_distance) {
            nearest_distance = collision.distance;
            result.did_intersect = true;
            result.collider_index = i;
            result.position = collision.point;
            result.normal = collision.normal;
        }
    }

    return result;
}

bool physics_linecast_entities(
    const Entity* entities,
    int entity_count,
    const Entity* ignored_entity,
    Vector3 start,
    Vector3 end,
    int* hit_entity_index,
    Vector3* hit_position
) {
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

    for (int i = 0; i < entity_count; i++) {
        const Entity* entity = &entities[i];
        if (!entity->active || entity->dead || entity == ignored_entity) {
            continue;
        }

        const PhysicsBody* body = &entity->physics;
        BoundingBox bounds = {
            .min = {
                body->position.x - body->collision_radius,
                body->position.y,
                body->position.z - body->collision_radius
            },
            .max = {
                body->position.x + body->collision_radius,
                body->position.y + body->collision_height,
                body->position.z + body->collision_radius
            }
        };
        RayCollision collision = GetRayCollisionBox(ray, bounds);

        if (collision.hit && collision.distance <= nearest_distance) {
            nearest_distance = collision.distance;
            *hit_entity_index = i;
            *hit_position = collision.point;
            hit = true;
        }
    }

    return hit;
}
