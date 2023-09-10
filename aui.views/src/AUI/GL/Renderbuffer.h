#pragma once

#include <AUI/GL/gl.h>
#include <AUI/GL/GLEnums.h>
#include <glm/glm.hpp>

namespace gl {
    template<Multisampling multisampling = Multisampling::DISABLED>
    class Renderbuffer {
    public:
        Renderbuffer() {
            glGenRenderbuffers(1, &mHandle);
        }
        Renderbuffer(const Renderbuffer&) = delete;
        ~Renderbuffer() {
            glDeleteRenderbuffers(1, &mHandle);
        }

        [[nodiscard]]
        GLuint handle() const noexcept {
            return mHandle;
        }

        void bind() {
            glBindRenderbuffer(GL_RENDERBUFFER, mHandle);
        }
        void storage(glm::u32vec2 size, InternalFormat internalFormat) {
            bind();
            if constexpr (multisampling == Multisampling::ENABLED) {
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, (int)internalFormat, size.x, size.y);
            } else {
                glRenderbufferStorage(GL_RENDERBUFFER, (int)internalFormat, size.x, size.y);
            }
        }

    private:
        GLuint mHandle;
    };
}
