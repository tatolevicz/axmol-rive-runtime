# Axmol C++ macOS Desktop App (HelloWorld)

## Project Overview
This project is a **macOS Desktop Application** built using the **Axmol Engine** (a high-performance, cross-platform C++ game engine, forked from Cocos2d-x).

The primary goal of this application is to serve as a foundational "Hello World" setup and to demonstrate custom 2D drawing capabilities using Axmol's `DrawNode` API.

## Key Features
*   **Cross-Platform Architecture**: Built on Axmol, allowing for potential deployment to mobile (iOS/Android), desktop (Windows/Linux/macOS), and web (WASM).
*   **Custom Drawing**: Implements various geometric primitives and shapes directly in `Source/MainScene.cpp` using the `DrawNode` class.
    *   **Primitives**: Dots, Lines, Rectangles (filled/outline), Circles (filled/outline).
    *   **Complex Shapes**: Polygons (convex/concave), Bezier curves (quadratic/cubic), and Cardinal Splines.
*   **Apple Silicon Support**: Configured specifically to build for macOS arm64 architecture (M1/M2/M3).

## Project Structure
The project follows the standard Axmol directory layout:

*   **`Source/`**: Contains the cross-platform C++ source code.
    *   `AppDelegate.cpp/h`: Handles application lifecycle events (launch, background, foreground).
    *   `MainScene.cpp/h`: The main scene of the application where the drawing logic resides.
*   **`proj.ios_mac/`**: Platform-specific project configuration for macOS and iOS.
*   **`CMakeLists.txt`**: The CMake build configuration file defining dependencies and build targets.
*   **`Content/`**: Assets (images, fonts, audio) used by the application.

## How to Build and Run

This project includes custom helper scripts to streamline the build process for macOS users, specifically targeting Apple Silicon.

### 1. Build the Project
Use the `build.sh` script to compile the application.

```bash
./build.sh
```

**What this script does:**
*   Configures the project using `cmake`.
*   Generates an Xcode project structure (`-GXcode`).
*   Explicitly targets Apple Silicon (`-DCMAKE_OSX_ARCHITECTURES=arm64`).
*   Builds the `Debug` configuration.

### 2. Run the Application
Use the `run.sh` script to launch the application.

```bash
./run.sh
```
Important Instruction — Never Perform Automatic Builds

You must never run, simulate, assume, or initiate any build process on your own.
Under no circumstances should you trigger, describe step-by-step builds, or start compiling unless I explicitly ask you with a clear command such as “build now”, “run build”, or “execute the build”.

If a situation arises where you think a build might be needed, you must always ask me first rather than doing anything automatically.

Reason:
To avoid polluting the conversation context, wasting tokens, or performing unnecessary or unintended operations.

Your responsibilities:

You may explain concepts, answer questions, clarify configurations, or review scripts.

You must not build, run, or simulate building unless I clearly and explicitly instruct you to do so.

If I ask something ambiguous that could imply a build, always respond with:
“Do you want me to run the build?”

Summary:

❌ Do NOT build automatically

❌ Do NOT assume I want a build

❌ Do NOT run build steps unless explicitly told

✔️ ALWAYS ask before building

✔️ Focus on explanations, code reviews, or textual guidance unless explicitly told otherwise

**What this script does:**
*   Locates the compiled `.app` bundle within the build directory (typically found at `build/bin/HelloWorld/Debug/HelloWorld.app`).
*   Launches the application using the `open` command.

## Requirements
*   **Axmol Engine**: Must be installed and properly set up (environment variables loaded).
*   **CMake**: Version 3.28.1 or higher.
*   **Xcode**: Version 14.2 or higher.
*   **PowerShell**: Version 7+ (required for internal Axmol setup scripts).
