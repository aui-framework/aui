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
// Created by alex2772 on 9/4/23.
//

#include <AUI/Platform/SoftwareRenderingContext.h>
#include "AStubWindowManager.h"

class StubRenderingContext: public SoftwareRenderingContext {
public:
    StubRenderingContext(AStubWindowManager& parent): mParent(parent) {}

    void init(const Init& init) override {
        IRenderingContext::init(init);
        reallocate(init.window);
    }

    ~StubRenderingContext() override = default;

    void destroyNativeWindow(AWindowBase& window) override {

    }

    void beginPaint(AWindowBase& window) override {
        std::memset(mStencilBlob.data(), 0, mStencilBlob.getSize());
    }

    void endPaint(AWindowBase& window) override {}

    IRenderer& renderer() override {
        return *mParent.mConfig.renderer;
    }

private:
    AStubWindowManager& mParent;
};

void AStubWindowManager::initNativeWindow(const IRenderingContext::Init& init) {
    auto context = std::make_unique<StubRenderingContext>(*this);
    context->init(init);
    init.setRenderingContext(std::move(context));
}

void AStubWindowManager::drawFrame() {
    auto wm = dynamic_cast<AStubWindowManager*>(&AWindow::getWindowManager());
    AUI_ASSERTX(wm != nullptr, "AStubWindowManager is excepted to be used when calling AStubWindowManager::drawFrame(); call uitest::setup()");
    wm->drawFrameImpl();
}

void AStubWindowManager::drawFrameImpl() {
    for (auto& w : getWindows()) {
        if (mConfig.layout) {
            w->getRenderingContext()->beginResize(*w);
            w->pack();
            w->getRenderingContext()->endResize(*w);
        }
        if (mConfig.paint) {
            w->redraw();
        }
    }
}

void AStubWindowManager::setConfig(Config config) {
    auto wm = dynamic_cast<AStubWindowManager*>(&AWindow::getWindowManager());
    AUI_ASSERTX(wm != nullptr, "AStubWindowManager is excepted to be used when calling AStubWindowManager::setConfig(); call uitest::setup()");
    wm->mConfig = std::move(config);
}

AImage AStubWindowManager::makeScreenshot(aui::no_escape<AWindow> window) {
    drawFrame();
    return window->getRenderingContext()->makeScreenshot();
}
