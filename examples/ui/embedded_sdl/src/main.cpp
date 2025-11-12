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

#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AGLEmbedContext.h>
#include <AUI/GL/OpenGLRenderer.h>
#include <AUI/Util/Declarative/Containers.h>
#include <AUI/View/AButton.h>
#include <SDL3/SDL.h>

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

struct EmbedWindow : AGLEmbedContext {
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
    }

    void onNotifyProcessMessages() override {}
};

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
            break;
        case SDL_EVENT_KEY_UP:
            break;
        case SDL_EVENT_TEXT_INPUT:
            break;
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

using namespace declarative;

AUI_ENTRY {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        ALogger::err("SDL3") << SDL_GetError();
        return 1;
    }

    EmbedWindow window;
    window.sdl_window = SDL_CreateWindow("AUI + SDL3", 600, 400, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window.sdl_window) {
        ALogger::err("SDL3") << SDL_GetError();
        return 1;
    }

    window.gl_context = SDL_GL_CreateContext(window.sdl_window);
    if (!window.gl_context) {
        ALogger::err("SDL3") << SDL_GetError();
        return 1;
    }
    SDL_GL_MakeCurrent(window.sdl_window, window.gl_context);

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

    return 0;
}
