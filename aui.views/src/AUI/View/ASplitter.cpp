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

#include "ASplitter.h"
#include <AUI/Layout/HVLayout.h>
#include <AUI/Util/SplitterSizeInjector.h>
#include <AUI/Util/UIBuildingHelpers.h>

ASplitter::ASplitter() {}

void ASplitter::onPointerPressed(const APointerPressedEvent& event) {
    if (mHelper.isDraggingArea(event.position)) {
        AView::onPointerPressed(event);   // NOLINT(*-parent-virtual-call)
        mHelper.beginDrag(event.position);
        return;
    }
    AViewContainerBase::onPointerPressed(event);
}

void ASplitter::setSize(glm::ivec2 size) { AViewContainerBase::setSize(size); }

void ASplitter::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    if (mHelper.isDragging() || (mHelper.isDraggingArea(pos) && !isPressed())) {
        setCursor(mHelper.mDirection == ALayoutDirection::HORIZONTAL ? ACursor::EW_RESIZE : ACursor::NS_RESIZE);
        AView::onPointerMove(pos, event);   // NOLINT(*-parent-virtual-call)
        if (mHelper.mouseDrag(pos)) {
            applyGeometryToChildrenIfNecessary();
            redraw();
        }
        return;
    }
    setCursor({});
    AViewContainerBase::onPointerMove(pos, event);
}

void ASplitter::onPointerReleased(const APointerReleasedEvent& event) {
    if (mHelper.isDragging()) {
        AView::onPointerReleased(event);   // NOLINT(*-parent-virtual-call)
        mHelper.endDrag();
        return;
    }
    AViewContainerBase::onPointerReleased(event);
}

void ASplitter::onClickPrevented() {
    AViewContainerBase::onClickPrevented();
    mHelper.endDrag();
}

namespace {
template <ALayoutDirection Direction>
class ASplitterLayout final : public ALayout {
public:
    ASplitterLayout(ASplitterHelper& splitterHelper) : mSplitterHelper(splitterHelper) {}

    void setSpacing(int spacing) override {
        if (mSpacing == spacing) {
            return;
        }
        mSpacing = spacing;
        ALayout::requestLayout();
    }

    void addView(const _<AView>& view, AOptional<size_t> index = std::nullopt) override {
        auto at = mViews.end();
        if (index) {
            at = mViews.begin() + *index;
        }
        mViews.insert(at, view);
        mSplitterHelper.setItems(mViews);
        ALayout::requestLayout();
    }

    void removeView(aui::no_escape<AView> view, size_t index) override {
        AUI_ASSERT(mViews[index].get() == view.ptr());
        mViews.removeAt(index);
        mSplitterHelper.setItems(mViews);
        ALayout::requestLayout();
    }

    AVector<_<AView>> getAllViews() override {
        return mViews;
    }

    ALayoutDirection getLayoutDirection() override {
        return Direction;
    }

    void layout(int x, int y, int width, int height) override {
        auto injected = viewsWithFixedSizeInjected();
        aui::HVLayout<Direction>::layout({ x, y }, { width, height }, injected, mSpacing);
    }

    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        auto injected = viewsWithFixedSizeInjected();
        return aui::HVLayout<Direction>::onIntrinsicMeasure(injected, mSpacing, constraints);
    }

    AMinMaxAxis onComputeIntrinsicMinMaxAxis(int height) override {
        auto injected = viewsWithFixedSizeInjected();
        auto result = aui::HVLayout<Direction>::computeIntrinsicMinMaxSizes(injected, mSpacing);
        result.max = aui::HVLayout<Direction>::preferredWidth(injected, mSpacing, height);
        result.max = glm::max(result.max, result.min);
        return result;
    }

    ~ASplitterLayout() override = default;

private:
    ASplitterHelper& mSplitterHelper;
    AVector<_<AView>> mViews;
    int mSpacing = 0;
    AVector<SizeInjector<Direction>> viewsWithFixedSizeInjected() const {
        AVector<SizeInjector<Direction>> result;
        result.reserve(mSplitterHelper.items().size());
        for (const auto& item : mSplitterHelper.items()) {
            result << SizeInjector<Direction> { item };
        }
        return result;
    }
};
}   // namespace

template <typename Layout>
_<AView> ASplitter::Builder<Layout>::build() {
    auto splitter = aui::ptr::manage_shared(new ASplitter);
    splitter->setExpanding(mExpanding);
    splitter->mHelper.setDirection(Layout::DIRECTION);

    splitter->setLayout(std::make_unique<ASplitterLayout<Layout::DIRECTION>>(splitter->mHelper));

    bool atLeastOneItemHasExpanding = false;
    for (auto& item : mItems) {
        splitter->addView(item);
        item->ensureAssUpdated();
        atLeastOneItemHasExpanding |= splitter->mHelper.getAxisValue(item->getExpanding()) > 0;
    }
    if (!atLeastOneItemHasExpanding) {
        for (auto& item : mItems) {
            auto expanding = item->getExpanding();
            splitter->mHelper.getAxisValue(expanding) = 1;
            item->setExpanding(expanding);
        }
    }

    splitter->mHelper.setItems(std::move(mItems));

    return splitter;
}

template _<AView> ASplitter::Builder<AHorizontalLayout>::build();
template _<AView> ASplitter::Builder<AVerticalLayout>::build();
