/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <AUI/Common/AUtf8.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AGLEmbedContext.h>
#include <AUI/GL/OpenGLRenderer.h>
#include <AUI/Util/Declarative/Containers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ATextField.h>
#include <AUI/Thread/AEventLoop.h>
#include <SDL3/SDL.h>
#include <unordered_map>

/// [sdlToAInput]
static auto sdlToAInput(Uint32 key) -> AInput::Key {
    static const std::unordered_map<SDL_Keycode, AInput::Key> mapping = {
        {SDLK_A, AInput::Key::A},
        {SDLK_B, AInput::Key::B},
        {SDLK_C, AInput::Key::C},
        {SDLK_D, AInput::Key::D},
        {SDLK_E, AInput::Key::E},
        {SDLK_F, AInput::Key::F},
        {SDLK_G, AInput::Key::G},
        {SDLK_H, AInput::Key::H},
        {SDLK_I, AInput::Key::I},
        {SDLK_J, AInput::Key::J},
        {SDLK_K, AInput::Key::K},
        {SDLK_L, AInput::Key::L},
        {SDLK_M, AInput::Key::M},
        {SDLK_N, AInput::Key::N},
        {SDLK_O, AInput::Key::O},
        {SDLK_P, AInput::Key::P},
        {SDLK_Q, AInput::Key::Q},
        {SDLK_R, AInput::Key::R},
        {SDLK_S, AInput::Key::S},
        {SDLK_T, AInput::Key::T},
        {SDLK_U, AInput::Key::U},
        {SDLK_V, AInput::Key::V},
        {SDLK_W, AInput::Key::W},
        {SDLK_X, AInput::Key::X},
        {SDLK_Y, AInput::Key::Y},
        {SDLK_Z, AInput::Key::Z},

        {SDLK_0, AInput::Key::NUM0},
        {SDLK_1, AInput::Key::NUM1},
        {SDLK_2, AInput::Key::NUM2},
        {SDLK_3, AInput::Key::NUM3},
        {SDLK_4, AInput::Key::NUM4},
        {SDLK_5, AInput::Key::NUM5},
        {SDLK_6, AInput::Key::NUM6},
        {SDLK_7, AInput::Key::NUM7},
        {SDLK_8, AInput::Key::NUM8},
        {SDLK_9, AInput::Key::NUM9},

        {SDLK_F1, AInput::Key::F1},
        {SDLK_F2, AInput::Key::F2},
        {SDLK_F3, AInput::Key::F3},
        {SDLK_F4, AInput::Key::F4},
        {SDLK_F5, AInput::Key::F5},
        {SDLK_F6, AInput::Key::F6},
        {SDLK_F7, AInput::Key::F7},
        {SDLK_F8, AInput::Key::F8},
        {SDLK_F9, AInput::Key::F9},
        {SDLK_F10, AInput::Key::F10},
        {SDLK_F11, AInput::Key::F11},
        {SDLK_F12, AInput::Key::F12},
        {SDLK_F13, AInput::Key::F13},
        {SDLK_F14, AInput::Key::F14},
        {SDLK_F15, AInput::Key::F15},
        {SDLK_F16, AInput::Key::F16},
        {SDLK_F17, AInput::Key::F17},
        {SDLK_F18, AInput::Key::F18},
        {SDLK_F19, AInput::Key::F19},

        {SDLK_EQUALS, AInput::Key::EQUAL},
        {SDLK_MINUS, AInput::Key::DASH},
        {SDLK_RIGHTBRACKET, AInput::Key::RBRACKET},
        {SDLK_LEFTBRACKET, AInput::Key::LBRACKET},
        {SDLK_APOSTROPHE, AInput::Key::QUOTE},
        {SDLK_SEMICOLON, AInput::Key::SEMICOLON},
        {SDLK_BACKSLASH, AInput::Key::BACKSLASH},
        {SDLK_COMMA, AInput::Key::COMMA},
        {SDLK_SLASH, AInput::Key::SLASH},
        {SDLK_PERIOD, AInput::Key::PERIOD},
        {SDLK_GRAVE, AInput::Key::TILDE},

        {SDLK_KP_DECIMAL, AInput::Key::NUMPAD_PERIOD},
        {SDLK_KP_MULTIPLY, AInput::Key::NUMPAD_MULTIPLY},
        {SDLK_KP_PLUS, AInput::Key::NUMPAD_ADD},
        {SDLK_CLEAR, AInput::Key::NUMLOCKCLEAR},
        {SDLK_KP_DIVIDE, AInput::Key::NUMPAD_DIVIDE},
        {SDLK_KP_ENTER, AInput::Key::NUMPAD_RETURN},
        {SDLK_KP_MINUS, AInput::Key::NUMPAD_SUBTRACT},
        {SDLK_KP_EQUALS, AInput::Key::NUMPAD_EQUAL},
        {SDLK_KP_0, AInput::Key::NUMPAD_0},
        {SDLK_KP_1, AInput::Key::NUMPAD_1},
        {SDLK_KP_2, AInput::Key::NUMPAD_2},
        {SDLK_KP_3, AInput::Key::NUMPAD_3},
        {SDLK_KP_4, AInput::Key::NUMPAD_4},
        {SDLK_KP_5, AInput::Key::NUMPAD_5},
        {SDLK_KP_6, AInput::Key::NUMPAD_6},
        {SDLK_KP_7, AInput::Key::NUMPAD_7},
        {SDLK_KP_8, AInput::Key::NUMPAD_8},
        {SDLK_KP_9, AInput::Key::NUMPAD_9},

        {SDLK_RETURN, AInput::Key::RETURN},
        {SDLK_TAB, AInput::Key::TAB},
        {SDLK_SPACE, AInput::Key::SPACE},
        {SDLK_DELETE, AInput::Key::DEL},
        {SDLK_ESCAPE, AInput::Key::ESCAPE},
        {SDLK_LGUI, AInput::Key::LSYSTEM},
        {SDLK_LSHIFT, AInput::Key::LSHIFT},
        {SDLK_CAPSLOCK, AInput::Key::CAPSLOCK},
        {SDLK_LALT, AInput::Key::LALT},
        {SDLK_LCTRL, AInput::Key::LCONTROL},
        {SDLK_RGUI, AInput::Key::RSYSTEM},
        {SDLK_RSHIFT, AInput::Key::RSHIFT},
        {SDLK_RALT, AInput::Key::RALT},
        {SDLK_RCTRL, AInput::Key::RCONTROL},
        {SDLK_VOLUMEUP, AInput::Key::VOLUMEUP},
        {SDLK_VOLUMEDOWN, AInput::Key::VOLUMEDOWN},
        {SDLK_MUTE, AInput::Key::MUTE},
        {SDLK_MENU, AInput::Key::MENU},
        {SDLK_HELP, AInput::Key::INSERT},
        {SDLK_HOME, AInput::Key::HOME},
        {SDLK_PAGEUP, AInput::Key::PAGEUP},
        {SDLK_END, AInput::Key::END},
        {SDLK_PAGEDOWN, AInput::Key::PAGEDOWN},
        {SDLK_LEFT, AInput::Key::LEFT},
        {SDLK_RIGHT, AInput::Key::RIGHT},
        {SDLK_DOWN, AInput::Key::DOWN},
        {SDLK_UP, AInput::Key::UP},
        {SDLK_BACKSPACE, AInput::Key::BACKSPACE},
    };
    auto it = mapping.find(static_cast<SDL_Keycode>(key));
    if (it != mapping.end()) {
        return it->second;
    }
    return AInput::UNKNOWN;
}
/// [sdlToAInput]

/// [sdlToAPointer]
static auto sdlToAPointer(Uint8 button) -> APointerIndex {
    switch (button) {
        case SDL_BUTTON_LEFT:
            return APointerIndex::button(AInput::LBUTTON);
        case SDL_BUTTON_MIDDLE:
            return APointerIndex::button(AInput::CBUTTON);
        case SDL_BUTTON_RIGHT:
            return APointerIndex::button(AInput::RBUTTON);
        default:
            return {};
    }
}
/// [sdlToAPointer]

/// [EmbedRenderingContext]
struct EmbedRenderingContext : IRenderingContext {
    std::shared_ptr<OpenGLRenderer> m_renderer;

    ~EmbedRenderingContext() override {
    }

    void destroyNativeWindow(ASurface& window) override {}

    AImage makeScreenshot() override {
        return {};
    }

    void beginPaint(ASurface& window) override {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window.getSize().x, window.getSize().y);
        m_renderer->beginPaint(window.getSize());
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    void endPaint(ASurface& window) override {
        m_renderer->endPaint();
    }

    void beginResize(ASurface& window) override {
    }
    void endResize(ASurface& window) override {
    }

    IRenderer& renderer() override {
        return *m_renderer;
    }
};
/// [EmbedRenderingContext]

/// [EmbedWindow]
struct EmbedWindow : AGLEmbedContext, AObject {
    SDL_Window* sdl_window = nullptr;
    SDL_GLContext gl_context = nullptr;
    bool close = false;

    ~EmbedWindow() override {
        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(sdl_window);
    }

    void init(std::unique_ptr<EmbedRenderingContext>&& context) {
        windowInit(std::move(context));

        int width = 0;
        int height = 0;
        SDL_GetWindowSizeInPixels(sdl_window, &width, &height);
        setViewportSize(width, height);

        setCustomDpiRatio(SDL_GetWindowDisplayScale(sdl_window));

        connect(getWindow()->touchscreenKeyboardShown, this, [this] {
            SDL_StartTextInput(sdl_window);
        });
        connect(getWindow()->touchscreenKeyboardHidden, this, [this] {
            SDL_StopTextInput(sdl_window);
        });
    }

    void onNotifyProcessMessages() override {}
};
/// [EmbedWindow]

/// [handleSDLEvent]
void handleSDLEvent(SDL_Event* event, EmbedWindow& window) {
    switch (event->type) {
        case SDL_EVENT_QUIT:
            window.close = true;
            break;
        case SDL_EVENT_WINDOW_RESIZED: {
            int width = 0;
            int height = 0;
            SDL_GetWindowSizeInPixels(window.sdl_window, &width, &height);
            window.setViewportSize(width, height);
        } break;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
            int width = 0;
            int height = 0;
            SDL_GetWindowSizeInPixels(window.sdl_window, &width, &height);
            window.setViewportSize(width, height);
        } break;
        case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
            break;
        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED: {
            window.setCustomDpiRatio(SDL_GetWindowDisplayScale(window.sdl_window));
        } break;
        case SDL_EVENT_KEY_DOWN:
            window.onKeyPressed(sdlToAInput(event->key.key));
            break;
        case SDL_EVENT_KEY_UP:
            window.onKeyReleased(sdlToAInput(event->key.key));
            break;
        case SDL_EVENT_TEXT_INPUT: {
            std::string_view text(event->text.text);
            AUtf8ConstIterator it(text);
            AUtf8ConstIterator it_end(text, text.size());
            for (; it != it_end; ++it) {
                const AChar ch = *it;
                window.onCharEntered(ch);
            }
        } break;
        case SDL_EVENT_MOUSE_MOTION:
            window.onPointerMove(event->motion.x, event->motion.y);
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            window.onPointerPressed(event->button.x, event->button.y, sdlToAPointer(event->button.button));
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            window.onPointerReleased(event->button.x, event->button.y, sdlToAPointer(event->button.button));
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            window.onScroll(event->wheel.mouse_x, event->wheel.mouse_y, event->wheel.x, event->wheel.y);
            break;
        default: break;
    }
}
/// [handleSDLEvent]

using namespace declarative;

AUI_ENTRY {
    /// [SDL_Init]
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        ALogger::err("SDL3") << SDL_GetError();
        return 1;
    }
    /// [SDL_Init]

    /// [CreateWindow]
    EmbedWindow window;
    window.sdl_window = SDL_CreateWindow("AUI + SDL3", 600, 400, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window.sdl_window) {
        ALogger::err("SDL3") << SDL_GetError();
        return 1;
    }
    /// [CreateWindow]

    /// [GLContext]
    window.gl_context = SDL_GL_CreateContext(window.sdl_window);
    if (!window.gl_context) {
        ALogger::err("SDL3") << SDL_GetError();
        return 1;
    }
    SDL_GL_MakeCurrent(window.sdl_window, window.gl_context);
    /// [GLContext]

    /// [RendererSetup]
    if (!OpenGLRenderer::loadGL((OpenGLRenderer::GLLoadProc)SDL_GL_GetProcAddress)) {
        ALogger::err("OpenGLRenderer") << "Failed to load GL";
        return 1;
    }

    auto renderer = std::make_shared<OpenGLRenderer>();
    {
        auto rendering_context = std::make_unique<EmbedRenderingContext>();
        rendering_context->m_renderer = renderer;
        window.init(std::move(rendering_context));
    }
    /// [RendererSetup]

    /// [UI]
    window.setContainer(Centered {
        Vertical {
            Label { "Hello, World!" },
            _new<ATextField>(),
            Button {
                .content = Label { "Click me" },
                .onClick = [] {
                    ALogger::info("Test") << "Hello world!";
                }
            }
        }
    });
    /// [UI]

    /// [MainLoop]
    while (!window.close) {
        /// [EventProcessing]
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handleSDLEvent(&event, window);
        }
        AThread::processMessages();
        /// [EventProcessing]

        /// [Rendering]
        if (window.requiresRedraw()) {
            ARenderContext render_context {
                .clippingRects = {},
                .render = *renderer,
            };
            window.render(render_context);
            SDL_GL_SwapWindow(window.sdl_window);
        }
        /// [Rendering]

        /// [FrameRateLimit]
        const SDL_DisplayMode* dm = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window.sdl_window));
        Sint32 refresh_ms = static_cast<Sint32>(1000.0f / dm->refresh_rate);
        SDL_Delay(refresh_ms);
        /// [FrameRateLimit]
    }
    /// [MainLoop]

    return 0;
}
