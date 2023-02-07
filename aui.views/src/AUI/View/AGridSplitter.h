// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once


#include "AViewContainer.h"
#include "ASplitterHelper.h"
#include <AUI/Util/UIBuildingHelpers.h>

/**
 * @brief A resizable grid layout.
 * @ingroup useful_views
 * @details
 * AGridSplitter represents a grid layout which can be resized by user. Unlike ASplitter, works in both directions.
 * AGridSplitter can be constructed with AGridSplitter::Builder.
 */
class API_AUI_VIEWS AGridSplitter: public AViewContainer {
public:
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
            auto splitter = aui::ptr::manage(new AGridSplitter);
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

public:
    void updateSplitterItems();

    void onPointerPressed(const APointerPressedEvent& event) override;

    void onMouseMove(glm::ivec2 pos) override;

    void onPointerReleased(const APointerReleasedEvent& event) override;

    bool consumesClick(const glm::ivec2& pos) override;
};


