#include <AUI/GL/Framebuffer.h>

static gl::Framebuffer* gFramebufferCurrent = nullptr;

gl::Framebuffer::Framebuffer() {
    glGenFramebuffers(1, &mHandle);
}

gl::Framebuffer::~Framebuffer() {
    if (gFramebufferCurrent == this) {
        gFramebufferCurrent = nullptr;
    }
    if (mHandle == 0) {
        return;
    }
    glDeleteFramebuffers(1, &mHandle);
}

void gl::Framebuffer::bind()
{
    gFramebufferCurrent = this;
  	glBindFramebuffer(GL_FRAMEBUFFER, mHandle);
}

void gl::Framebuffer::bindForRead()
{
    gFramebufferCurrent = this;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mHandle);
}

void gl::Framebuffer::bindForWrite()
{
    gFramebufferCurrent = this;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mHandle);
}

void gl::Framebuffer::unbind()
{
    gFramebufferCurrent = nullptr;
    glBindFramebuffer(GL_FRAMEBUFFER, DEFAULT_FB);
}

void gl::Framebuffer::resize(glm::u32vec2 newSize) {
    if (mSize == newSize) {
        return;
    }
    AUI_ASSERT(newSize.x != 0);
    AUI_ASSERT(newSize.y != 0);
    mSize = newSize;
    bind();
    auto o = supersampledSize();
    for (auto& t : mAttachedTargets)
    {
        t->onFramebufferResize(o);
    }
}

gl::Framebuffer* gl::Framebuffer::current() {
    return gFramebufferCurrent;
}

