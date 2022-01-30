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
 
 * Original code located at https://github.com/aui-framework/aui
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
#include <AUI/GL/OpenGLRenderer.h>
#include <AUI/Util/ALayoutInflater.h>

class AEmbedAuiWrap::EmbedWindow: public ABaseWindow {
    friend class AEmbedAuiWrap;
private:
    AEmbedAuiWrap* mTheWrap;
    bool mRequiresRedraw = false;
    bool mRequiresLayoutUpdate = false;

public:
    EmbedWindow(AEmbedAuiWrap* theWrap): mTheWrap(theWrap) {
        currentWindowStorage() = this;
    }

    _<AOverlappingSurface> createOverlappingSurfaceImpl(const glm::ivec2& position, const glm::ivec2& size) override {
        auto container = _new<AOverlappingSurface>();
        addViewCustomLayout(container);

        auto windowSize = getSize();

        container->setGeometry(position.x, position.y, size.x, size.y);
        return container;
    }

    void closeOverlappingSurfaceImpl(AOverlappingSurface* surface) override {
        removeView(surface);
    }

    void flagUpdateLayout() override {
        flagRedraw();
        mRequiresLayoutUpdate = true;
    }

    void flagRedraw() override {
        ABaseWindow::flagRedraw();
        mRequiresRedraw = true;
    }

protected:
    float fetchDpiFromSystem() const override {
        if (mTheWrap->mCustomDpiRatio) {
            return *mTheWrap->mCustomDpiRatio;
        }
        return ABaseWindow::fetchDpiFromSystem();
    }
};

void AEmbedAuiWrap::onMouseScroll(int mouseX, int mouseY, int scrollX, int scrollY) {
    mContainer->onMouseWheel({mouseX, mouseY}, {scrollX, scrollY});
}

AEmbedAuiWrap::AEmbedAuiWrap():
        mEventLoopHandle(this)
{
}

void AEmbedAuiWrap::windowMakeCurrent() {
    mContainer->makeCurrent();
}

void AEmbedAuiWrap::windowRender() {
    AThread::current()->processMessages();
    Render::setWindow(mContainer.get());
    if (mContainer->mRequiresLayoutUpdate) {
        mContainer->mRequiresLayoutUpdate = false;
        mContainer->updateLayout();
    }
    nullsafe(mContainer->getRenderingContext())->beginPaint(*mContainer);
    mContainer->mRequiresRedraw = false;
    mContainer->render();
    nullsafe(mContainer->getRenderingContext())->endPaint(*mContainer);
}

void AEmbedAuiWrap::setContainer(const _<AViewContainer>& container) {
    ALayoutInflater::inflate(mContainer, container);
    mContainer->setPosition({0, 0});
    container->setPosition({0, 0});
    mContainer->makeCurrent();
    mContainer->flagUpdateLayout();
    mContainer->flagRedraw();
}

void AEmbedAuiWrap::setViewportSize(int width, int height) {
    mContainer->makeCurrent();
    mSize = { width, height };
    nullsafe(mContainer->getRenderingContext())->beginResize(*mContainer);
    mContainer->setSize(width, height);
    nullsafe(mContainer->getRenderingContext())->endResize(*mContainer);
    mContainer->mRequiresRedraw = true;
}


void AEmbedAuiWrap::onMousePressed(int x, int y, AInput::Key button) {
    mContainer->makeCurrent();
    AThread::current()->processMessages();
    mContainer->onMousePressed(glm::ivec2{ x, y }, button);
}

void AEmbedAuiWrap::onMouseReleased(int x, int y, AInput::Key button) {
    mContainer->makeCurrent();
    mContainer->onMouseReleased(glm::ivec2{ x, y }, button);
}

bool AEmbedAuiWrap::isUIConsumesMouseAt(int x, int y) {
    return mContainer->consumesClick(glm::ivec2{ x, y });
}

void AEmbedAuiWrap::onMouseMove(int x, int y) {
    mContainer->makeCurrent();
    mContainer->onMouseMove(glm::ivec2{ x, y });
}


void AEmbedAuiWrap::onCharEntered(wchar_t c) {
    mContainer->makeCurrent();
    mContainer->onCharEntered(c);
}

void AEmbedAuiWrap::onKeyPressed(AInput::Key key) {
    mContainer->makeCurrent();
    mContainer->onKeyDown(key);
}

void AEmbedAuiWrap::onKeyReleased(AInput::Key key) {
    mContainer->makeCurrent();
    mContainer->onKeyUp(key);
}

void AEmbedAuiWrap::loop() {
    // stub
}

void AEmbedAuiWrap::clearFocus() {
    mContainer->setFocusedView(nullptr);
}

ABaseWindow* AEmbedAuiWrap::getWindow() {
    return mContainer.get();
}

void AEmbedAuiWrap::windowInit(_unique<IRenderingContext> context) {
    mContainer = _new<EmbedWindow>(this);
    mContainer->mRenderingContext = std::move(context);
    mContainer->setPosition({ 0, 0 });
}

void AEmbedAuiWrap::setCustomDpiRatio(float r) {
    if (mCustomDpiRatio) {
        if (*mCustomDpiRatio == r) {
            return;
        }
    }
    mCustomDpiRatio = r;
    mContainer->updateDpi();
}
bool AEmbedAuiWrap::requiresRedraw() {
    return mContainer->mRequiresRedraw;
}

void AEmbedAuiWrap::notifyProcessMessages() {
    onNotifyProcessMessages();
}
