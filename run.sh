#!/bin/bash
set -e
# Check if the app bundle exists
APP_PATH="build/bin/HelloWorld/Debug/HelloWorld.app"
if [ ! -d "$APP_PATH" ]; then
    # Try alternative name (depends on CMakeLists.txt project name)
    APP_PATH="build/Debug/HelloWorld.app"
fi

if [ -d "$APP_PATH" ]; then
    echo "Launching $APP_PATH..."
    open "$APP_PATH"
else
    echo "Error: App bundle not found at $APP_PATH"
    echo "Make sure you have built the project using ./build.sh"
    exit 1
fi
