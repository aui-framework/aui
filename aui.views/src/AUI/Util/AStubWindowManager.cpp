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

namespace {
    class StubRenderingContext: public SoftwareRenderingContext {
    public:
        void init(const Init& init) override {
            IRenderingContext::init(init);
            reallocate(init.window);
        }

        ~StubRenderingContext() override = default;

        void destroyNativeWindow(ASurface& window) override {

        }

        void beginPaint(ASurface& window) override {
            std::memset(mStencilBlob.data(), 0, mStencilBlob.getSize());
        }

        void endPaint(ASurface& window) override {

        }
    };
}

void AStubWindowManager::initNativeWindow(const IRenderingContext::Init& init) {
    auto context = std::make_unique<StubRenderingContext>();
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

AImage AStubWindowManager::makeScreenshot(aui::no_escape<AWindow> window) {
    drawFrame();
    return window->getRenderingContext()->makeScreenshot();
}
