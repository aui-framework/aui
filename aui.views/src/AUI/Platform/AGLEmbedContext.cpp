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

//
// Created by Alex2772 on 1/24/2022.
//

#include "AGLEmbedContext.h"

#include <AUI/GL/State.h>
#include <AUI/GL/OpenGLRenderer.h>

AGLEmbedContext::AGLEmbedContext() {
#if !(AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS)
    //glewExperimental = true;
    //auto r = glewInit();
    //AUI_ASSERT(r == 0);
#endif
}

AGLEmbedContext::~AGLEmbedContext() = default;

void AGLEmbedContext::render(const ARenderContext& context) {
    windowMakeCurrent();
    AThread::processMessages();

    glEnable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    gl::State::activeTexture(0);
    gl::State::useProgram(0);
    gl::State::bindVertexArray(0);
    gl::State::bindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    context.render.setColorForced(1.f);
    context.render.setTransformForced(glm::ortho(0.f,
                                           static_cast<float>(mSize.x),
                                           static_cast<float>(mSize.y),
                                           0.f));
    windowRender();

    resetGLState();
}

void AGLEmbedContext::resetGLState() {
    glBindTexture(GL_TEXTURE_2D, 0);
    if (glBindVertexArray) {
        glBindVertexArray(0);
    }
}
