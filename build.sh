#!/bin/sh
set -eu

build_dir="build/linux"
obj_dir="$build_dir/obj"

mkdir -p "$obj_dir"

headers=""
for header in ./*.hpp; do
    if [ -f "$header" ]; then
        headers="$headers $header"
    fi
done

objects=""
needs_link=0

if [ ! -f "$build_dir/game" ]; then
    needs_link=1
fi

for source in ./*.cpp; do
    base="$(basename "$source" .cpp)"
    object="$obj_dir/$base.o"
    objects="$objects $object"

    needs_compile=0

    if [ ! -f "$object" ] || [ "$source" -nt "$object" ]; then
        needs_compile=1
    fi

    for header in $headers; do
        if [ "$header" -nt "$object" ]; then
            needs_compile=1
        fi
    done

    if [ "$needs_compile" -eq 1 ]; then
        clang++ -std=c++17 -O0 -Iinclude -c "$source" -o "$object"
        needs_link=1
    fi

    if [ -f "$build_dir/game" ] && [ "$object" -nt "$build_dir/game" ]; then
        needs_link=1
    fi
done

if [ -f libs/libraylib.a ] && [ -f "$build_dir/game" ] && [ libs/libraylib.a -nt "$build_dir/game" ]; then
    needs_link=1
fi

if [ "$needs_link" -eq 1 ]; then
    clang++ $objects -o "$build_dir/game" \
        -Llibs \
        -lraylib \
        -lm \
        -Wl,-rpath,'$ORIGIN/../../libs'
fi
