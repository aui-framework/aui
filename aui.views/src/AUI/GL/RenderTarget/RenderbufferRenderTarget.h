#pragma once


#include <AUI/Util/kAUI.h>
#include <AUI/GL/Framebuffer.h>
#include <AUI/GL/GLEnums.h>
#include <AUI/GL/Renderbuffer.h>
#include <memory>

namespace gl {


    template<InternalFormat internalFormat /* = GL_DEPTH_COMPONENT24 */,
             Multisampling multisampling = Multisampling::DISABLED>
    class RenderbufferRenderTarget final: public Framebuffer::IRenderTarget {
    public:
        RenderbufferRenderTarget() {
        }

    protected:
        void attach(Framebuffer& to, GLenum attachmentType) override {
            glGetError();
            onFramebufferResize(to.size());
            if (glGetError() != GL_NO_ERROR) {
                throw AException("unable to initialize renderbuffer storage");
            }
            to.bind();
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, mRenderbuffer.handle());
            if (glGetError() != GL_NO_ERROR) {
                throw AException("unable to create renderbuffer target");
            }
        }

        void onFramebufferResize(glm::u32vec2 size) override {
            mRenderbuffer.storage(size, internalFormat);
        }

    private:
        Renderbuffer<multisampling> mRenderbuffer;
    };
}
