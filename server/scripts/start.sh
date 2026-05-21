#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SERVER_DIR="$(dirname "$SCRIPT_DIR")"

cd "$SERVER_DIR" || exit 1

if [ ! -f "config/server.conf" ]; then
    echo "Error: config/server.conf not found"
    exit 1
fi

mkdir -p data logs static/avatars static/covers static/recordings static/gifts

if [ ! -f "build/LiveKitServer" ]; then
    echo "Building LiveKitServer..."
    mkdir -p build
    cd build
    cmake .. || exit 1
    make -j$(nproc) || exit 1
    cd ..
fi

echo "Starting LiveKitServer..."
exec ./build/LiveKitServer config/server.conf
