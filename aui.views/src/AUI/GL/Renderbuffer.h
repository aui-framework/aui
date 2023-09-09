#pragma once

#include <AUI/GL/gl.h>
#include <AUI/GL/GLEnums.h>
#include <glm/glm.hpp>

namespace gl {
    class Renderbuffer {
    public:
        Renderbuffer();
        Renderbuffer(const Renderbuffer&) = delete;
        ~Renderbuffer();

        [[nodiscard]]
        GLuint handle() const noexcept {
            return mHandle;
        }

        void bind();
        void storage(glm::u32vec2 size, InternalFormat internalFormat);

    private:
        GLuint mHandle;
    };
}
