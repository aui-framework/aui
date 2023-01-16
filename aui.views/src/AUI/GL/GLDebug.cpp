// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2 on 21.11.2020.
//

#include "GLDebug.h"

#include <AUI/GL/gl.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Traits/strings.h>

#if !(AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS)
static void GLAPIENTRY debugProc(GLenum source,
                                 GLenum type,
                                 GLuint id,
                                 GLenum severity,
                                 GLsizei length,
                                 const GLchar* message,
                                 const void* userParam)
{
    const char* typeString;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR: {
            ALogger::err("OpenGL") << "[Error] " << message << AStacktrace::capture(2);
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
    if (glDebugMessageCallback) {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(debugProc, nullptr);
    }
}
#else

void gl::setupDebug() {
    // stub
}

#endif
