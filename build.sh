#!/bin/bash
set -e
echo "Building for macOS (Apple Silicon)..."
# Configure CMake for Xcode
cmake -S . -B build -GXcode -DCMAKE_OSX_ARCHITECTURES=arm64

# Build the Debug configuration
cmake --build build --config Debug
echo "Build complete."
