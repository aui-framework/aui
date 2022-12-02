// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
// Created by Alex2772 on 12/7/2021.
//

#include <AUI/GL/gl.h>
#include <AUI/Platform/OpenGLRenderingContext.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/GL/GLDebug.h>

#include <AUI/GL/OpenGLRenderer.h>
#include <AUI/GL/State.h>


void OpenGLRenderingContext::init(const Init& init) {
    CommonRenderingContext::init(init);
    Render::setRenderer(std::make_unique<OpenGLRenderer>());
}

void OpenGLRenderingContext::destroyNativeWindow(ABaseWindow& window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void OpenGLRenderingContext::beginPaint(ABaseWindow& window) {
    CommonRenderingContext::beginPaint(window);
    
    gl::State::activeTexture(0);
    gl::State::bindTexture(GL_TEXTURE_2D, 0);
    gl::State::bindVertexArray(0);
    gl::State::useProgram(0);

    glViewport(0, 0, window.getWidth(), window.getHeight());

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // stencil
    glClear(GL_STENCIL_BUFFER_BIT);
    glClearStencil(0);
    glStencilMask(0xff);
    glDisable(GL_SCISSOR_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 0, 0xff);
}

void OpenGLRenderingContext::beginResize(ABaseWindow& window) {
}

void OpenGLRenderingContext::endResize(ABaseWindow& window) {

}

void OpenGLRenderingContext::endPaint(ABaseWindow& window) {
    CommonRenderingContext::endPaint(window);
}

OpenGLRenderingContext::~OpenGLRenderingContext() {
}

AImage OpenGLRenderingContext::makeScreenshot() {
    // stub
    return AImage();
}
