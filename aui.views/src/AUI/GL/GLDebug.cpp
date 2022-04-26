/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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
            ALogger::err("GL") << "[Error] " << message << AStacktrace::capture(2);
        }
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeString = "Deprecated";  break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeString = "Undefined";   break;
        case GL_DEBUG_TYPE_PORTABILITY:         typeString = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeString = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              typeString = "Marker";      break;
        default:                                return;
    }

    ALogger::info("GL") << "[" << typeString << "] " << message;
}

void GL::setupDebug() {
    if (glDebugMessageCallback) {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(debugProc, nullptr);
    }
}
#else

void GL::setupDebug() {
    // stub
}

#endif
