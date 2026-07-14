#!/bin/sh
set -eu

game_path="./build/linux/game"

./build.sh

if command -v pkill >/dev/null 2>&1; then
    pkill -f "$game_path" || true
fi

"$game_path"
