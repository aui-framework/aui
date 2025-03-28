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
static constexpr auto INFLATE_THRESHOLD_PX = RENDER_TO_TEXTURE_TILE_SIZE / 2;
static constexpr auto INFLATE_STEP_PX = RENDER_TO_TEXTURE_TILE_SIZE * 2;
static constexpr auto POTENTIAL_PERFORMANCE_ISSUE_VIEWS_COUNT_THRESHOLD = 100;
static constexpr auto CACHE_COUNT_CAP = 50;
static constexpr auto LOG_TAG = "AForEachUIBase";

void AForEachUIBase::setModelImpl(AForEachUIBase::List model) {
    putOurViewsToSharedCache();
    mViewsModelCapabilities = model.capabilities();
    mViewsModel = std::move(model);
}

void AForEachUIBase::putOurViewsToSharedCache() {
    removeAllViews();
    if (!mCache) {
        return;
    }
    if (auto viewsCache = getViewsCache()) {
        if (viewsCache->size() + mCache->items.size() > 50) {
            viewsCache->clear();
        }
        for (auto& e : mCache->items) {
            (*viewsCache)[e.id] = std::move(e.view);
            ALOG_DEBUG(LOG_TAG) << this << "(" << AReflect::name(this) << ") Cached view for id: " << e.id;
        }
    }
    mCache.reset();
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

    if (mViews.empty()) {
        return;
    }
    //    ALOG_DEBUG(LOG_TAG) << this << " compensateLayoutUpdatesByScroll";
    mViewport->compensateLayoutUpdatesByScroll(
        mViews.first(), [this] { AViewContainerBase::applyGeometryToChildren(); }, axisMask());
}

void AForEachUIBase::onViewGraphSubtreeChanged() {
    ALOG_DEBUG(LOG_TAG) << this << "(" << AReflect::name(this) << ") onViewGraphSubtreeChanged";
    AViewContainerBase::onViewGraphSubtreeChanged();

    AUI_DEFER {
        // if parent AUI_DECLARATIVE_FOR was invalidated, it would call ours onViewGraphSubtreeChanged. We might depend
        // on parent's AUI_DECLARATIVE_FOR subrange, so we put our views to shared cache.
        putOurViewsToSharedCache();
    };

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
        if (glm::abs(diff) < INFLATE_THRESHOLD_PX) {
            return;
        }
        mLastInflatedScroll = glm::ivec2(scroll);

        getThread()->enqueue([this, keepMeAlive = shared_from_this(), diff] {
            if (getParent() == nullptr) {
                // lost parent before queue message was processed - no need to operate.
                return;
            }
            inflate({ .backward = diff < 0, .forward = diff > 0 });
            mLastInflatedScroll = *mViewport->scroll();
        });
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

void AForEachUIBase::inflate(aui::for_each_ui::detail::InflateOpts opts) {
    AUI_ASSERT(opts.forward || opts.backward);

#if AUI_DEBUG
    const auto prevViewsCount = mViews.size();
    AUI_DEFER {
        if (glm::abs(int(prevViewsCount) - int(mViews.size())) > POTENTIAL_PERFORMANCE_ISSUE_VIEWS_COUNT_THRESHOLD) {
            ALogger::warn(LOG_TAG)
                << "It appears like a large update to inflated AForEachUIBase views caused (view count "
                << prevViewsCount << " -> " << mViews.size()
                << "), which signals about a potential performance issue. If you really intend to work with >"
                << POTENTIAL_PERFORMANCE_ISSUE_VIEWS_COUNT_THRESHOLD
                << " views visible, you might end up ignoring this warning (it appears on debug build only). "
                   "Otherwise, "
                   "please report it.";
        }
    };
#endif

    bool needsMinSizeUpdate = false;

    auto posWithinSlidingSurface = calculateOffsetWithinViewportSlidingSurface();
    auto lastScroll = glm::max(mLastInflatedScroll.valueOr(glm::ivec2(0)), glm::ivec2(0));

    // remove old views
    if (mViewsModelCapabilities.implementsOperatorMinusMinus) {
        if (opts.backward && mCache) {
            [&] {
                const auto uninflateFrom =
                    lastScroll - posWithinSlidingSurface + glm::ivec2(INFLATE_STEP_PX) + mViewport->getSize() * 3;
                auto firstValidView =
                    ranges::find_if(mViews | ranges::views::reverse, [&](const _<AView>& view) {
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
                const auto uninflateFrom =
                    lastScroll - posWithinSlidingSurface - glm::ivec2(INFLATE_STEP_PX) - mViewport->getSize() * 2;
                auto firstValidView = ranges::find_if(mViews, [&](const _<AView>& view) {
                    return glm::all(glm::greaterThan(view->getPosition() + view->getSize(), uninflateFrom));
                });
                if (firstValidView == mViews.begin()) {
                    return;
                }
                if (firstValidView == mViews.end()) {
                    return;
                }

                removeViews({ mViews.begin(), firstValidView });
            }();
        }
    }

    const auto end = mViewsModel.end();
    // append new views
    if (opts.backward && mCache && !mCache->items.empty() && mViewsModelCapabilities.implementsOperatorMinusMinus) {
        auto inflateTill =
            lastScroll - glm::ivec2(INFLATE_STEP_PX) - posWithinSlidingSurface - mViews.first()->getPosition();
        for (auto it = mCache->items.first().iterator;
             it != mViewsModel.begin() && glm::all(glm::lessThanEqual(inflateTill, glm::ivec2(0)));) {
            --it;
            auto prevFirstView = mViews.first();
            addView(it, 0);

            mViewport->compensateLayoutUpdatesByScroll(
                prevFirstView, [this] { AViewContainerBase::applyGeometryToChildren(); }, axisMask());
            auto diff = prevFirstView->getPosition() - mViews.first()->getPosition();
            inflateTill += diff;

            if (it == mViewsModel.begin()) {
                break;
            }
        }
    }

    if (opts.forward) {
        const auto inflateTill =
            mViewport->getSize() + glm::ivec2(INFLATE_STEP_PX) + lastScroll - posWithinSlidingSurface;
        auto it = [&] {
            if (mCache) {
                if (!mCache->items.empty()) {
                    return std::next(mCache->items.last().iterator);
                }
            }
            return mViewsModel.begin();
        }();
        for (; it != end; ++it) {
            if (!mViews.empty()) {
                const auto& lastView = mViews.last();
                if (glm::any(glm::greaterThanEqual(lastView->getPosition() + lastView->getSize(), inflateTill))) {
                    break;
                }
            }
            addView(it);
            needsMinSizeUpdate = true;
            mViewport->compensateLayoutUpdatesByScroll(
                mViews.first(), [this] { AViewContainerBase::applyGeometryToChildren(); }, axisMask());
        }
    }

    if (needsMinSizeUpdate) {
        markMinContentSizeInvalid();
    }
}

glm::ivec2 AForEachUIBase::calculateOffsetWithinViewportSlidingSurface() {
    AUI_ASSERTX(mViewport != nullptr, "can't be used without a viewport set");
    AUI_ASSERTX(getParent() != nullptr, "parent is undefined");

    glm::ivec2 accumulator {};
    for (AView* p = this; p != nullptr; p = p->getParent()) {
        if (mViewport.get() == p->getParent()) {
            return accumulator;
        }
        accumulator += p->getPosition();
    }
    ALogger::err(LOG_TAG) << "Divergent mViewport! this = " << this;
    //    AUI_ASSERT_NO_CONDITION("divergent mViewport");
    return {};
}

void AForEachUIBase::addView(List::iterator iterator, AOptional<std::size_t> index) {
    if (!mCache) {
        if (index) {
            AViewContainerBase::addView(*index, (*iterator).view);
        } else {
            AViewContainerBase::addView((*iterator).view);
        }
        return;
    }
    AUI_ASSERT(mViews.size() == mCache->items.size());
    Cache::LazyListItemInfo entry { *iterator };
    if (index) {
        AViewContainerBase::addView(*index, entry.view);
    } else {
        AViewContainerBase::addView(entry.view);
    }
    auto at = mCache->items.end();
    if (index) {
        at = mCache->items.begin() + *index;
    }
    entry.iterator = std::move(iterator);
    mCache->items.insert(at, std::move(entry));
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

glm::ivec2 AForEachUIBase::axisMask() {
    if (getLayout()->getLayoutDirection() == ALayoutDirection::HORIZONTAL) {
        return glm::ivec2 { 1, 0 };
    }
    return glm::ivec2 { 0, 1 };
}
