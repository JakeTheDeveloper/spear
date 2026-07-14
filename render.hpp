#pragma once

#include "camera.hpp"
#include "entity.hpp"
#include "physics.hpp"

void render(const GameCamera* game_camera, const Entity* player, const PhysicsWorld* physics);
