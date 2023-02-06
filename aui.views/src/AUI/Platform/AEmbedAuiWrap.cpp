// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

    unsigned mFrameMillis = 1;

public:
    EmbedWindow(AEmbedAuiWrap* theWrap): mTheWrap(theWrap) {
        currentWindowStorage() = this;
    }

    unsigned int frameMillis() const noexcept override {
        return mFrameMillis;
    }

    void setFrameMillis(unsigned int frameMillis) {
        mFrameMillis = frameMillis;
    }

    _<AOverlappingSurface> createOverlappingSurfaceImpl(const glm::ivec2& position, const glm::ivec2& size) override {
        class MyOverlappingSurface: public AOverlappingSurface {
        public:
            void setOverlappingSurfacePosition(glm::ivec2 position) override {
                emit positionSet(position);
            }

            void setOverlappingSurfaceSize(glm::ivec2 size) override {
                emit sizeSet(size);
            }

        signals:
            emits<glm::ivec2> positionSet;
            emits<glm::ivec2> sizeSet;
        };

        auto container = _new<MyOverlappingSurface>();
        connect(container->positionSet, [container = container.get()](glm::ivec2 p) {
            container->setPosition(p);
        });
        connect(container->sizeSet, [container = container.get()](glm::ivec2 p) {
            container->setSize(p);
        });
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
    mContainer->onScroll({
        .origin = { mouseX, mouseY },
        .delta = { scrollX, scrollY },
    });
}

AEmbedAuiWrap::AEmbedAuiWrap():
        mEventLoopHandle(this)
{
}

void AEmbedAuiWrap::windowMakeCurrent() {
    mContainer->makeCurrent();
}

void AEmbedAuiWrap::windowRender() {
    AThread::processMessages();
    Render::setWindow(mContainer.get());
    if (mContainer->mRequiresLayoutUpdate) {
        mContainer->mRequiresLayoutUpdate = false;
        mContainer->updateLayout();
    }
    AUI_NULLSAFE(mContainer->getRenderingContext())->beginPaint(*mContainer);
    mContainer->mRequiresRedraw = false;
    mContainer->render();
    AUI_NULLSAFE(mContainer->getRenderingContext())->endPaint(*mContainer);
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
    AUI_NULLSAFE(mContainer->getRenderingContext())->beginResize(*mContainer);
    mContainer->setSize({width, height});
    AUI_NULLSAFE(mContainer->getRenderingContext())->endResize(*mContainer);
    mContainer->mRequiresRedraw = true;
}


void AEmbedAuiWrap::onMousePressed(int x, int y, AInput::Key button) {
    mContainer->makeCurrent();
    AThread::processMessages();
    mContainer->onMousePressed({
        .position = glm::ivec2{x, y},
        .button = button
    });
}

void AEmbedAuiWrap::onMouseReleased(int x, int y, AInput::Key button) {
    mContainer->makeCurrent();
    mContainer->onMouseReleased({
        .position = glm::ivec2{x, y},
        .button = button
    });
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
