# Axmol-Rive Integration: Project Goals & Status

This document chronicles the journey of integrating the **Rive C++ Runtime** into the **Axmol Engine**. It serves as a technical retrospective, an implementation guide, and a roadmap for future development.

## 1. Project Overview

The goal was to create a robust, high-performance integration that allows Rive animations (`.riv` files) to run natively within an Axmol application. This involves bridging Rive's renderer-agnostic C++ runtime with Axmol's scene graph and rendering pipeline (`DrawNode`, `Node`, `ClippingNode`, etc.).

## 2. Implementation Journey

### Phase 1: Project Setup & Dependencies
*   **Axmol Project**: Created a standard "Hello World" Axmol project for macOS (Apple Silicon support).
*   **Rive Runtime**: Integrated the Rive C++ runtime source code directly into the project tree.
*   **Dependencies**: Added `earcut.hpp` (for triangulation) and managed `libtess2` sources.
*   **Build System**: Configured `CMakeLists.txt` to compile Rive sources, define necessary macros (like `_RIVE_INTERNAL_`), and handle platform-specifics.

### Phase 2: The Rendering Bridge (`AxmolRenderer`)
Rive requires a concrete implementation of its `Renderer` class. We created `AxmolRenderer` (inheriting from `rive::TessRenderer`):
*   **Tessellation**: Used Rive's `TessRenderPath` to convert vector paths into triangles.
*   **Drawing**: Initially used `ax::DrawNode` to render these triangles.
*   **Coordinate Systems**:
    *   Rive uses Top-Left origin (Y-down).
    *   Axmol uses Bottom-Left origin (Y-up).
    *   **Solution**: The container node is vertically flipped (`setScaleY(-1.0f)`) and positioned at the top of the screen to align the coordinate systems transparently.

### Phase 3: Solving Rendering Artifacts
*   **"Wireframe" / Missing Fills**: Caused by improper triangulation handling or missing shaders.
*   **Fills Out of Place**: The `TessRenderPath` logic resets coordinates to local space for single paths but uses parent-relative for containers. We implemented a robust transform pipeline in `drawPath` to ensure all vertices are correctly transformed to world space before rendering.
*   **Memory Leaks (Unbounded Vertex Growth)**: `TessRenderPath` caches geometry but doesn't automatically clear it for external consumers. We implemented a custom `AxmolRenderPath::prune` method to detect re-triangulation and discard old geometry, keeping memory usage stable.
*   **Clipping**: Rive relies heavily on clipping. We implemented `clipPath` by managing a stack of `ax::ClippingNode`s. We refactored `AxmolRenderer` to manage its own scene graph (`_containerStack`) rather than drawing to a single persistent `DrawNode`. Each frame, `startFrame()` resets the scene graph to handle the immediate-mode nature of Rive rendering.

### Phase 4: Visual Fidelity (Gradients)
*   **Flat Colors**: Initial implementation used a single average color per triangle, resulting in blocky rendering for gradients.
*   **Smooth Gradients**: We implemented `AxmolRenderShader` (wrapping `LinearGradient` and `RadialGradient`).
*   **Per-Vertex Coloring**: Instead of a single color, we now calculate the exact gradient color for **each vertex** of the generated triangles using the shader logic. We switched to `_drawNode->drawColoredTriangle`, allowing Axmol to interpolate colors smoothly across the face.

### Phase 5: Animation & Interaction
*   **State Machines**: Implemented support for `rive::StateMachineInstance`, allowing complex interactive logic designed in the Rive editor.
*   **Fallback**: Added fallback logic to play simple linear animations (`animationAt(0)`) if no State Machine is found.
*   **Input Handling**: Mapped Axmol touch events (`Began`, `Moved`, `Ended`) to Rive input events (`pointerDown`, `pointerMove`, `pointerUp`), correctly transforming coordinates through the alignment matrix.
*   **Artboard Switching**: Added a debug feature to cycle through multiple artboards in a file by clicking the top-right corner.

## 3. Technical Concepts Learned

*   **Rive Architecture**: Understanding `Artboard`, `File`, `Animation`, and the separation between the "Model" (Data) and the "Renderer" (View).
*   **Tessellation**: How vector paths are broken down into triangles (Monotone, Ear Clipping) and how Rive's `TessRenderer` manages this cache.
*   **Axmol Rendering**: Deep dive into `DrawNode`, `ClippingNode`, and `TriangleCommand`.
*   **Coordinate Spaces**: The importance of consistent matrix transformations when bridging two different 2D engines.

## 4. Current Status & Known Limitations

The current integration is **stable** and **functional**:
*   ✅ No crashes or memory leaks.
*   ✅ Smooth 60fps animation.
*   ✅ Interactive input support.
*   ✅ Basic clipping and gradients support.

**However, there is still work to do:**
*   **Complex Blending**: `BlendMode`s (Screen, Overlay, etc.) are currently ignored or approximated.
*   **Advanced Clipping**: Nested clipping or complex stencil operations might still have edge cases.
*   **Stroke Caps/Joins**: While passed to the extruder, complex stroke styles might need visual verification.
*   **Performance**: We transform vertices on the CPU. A full GPU-based approach (custom shaders for the whole path) would be faster for very complex scenes.
*   **Text & Images**: `drawImage` is currently a stub. Rive text rendering needs a font implementation.

## 5. Future Roadmap

The vision is to turn this into a first-class Axmol extension library.

1.  **Image Support**: Implement `drawImage` to render textured meshes (skins, bitmaps).
2.  **Text Support**: Integrate Rive's text engine.
3.  **Blend Modes**: Map Rive blend modes to OpenGL/Axmol blend functions correctly.
4.  **Optimization**: Move vertex transformation to a vertex shader.
5.  **Componentization**: Wrap `AxmolRenderer` into a clean `ax::RiveNode` component that acts like any other Axmol Node.

We will tackle these issues one by one, tracking artifacts in specific artboards and resolving them systematically!
