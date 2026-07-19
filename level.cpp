#include "level.hpp"

#include "game.hpp"

#include <cstdio>
#include <cstring>

static bool report_level_error(FILE* file, const char* path, int line_number, const char* message) {
    std::fclose(file);
    std::fprintf(stderr, "%s:%d: %s\n", path, line_number, message);
    return false;
}

bool level_load(Game* game, const char* path) {
    FILE* file = std::fopen(path, "r");
    if (!file) {
        std::fprintf(stderr, "Could not open level: %s\n", path);
        return false;
    }

    StaticColliders loaded_colliders = {};
    GameplayState loaded_gameplay = {};
    loaded_gameplay.player_entity_index = -1;

    bool found_player = false;
    char line[512] = {};
    int line_number = 1;

    if (!std::fgets(line, sizeof(line), file)) {
        return report_level_error(file, path, line_number, "Missing version declaration");
    }

    int version = 0;
    if (std::sscanf(line, "version %d", &version) != 1 || version != 1) {
        return report_level_error(file, path, line_number, "Expected supported version declaration");
    }

    while (std::fgets(line, sizeof(line), file)) {
        line_number++;

        char record_type[32] = {};

        if (std::sscanf(line, "%31s", record_type) != 1 || record_type[0] == '#') continue;

        if (std::strcmp(record_type, "box") == 0) {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
            float half_x = 0.0f;
            float half_y = 0.0f;
            float half_z = 0.0f;
            char trailing = 0;

            if (std::sscanf(
                    line,
                    " box %f %f %f %f %f %f %c",
                    &x,
                    &y,
                    &z,
                    &half_x,
                    &half_y,
                    &half_z,
                    &trailing
                ) != 6) {
                return report_level_error(file, path, line_number, "Invalid box record");
            }

            if (half_x <= 0.0f || half_y <= 0.0f || half_z <= 0.0f) {
                return report_level_error(file, path, line_number, "Box half-sizes must be positive");
            }

            StaticColliders* colliders = &loaded_colliders;
            if (colliders->count >= MAX_STATIC_COLLIDERS) {
                return report_level_error(file, path, line_number, "Too many static colliders");
            }

            int collider_index = colliders->count++;
            colliders->position[collider_index] = { x, y, z };
            colliders->half_size[collider_index] = { half_x, half_y, half_z };
            continue;
        }

        if (std::strcmp(record_type, "entity") == 0) {
            char entity_type[32] = {};
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
            char trailing = 0;

            if (std::sscanf(
                    line,
                    " entity %31s %f %f %f %c",
                    entity_type,
                    &x,
                    &y,
                    &z,
                    &trailing
                ) != 4) {
                return report_level_error(file, path, line_number, "Invalid entity record");
            }

            GameplayState* gameplay = &loaded_gameplay;
            if (gameplay->entity_count >= MAX_ENTITIES) {
                return report_level_error(file, path, line_number, "Too many entities");
            }

            Entity* entity = &gameplay->entities[gameplay->entity_count];

            if (std::strcmp(entity_type, "player") == 0) {
                if (found_player) {
                    return report_level_error(file, path, line_number, "Level contains more than one player");
                }

                entity->active = true;
                entity->spawn_position = { x, y, z };
                entity->physics.position = { x, y, z };
                entity->physics.collision_radius = 0.35f;
                entity->physics.collision_height = 1.8f;
                entity->spear.stuck_entity_index = -1;
                entity->controller.type = CONTROLLER_PLAYER;
                entity->move_speed = 7.5f;
                entity->sprint_speed = 11.0f;
                entity->jump_speed = 7.0f;

                gameplay->player_entity_index = gameplay->entity_count;
                found_player = true;
            } else if (std::strcmp(entity_type, "enemy") == 0) {
                entity->active = true;
                entity->spawn_position = { x, y, z };
                entity->physics.position = { x, y, z };
                entity->physics.collision_radius = 0.45f;
                entity->physics.collision_height = 1.8f;
                entity->controller.type = CONTROLLER_AI;
                entity->move_speed = 3.5f;
                entity->awareness_radius = 12.0f;
            } else {
                return report_level_error(file, path, line_number, "Unknown entity type");
            }

            gameplay->entity_count++;
            continue;
        }

        return report_level_error(file, path, line_number, "Unknown level record");
    }

    if (std::ferror(file)) {
        return report_level_error(file, path, line_number, "Failed while reading level");
    }

    std::fclose(file);

    if (!found_player) {
        std::fprintf(stderr, "%s: Missing player entity\n", path);
        return false;
    }

    game->static_colliders = loaded_colliders;
    game->gameplay = loaded_gameplay;
    return true;
}

bool level_save(const Game* game, const char* path) {
    FILE* file = std::fopen(path, "w");
    if (!file) {
        std::fprintf(stderr, "Could not open level for writing: %s\n", path);
        return false;
    }

    std::fprintf(file, "version 1\n\n");
    std::fprintf(file, "# center_x center_y center_z half_x half_y half_z\n");

    const StaticColliders* colliders = &game->static_colliders;
    for (int i = 0; i < colliders->count; i++) {
        Vector3 position = colliders->position[i];
        Vector3 half_size = colliders->half_size[i];

        std::fprintf(
            file,
            "box %.9g %.9g %.9g %.9g %.9g %.9g\n",
            position.x,
            position.y,
            position.z,
            half_size.x,
            half_size.y,
            half_size.z
        );
    }

    std::fprintf(file, "\n# type position_x position_y position_z\n");

    const GameplayState* gameplay = &game->gameplay;
    for (int i = 0; i < gameplay->entity_count; i++) {
        const Entity* entity = &gameplay->entities[i];
        if (!entity->active) {
            continue;
        }

        const char* entity_type = nullptr;
        if (entity->controller.type == CONTROLLER_PLAYER) {
            entity_type = "player";
        } else if (entity->controller.type == CONTROLLER_AI) {
            entity_type = "enemy";
        } else {
            std::fprintf(
                stderr,
                "Cannot save unknown entity type at index %d\n",
                i
            );
            std::fclose(file);
            return false;
        }

        std::fprintf(
            file,
            "entity %s %.9g %.9g %.9g\n",
            entity_type,
            entity->spawn_position.x,
            entity->spawn_position.y,
            entity->spawn_position.z
        );
    }

    bool write_failed = std::ferror(file);
    bool close_failed = std::fclose(file) != 0;
    if (write_failed || close_failed) {
        std::fprintf(stderr, "Failed while writing level: %s\n", path);
        return false;
    }

    return true;
}
