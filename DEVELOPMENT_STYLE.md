# Development Style

This document captures the coding and architecture preferences established while prototyping this project.

## General Approach

- Prefer the simplest implementation that solves the current problem and can be extended later.
- Do not build abstractions for hypothetical future requirements.
- Favor C-style C++: plain structs, free functions, fixed arrays, direct data access, and explicit control flow.
- Do not use smart pointers unless a concrete ownership problem requires them.
- Data-oriented design is the guiding philosophy, but do not split data into separate arrays without a demonstrated access-pattern or performance benefit.
- Ask when behavior or intent is genuinely ambiguous instead of choosing a complex interpretation.

## Abstraction

- Use raylib types and functions directly while raylib is the active backend.
- Do not sandbox raylib behind backend-neutral adapter types before another renderer is actually needed.
- Avoid wrappers that only rename or forward an existing operation.
- Avoid primitive type aliases such as `EntityId` when they provide no type safety and only hide the underlying type.
- Add indirection only when there is a concrete need, such as different lifetimes, dense subsystem iteration, stale-handle detection, serialization, or networking.

## Functions

- Keep related logic together until a function becomes difficult to read.
- Break large update functions into a small number of substantial, clearly named stages.
- Do not create tiny single-use helpers merely to reduce line count.
- Prefer names that state the actual responsibility, such as `update_movement()`, `update_held_spear()`, and `update_flying_spear()`.
- Keep `main.cpp` focused on initialization and subsystem update order. Gameplay-specific behavior belongs in gameplay functions.

## Entities And Controllers

- Use a straightforward fixed array of concrete `Entity` structs at the current scale.
- Store the controller and physics body directly on `Entity`; do not make an entity an ID bag with unnecessary lookup indirection.
- A controller stores current intent, such as movement, aiming, jumping, throwing, and recalling.
- Do not add a separate `EntityCommand` structure when the controller already carries the same intent.
- Controllers describe what an entity wants to do. Shared gameplay and physics systems apply the rules and execute that intent.
- Only update systems that apply to an entity. For example, update the player spear once for the player rather than running spear logic for every entity in anticipation of future use.

## Data Ownership

- Put data with the concept it describes.
- `PhysicsBody` owns physical state such as position, velocity, collision radius, and collision height.
- Gameplay perception values such as `awareness_radius` belong on the entity, not on physics and not necessarily on the controller.
- Camera state owns camera yaw, pitch, lean, position, FOV, and derived view vectors.
- Gameplay may consume camera orientation for camera-relative player controls, but camera state should not be stored in general gameplay state.
- Keep transient values local when possible. Do not retain duplicate state or unnecessary temporary variables.

## Subsystem Boundaries

- Keep input, camera, gameplay, physics, and rendering responsibilities recognizable and separate.
- Separation should make code easier to follow, not introduce adapter layers or excessive plumbing.
- Input samples hardware state.
- Controllers translate player input or AI decisions into entity intent.
- Gameplay applies movement, weapon, combat, and state-transition rules.
- Physics updates physical bodies and resolves collisions.
- Rendering reads current state and draws it.
- Derive information from data already passed to a subsystem instead of adding redundant parameters. For example, rendering can locate the player in the entity collection.

## Build Workflow

- Preserve the build and run command pattern used by `~/code/ray` so existing editor keybindings continue to work.
- Linux entry points are `build.sh` and `run.sh`, producing `build/linux/game`.
- Windows entry points are `build.ps1` and `run.ps1`, producing `build/windows/game.exe`.
- Keep generated build output out of version control.

## When To Generalize

Generalize only after a real requirement appears. Examples include:

- Split physics into dense arrays when entity count or profiling shows iteration cost matters.
- Introduce stable entity handles when entities are frequently created, destroyed, persisted, or referenced across lifetimes.
- Separate optional components when most entities do not carry them and the distinction has practical value.
- Add a rendering abstraction when a second rendering backend is actively being implemented.
