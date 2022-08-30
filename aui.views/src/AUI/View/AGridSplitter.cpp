//
// Created by Alex2772 on 11/11/2021.
//

#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/Util/ALayoutDirection.h>
#include "AGridSplitter.h"

const auto CLICK_BIAS = 8_dp;

AGridSplitter::AGridSplitter():
    mHorizontalHelper(ALayoutDirection::HORIZONTAL),
    mVerticalHelper(ALayoutDirection::VERTICAL)
{

}

void AGridSplitter::onMousePressed(glm::ivec2 mousePos, AInput::Key button) {
    AViewContainer::onMousePressed(mousePos, button);
    bool doVerticalDrag = true;
    for (auto& r : mItems) {
        auto& v = r.first();
        auto viewPos = v->getPosition().y;
        auto viewSize = v->getSize().y;

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
        auto viewSize = v->getSize().x + - CLICK_BIAS.getValuePx() * 2.f;

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

void AGridSplitter::onMouseMove(glm::ivec2 pos) {
    AViewContainer::onMouseMove(pos);
    mVerticalHelper.mouseDrag(pos);
    mHorizontalHelper.mouseDrag(pos);

    if (mVerticalHelper.isDragging() || mHorizontalHelper.isDragging()) {
        updateLayout();
        redraw();
    }
}

void AGridSplitter::onMouseReleased(glm::ivec2 pos, AInput::Key button) {
    AViewContainer::onMouseReleased(pos, button);
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
