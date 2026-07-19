# gonefishin

Minimal first-person 3D prototype scaffold using raylib.

The current test scene includes a simple AI enemy that chases the player while inside its awareness radius.

## Build and run

```sh
./run.sh
```

`run.sh` builds to `build/linux/game`, stops an already-running instance, and launches it. The `build.sh`, `run.sh`, `build.ps1`, and `run.ps1` entry points match the layout and command pattern used by `~/code/ray`.

Controls: WASD moves, Shift sprints, Space or Delete jumps, left mouse throws the spear, right mouse performs a melee attack, Q recalls it, the mouse looks, and Escape quits. A melee strike against a wall pins the player in place until they jump.

## Structure

- `input.cpp` samples per-frame controls.
- `camera.cpp` owns camera orientation, lean, position, and view basis.
- `entity.hpp` defines entities with embedded controller, physics, and spear state.
- `gameplay.cpp` updates controllers and movement rules for active entities.
- `spear.cpp` updates held, thrown, returning, and melee spear behavior.
- `physics.cpp` updates entity physics bodies and resolves them against static colliders.
- `render.cpp` draws the current gameplay and physics state.
- `main.cpp` owns the frame loop and passes plain data between subsystems.

The code uses raylib types directly. Input, gameplay, physics, and rendering remain separate, but there is no backend abstraction until the prototype needs one.
