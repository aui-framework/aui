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
#include <AUI/Platform/AGLEmbedAuiWrap.h>
#include <AUI/GL/OpenGLRenderer.h>
#include <SDL3/SDL.h>

struct EmbedWindow {
    SDL_Window* sdl_window = nullptr;
    SDL_GLContext gl_context = nullptr;
    bool close = false;

    ~EmbedWindow() {
        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(sdl_window);
    }
};

void handleSDLEvent(SDL_Event* event, EmbedWindow& window) {
    switch (event->type) {
        case SDL_EVENT_QUIT:
            window.close = true;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            break;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            break;
        case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
            break;
        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
            break;
        case SDL_EVENT_KEY_DOWN:
            break;
        case SDL_EVENT_KEY_UP:
            break;
        case SDL_EVENT_TEXT_INPUT:
            break;
        case SDL_EVENT_MOUSE_MOTION:
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            break;
        default: break;
    }
}

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
    OpenGLRenderer renderer;

    while (!window.close) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handleSDLEvent(&event, window);
        }

        const SDL_DisplayMode* dm = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window.sdl_window));
        Sint32 refresh_ms = static_cast<Sint32>(1000.0f / dm->refresh_rate);
        SDL_Delay(refresh_ms);
    }

    return 0;
}
