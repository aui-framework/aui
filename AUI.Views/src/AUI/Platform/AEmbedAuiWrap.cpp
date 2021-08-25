/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 6/6/2021.
//


#include "AEmbedAuiWrap.h"
#include "ABaseWindow.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <AUI/GL/State.h>
#include <AUI/Platform/AWindow.h>

class AEmbedAuiWrap::FakeWindow: public ABaseWindow {
friend class AEmbedAuiWrap;
public:
    FakeWindow() {
        currentWindowStorage() = this;
        checkForStencilBits();
    }
};

AEmbedAuiWrap::AEmbedAuiWrap() {
    auto r = glewInit();
    assert(r == 0);
    mContainer = _new<FakeWindow>();
}

void AEmbedAuiWrap::setContainer(const _<AViewContainer>& container) {
    mContainer->setContents(container);
}

void AEmbedAuiWrap::setSize(int width, int height) {
    mWidth = width;
    mHeight = height;
    mContainer->setSize(width, height);

    resetGLState();
}

void AEmbedAuiWrap::render() {
    AThread::current()->processMessages();

    glEnable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL::State::activeTexture(0);
    GL::State::useProgram(0);
    GL::State::bindVertexArray(0);
    GL::State::bindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Render::inst().setColorForced(1.f);
    Render::inst().setTransformForced(glm::ortho(0.f,
                                                          static_cast<float>(mWidth),
                                                          static_cast<float>(mHeight),
                                                          0.f));
    mContainer->render();

    resetGLState();
}

void AEmbedAuiWrap::resetGLState() {
    glBindTexture(GL_TEXTURE_2D, 0);
    if (glBindVertexArray) {
        glBindVertexArray(0);
    }
}

void AEmbedAuiWrap::onMousePressed(int x, int y, AInput::Key button) {
    AThread::current()->processMessages();
    mContainer->onMousePressed({x, y}, button);
}

void AEmbedAuiWrap::onMouseReleased(int x, int y, AInput::Key button) {
    mContainer->onMouseReleased({x, y}, button);
}

bool AEmbedAuiWrap::isUIConsumesMouseAt(int x, int y) {
    return mContainer->consumesClick({x, y});
}

void AEmbedAuiWrap::onMouseMove(int x, int y) {
    mContainer->onMouseMove({x, y});
}


void AEmbedAuiWrap::onCharEntered(wchar_t c) {
    mContainer->onCharEntered(c);
}

void AEmbedAuiWrap::onKeyPressed(AInput::Key key) {
    mContainer->onKeyDown(key);
}

void AEmbedAuiWrap::onKeyReleased(AInput::Key key) {
    mContainer->onKeyUp(key);
}
