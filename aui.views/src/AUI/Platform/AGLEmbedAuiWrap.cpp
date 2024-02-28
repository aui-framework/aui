// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by Alex2772 on 1/24/2022.
//

#include "AGLEmbedAuiWrap.h"
#include "AUI/GL/State.h"
#include "AUI/GL/OpenGLRenderer.h"

void AGLEmbedAuiWrap::render(ClipOptimizationContext context) {
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

    ARender::setColorForced(1.f);
    ARender::setTransformForced(glm::ortho(0.f,
                                           static_cast<float>(mSize.x),
                                           static_cast<float>(mSize.y),
                                           0.f));
    windowRender();

    resetGLState();
}

void AGLEmbedAuiWrap::resetGLState() {
    glBindTexture(GL_TEXTURE_2D, 0);
    if (glBindVertexArray) {
        glBindVertexArray(0);
    }
}

AGLEmbedAuiWrap::AGLEmbedAuiWrap() {
#if !(AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS)
    glewExperimental = true;
    auto r = glewInit();
    AUI_ASSERT(r == 0);
#endif
    ARender::setRenderer(std::make_unique<OpenGLRenderer>());
}
