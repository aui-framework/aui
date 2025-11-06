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

#include <AUI/Util/ALayoutInflater.h>
#include "AScrollAreaViewport.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Render/RenderHints.h"
#include "AUI/View/AView.h"

static constexpr auto RENDER_TO_TEXTURE_THRESHOLD_AREA = 128 * 128;

class AScrollAreaViewport::Inner: public AViewContainer {
public:
    Inner() {
        addAssName("AScrollAreaViewportInner");
    }
};

AScrollAreaViewport::AScrollAreaViewport(_<declarative::ScrollAreaViewport::State> state): mState(std::move(state)) {
    addAssName("AScrollAreaViewport");
    addView(mInner = _new<Inner>());
    connect(mState->scroll, [this](glm::uvec2 v) { mInner->setPosition(-glm::ivec2(v)); });
    connect(size(), mState->viewportSize);
    connect(mInner->size(), mState->fullContentSize);
}

AScrollAreaViewport::~AScrollAreaViewport() {

}

void AScrollAreaViewport::setContents(_<AView> content) {
    mInner->setPosition({0, 0});
    ALayoutInflater::inflate(mInner, content);
    mContents = std::move(content);
    mState->scroll.notify();
}

void AScrollAreaViewport::applyGeometryToChildren() {
    AViewContainerBase::applyGeometryToChildren();
    mInner->setSize(glm::max(mInner->getMinimumSize(), getSize()));
    if (mInner->getSize().x * mInner->getSize().y >= RENDER_TO_TEXTURE_THRESHOLD_AREA) {
        if (!IRenderViewToTexture::isEnabledForView(*mInner)) {
            auto w = AWindow::current();
            if (!w) {
                return;
            }

            IRenderViewToTexture::enableForView(w->getRenderingContext()->renderer(), *mInner);
        }
    } else {
        IRenderViewToTexture::disableForView(*mInner);
    }
}

_<AView> declarative::ScrollAreaViewport::operator()() {
    auto s = _new<AScrollAreaViewport>(std::move(state));
    s->setContents(std::move(content));
    return s;
}
