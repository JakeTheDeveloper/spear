#include "gameplay.hpp"
#include "spear.hpp"

#include <cmath>

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
    controller->melee_pressed = input->secondary_pressed;
    controller->recall_pressed = input->recall_pressed;

    if (entity->stuck_to_wall) {
        controller->move_direction = {};
        controller->sprint_down = false;
        controller->throw_down = false;
        controller->throw_released = false;
        controller->melee_pressed = false;
    }
}

static void update_ai_controller(Entity* entity, const Entity* player) {
    constexpr float STOPPING_DISTANCE = 2.0f;

    Controller* controller = &entity->controller;
    float delta_x = player->physics.position.x - entity->physics.position.x;
    float delta_z = player->physics.position.z - entity->physics.position.z;
    float distance = std::sqrt(delta_x * delta_x + delta_z * delta_z);

    controller->move_direction = {};
    controller->sprint_down = false;
    controller->jump_pressed = false;
    controller->throw_down = false;
    controller->throw_released = false;
    controller->melee_pressed = false;
    controller->recall_pressed = false;

    if (distance <= 0.0f) {
        return;
    }

    controller->aim_direction = { delta_x / distance, 0.0f, delta_z / distance };
    controller->aim_up = { 0.0f, 1.0f, 0.0f };
    controller->aim_right = {
        -controller->aim_direction.z,
        0.0f,
        controller->aim_direction.x
    };

    if (distance <= entity->awareness_radius && distance > STOPPING_DISTANCE) {
        controller->move_direction = controller->aim_direction;
    }
}

static void update_movement(Entity* entity) {
    Controller* controller = &entity->controller;
    PhysicsBody* physics = &entity->physics;

    if (entity->stuck_to_wall) {
        physics->velocity = {};
        if (controller->jump_pressed) {
            entity->stuck_to_wall = false;
            physics->velocity.y = entity->jump_speed;
        }
        return;
    }

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

    if (controller->jump_pressed && physics->grounded) {
        physics->grounded = false;
        physics->velocity.y = entity->jump_speed;
    }
}

void gameplay_update(GameplayState* gameplay, const InputState* input, GameCamera* camera, const StaticColliders* colliders, float delta_time) {
    Entity* player = &gameplay->entities[gameplay->player_entity_index];

    for (int i = 0; i < gameplay->entity_count; i++) {
        Entity* entity = &gameplay->entities[i];
        if (!entity->active) {
            continue;
        }

        if (entity->dead) {
            entity->physics.velocity.x = 0.0f;
            entity->physics.velocity.z = 0.0f;
            entity->fall_amount += delta_time / 0.3f;
            if (entity->fall_amount > 1.0f) entity->fall_amount = 1.0f;
            continue;
        }

        if (entity->controller.type == CONTROLLER_PLAYER) {
            update_player_controller(entity, input, camera);
        } else if (entity->controller.type == CONTROLLER_AI) {
            update_ai_controller(entity, player);
        }

        update_movement(entity);
    }

    spear_update(player, gameplay->entities, gameplay->entity_count, camera, colliders, delta_time);
}
