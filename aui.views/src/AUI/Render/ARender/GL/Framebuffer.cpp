/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <AUI/Render/ARender/GL/Framebuffer.h>
#include <AUI/Render/ARender/GL/State.h>


#if AUI_PLATFORM_LINUX
int gl::Framebuffer::DEFAULT_FB = 0;
#endif

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
    for (auto& t : mAttachedTargets)
    {
        t->onFramebufferResize(mSize);
    }
}

void gl::Framebuffer::bindViewport() {
    glViewport(0, 0, mSize.x, mSize.y);
}

void gl::Framebuffer::label(const AString& name) {
    gl::State::label(GL_FRAMEBUFFER, mHandle, name);
}
