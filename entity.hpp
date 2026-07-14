#pragma once

#include "raylib.h"

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
    bool recall_pressed;
};

struct PhysicsBody {
    Vector3 position;
    Vector3 velocity;
    float radius;
};

struct Spear {
    Vector3 position;
    Vector3 direction;
    float pullback;
    float flight_time;
    bool launched;
    bool stuck;
    bool returning;
};

struct Entity {
    bool active;
    PhysicsBody physics;
    Controller controller;
    Spear spear;
    float move_speed;
    float sprint_speed;
    float jump_speed;
};
