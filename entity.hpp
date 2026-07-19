#pragma once

#include "raylib.h"
#include "spear.hpp"

enum ControllerType {
    CONTROLLER_NONE,
    CONTROLLER_PLAYER,
    CONTROLLER_AI
};

struct Controller {
    ControllerType type;
    Vector3 move_direction;
    Vector3 aim_direction;
    Vector3 aim_up;
    Vector3 aim_right;
    bool sprint_down;
    bool jump_pressed;
    bool throw_down;
    bool throw_released;
    bool melee_pressed;
    bool recall_pressed;
};

struct PhysicsBody {
    Vector3 position;
    Vector3 velocity;
    float collision_radius;
    float collision_height;
    bool grounded;
};

struct Entity {
    bool active;
    bool dead;
    bool stuck_to_wall;
    Vector3 spawn_position;
    PhysicsBody physics;
    Controller controller;
    Spear spear;
    float move_speed;
    float sprint_speed;
    float jump_speed;
    float awareness_radius;
    float fall_amount;
};
