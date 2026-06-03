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
#include <AUI/Software/SoftwareRenderer.h>
#include "AStubWindowManager.h"

static aui::lazy<AStubWindowManager::Config> gStubWindowManagerConfig = [] {
    return AStubWindowManager::Config{
        .renderer = std::make_unique<SoftwareRenderer>()
    };
};

class StubRenderingContext: public SoftwareRenderingContext {
public:
    StubRenderingContext(AStubWindowManager& parent): mParent(parent) {}

    void init(const Init& init)
#if !AUI_PLATFORM_LINUX
      override
#endif
    {
        reallocate(init.window);
        mCanvas = std::make_unique<ADisplayListCanvas>(mDisplayList, *gStubWindowManagerConfig->renderer);
        mRendererWrapper = std::make_unique<RendererCanvas>(*mCanvas, *gStubWindowManagerConfig->renderer);
    }

    ~StubRenderingContext() override = default;

    void destroyNativeWindow(ASurface& window) override {

    }

    void beginPaint(ASurface& window) override {
        SoftwareRenderingContext::beginPaint(window);
    }

    void endPaint(ASurface& window) override {
        mDisplayList.optimize();
        mDisplayList.draw(backend(), mWindowTarget);
        mDisplayList.clear();
    }

    IRendererBackend& backend() override {
        return *gStubWindowManagerConfig->renderer;
    }

    ACanvas& canvas() override {
        return *mCanvas;
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
    for (auto& w : AWindow::getWindowManager().getWindows()) {
        w->getRenderingContext()->beginResize(*w);
        w->pack();
        w->getRenderingContext()->endResize(*w);
        w->redraw();
    }
}

void AStubWindowManager::setConfig(Config config) {
    gStubWindowManagerConfig = std::move(config);
}

AImage AStubWindowManager::makeScreenshot(aui::no_escape<AWindow> window) {
    drawFrame();
    return window->getRenderingContext()->makeScreenshot();
}
