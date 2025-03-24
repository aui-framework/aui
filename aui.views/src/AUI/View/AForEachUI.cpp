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
            mCache.emplace();
            removeAllViews();
            for (auto i = mViewsModel.begin(); i != mViewsModel.end(); ++i) {
                addView(i);
            }
        }
        AViewContainerBase::applyGeometryToChildren();
        return;
    }

    if (!mLastInflatedScroll) {
        mLastInflatedScroll = -calculateOffsetWithinViewportSlidingSurface();
    }

    if (!mCache) {
        mCache.emplace();
        inflate();
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

        inflate({ .backward = diff < 0, .forward = diff > 0 });
    });
    mLastInflatedScroll.reset();
    mViewport = std::move(viewport);
}

void AForEachUIBase::setPosition(glm::ivec2 position) {
    auto prevPosition = getPosition();
    AView::setPosition(position);
    if (getPosition() == prevPosition) {
        return;
    }
    if (!mCache) {
        return;
    }
    mLastInflatedScroll.reset();
    inflate();
}

void AForEachUIBase::inflate(aui::detail::InflateOpts opts) {
    AUI_ASSERT(opts.forward || opts.backward);
    bool needsMinSizeUpdate = false;

    const auto posWithinSlidingSurface = calculateOffsetWithinViewportSlidingSurface();
    const auto lastScroll = glm::max(mLastInflatedScroll.valueOr(glm::ivec2(0)), glm::ivec2(0));

    // remove old views
    if (opts.backward && mCache) {
        [&] {
          const auto uninflateFrom = lastScroll - posWithinSlidingSurface + glm::ivec2(RENDER_TO_TEXTURE_TILE_SIZE) + mViewport->getSize();
          auto firstValidView = ranges::find_if(mViews | ranges::views::reverse, [&](const _<AView>& view) {
            return glm::all(glm::lessThanEqual(view->getPosition(), uninflateFrom));
          }).base();
          if (firstValidView == mViews.end()) {
              return;
          }
          removeViews({ firstValidView, mViews.end() });
        }();
    }
    if (opts.forward) {
        [&] {
            const auto uninflateFrom = lastScroll - posWithinSlidingSurface - glm::ivec2(RENDER_TO_TEXTURE_TILE_SIZE);
            auto firstValidView = ranges::find_if(mViews, [&](const _<AView>& view) {
                return glm::all(glm::greaterThan(view->getPosition() + view->getSize(), uninflateFrom));
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
    }

    const auto end = mViewsModel.end();
    // append new views
    if (opts.backward && mCache && !mCache->items.empty()) {
        const auto inflateTill = lastScroll - glm::ivec2(RENDER_TO_TEXTURE_TILE_SIZE) - posWithinSlidingSurface;
        for (auto it = mCache->items.first().iterator; it != mViewsModel.begin();) {
            --it;
            if (!mViews.empty()) {
                const auto& firstView = mViews.first();
            }
            if (it == mViewsModel.begin()) {
                mFakeBeginOffset.reset();
                break;
            }
        }
    }

    if (opts.forward) {
        const auto inflateTill =
            mViewport->getSize() + glm::ivec2(RENDER_TO_TEXTURE_TILE_SIZE) + lastScroll - posWithinSlidingSurface;
        for (auto it = mCache->items.empty() ? mViewsModel.begin() : std::next(mCache->items.last().iterator); it != end; ++it) {
            if (!mViews.empty()) {
                const auto& lastView = mViews.last();
                if (glm::any(glm::greaterThanEqual(lastView->getPosition() + lastView->getSize(), inflateTill))) {
                    break;
                }
            }
            addView(it);
            needsMinSizeUpdate = true;
            applyGeometryToChildren();
        }
    }

    if (needsMinSizeUpdate) {
        markMinContentSizeInvalid();
    } else {
        applyGeometryToChildren();
    }
}

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
}

void AForEachUIBase::addView(List::iterator iterator) {
    if (!mCache) {
        AViewContainerBase::addView(*iterator);
        return;
    }
    AUI_ASSERT(mViews.size() == mCache->items.size());
    auto view = *iterator;
    AViewContainerBase::addView(view);
    mCache->items.push_back({ .iterator = std::move(iterator), .view = std::move(view) });
}

void AForEachUIBase::removeViews(aui::range<AVector<_<AView>>::iterator> iterators) {
    if (!mCache) {
        AViewContainerBase::removeViews(iterators);
        return;
    }
    AUI_ASSERT(mViews.size() == mCache->items.size());
    auto idx = std::distance(mViews.begin(), iterators.begin());
    auto size = iterators.size();
    AViewContainerBase::removeViews(iterators);
    mCache->items.erase(mCache->items.begin() + idx, mCache->items.begin() + idx + size);
}
