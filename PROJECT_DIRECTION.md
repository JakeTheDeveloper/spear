# Project Direction

This document summarizes the current architecture, important design decisions, editor workflow, and intended direction for future development.

Read `DEVELOPMENT_STYLE.md` first for the project's coding and architecture standards.

## Project Goal

The immediate goal is to prove out one complete playable level with multiple vertical elevations.

The project should support:

- Loading level layouts from disk.
- Building layouts with an in-game blockout editor.
- Static box geometry at arbitrary elevations.
- Player and enemy spawn points.
- Traversal using jumping and spear-based wall attachment.
- Spear throwing, melee combat, and recall.
- Saving edited levels back to disk.

Prefer concrete functionality needed by this level over generalized engine systems.

## Collaboration Workflow

Before changing code:

1. Show the exact proposed code chunk or diff.
2. Wait for review.
3. The user replies `a` to approve or `r` to reject.
4. Apply only the approved chunk.
5. Build and check the diff after each meaningful phase.

Do not combine unreviewed changes into an approved chunk.

## Runtime State

The executable has one active global game:

```cpp
extern Game g_game;
```

`g_game` is declared in `game.hpp` and defined in `game.cpp`.

`Game` currently owns:

- The active level path.
- Static colliders.
- Gameplay and entity state.
- Camera state.
- Editor state.

Lower-level systems should still receive explicit pointers to the data they use. They should not access `g_game` directly.

This keeps global state at the composition level while preserving visible subsystem dependencies.

## Main Loop

`main.cpp` owns top-level update order.

At startup:

1. Set the active level path.
2. Load the level into `g_game`.
3. Initialize camera and editor runtime state.
4. Position the camera at the loaded player.

Each frame:

1. Sample input.
2. Toggle editor mode when F1 is pressed.
3. Run `editor_update()` when editing.
4. Otherwise run camera, gameplay, and physics updates.
5. Render the current state.

Gameplay and physics are paused in editor mode.

## Level Files

Level loading and saving live in `level.hpp` and `level.cpp`.

The public API is:

```cpp
bool level_load(Game* game, const char* path);
bool level_save(const Game* game, const char* path);
```

The `Game*` is explicit so tests can load into their own game state.

Loading is atomic. It parses into temporary collider and gameplay collections, then replaces the corresponding game members only after the entire file succeeds.

The current level is:

```text
levels/test.level
```

### Level Format

The first line must be:

```text
version 1
```

Static boxes use center position and half-size:

```text
# center_x center_y center_z half_x half_y half_z
box 0 -0.5 0 35 0.5 35
```

Entities use a type name and spawn position:

```text
# type position_x position_y position_z
entity player 0 0 8
entity enemy 4 0 0
```

Supported entity types are currently:

- `player`
- `enemy`

Entity types are not yet represented by a dedicated runtime enum. Loading currently creates entities based on the type string.

## Entity State

Entities are stored in a fixed array inside `GameplayState`.

Important entity data includes:

- Controller intent.
- Physics body.
- Spear state.
- Runtime position and velocity.
- Authored spawn position.
- Movement values.
- AI awareness.
- Death and falling state.
- Wall-attachment state.

`spawn_position` is separate from `physics.position`.

Gameplay can modify `physics.position` without changing the authored spawn saved to disk.

## Physics

Physics uses a fixed array of axis-aligned static boxes:

```cpp
struct StaticColliders {
    int count;
    Vector3 position[MAX_STATIC_COLLIDERS];
    Vector3 half_size[MAX_STATIC_COLLIDERS];
};
```

`PhysicsWorld` was removed because it only wrapped `StaticColliders` and added no useful behavior or ownership.

Entity bodies use:

- A horizontal collision radius.
- A vertical collision height.
- A feet position.
- Velocity.
- Grounded state.

Physics currently supports:

- Gravity.
- Static floors at arbitrary elevations.
- Landing on boxes.
- Ceiling collision.
- Height-aware wall collision.
- Sliding along walls through separate X and Z resolution.
- Wall attachment that disables gravity until jumping.

Physics does not currently support:

- Slopes.
- Stairs or automatic step-up.
- Moving platforms.
- Entity-to-entity collision.
- Rotated colliders.

### Line Intersection

Static line queries return a result struct:

```cpp
struct LineColliderIntersection {
    bool did_intersect;
    int collider_index;
    Vector3 position;
    Vector3 normal;
};
```

The query is:

```cpp
LineColliderIntersection physics_line_intersects_colliders(
    const StaticColliders* colliders,
    Vector3 start,
    Vector3 end
);
```

It returns the nearest static collider intersection.

The spear uses it for:

- Throw aiming.
- Flying collision.
- Melee wall detection.
- Wall-attachment surface normals.

The editor uses the collider index for selection.

## Spear

Spear data and behavior live in `spear.hpp` and `spear.cpp`.

`gameplay.cpp` calls one public spear update stage:

```cpp
spear_update(...);
```

Internal spear behavior includes:

- Held spear charging.
- Throwing.
- Gravity during flight.
- Entity and static collision.
- Sticking into enemies and geometry.
- Recall.
- Melee attacks.
- Melee wall attachment.
- Catch animation triggering.

The spear is hidden while editor mode is active.

## Editor

Editor state and behavior live in `editor.hpp` and `editor.cpp`.

`main.cpp` only chooses between:

```cpp
editor_update(...);
```

and the normal gameplay update path.

Individual editor actions should remain private to `editor.cpp`.

### Editor Controls

General:

- F1: enter or leave editor mode.
- Left mouse: select the collider under the crosshair.
- Insert: add a one-unit box.
- Delete: remove the selected box.
- Ctrl+S: save the active level.
- Ctrl+L: reload the active level.

Camera mode:

- WASD: free-fly movement.
- Space: move up.
- E: move down.
- Shift: move faster.
- Mouse: look.

Transform modes:

- M: toggle move mode.
- R: toggle resize mode.
- Press the active mode key again to return to camera mode.
- WASD: adjust X/Z.
- Q/E: adjust Y.
- Shift: use a larger transform step.

Normal transform steps are `0.25` units. Shift uses `1.0` unit.

Resize mode modifies half-size symmetrically and clamps each half-size axis to at least `0.25`.

### Box Placement

Insert creates a box with this half-size:

```cpp
{ 0.5f, 0.5f, 0.5f }
```

When aiming at existing geometry, the new box is placed against the hit surface. Otherwise it is placed five units ahead of the camera.

### Current Editor Limitations

The editor currently manipulates static boxes only.

It does not yet support:

- Player spawn visualization or movement.
- Enemy spawn visualization, creation, movement, or deletion.
- Undo and redo.
- Copy and paste.
- Rotated geometry.
- Mouse gizmos.
- Multiple open levels.
- Save confirmation or error UI.

Editor code is always compiled. Add a compile-time editor flag only when a real release build exists.

## Rendering

Rendering reads current game and physics state.

Static colliders are also used as visible blockout geometry for now. This is intentional while collision boxes and level visuals are the same thing.

Separate visual geometry from physics colliders only when the project needs geometry that differs from collision.

Editor rendering currently adds:

- Selected-collider highlighting.
- Editor mode text.
- Current camera, move, or resize mode.
- Editor control hints.

## File Structure

Important source files:

- `main.cpp`: startup and top-level update order.
- `game.hpp`, `game.cpp`: active game aggregation and global definition.
- `level.hpp`, `level.cpp`: level loading and saving.
- `editor.hpp`, `editor.cpp`: blockout editor state and behavior.
- `input.hpp`, `input.cpp`: per-frame hardware input sampling.
- `camera.hpp`, `camera.cpp`: camera orientation and feedback.
- `gameplay.hpp`, `gameplay.cpp`: controllers, AI, movement, and update order.
- `spear.hpp`, `spear.cpp`: spear and melee behavior.
- `physics.hpp`, `physics.cpp`: entity movement and static collision.
- `render.hpp`, `render.cpp`: gameplay and editor drawing.
- `entity.hpp`: entity, controller, and physics-body data.
- `levels/test.level`: current authored level layout.

## Build Workflow

Linux:

```sh
./build.sh
./run.sh
```

Output:

```text
build/linux/game
```

Windows:

```powershell
./build.ps1
./run.ps1
```

Output:

```text
build/windows/game.exe
```

Both build scripts automatically compile root-level `.cpp` files.

## Next Direction

The next editor milestone is entity spawn editing:

- Render player and enemy spawn markers in editor mode.
- Select spawn markers independently from colliders.
- Move spawn markers.
- Add and delete enemy spawns.
- Keep exactly one player spawn.
- Save spawn changes through `spawn_position`.

After spawn editing, build one complete multi-height level and test the full loop:

- Load.
- Traverse.
- Wall attach.
- Fight enemies.
- Reach an end condition.
- Edit.
- Save.
- Reload.
