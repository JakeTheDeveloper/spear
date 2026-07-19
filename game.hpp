#pragma once

#include "camera.hpp"
#include "editor.hpp"
#include "gameplay.hpp"
#include "physics.hpp"

struct Game {
    const char* level_path;
    StaticColliders static_colliders;
    GameplayState gameplay;
    GameCamera camera;
    EditorState editor;
};

extern Game g_game;
