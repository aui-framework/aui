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

#include <range/v3/all.hpp>
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

    auto fakeBeginOffset = mFakeBeginOffset.valueOr(glm::ivec2(0));
    auto fakeEndOffset = mFakeEndOffset.valueOr(glm::ivec2(0));
    getLayout()->onResize(
        mPadding.left + fakeBeginOffset.x, mPadding.top + fakeBeginOffset.y,
        getSize().x - mPadding.horizontal() - fakeEndOffset.x, getSize().y - mPadding.vertical() - fakeEndOffset.y);
}

int AForEachUIBase::getContentMinimumWidth() {
    return AViewContainerBase::getContentMinimumWidth() + mFakeBeginOffset.valueOr(glm::ivec2(0)).x +
           mFakeEndOffset.valueOr(glm::ivec2(0)).x;
}
int AForEachUIBase::getContentMinimumHeight() {
    return AViewContainerBase::getContentMinimumHeight() + mFakeBeginOffset.valueOr(glm::ivec2(0)).y +
           mFakeEndOffset.valueOr(glm::ivec2(0)).y;
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
        const auto diffVec = glm::ivec2(scroll) - mLastInflatedScroll.valueOr(glm::ivec2 { 0 });
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
    mLastInflatedScroll.reset();
    inflateBackward();
    inflateForward();
}

void AForEachUIBase::inflateForward() {
    bool needsMinSizeUpdate = false;

    const auto posWithinSlidingSurface = calculateOffsetWithinViewportSlidingSurface();
    const auto lastScroll = glm::max(mLastInflatedScroll.valueOr(glm::ivec2(0)), glm::ivec2(0));

    // remove old views
    [&] {
        const auto uninflateFrom = lastScroll - posWithinSlidingSurface - glm::ivec2(RENDER_TO_TEXTURE_TILE_SIZE);
        auto firstValidView = ranges::find_if(mViews, [&](const _<AView>& view) {
            return glm::all(glm::greaterThan(view->getPosition(), uninflateFrom));
        });
        if (firstValidView == mViews.begin()) {
            return;
        }
        if (firstValidView == mViews.end()) {
            return;
        }

        mFakeBeginOffset = (*firstValidView)->getPosition() - (*firstValidView)->getMargin().leftTop();
        removeViews({ mViews.begin(), firstValidView });
    }();

    const auto end = mViewsModel.end();
    // append new views
    {
        const auto inflateTill =
            mViewport->getSize() + glm::ivec2(RENDER_TO_TEXTURE_TILE_SIZE) + lastScroll - posWithinSlidingSurface;
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
            applyGeometryToChildren();
        }
        if (needsMinSizeUpdate) {
            mCache->inflatedRange = { mCache->inflatedRange.begin(), it };
        }
    }

    if (needsMinSizeUpdate) {
        markMinContentSizeInvalid();
    } else {
        applyGeometryToChildren();
    }
}

void AForEachUIBase::inflateBackward() {}

glm::ivec2 AForEachUIBase::calculateOffsetWithinViewportSlidingSurface() {
    AUI_ASSERT(mViewport != nullptr);

    glm::ivec2 accumulator {};
    for (AView* p = this; p != nullptr; p = p->getParent()) {
        if (mViewport.get() == p->getParent()) {
            return accumulator;
        }
        accumulator += p->getPosition();
    }
    AUI_ASSERT_NO_CONDITION("divergent mViewport");
    return {};
}
