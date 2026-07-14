#include "gameplay.hpp"

#include <cmath>

void gameplay_initialize(GameplayState* gameplay) {
    *gameplay = {};
    gameplay->entity_count = 1;
    gameplay->player_entity_index = 0;

    Entity* player = &gameplay->entities[gameplay->player_entity_index];
    player->active = true;
    player->physics.position = { 0.0f, 0.0f, 8.0f };
    player->physics.radius = 0.35f;
    player->controller.type = CONTROLLER_PLAYER;
    player->move_speed = 7.5f;
    player->sprint_speed = 11.0f;
    player->jump_speed = 7.0f;
}

static void update_player_controller(Entity* entity, const InputState* input, const GameCamera* camera) {
    Controller* controller = &entity->controller;
    float yaw_sin = std::sin(camera->yaw);
    float yaw_cos = std::cos(camera->yaw);

    controller->move_direction = {
        yaw_sin * input->move_forward + yaw_cos * input->move_right,
        0.0f,
        -yaw_cos * input->move_forward + yaw_sin * input->move_right
    };
    controller->aim_direction = camera->forward;
    controller->aim_up = camera->up;
    controller->aim_right = camera->right;
    controller->sprint_down = input->sprint_down;
    controller->jump_pressed = input->jump_pressed;
    controller->throw_down = input->primary_down;
    controller->throw_released = input->primary_released;
    controller->recall_pressed = input->recall_pressed;
}

static void update_movement(Entity* entity) {
    Controller* controller = &entity->controller;
    PhysicsBody* physics = &entity->physics;
    float move_length = std::sqrt(
        controller->move_direction.x * controller->move_direction.x +
        controller->move_direction.z * controller->move_direction.z
    );
    float move_x = controller->move_direction.x;
    float move_z = controller->move_direction.z;

    if (move_length > 1.0f) {
        move_x /= move_length;
        move_z /= move_length;
    }

    float move_speed = controller->sprint_down ? entity->sprint_speed : entity->move_speed;
    physics->velocity.x = move_x * move_speed;
    physics->velocity.z = move_z * move_speed;

    if (controller->jump_pressed && physics->position.y <= 0.0f) {
        physics->velocity.y = entity->jump_speed;
    }
}

static void update_held_spear(Entity* entity, float delta_time) {
    Controller* controller = &entity->controller;
    PhysicsBody* physics = &entity->physics;
    Spear* spear = &entity->spear;

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
        spear->launched = true;
        spear->stuck = false;
        spear->returning = false;
    } else if (!controller->throw_down) {
        spear->pullback = 0.0f;
    }
}

static void update_returning_spear(Entity* entity, float delta_time) {
    constexpr float SPEAR_RETURN_SPEED = 90.0f;

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
    float return_step = SPEAR_RETURN_SPEED * delta_time;

    if (return_step >= return_distance) {
        spear->position = rest_position;
        spear->launched = false;
        spear->returning = false;
    } else {
        spear->position.x += return_delta.x / return_distance * return_step;
        spear->position.y += return_delta.y / return_distance * return_step;
        spear->position.z += return_delta.z / return_distance * return_step;
    }
}

static void update_flying_spear(Spear* spear, float delta_time) {
    constexpr float SPEAR_SPEED = 55.0f;

    spear->position.x += spear->direction.x * SPEAR_SPEED * delta_time;
    spear->position.y += spear->direction.y * SPEAR_SPEED * delta_time;
    spear->position.z += spear->direction.z * SPEAR_SPEED * delta_time;
    spear->flight_time += delta_time;

    if (spear->flight_time >= 3.0f) {
        spear->launched = false;
    }
}

static void update_spear(Entity* entity, GameCamera* camera, const PhysicsWorld* physics, float delta_time) {
    Spear* spear = &entity->spear;
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
        Vector3 wall_hit = {};
        if (physics_linecast_static(physics, camera->position, aim_position, &wall_hit)) {
            aim_position = wall_hit;
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
    }

    if (was_flying && spear->launched && !spear->returning) {
        Vector3 tip = {
            spear->position.x + spear->direction.x * 2.6f,
            spear->position.y + spear->direction.y * 2.6f,
            spear->position.z + spear->direction.z * 2.6f
        };
        Vector3 hit_position = {};
        if (physics_linecast_static(physics, previous_tip, tip, &hit_position)) {
            spear->position = {
                hit_position.x - spear->direction.x * 2.35f,
                hit_position.y - spear->direction.y * 2.35f,
                hit_position.z - spear->direction.z * 2.35f
            };
            spear->stuck = true;
        }
    }
}

void gameplay_update(
    GameplayState* gameplay,
    const InputState* input,
    GameCamera* camera,
    const PhysicsWorld* physics,
    float delta_time
) {
    for (int i = 0; i < gameplay->entity_count; i++) {
        Entity* entity = &gameplay->entities[i];
        if (!entity->active) {
            continue;
        }

        if (entity->controller.type == CONTROLLER_PLAYER) {
            update_player_controller(entity, input, camera);
            update_spear(entity, camera, physics, delta_time);
        }

        update_movement(entity);
    }
}
