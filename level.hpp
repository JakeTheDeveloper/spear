#pragma once

#include "game.hpp"

bool level_load(Game* game, const char* path);
bool level_save(const Game* game, const char* path);
