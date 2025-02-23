#!/bin/bash

if [ "$#" -eq 0 ]; then
    echo "Usage: $0 [debug|release]"
    exit 1
fi

# Normalize input to lowercase
config=$(echo "$1" | tr '[:upper:]' '[:lower:]')

if [ "$config" = "debug" ]; then
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
elif [ "$config" = "release" ]; then
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
else
    echo "Invalid build configuration. Use 'debug' or 'release'."
    exit 1
fi
