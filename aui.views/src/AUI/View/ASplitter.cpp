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

#include <range/v3/view/transform.hpp>

#include "ASplitter.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Layout/HVLayout.h>

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
template <aui::derived_from<ALayout> BaseLayout>
class ASplitterLayout final : public BaseLayout {
public:
    /**
     * @brief AView-like object that forwards some methods from AView and injects overridedSize if set.
     */
    struct SizeInjector {
        const ASplitterHelper::Item& item;
        auto operator->() const { return this; }

#define ASPLITTER_IMPL_FORWARD_METHOD(name)                  \
    template <typename... Args>                              \
    auto name(Args&&... args) const {                        \
        return item.view->name(std::forward<Args>(args)...); \
    }
        ASPLITTER_IMPL_FORWARD_METHOD(ensureAssUpdated)
        ASPLITTER_IMPL_FORWARD_METHOD(getExpanding)
        ASPLITTER_IMPL_FORWARD_METHOD(getVisibility)
        ASPLITTER_IMPL_FORWARD_METHOD(getMargin)
        ASPLITTER_IMPL_FORWARD_METHOD(setGeometry)
        ASPLITTER_IMPL_FORWARD_METHOD(getSize)
        ASPLITTER_IMPL_FORWARD_METHOD(getFixedSize)
        ASPLITTER_IMPL_FORWARD_METHOD(getMaxSize)

        glm::ivec2 getMinimumSize() const {
            auto size = item.view->getMinimumSize();
            if (item.overridedSize) {
                auto& value = aui::layout_direction::getAxisValue(BaseLayout::DIRECTION, size);
                value = glm::max(*item.overridedSize, value);
            }
            return size;
        }

        bool operator==(const SizeInjector& rhs) const noexcept { return item.view == rhs.item.view; }
    };

    ranges::range auto viewsWithFixedSizeInjected() {
        return mSplitterHelper.items() |
               ranges::views::transform([](const ASplitterHelper::Item& item) { return SizeInjector { item }; });
    }

    ASplitterLayout(ASplitterHelper& splitterHelper) : mSplitterHelper(splitterHelper) {}

    using HVLayout = aui::HVLayout<BaseLayout::DIRECTION>;

    void onResize(int x, int y, int width, int height) override {
        glm::ivec2 paddedPos = { x, y };
        glm::ivec2 paddedSize = { width, height };

        // reclaim space if provided size is less than the current layout size.
        if constexpr (BaseLayout::DIRECTION == ALayoutDirection::HORIZONTAL) {
            const auto fixedSizeMinWidth = HVLayout::getMinimumWidth(viewsWithFixedSizeInjected(), BaseLayout::getSpacing());
            if (width < fixedSizeMinWidth) {
                mSplitterHelper.reclaimSpace(width - fixedSizeMinWidth);
            }
        }
        if constexpr (BaseLayout::DIRECTION == ALayoutDirection::VERTICAL) {
            const auto fixedSizeMinHeight = HVLayout::getMinimumHeight(viewsWithFixedSizeInjected(), BaseLayout::getSpacing());
            if (height < fixedSizeMinHeight) {
                mSplitterHelper.reclaimSpace(height - fixedSizeMinHeight);
            }
        }

        HVLayout::onResize(paddedPos, paddedSize, viewsWithFixedSizeInjected(), BaseLayout::getSpacing());
    }

    ~ASplitterLayout() override = default;

private:
    ASplitterHelper& mSplitterHelper;
};
}   // namespace

template <typename Layout>
_<AView> ASplitter::Builder<Layout>::build() {
    auto splitter = aui::ptr::manage(new ASplitter);
    splitter->setExpanding(mExpanding);
    splitter->mHelper.setDirection(Layout::DIRECTION);

    splitter->setLayout(std::make_unique<ASplitterLayout<Layout>>(splitter->mHelper));

    bool atLeastOneItemHasExpanding = false;
    for (auto& item : mItems) {
        splitter->addView(item);
        item->ensureAssUpdated();
        atLeastOneItemHasExpanding |= splitter->mHelper.getAxisValue(item->getExpanding()) > 0;
    }
    if (!atLeastOneItemHasExpanding) {
        auto spacer = _new<ASpacerExpanding>();
        splitter->addView(spacer);
        mItems << spacer;
    }

    splitter->mHelper.setItems(std::move(mItems));

    return splitter;
}

template _<AView> ASplitter::Builder<AHorizontalLayout>::build();
template _<AView> ASplitter::Builder<AVerticalLayout>::build();
