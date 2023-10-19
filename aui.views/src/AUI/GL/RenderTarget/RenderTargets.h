#pragma once

#include <AUI/GL/Framebuffer.h>

namespace gl {
    struct API_AUI_VIEWS RenderTargetDepth {
    public:
        static std::unique_ptr<Framebuffer::IRenderTarget> attach(gl::Framebuffer& to);
    };

    struct API_AUI_VIEWS RenderTargetRGBA8 {
    public:
        static std::unique_ptr<Framebuffer::IRenderTarget> attach(gl::Framebuffer& to, GLenum attachment /* = GL_COLOR_ATTACHMENT0 */);
    };
}
