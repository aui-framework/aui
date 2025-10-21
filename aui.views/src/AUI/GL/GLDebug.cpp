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

//
// Created by alex2 on 21.11.2020.
//

#include "GLDebug.h"

#include <AUI/GL/gl.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Traits/strings.h>
#include <string_view>

#ifndef GLAPIENTRY
#define GLAPIENTRY
#define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH 0x8243
#define GL_DEBUG_CALLBACK_FUNCTION 0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM 0x8245
#define GL_DEBUG_SOURCE_API 0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM 0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER 0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY 0x8249
#define GL_DEBUG_SOURCE_APPLICATION 0x824A
#define GL_DEBUG_SOURCE_OTHER 0x824B
#define GL_DEBUG_TYPE_ERROR 0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 0x824E
#define GL_DEBUG_TYPE_PORTABILITY 0x824F
#define GL_DEBUG_TYPE_PERFORMANCE 0x8250
#define GL_DEBUG_TYPE_OTHER 0x8251
#define GL_DEBUG_TYPE_MARKER 0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP 0x8269
#define GL_DEBUG_TYPE_POP_GROUP 0x826A
#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B


#endif

namespace gl {
    bool silenceDebug = false;
}

static void GLAPIENTRY debugProc(GLenum source,
                                 GLenum type,
                                 GLuint id,
                                 GLenum severity,
                                 GLsizei length,
                                 const GLchar* message,
                                 const void* userParam)
{
    // filter out repeated messages; they are spamming and annoying
#if AUI_PLATFORM_APPLE
    const auto h = std::hash<std::string>{}(std::string(message));
#else
    const auto h = std::hash<std::string_view>{}(std::string_view(message));
#endif
    static ASet<size_t> messages;
    if (messages.contains(h)) {
        return;
    }
    messages << h;


    if (gl::silenceDebug) {
        return;
    }
    const char* typeString;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR: {
            ALogger::err("OpenGL") << "[Error] " << message << AStacktrace::capture(2);
            return;
        }
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeString = "Deprecated";  break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeString = "Undefined";   break;
        case GL_DEBUG_TYPE_PORTABILITY:         typeString = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeString = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              typeString = "Marker";      break;
        default:                                return;
    }

    ALogger::info("OpenGL") << "[" << typeString << "] " << message;
}

void gl::setupDebug() {
#if !AUI_PLATFORM_IOS && !AUI_PLATFORM_EMSCRIPTEN
    if (glDebugMessageCallback) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(debugProc, nullptr);
    }
#endif
}

