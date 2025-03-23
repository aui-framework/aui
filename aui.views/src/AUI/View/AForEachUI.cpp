/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "AForEachUI.h"

static constexpr auto RENDER_TO_TEXTURE_TILE_SIZE = 256;

void AForEachUIBase::setModelImpl(AForEachUIBase::List model) {
    removeAllViews();
    mCache.reset();
    mViewsModel = std::move(model);
    markMinContentSizeInvalid();
}

void AForEachUIBase::applyGeometryToChildren() {
    if (!mViewport) {
        if (!mCache) {
            mCache = Cache {
                .inflatedRange = { mViewsModel.begin(), mViewsModel.end() },
            };
            removeAllViews();
            for (auto v : mCache->inflatedRange) {
                addView(std::move(v));
            }
        }
        AViewContainerBase::applyGeometryToChildren();
        return;
    }

    if (!mLastInflatedScroll) {
        mLastInflatedScroll = -calculateOffsetWithinViewportSlidingSurface();
    }

    if (!mCache) {
        mCache = Cache {
            .inflatedRange = { mViewsModel.begin(), mViewsModel.begin() },
        };
        inflateForward();
        return;
    }
    AViewContainerBase::applyGeometryToChildren();
}

void AForEachUIBase::onViewGraphSubtreeChanged() {
    AViewContainerBase::onViewGraphSubtreeChanged();
    AUI_NULLSAFE(mViewport)->scroll().changed.clearAllOutgoingConnectionsWith(this);
    auto viewport = [&]() -> _<AScrollAreaViewport> {
        for (auto p = getParent(); p != nullptr; p = p->getParent()) {
            if (auto viewport = _cast<AScrollAreaViewport>(p->shared_from_this())) {
                return viewport;
            }
        }
        return nullptr;
    }();
    if (!viewport) {
        mViewport = nullptr;
        return;
    }
    connect(viewport->scroll().changed, [this](glm::uvec2 scroll) {
        const auto diffVec = glm::ivec2(scroll) - mLastInflatedScroll.valueOr(glm::ivec2{0});
        const auto diff = diffVec.x + diffVec.y;
        if (glm::abs(diff) < RENDER_TO_TEXTURE_TILE_SIZE) {
            return;
        }
      mLastInflatedScroll = glm::ivec2(scroll);
        if (diff > 0) {
            inflateForward();
        } else {
            inflateBackward();
        }
    });
    mLastInflatedScroll.reset();
    mViewport = std::move(viewport);
}

void AForEachUIBase::setPosition(glm::ivec2 position) {
    AView::setPosition(position);
    if (!mCache) {
        return;
    }
    mLastInflatedScroll = -calculateOffsetWithinViewportSlidingSurface();
    inflateBackward();
    inflateForward();
}

void AForEachUIBase::inflateForward() {
    const auto inflateTill = mViewport->getSize() + glm::ivec2(RENDER_TO_TEXTURE_TILE_SIZE) + glm::max(mLastInflatedScroll.valueOr(glm::ivec2(0)), glm::ivec2(0));
    const auto end = mViewsModel.end();
    bool needsMinSizeUpdate = false;
    {
        auto it = mCache->inflatedRange.end();
        for (; it != end; ++it) {
            if (!mViews.empty()) {
                const auto& lastView = mViews.last();
                if (glm::any(glm::greaterThanEqual(lastView->getPosition() + lastView->getSize(), inflateTill))) {
                    break;
                }
            }
            addView(*it);
            needsMinSizeUpdate = true;
            AViewContainerBase::applyGeometryToChildren();
        }
        if (needsMinSizeUpdate) {
            mCache->inflatedRange = { mCache->inflatedRange.begin(), it };
        }
    }

    if (needsMinSizeUpdate) {
        markMinContentSizeInvalid();
    } else {
        AViewContainerBase::applyGeometryToChildren();
    }
}

void AForEachUIBase::inflateBackward() {}

glm::ivec2 AForEachUIBase::calculateOffsetWithinViewportSlidingSurface() {
    AUI_ASSERT(mViewport != nullptr);

    glm::ivec2 accumulator{};
    for (AView* p = this; p != nullptr; p = p->getParent()) {
        if (mViewport.get() == p) {
            return accumulator;
        }
        accumulator += p->getPosition();
    }
    AUI_ASSERT_NO_CONDITION("divergent mViewport");
    return {};
}
