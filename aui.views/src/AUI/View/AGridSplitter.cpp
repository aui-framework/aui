// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

//
// Created by Alex2772 on 11/11/2021.
//

#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/Util/ALayoutDirection.h>
#include "AGridSplitter.h"

static const auto CLICK_BIAS = 8_dp;

AGridSplitter::AGridSplitter():
    mHorizontalHelper(ALayoutDirection::HORIZONTAL),
    mVerticalHelper(ALayoutDirection::VERTICAL)
{

}

void AGridSplitter::onPointerPressed(const APointerPressedEvent& event) {
    AViewContainer::onPointerPressed(event);
    auto mousePos = event.position;
    bool doVerticalDrag = true;
    for (auto& r : mItems) {
        auto& v = r.first();
        auto viewPos = v->getPosition().y + CLICK_BIAS.getValuePx();
        auto viewSize = v->getSize().y - CLICK_BIAS.getValuePx() * 2.f;

        if (mousePos.y > viewPos && mousePos.y < viewPos + viewSize) {
            doVerticalDrag = false;
            break;
        }

        if (mousePos.y <= viewPos) {
            break;
        }
    }
    bool doHorizontalDrag = true;
    for (auto& v : mItems.first()) {
        auto viewPos = v->getPosition().x + CLICK_BIAS.getValuePx();
        auto viewSize = v->getSize().x - CLICK_BIAS.getValuePx() * 2.f;

        if (mousePos.x > viewPos && mousePos.x < viewPos + viewSize) {
            doHorizontalDrag = false;
            break;
        }

        if (mousePos.x <= viewPos) {
            break;
        }
    }
    if (doVerticalDrag) {
        mVerticalHelper.beginDrag(mousePos);
    }
    if (doHorizontalDrag) {
        mHorizontalHelper.beginDrag(mousePos);
    }
}

void AGridSplitter::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AViewContainer::onPointerMove(pos, {});
    mVerticalHelper.mouseDrag(pos);
    mHorizontalHelper.mouseDrag(pos);

    if (mVerticalHelper.isDragging() || mHorizontalHelper.isDragging()) {
        updateLayout();
        redraw();
    }
}

void AGridSplitter::onPointerReleased(const APointerReleasedEvent& event) {
    AViewContainer::onPointerReleased(event);
    mVerticalHelper.endDrag();
    mHorizontalHelper.endDrag();
}

void AGridSplitter::updateSplitterItems() {
    setLayout(_new<AAdvancedGridLayout>(mItems.first().size(), mItems.size()));
    AVector<_<AView>> horizontal;
    AVector<_<AView>> vertical;

    for (auto& row : mItems) {
        for (auto& v : row) {
            addView(v);
        }
        vertical << row.first();
    }
    for (auto& v : mItems.first()) {
        horizontal << v;
    }

    mVerticalHelper.setItems(std::move(vertical));
    mHorizontalHelper.setItems(std::move(horizontal));
}

bool AGridSplitter::consumesClick(const glm::ivec2& pos) {
    return true;
}
