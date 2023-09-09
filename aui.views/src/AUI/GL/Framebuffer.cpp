#include <AUI/GL/Framebuffer.h>


gl::Framebuffer::Framebuffer() {
    glGenFramebuffers(1, &mHandle);
}

gl::Framebuffer::~Framebuffer() {
    if (mHandle == 0) {
        return;
    }
    glDeleteFramebuffers(1, &mHandle);
}

void gl::Framebuffer::bind()
{
  	glBindFramebuffer(GL_FRAMEBUFFER, mHandle);
}

void gl::Framebuffer::bindForRead()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mHandle);
}

void gl::Framebuffer::bindForWrite()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mHandle);
}

void gl::Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void gl::Framebuffer::resize(glm::u32vec2 newSize) {
    assert(newSize.x != 0);
    assert(newSize.y != 0);
    mSize = newSize;
    bind();
    for (auto& t : mAttachedTargets)
    {
        t->onFramebufferResize(newSize);
    }
}

