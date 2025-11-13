# AUI + SDL3 Example

This code demonstrates how to integrate the AUI Framework with SDL3 to create a window with OpenGL rendering. 

## Overview

This application creates a simple GUI window using SDL3 for window management and input handling, while AUI Framework handles the UI components and OpenGL rendering. The result is a window displaying "Hello, World!" text and a clickable button.

---

## Core Components

### EmbedRenderingContext

```cpp
struct EmbedRenderingContext : IRenderingContext
```

**Purpose:** This structure acts as a bridge between AUI's rendering system and the SDL3/OpenGL environment.

**Key responsibilities:**
- **m_renderer**: Holds the OpenGL renderer instance
- **beginPaint()**: Prepares the framebuffer for drawing by binding it and clearing it with white color
- **endPaint()**: Finalizes the drawing operations
- **renderer()**: Provides access to the OpenGL renderer

---

### EmbedWindow

```cpp
struct EmbedWindow : AGLEmbedContext
```

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

### sdlToAPointer()

```cpp
static auto sdlToAPointer(Uint8 button) -> APointerIndex
```

**Purpose:** Converts SDL3 mouse button constants to AUI's pointer index format.

**Mappings:**
- SDL_BUTTON_LEFT → Left mouse button
- SDL_BUTTON_MIDDLE → Middle mouse button
- SDL_BUTTON_RIGHT → Right mouse button

---

### handleSDLEvent()

```cpp
void handleSDLEvent(SDL_Event* event, EmbedWindow& window)
```

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

```cpp
if (!SDL_Init(SDL_INIT_VIDEO)) {
    ALogger::err("SDL3") << SDL_GetError();
    return 1;
}
```

**Purpose:** Initializes SDL3's video subsystem. This must be called before any other SDL functions. If initialization fails, the error is logged and the application exits.

---

### Window Creation

```cpp
window.sdl_window = SDL_CreateWindow("AUI + SDL3", 600, 400, 
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
```

**Purpose:** Creates the main window with the following properties:
- Title: "AUI + SDL3"
- Initial size: 600×400 pixels
- **SDL_WINDOW_OPENGL**: Enables OpenGL rendering
- **SDL_WINDOW_RESIZABLE**: Allows the user to resize the window
- **SDL_WINDOW_HIGH_PIXEL_DENSITY**: Supports high-DPI displays (Retina, 4K, etc.)

---

### OpenGL Context Setup

```cpp
window.gl_context = SDL_GL_CreateContext(window.sdl_window);
SDL_GL_MakeCurrent(window.sdl_window, window.gl_context);
```

**Purpose:** Creates an OpenGL context and makes it current for rendering operations. This is necessary before any OpenGL calls can be made.

---

### OpenGL Loading

```cpp
if (!OpenGLRenderer::loadGL((OpenGLRenderer::GLLoadProc)SDL_GL_GetProcAddress)) {
    ALogger::err("OpenGLRenderer") << "Failed to load GL";
    return 1;
}
```

**Purpose:** Loads OpenGL function pointers using SDL's procedure address function. This is required because OpenGL functions must be loaded at runtime on most platforms.

---

### Renderer Setup

```cpp
auto renderer = std::make_shared<OpenGLRenderer>();
auto rendering_context = std::make_unique<EmbedRenderingContext>();
rendering_context->m_renderer = renderer;
window.init(std::move(rendering_context));
```

**Purpose:** Creates the rendering pipeline by:
1. Instantiating the OpenGL renderer
2. Creating the rendering context wrapper
3. Linking them together
4. Initializing the AUI window with this rendering setup

---

### UI Content Declaration

```cpp
window.setContainer(Centered {
    Vertical {
        Label { "Hello, World!" },
        Button {
            .content = Label { "Click me" },
            .onClick = [] {
                ALogger::info("Test") << "Hello world!";
            }
        }
    }
});
```

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

```cpp
while (!window.close) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        handleSDLEvent(&event, window);
    }
    
    if (window.requiresRedraw()) {
        ARenderContext render_context {
            .clippingRects = {},
            .render = *renderer,
        };
        window.render(render_context);
        SDL_GL_SwapWindow(window.sdl_window);
    }
    
    const SDL_DisplayMode* dm = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window.sdl_window));
    Sint32 refresh_ms = static_cast<Sint32>(1000.0f / dm->refresh_rate);
    SDL_Delay(refresh_ms);
}
```

### Event Processing

```cpp
while (SDL_PollEvent(&event)) {
    handleSDLEvent(&event, window);
}
```

**Purpose:** Polls and processes all pending SDL events (mouse, keyboard, window events) in the queue. Non-blocking - returns immediately if no events are available.

---

### Rendering

```cpp
if (window.requiresRedraw()) {
    ARenderContext render_context {
        .clippingRects = {},
        .render = *renderer,
    };
    window.render(render_context);
    SDL_GL_SwapWindow(window.sdl_window);
}
```

**Purpose:** Conditionally renders the UI only when needed (dirty flag system):
1. **requiresRedraw()**: Checks if the UI needs updating (e.g., after user interaction or animation)
2. **ARenderContext**: Creates a rendering context with no clipping and the OpenGL renderer
3. **window.render()**: Executes the actual drawing operations
4. **SDL_GL_SwapWindow()**: Swaps the front and back buffers to display the newly rendered frame (double buffering)

---

### Frame Rate Limiting

```cpp
const SDL_DisplayMode* dm = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window.sdl_window));
Sint32 refresh_ms = static_cast<Sint32>(1000.0f / dm->refresh_rate);
SDL_Delay(refresh_ms);
```

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
