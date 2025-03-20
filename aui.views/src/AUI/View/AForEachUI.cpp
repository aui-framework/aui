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

    if (!mCache) {
        mCache = Cache {
            .inflatedRange = { mViewsModel.begin(), mViewsModel.begin() },
        };
    }
    const auto viewportSize = mViewport->getSize();
    const auto end = mViewsModel.end();
    bool needsMinSizeUpdate = false;
    for (auto it = mCache->inflatedRange.end(); it != end; ++it) {
        if (!mViews.empty()) {
            const auto& lastView = mViews.last();
            if (glm::any(glm::greaterThanEqual(lastView->getPosition() + lastView->getSize(), viewportSize))) {
                break;
            }
        }
        addView(*it);
        needsMinSizeUpdate = true;
        AViewContainerBase::applyGeometryToChildren();
    }

    if (needsMinSizeUpdate) {
        markMinContentSizeInvalid();
    } else {
        AViewContainerBase::applyGeometryToChildren();
    }
}

void AForEachUIBase::onViewGraphSubtreeChanged() {
    AViewContainerBase::onViewGraphSubtreeChanged();
    mViewport = [&]() -> _<AScrollAreaViewport> {
        for (auto p = getParent(); p != nullptr; p = p->getParent()) {
            if (auto viewport = _cast<AScrollAreaViewport>(p->shared_from_this())) {
                return viewport;
            }
        }
        return nullptr;
    }();
}
