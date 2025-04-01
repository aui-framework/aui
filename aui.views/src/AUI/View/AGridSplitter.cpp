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

//
// Created by Alex2772 on 11/11/2021.
//

#include <range/v3/view/transform.hpp>
#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/Util/ALayoutDirection.h>
#include "AGridSplitter.h"
#include "AUI/Layout/HVLayout.h"
#include <AUI/Util/SplitterSizeInjector.h>

class AGridSplitterLayout : public AAdvancedGridLayout {
public:
    AGridSplitterLayout(int cellsX, int cellsY, AGridSplitter& gridSplitter)
      : AAdvancedGridLayout(cellsX, cellsY), mGridSplitter(gridSplitter) {}
    ~AGridSplitterLayout() override = default;
    void onResize(int x, int y, int width, int height) override {
        {
            auto splitterWidth = aui::HVLayout<ALayoutDirection::HORIZONTAL>::getMinimumWidth(
                mGridSplitter.mHorizontalHelper.items() |
                    ranges::views::transform([](const ASplitterHelper::Item& item) {
                        return SizeInjector<ALayoutDirection::HORIZONTAL> { item };
                    }),
                mSpacing);

            if (width < splitterWidth) {
                mGridSplitter.mHorizontalHelper.reclaimSpace(width - splitterWidth);
            }
        }

        {
            auto splitterHeight = aui::HVLayout<ALayoutDirection::VERTICAL>::getMinimumHeight(
                mGridSplitter.mVerticalHelper.items() | ranges::views::transform([](const ASplitterHelper::Item& item) {
                    return SizeInjector<ALayoutDirection::VERTICAL> { item };
                }),
                mSpacing);

            if (height < splitterHeight) {
                mGridSplitter.mVerticalHelper.reclaimSpace(height - splitterHeight);
            }
        }

        AAdvancedGridLayout::onResize(x, y, width, height);
    }

protected:
    void prepareCache(AVector<CompositionCache>& columns, AVector<CompositionCache>& rows) override {
        for (auto& cell : mCells) {
            cell.view->ensureAssUpdated();
            if (!(cell.view->getVisibility() & Visibility::FLAG_CONSUME_SPACE))
                continue;

            glm::ivec2 e = { cell.view->getExpandingHorizontal(), cell.view->getExpandingVertical() };
            auto fixed = cell.view->getFixedSize();
            if (fixed.x != 0)
                e.x = 0;
            if (fixed.y != 0)
                e.y = 0;
            glm::ivec2 m = { cell.view->getMinimumWidth(), cell.view->getMinimumHeight() };
            if (cell.x == 0) {
                auto& i = mGridSplitter.mVerticalHelper.items()[cell.y];
                m.y = glm::max(m.y, i.overridedSize.valueOr(0));
            }
            if (cell.y == 0) {
                auto& i = mGridSplitter.mHorizontalHelper.items()[cell.x];
                m.x = glm::max(m.x, i.overridedSize.valueOr(0));
            }
            glm::ivec2 minSpace =
                m + glm::ivec2 { cell.view->getMargin().horizontal(), cell.view->getMargin().vertical() };

            columns[cell.x].expandingSum += e.x;
            rows[cell.y].expandingSum += e.y;

            columns[cell.x].minSize = glm::max(columns[cell.x].minSize, minSpace.x);
            rows[cell.y].minSize = glm::max(rows[cell.y].minSize, minSpace.y);
        }
    }

private:
    AGridSplitter& mGridSplitter;
};

AGridSplitter::AGridSplitter()
  : mHorizontalHelper(ALayoutDirection::HORIZONTAL), mVerticalHelper(ALayoutDirection::VERTICAL) {}

glm::bvec2 AGridSplitter::isDraggingArea(glm::ivec2 position) {
    return { mHorizontalHelper.isDraggingArea(position), mVerticalHelper.isDraggingArea(position) };
}

void AGridSplitter::onPointerPressed(const APointerPressedEvent& event) {
    auto isDrag = isDraggingArea(event.position);
    if (isDrag.y) {
        mVerticalHelper.beginDrag(event.position);
    }
    if (isDrag.x) {
        mHorizontalHelper.beginDrag(event.position);
    }
    if (glm::any(isDrag)) {
        AView::onPointerPressed(event);   // NOLINT(*-parent-virtual-call)
        return;
    }
    AViewContainerBase::onPointerPressed(event);
}

void AGridSplitter::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    mVerticalHelper.mouseDrag(pos);
    mHorizontalHelper.mouseDrag(pos);

    if (mVerticalHelper.isDragging() || mHorizontalHelper.isDragging()) {
        applyGeometryToChildrenIfNecessary();
        redraw();
        AView::onPointerMove(pos, event);   // NOLINT(*-parent-virtual-call)
        return;
    }
    if (!isPressed()) {
        auto area = isDraggingArea(pos);
        if (area.x && area.y) {
            setCursor(ACursor::MOVE);
            AView::onPointerMove(pos, event);   // NOLINT(*-parent-virtual-call)
            return;
        }
        if (area.x) {
            setCursor(ACursor::EW_RESIZE);
            AView::onPointerMove(pos, event);   // NOLINT(*-parent-virtual-call)
            return;
        }
        if (area.y) {
            setCursor(ACursor::NS_RESIZE);
            AView::onPointerMove(pos, event);   // NOLINT(*-parent-virtual-call)
            return;
        }
    }
    setCursor({});
    AViewContainerBase::onPointerMove(pos, {});
}

void AGridSplitter::onPointerReleased(const APointerReleasedEvent& event) {
    if (mVerticalHelper.isDragging() || mHorizontalHelper.isDragging()) {
        mVerticalHelper.endDrag();
        mHorizontalHelper.endDrag();
        AView::onPointerReleased(event);   // NOLINT(*-parent-virtual-call)
        return;
    }

    AViewContainerBase::onPointerReleased(event);
}

void AGridSplitter::updateSplitterItems() {
    setLayout(std::make_unique<AGridSplitterLayout>(mItems.first().size(), mItems.size(), *this));
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

bool AGridSplitter::consumesClick(const glm::ivec2& pos) { return true; }
