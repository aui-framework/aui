//
// Created by alex2 on 21.11.2020.
//

#include "GLDebug.h"

#include <AUI/GL/gl.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Traits/strings.h>

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
        case GL_DEBUG_TYPE_ERROR:               typeString = "Error";       break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeString = "Deprecated";  break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeString = "Undefined";   break;
        case GL_DEBUG_TYPE_PORTABILITY:         typeString = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeString = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              typeString = "Marker";      break;
        default:                                return;
    }

    ALogger::info("GL[{}] {}"_as.format(typeString, message));
}

void GL::setupDebug() {
    if (glDebugMessageCallback) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(debugProc, nullptr);
    }
}
