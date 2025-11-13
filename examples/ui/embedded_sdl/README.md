# AUI + SDL3 Example

<!-- aui:example ui -->
This code demonstrates how to integrate the AUI Framework with SDL3 to create a window with OpenGL rendering. 

## Overview

This application creates a simple GUI window using SDL3 for window management and input handling, while AUI Framework handles the UI components and OpenGL rendering. The result is a window displaying "Hello, World!" text and a clickable button.

---

## Core Components

### EmbedRenderingContext

<!-- aui:snippet examples/ui/embedded_sdl/src/main.cpp EmbedRenderingContext -->

**Purpose:** This structure acts as a bridge between AUI's rendering system and the SDL3/OpenGL environment.

**Key responsibilities:**
- **m_renderer**: Holds the OpenGL renderer instance
- **beginPaint()**: Prepares the framebuffer for drawing by binding it and clearing it with white color
- **endPaint()**: Finalizes the drawing operations
- **renderer()**: Provides access to the OpenGL renderer

---

### EmbedWindow

<!-- aui:snippet examples/ui/embedded_sdl/src/main.cpp EmbedWindow -->

**Purpose:** Represents the main application window and manages the connection between SDL3 and AUI Framework.

**Key members:**
- **sdl_window**: The SDL3 window handle
- **gl_context**: The OpenGL context for rendering
- **close**: Flag to signal when the application should exit

**Methods:**
- **init()**: Initializes the AUI window system with the rendering context, sets viewport size based on actual pixel dimensions, and configures DPI scaling for high-resolution displays
- **~EmbedWindow()**: Destructor that properly cleans up SDL resources (GL context and window)

---

## Input Handling

### sdlToAInput()

<!-- aui:snippet examples/ui/embedded_sdl/src/main.cpp sdlToAInput -->

**Purpose:** Converts SDL3 key code constants to AUI's AInput::Key format.

### sdlToAPointer()

<!-- aui:snippet examples/ui/embedded_sdl/src/main.cpp sdlToAPointer -->

**Purpose:** Converts SDL3 mouse button constants to AUI's pointer index format.

**Mappings:**
- SDL_BUTTON_LEFT → Left mouse button
- SDL_BUTTON_MIDDLE → Middle mouse button
- SDL_BUTTON_RIGHT → Right mouse button

---

### handleSDLEvent()

<!-- aui:snippet examples/ui/embedded_sdl/src/main.cpp handleSDLEvent -->

**Purpose:** Central event dispatcher that processes all SDL events and forwards them to the AUI window.

**Handled events:**

- **SDL_EVENT_QUIT**: Sets the close flag to terminate the application
- **SDL_EVENT_WINDOW_RESIZED** / **SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED**: Updates the viewport size when window dimensions change
- **SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED**: Updates DPI ratio when the window moves between displays with different scaling
- **SDL_EVENT_MOUSE_MOTION**: Forwards mouse movement to AUI's pointer tracking
- **SDL_EVENT_MOUSE_BUTTON_DOWN/UP**: Forwards mouse clicks with proper button mapping
- **SDL_EVENT_MOUSE_WHEEL**: Forwards scroll wheel events with position and scroll delta
- **SDL_EVENT_KEY_DOWN/UP/TEXT_INPUT**: Forwards keyboard input to ASurface

---

## Main Application (AUI_ENTRY)

### SDL3 Initialization

<!-- aui:snippet examples/ui/embedded_sdl/src/main.cpp SDL_Init -->

**Purpose:** Initializes SDL3's video subsystem. This must be called before any other SDL functions. If initialization fails, the error is logged and the application exits.

---

### Window Creation

<!-- aui:snippet examples/ui/embedded_sdl/src/main.cpp CreateWindow -->

**Purpose:** Creates the main window with the following properties:
- Title: "AUI + SDL3"
- Initial size: 600×400 pixels
- **SDL_WINDOW_OPENGL**: Enables OpenGL rendering
- **SDL_WINDOW_RESIZABLE**: Allows the user to resize the window
- **SDL_WINDOW_HIGH_PIXEL_DENSITY**: Supports high-DPI displays (Retina, 4K, etc.)

---

### OpenGL Context Setup

<!-- aui:snippet examples/ui/embedded_sdl/src/main.cpp GLContext -->

**Purpose:** Creates an OpenGL context and makes it current for rendering operations. This is necessary before any OpenGL calls can be made.

---

### Renderer Setup

<!-- aui:snippet examples/ui/embedded_sdl/src/main.cpp RendererSetup -->

**Purpose:** Creates the rendering pipeline by:
1. Loads OpenGL function pointers using SDL's procedure address function.
2. Instantiating the OpenGL renderer
3. Creating the rendering context wrapper
4. Linking them together
5. Initializing the AUI window with this rendering setup

---

### UI Content Declaration

<!-- aui:snippet examples/ui/embedded_sdl/src/main.cpp UI -->

**Purpose:** Defines the UI layout using AUI's declarative syntax:
- **Centered**: Centers the content in the window
- **Vertical**: Arranges children vertically
- **Label**: Displays "Hello, World!" text
- **Button**: Creates a clickable button with:
    - Label saying "Click me"
    - onClick handler that logs "Hello world!" when clicked

This is similar to modern declarative UI frameworks like SwiftUI or React.

---

## Main Event Loop

<!-- aui:snippet examples/ui/embedded_sdl/src/main.cpp MainLoop -->

### Event Processing

<!-- aui:snippet examples/ui/embedded_sdl/src/main.cpp EventProcessing -->

**Purpose:** Polls and processes all pending SDL events (mouse, keyboard, window events) in the queue. Non-blocking - returns immediately if no events are available.

---

### Rendering

<!-- aui:snippet examples/ui/embedded_sdl/src/main.cpp Rendering -->

**Purpose:** Conditionally renders the UI only when needed (dirty flag system):
1. **requiresRedraw()**: Checks if the UI needs updating (e.g., after user interaction or animation)
2. **ARenderContext**: Creates a rendering context with no clipping and the OpenGL renderer
3. **window.render()**: Executes the actual drawing operations
4. **SDL_GL_SwapWindow()**: Swaps the front and back buffers to display the newly rendered frame (double buffering)

---

### Frame Rate Limiting

<!-- aui:snippet examples/ui/embedded_sdl/src/main.cpp FrameRateLimit -->

**Purpose:** Limits the frame rate to match the display's refresh rate to:
- Prevent unnecessary CPU/GPU usage
- Synchronize with the monitor's refresh cycle
- Save battery on laptops
- Reduce heat generation

**How it works:**
1. Gets the display mode for the monitor showing the window
2. Calculates delay time based on refresh rate (e.g., 60Hz → ~16ms delay)
3. Sleeps the thread for that duration

**Example:** For a 60Hz display: 1000ms / 60 = ~16.67ms delay per frame

---

## Summary

This code demonstrates a complete integration pattern between SDL3 (for windowing and input) and AUI Framework (for UI rendering and layout). The architecture separates concerns cleanly:

- **SDL3** handles: Window creation, OpenGL context, input events, display management
- **AUI Framework** handles: UI layout, rendering abstraction, event routing to UI components
- **Bridge layer** (EmbedRenderingContext, EmbedWindow): Connects the two systems

The result is a maintainable, cross-platform GUI application with hardware-accelerated rendering and modern declarative UI code.
