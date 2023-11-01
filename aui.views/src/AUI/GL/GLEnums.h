#pragma once
#include <AUI/GL/gl.h>

namespace gl {
    enum class Multisampling {
        DISABLED,
        ENABLED,
    };

    enum class InternalFormat {
        DEPTH_COMPONENT   = 6402,
        DEPTH_COMPONENT24 = 33190,
        DEPTH_COMPONENT16 = 33189,
        DEPTH24_STENCIL8 = 0x88F0,
        STENCIL8 = 0x8D48,
        RGBA8 = GL_RGBA8,
        RGBA = GL_RGBA,
    };

    enum class Type {
        UNSIGNED_BYTE   = GL_UNSIGNED_BYTE,
        UNSIGNED_SHORT  = GL_UNSIGNED_SHORT,
        UNSIGNED_INT    = GL_UNSIGNED_INT,
        FLOAT           = GL_FLOAT,
    };

    enum class Format {
        DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
        RGBA            = GL_RGBA,
    };
}
