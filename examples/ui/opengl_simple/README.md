# OpenGL Example

<!-- aui:example ui -->
Demonstrates how to integrate custom OpenGL rendering with AUI Framework.

It shows a simple triangle rendered using OpenGL ES 2.0 shaders while maintaining compatibility with AUI's rendering
system.

## Features

* Custom OpenGL rendering within AUI view
* FPS counter display
* Proper OpenGL state management
* Basic shader compilation and usage
* Vertex buffer creation and rendering

## Implementation Details

The example consists of a custom renderer (`MyRenderer`) that:

1. Sets up basic vertex and fragment shaders
2. Creates and manages OpenGL resources (shaders, program, VBO)
3. Renders a colored triangle
4. Calculates and displays FPS
5. Properly cleans up OpenGL state to maintain compatibility with AUI

The rendering is performed in real-time with continuous updates, demonstrating smooth integration between custom OpenGL
code and AUI's widget system.
