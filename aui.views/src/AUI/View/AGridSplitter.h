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

#pragma once


#include "AViewContainer.h"
#include "ASplitterHelper.h"
#include <AUI/Util/UIBuildingHelpers.h>

/**
 * @brief A resizable grid layout.
 *
 * ![](imgs/Views/AGridSplitter.png)
 *
 * @ingroup views_actions
 * @details
 * AGridSplitter represents a grid layout which can be resized by user. Unlike ASplitter, works in both directions.
 * AGridSplitter can be constructed with AGridSplitter::Builder.
 */
class API_AUI_VIEWS AGridSplitter: public AViewContainerBase {
public:
    friend class AGridSplitterLayout;
    class Builder {
        friend class ASplitter;
    private:
        AVector<AVector<_<AView>>> mItems;
        bool mAddSpacers = true;

    public:
        Builder& withItems(AVector<AVector<_<AView>>> items) {
            mItems = std::move(items);
            return *this;
        }

        Builder& noDefaultSpacers() {
            mAddSpacers = false;
            return *this;
        }

        _<AView> build() {
            auto splitter = aui::ptr::manage_shared(new AGridSplitter);
            if (mAddSpacers) {
                for (auto& row: mItems) {
                    row.push_back(_new<ASpacerExpanding>());
                }
                mItems.push_back(
                        AVector<_<AView>>::generate(mItems.first().size(), [](size_t) { return _new<ASpacerExpanding>(); }));
            }
            splitter->mItems = std::move(mItems);
            splitter->updateSplitterItems();
            return splitter;
        }

        operator _<AView>() {
            return build();
        }
    };

private:
    friend class Builder;
    AVector<AVector<_<AView>>> mItems;
    ASplitterHelper mHorizontalHelper;
    ASplitterHelper mVerticalHelper;

    AGridSplitter();

    glm::bvec2 isDraggingArea(glm::ivec2 position);

public:
    void updateSplitterItems();

    void onPointerPressed(const APointerPressedEvent& event) override;

    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;

    void onPointerReleased(const APointerReleasedEvent& event) override;

    bool consumesClick(const glm::ivec2& pos) override;
};


