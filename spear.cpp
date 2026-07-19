#include "spear.hpp"

#include "camera.hpp"
#include "entity.hpp"
#include "physics.hpp"

#include <cmath>

static void update_held_spear(Entity* owner_entity, float delta_time) {
    Controller* controller = &owner_entity->controller;
    PhysicsBody* physics = &owner_entity->physics;
    Spear* spear = &owner_entity->spear;

    if (owner_entity->stuck_to_wall || spear->melee_time > 0.0f) {
        spear->pullback = 0.0f;
        return;
    }

    if (controller->throw_down) {
        spear->pullback += delta_time / 0.7f;
        if (spear->pullback > 1.0f) spear->pullback = 1.0f;
    }

    if (controller->throw_released) {
        float forward_offset = 0.3f - spear->pullback * 1.4f;
        spear->position = {
            physics->position.x + controller->aim_right.x * 0.45f + controller->aim_up.x * 0.35f + controller->aim_direction.x * forward_offset,
            physics->position.y + 1.7f + controller->aim_right.y * 0.45f + controller->aim_up.y * 0.35f + controller->aim_direction.y * forward_offset,
            physics->position.z + controller->aim_right.z * 0.45f + controller->aim_up.z * 0.35f + controller->aim_direction.z * forward_offset
        };
        spear->direction = controller->aim_direction;
        spear->pullback = 0.0f;
        spear->flight_time = 0.0f;
        spear->return_time = 0.0f;
        spear->stuck_entity_index = -1;
        spear->launched = true;
        spear->stuck = false;
        spear->returning = false;
    } else if (!controller->throw_down) {
        spear->pullback = 0.0f;
    }
}

static void update_returning_spear(Entity* entity, float delta_time) {
    constexpr float SPEAR_RETURN_START_SPEED = 55.0f;
    constexpr float SPEAR_RETURN_ACCELERATION = 160.0f;
    constexpr float SPEAR_RETURN_MAX_SPEED = 140.0f;

    Spear* spear = &entity->spear;
    Controller* controller = &entity->controller;
    PhysicsBody* physics = &entity->physics;
    Vector3 rest_position = {
        physics->position.x + controller->aim_right.x * 0.45f + controller->aim_up.x * 0.35f + controller->aim_direction.x * 0.3f,
        physics->position.y + 1.7f + controller->aim_right.y * 0.45f + controller->aim_up.y * 0.35f + controller->aim_direction.y * 0.3f,
        physics->position.z + controller->aim_right.z * 0.45f + controller->aim_up.z * 0.35f + controller->aim_direction.z * 0.3f
    };
    Vector3 return_delta = {
        rest_position.x - spear->position.x,
        rest_position.y - spear->position.y,
        rest_position.z - spear->position.z
    };
    float return_distance = std::sqrt(
        return_delta.x * return_delta.x +
        return_delta.y * return_delta.y +
        return_delta.z * return_delta.z
    );
    spear->return_time += delta_time;
    float return_speed = SPEAR_RETURN_START_SPEED + SPEAR_RETURN_ACCELERATION * spear->return_time;
    if (return_speed > SPEAR_RETURN_MAX_SPEED) return_speed = SPEAR_RETURN_MAX_SPEED;
    float return_step = return_speed * delta_time;

    if (return_step >= return_distance) {
        spear->position = rest_position;
        spear->stuck_entity_index = -1;
        spear->return_time = 0.0f;
        spear->launched = false;
        spear->returning = false;
    } else {
        spear->position.x += return_delta.x / return_distance * return_step;
        spear->position.y += return_delta.y / return_distance * return_step;
        spear->position.z += return_delta.z / return_distance * return_step;
    }
}

static void update_flying_spear(Spear* spear, float delta_time) {
    constexpr float SPEAR_DROP_ACCELERATION = 8.0f;

    spear->velocity.y -= SPEAR_DROP_ACCELERATION * delta_time;
    spear->position.x += spear->velocity.x * delta_time;
    spear->position.y += spear->velocity.y * delta_time;
    spear->position.z += spear->velocity.z * delta_time;
    float speed = std::sqrt(
        spear->velocity.x * spear->velocity.x +
        spear->velocity.y * spear->velocity.y +
        spear->velocity.z * spear->velocity.z
    );
    spear->direction = {
        spear->velocity.x / speed,
        spear->velocity.y / speed,
        spear->velocity.z / speed
    };
    spear->flight_time += delta_time;

    if (spear->flight_time >= 3.0f) {
        spear->launched = false;
    }
}

static void update_stuck_spear(Spear* spear, Entity* entities, int entity_count) {
    if (!spear->stuck || spear->stuck_entity_index < 0 || spear->stuck_entity_index >= entity_count) {
        return;
    }

    Entity* stuck_entity = &entities[spear->stuck_entity_index];
    PhysicsBody* body = &stuck_entity->physics;
    float angle = stuck_entity->fall_amount * PI * 0.5f;
    float angle_sin = std::sin(angle);
    float angle_cos = std::cos(angle);
    float center_y = body->position.y +
        body->collision_height * 0.5f * (1.0f - stuck_entity->fall_amount) +
        body->collision_radius * stuck_entity->fall_amount;
    Vector3 center = { body->position.x, center_y, body->position.z };

    spear->position = {
        center.x + spear->stuck_offset.x * angle_cos - spear->stuck_offset.y * angle_sin,
        center.y + spear->stuck_offset.x * angle_sin + spear->stuck_offset.y * angle_cos,
        center.z + spear->stuck_offset.z
    };
    spear->direction = {
        spear->stuck_direction.x * angle_cos - spear->stuck_direction.y * angle_sin,
        spear->stuck_direction.x * angle_sin + spear->stuck_direction.y * angle_cos,
        spear->stuck_direction.z
    };
}

static void update_melee_attack(
    Entity* entity,
    Entity* entities,
    int entity_count,
    const GameCamera* camera,
    const StaticColliders* colliders,
    float delta_time
) {
    constexpr float MELEE_DURATION = 0.3f;
    constexpr float MELEE_RANGE = 3.0f;

    Spear* spear = &entity->spear;
    Controller* controller = &entity->controller;
    if (spear->melee_time > 0.0f) {
        spear->melee_time += delta_time;
        if (spear->melee_time >= MELEE_DURATION) spear->melee_time = 0.0f;
        return;
    }

    if (!controller->melee_pressed || controller->throw_down || spear->launched || entity->stuck_to_wall) {
        return;
    }

    spear->melee_time = delta_time;
    Vector3 attack_end = {
        camera->position.x + camera->forward.x * MELEE_RANGE,
        camera->position.y + camera->forward.y * MELEE_RANGE,
        camera->position.z + camera->forward.z * MELEE_RANGE
    };
    LineColliderIntersection collider_intersection =
        physics_line_intersects_colliders(colliders, camera->position, attack_end);
    int hit_entity_index = -1;
    Vector3 entity_hit = {};
    bool hit_entity = physics_linecast_entities(
        entities,
        entity_count,
        entity,
        camera->position,
        attack_end,
        &hit_entity_index,
        &entity_hit
    );
    float collider_distance_squared =
        (collider_intersection.position.x - camera->position.x) * (collider_intersection.position.x - camera->position.x) +
        (collider_intersection.position.y - camera->position.y) * (collider_intersection.position.y - camera->position.y) +
        (collider_intersection.position.z - camera->position.z) * (collider_intersection.position.z - camera->position.z);
    float entity_distance_squared =
        (entity_hit.x - camera->position.x) * (entity_hit.x - camera->position.x) +
        (entity_hit.y - camera->position.y) * (entity_hit.y - camera->position.y) +
        (entity_hit.z - camera->position.z) * (entity_hit.z - camera->position.z);

    if (hit_entity && (!collider_intersection.did_intersect || entity_distance_squared < collider_distance_squared)) {
        Entity* hit_target = &entities[hit_entity_index];
        hit_target->dead = true;
        hit_target->physics.velocity = {};
        hit_target->controller.move_direction = {};
    } else if (collider_intersection.did_intersect && std::fabs(collider_intersection.normal.y) < 0.5f) {
        entity->stuck_to_wall = true;
        entity->physics.velocity = {};
    }
}

void spear_update(
    Entity* entity,
    Entity* entities,
    int entity_count,
    GameCamera* camera,
    const StaticColliders* colliders,
    float delta_time
) {
    Spear* spear = &entity->spear;
    update_stuck_spear(spear, entities, entity_count);
    update_melee_attack(entity, entities, entity_count, camera, colliders, delta_time);

    bool was_launched = spear->launched;
    bool was_flying = spear->launched && !spear->stuck && !spear->returning;
    bool was_returning = spear->returning;
    Vector3 previous_tip = {
        spear->position.x + spear->direction.x * 2.6f,
        spear->position.y + spear->direction.y * 2.6f,
        spear->position.z + spear->direction.z * 2.6f
    };

    if (entity->controller.recall_pressed && spear->launched) {
        spear->returning = true;
        spear->stuck = false;
        spear->return_time = 0.0f;
    }

    if (!spear->launched) {
        update_held_spear(entity, delta_time);
    } else if (spear->returning) {
        update_returning_spear(entity, delta_time);
    } else if (!spear->stuck) {
        update_flying_spear(spear, delta_time);
    }

    if (was_returning && !spear->launched) {
        camera_play_spear_catch(camera);
    }

    if (!was_launched && spear->launched) {
        Vector3 aim_position = {
            camera->position.x + camera->forward.x * 100.0f,
            camera->position.y + camera->forward.y * 100.0f,
            camera->position.z + camera->forward.z * 100.0f
        };
        LineColliderIntersection collider_intersection =
            physics_line_intersects_colliders(colliders, camera->position, aim_position);
        int aimed_entity_index = -1;
        Vector3 entity_hit = {};
        bool hit_entity = physics_linecast_entities(
            entities,
            entity_count,
            entity,
            camera->position,
            aim_position,
            &aimed_entity_index,
            &entity_hit
        );

        float collider_distance_squared =
            (collider_intersection.position.x - camera->position.x) * (collider_intersection.position.x - camera->position.x) +
            (collider_intersection.position.y - camera->position.y) * (collider_intersection.position.y - camera->position.y) +
            (collider_intersection.position.z - camera->position.z) * (collider_intersection.position.z - camera->position.z);
        float entity_distance_squared =
            (entity_hit.x - camera->position.x) * (entity_hit.x - camera->position.x) +
            (entity_hit.y - camera->position.y) * (entity_hit.y - camera->position.y) +
            (entity_hit.z - camera->position.z) * (entity_hit.z - camera->position.z);

        if (hit_entity && (!collider_intersection.did_intersect || entity_distance_squared < collider_distance_squared)) {
            aim_position = entity_hit;
        } else if (collider_intersection.did_intersect) {
            aim_position = collider_intersection.position;
        }

        Vector3 aim_delta = {
            aim_position.x - spear->position.x,
            aim_position.y - spear->position.y,
            aim_position.z - spear->position.z
        };
        float aim_length = std::sqrt(
            aim_delta.x * aim_delta.x +
            aim_delta.y * aim_delta.y +
            aim_delta.z * aim_delta.z
        );
        spear->direction = {
            aim_delta.x / aim_length,
            aim_delta.y / aim_length,
            aim_delta.z / aim_length
        };
        spear->velocity = {
            spear->direction.x * 55.0f,
            spear->direction.y * 55.0f,
            spear->direction.z * 55.0f
        };
    }

    if (was_flying && spear->launched && !spear->returning) {
        Vector3 tip = {
            spear->position.x + spear->direction.x * 2.6f,
            spear->position.y + spear->direction.y * 2.6f,
            spear->position.z + spear->direction.z * 2.6f
        };
        LineColliderIntersection collider_intersection =
            physics_line_intersects_colliders(colliders, previous_tip, tip);
        int hit_entity_index = -1;
        Vector3 entity_hit = {};
        bool hit_entity = physics_linecast_entities(
            entities,
            entity_count,
            entity,
            previous_tip,
            tip,
            &hit_entity_index,
            &entity_hit
        );
        float collider_distance_squared =
            (collider_intersection.position.x - previous_tip.x) * (collider_intersection.position.x - previous_tip.x) +
            (collider_intersection.position.y - previous_tip.y) * (collider_intersection.position.y - previous_tip.y) +
            (collider_intersection.position.z - previous_tip.z) * (collider_intersection.position.z - previous_tip.z);
        float entity_distance_squared =
            (entity_hit.x - previous_tip.x) * (entity_hit.x - previous_tip.x) +
            (entity_hit.y - previous_tip.y) * (entity_hit.y - previous_tip.y) +
            (entity_hit.z - previous_tip.z) * (entity_hit.z - previous_tip.z);

        if (hit_entity && (!collider_intersection.did_intersect || entity_distance_squared < collider_distance_squared)) {
            spear->position = {
                entity_hit.x - spear->direction.x * 2.35f,
                entity_hit.y - spear->direction.y * 2.35f,
                entity_hit.z - spear->direction.z * 2.35f
            };
            spear->stuck = true;
            spear->stuck_entity_index = hit_entity_index;
            spear->stuck_direction = spear->direction;

            Entity* hit_target = &entities[hit_entity_index];
            hit_target->dead = true;
            hit_target->physics.velocity = {};
            hit_target->controller.move_direction = {};
            Vector3 enemy_center = {
                hit_target->physics.position.x,
                hit_target->physics.position.y + hit_target->physics.collision_height * 0.5f,
                hit_target->physics.position.z
            };
            spear->stuck_offset = {
                spear->position.x - enemy_center.x,
                spear->position.y - enemy_center.y,
                spear->position.z - enemy_center.z
            };
        } else if (collider_intersection.did_intersect) {
            spear->position = {
                collider_intersection.position.x - spear->direction.x * 2.35f,
                collider_intersection.position.y - spear->direction.y * 2.35f,
                collider_intersection.position.z - spear->direction.z * 2.35f
            };
            spear->stuck = true;
            spear->stuck_entity_index = -1;
        }
    }
}
