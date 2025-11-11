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
// Created by alex2 on 6/6/2021.
//


#include "AEmbedAuiWrap.h"
#include "AWindowBase.h"
#include "glm/fwd.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <AUI/GL/State.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/GL/OpenGLRenderer.h>
#include <AUI/Util/ALayoutInflater.h>

class AEmbedAuiWrap::EmbedWindow: public AWindowBase {
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

    void markMinContentSizeInvalid() override {
        flagRedraw();
        mRequiresLayoutUpdate = true;
    }

    void flagRedraw() override {
        AWindowBase::flagRedraw();
        mRequiresRedraw = true;
    }

protected:
    float fetchDpiFromSystem() const override {
        if (mTheWrap->mCustomDpiRatio) {
            return *mTheWrap->mCustomDpiRatio;
        }
        return AWindowBase::fetchDpiFromSystem();
    }
};

void AEmbedAuiWrap::onScroll(int mouseX, int mouseY, int scrollX, int scrollY) {
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
    auto& render = mContainer->getRenderingContext()->renderer();
    render.setWindow(mContainer.get());
    if (mContainer->mRequiresLayoutUpdate) {
        mContainer->mRequiresLayoutUpdate = false;
        mContainer->applyGeometryToChildrenIfNecessary();
    }
    AUI_NULLSAFE(mContainer->getRenderingContext())->beginPaint(*mContainer);
    mContainer->mRequiresRedraw = false;
    mContainer->render({.clippingRects = { ARect<int>{ .p1 = glm::ivec2(0), .p2 = mContainer->getSize() } }, .render = render });
    AUI_NULLSAFE(mContainer->getRenderingContext())->endPaint(*mContainer);
}

void AEmbedAuiWrap::setContainer(const _<AViewContainer>& container) {
    ALayoutInflater::inflate(mContainer, container);
    mContainer->setPosition({0, 0});
    container->setPosition({0, 0});
    mContainer->makeCurrent();
    mContainer->markMinContentSizeInvalid();
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


void AEmbedAuiWrap::onPointerPressed(int x, int y, APointerIndex pointerIndex) {
    mContainer->makeCurrent();
    AThread::processMessages();
    mContainer->onPointerPressed({
        .position = glm::ivec2{x, y},
        .pointerIndex = pointerIndex,
    });
}

void AEmbedAuiWrap::onPointerReleased(int x, int y, APointerIndex pointerIndex) {
    mContainer->makeCurrent();
    mContainer->onPointerReleased({
        .position = glm::ivec2{x, y},
        .pointerIndex = pointerIndex,
    });
}

bool AEmbedAuiWrap::isUIConsumesMouseAt(int x, int y) {
    return mContainer->consumesClick(glm::ivec2{ x, y });
}

void AEmbedAuiWrap::onPointerMove(int x, int y) {
    mContainer->makeCurrent();
    mContainer->onPointerMove(glm::ivec2{ x, y }, {APointerIndex::button(AInput::LBUTTON)});
}


void AEmbedAuiWrap::onCharEntered(AChar c) {
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

AWindowBase* AEmbedAuiWrap::getWindow() {
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
