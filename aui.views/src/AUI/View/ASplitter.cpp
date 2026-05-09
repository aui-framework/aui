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
#include <AUI/Util/UIBuildingHelpers.h>
#include <limits>
#include <vector>

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
        for (const auto& view : mViews) {
            view->ensureAssUpdated();
        }

        const auto resolved = resolveMainAxisSizes(
            getAxisValue(glm::ivec2 { width, height }),
            getPerpAxisValue(glm::ivec2 { width, height }),
            true,
            false);

        int position = getAxisValue(glm::ivec2 { x, y });
        std::size_t index = 0;
        for (const auto& view : mViews) {
            const auto& item = resolved[index++];
            if (!item.visible) {
                continue;
            }

            const auto margin = view->getMargin();
            const int availablePerpendicular = std::max(
                0,
                getPerpAxisValue(glm::ivec2 { width, height } - margin.occupiedSize()));
            const int perpendicularSize = computePerpendicularSize(view, item.mainSize, availablePerpendicular);

            const int positionMain = position + getAxisValue(margin.leftTop());
            const int positionPerp = getPerpAxisValue(glm::ivec2 { x, y } + margin.leftTop());

            if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
                view->layout(positionMain, positionPerp, item.mainSize, perpendicularSize);
            } else {
                view->layout(positionPerp, positionMain, perpendicularSize, item.mainSize);
            }

            position += item.mainSize + getAxisValue(margin.occupiedSize()) + mSpacing;
        }
    }

    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        const bool boundedMain = [&] {
            if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
                return !constraints.isUnlimitedWidth();
            } else {
                return !constraints.isUnlimitedHeight();
            }
        }();
        const int perpendicularConstraint = [&] {
            if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
                return constraints.isUnlimitedHeight() ? -1 : constraints.maxHeight;
            } else {
                return constraints.isUnlimitedWidth() ? -1 : constraints.maxWidth;
            }
        }();
        const int availableMain = boundedMain
            ? getAxisValue(glm::ivec2 { constraints.maxWidth, constraints.maxHeight })
            : 0;

        const auto resolved = resolveMainAxisSizes(
            availableMain,
            perpendicularConstraint,
            boundedMain,
            !boundedMain);

        glm::ivec2 result {};
        for (std::size_t i = 0; i < mViews.size(); ++i) {
            const auto& view = mViews[i];
            const auto& item = resolved[i];
            if (!item.visible) {
                continue;
            }
            const auto margin = view->getMargin().occupiedSize();
            const int perpendicularSize = perpendicularConstraint == -1
                ? computePerpendicularMinMaxSizes(view, item.mainSize).max
                : computePerpendicularSize(view, item.mainSize, perpendicularConstraint);
            if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
                result.x += item.mainSize + margin.x;
                result.y = std::max(result.y, perpendicularSize + margin.y);
            } else {
                result.x = std::max(result.x, perpendicularSize + margin.x);
                result.y += item.mainSize + margin.y;
            }
        }
        const int totalSpacing = std::max(0, int(countVisibleItems()) - 1) * mSpacing;
        getAxisValue(result) += totalSpacing;
        return result;
    }

    AMinMaxSizes onComputeIntrinsicMinMaxSizes(int) override {
        AMinMaxSizes result;
        for (const auto& view : mViews) {
            view->ensureAssUpdated();
            if (!(view->getVisibility() & Visibility::FLAG_CONSUME_SPACE)) {
                continue;
            }

            const auto minMax = view->computeMinMaxSizes();
            const auto margin = view->getMargin().occupiedSize();
            const int minMain = computeMinimumMainAxisSize(view, -1);
            const int maxMain = computePreferredMainAxisSize(view, -1, true);
            const auto minAtMinMain = computePerpendicularMinMaxSizes(view, minMain);
            const auto minAtMaxMain = computePerpendicularMinMaxSizes(view, maxMain);

            if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
                result.min.x += minMain + margin.x;
                result.max.x += maxMain + margin.x;
                result.min.y = std::max(result.min.y, std::max(minAtMinMain.min, minMax.min.y) + margin.y);
                result.max.y = std::max(result.max.y, std::max(minAtMaxMain.max, minMax.min.y) + margin.y);
            } else {
                result.min.x = std::max(result.min.x, std::max(minAtMinMain.min, minMax.min.x) + margin.x);
                result.max.x = std::max(result.max.x, std::max(minAtMaxMain.max, minMax.min.x) + margin.x);
                result.min.y += minMain + margin.y;
                result.max.y += maxMain + margin.y;
            }
        }
        const int totalSpacing = std::max(0, int(countVisibleItems()) - 1) * mSpacing;
        if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
            result.min.x += totalSpacing;
            result.max.x += totalSpacing;
        } else {
            result.min.y += totalSpacing;
            result.max.y += totalSpacing;
        }
        return result;
    }

    ~ASplitterLayout() override = default;

private:
    struct ResolvedItem {
        bool visible = false;
        int weight = 0;
        int mainSize = 0;
        int minMainSize = 0;
        int maxMainSize = std::numeric_limits<int>::max();
    };

    ASplitterHelper& mSplitterHelper;
    AVector<_<AView>> mViews;
    int mSpacing = 0;

    template <typename T>
    static T& getAxisValue(glm::tvec2<T>& v) {
        return aui::layout_direction::getAxisValue(Direction, v);
    }

    template <typename T>
    static T getAxisValue(const glm::tvec2<T>& v) {
        return aui::layout_direction::getAxisValue(Direction, v);
    }

    template <typename T>
    static T getPerpAxisValue(const glm::tvec2<T>& v) {
        return aui::layout_direction::getPerpendicularAxisValue(Direction, v);
    }

    int computeStructuralMinimumMainAxisSize(const _<AView>& view, int perpendicularConstraint) const {
        const int fixedSize = getAxisValue(view->getFixedSize());
        if (fixedSize > 0) {
            return fixedSize;
        }

        if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
            return view->computeMinMaxSizes(perpendicularConstraint).min.x;
        } else {
            return perpendicularConstraint == -1
                ? view->computeMinMaxSizes().min.y
                : view->measure(AConstraints::fixedWidth(perpendicularConstraint)).y;
        }
    }

    int computeOverridenMainAxisSize(const _<AView>& view, AOptional<int> overridedSize, int perpendicularConstraint) const {
        const int minimum = computeStructuralMinimumMainAxisSize(view, perpendicularConstraint);
        if (overridedSize) {
            return std::max(minimum, *overridedSize);
        }

        return -1;
    }

    int computePreferredMainAxisSize(const _<AView>& view, int perpendicularConstraint, bool expandingAsPreferred) const {
        const auto mainOverride = computeOverridenMainAxisSize(view, findOverridedSize(view), perpendicularConstraint);
        if (mainOverride != -1) {
            return mainOverride;
        }

        const int expanding = getAxisValue(view->getExpanding());
        if (expanding > 0 && !expandingAsPreferred) {
            return computeMinimumMainAxisSize(view, perpendicularConstraint);
        }

        if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
            return perpendicularConstraint == -1
                ? view->computeMinMaxSizes().max.x
                : view->measure(AConstraints::fixedHeight(perpendicularConstraint)).x;
        } else {
            return perpendicularConstraint == -1
                ? view->computeMinMaxSizes().max.y
                : view->measure(AConstraints::fixedWidth(perpendicularConstraint)).y;
        }
    }

    int computeMinimumMainAxisSize(const _<AView>& view, int perpendicularConstraint) const {
        const auto mainOverride = computeOverridenMainAxisSize(view, findOverridedSize(view), perpendicularConstraint);
        if (mainOverride != -1) {
            return mainOverride;
        }

        return computeStructuralMinimumMainAxisSize(view, perpendicularConstraint);
    }

    struct PerpendicularMinMaxSizes {
        int min = 0;
        int max = 0;
    };

    PerpendicularMinMaxSizes computePerpendicularMinMaxSizes(const _<AView>& view, int mainSize) const {
        const auto minMax = view->computeMinMaxSizes(mainSize);
        if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
            return {
                .min = minMax.min.y,
                .max = minMax.max.y,
            };
        } else {
            return {
                .min = minMax.min.x,
                .max = minMax.max.x,
            };
        }
    }

    int computePerpendicularSize(const _<AView>& view, int mainSize, int availablePerpendicular) const {
        if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
            const int minimum = view->computeMinMaxSizes(mainSize).min.y;
            if (availablePerpendicular != -1 &&
                getPerpAxisValue(view->getExpanding()) != 0 &&
                getPerpAxisValue(view->getFixedSize()) == 0) {
                return std::max(minimum, availablePerpendicular);
            }
            return std::max(minimum, view->measure({
                .minWidth = mainSize,
                .maxWidth = mainSize,
                .maxHeight = availablePerpendicular,
            }).y);
        } else {
            const int minimum = view->computeMinMaxSizes(mainSize).min.x;
            if (availablePerpendicular != -1 &&
                getPerpAxisValue(view->getExpanding()) != 0 &&
                getPerpAxisValue(view->getFixedSize()) == 0) {
                return std::max(minimum, availablePerpendicular);
            }
            return std::max(minimum, view->measure({
                .minHeight = mainSize,
                .maxHeight = mainSize,
                .maxWidth = availablePerpendicular,
            }).x);
        }
    }

    AOptional<int> findOverridedSize(const _<AView>& view) const {
        for (const auto& item : mSplitterHelper.items()) {
            if (item.view == view) {
                return item.overridedSize;
            }
        }
        return std::nullopt;
    }

    std::size_t countVisibleItems() const {
        return std::count_if(mViews.begin(), mViews.end(), [](const _<AView>& view) {
            return bool(view->getVisibility() & Visibility::FLAG_CONSUME_SPACE);
        });
    }

    std::vector<ResolvedItem> resolveMainAxisSizes(
        int availableMainSize,
        int perpendicularConstraint,
        bool bounded,
        bool expandingAsPreferred) const {
        std::vector<ResolvedItem> result;
        result.reserve(mViews.size());

        int occupiedMainSize = 0;
        int visibleCount = 0;
        for (const auto& view : mViews) {
            ResolvedItem item;
            if (!(view->getVisibility() & Visibility::FLAG_CONSUME_SPACE)) {
                result.push_back(item);
                continue;
            }

            item.visible = true;
            item.minMainSize = computeMinimumMainAxisSize(view, perpendicularConstraint);

            const int fixedSize = getAxisValue(view->getFixedSize());
            if (fixedSize > 0) {
                item.mainSize = fixedSize;
                item.maxMainSize = fixedSize;
            } else if (auto overridedSize = findOverridedSize(view)) {
                item.mainSize = std::max(item.minMainSize, *overridedSize);
                item.maxMainSize = item.mainSize;
            } else {
                item.weight = getAxisValue(view->getExpanding());
                item.mainSize = computePreferredMainAxisSize(view, perpendicularConstraint, expandingAsPreferred);
                const int maxSize = getAxisValue(view->getMaxSize());
                item.maxMainSize = maxSize > 0 ? std::max(item.mainSize, maxSize) : std::numeric_limits<int>::max();
                if (item.weight > 0 && !expandingAsPreferred) {
                    item.mainSize = item.minMainSize;
                }
            }

            occupiedMainSize += item.mainSize + getAxisValue(view->getMargin().occupiedSize());
            ++visibleCount;
            result.push_back(item);
        }
        occupiedMainSize += std::max(0, visibleCount - 1) * mSpacing;

        if (bounded && occupiedMainSize > availableMainSize) {
            int remainingToShrink = occupiedMainSize - availableMainSize;
            for (auto it = result.rbegin(); it != result.rend() && remainingToShrink > 0; ++it) {
                if (!it->visible) {
                    continue;
                }
                const int shrinkCapacity = it->mainSize - it->minMainSize;
                const int delta = std::min(shrinkCapacity, remainingToShrink);
                it->mainSize -= delta;
                remainingToShrink -= delta;
            }
        }

        if (bounded) {
            int remainingToExpand = availableMainSize;
            for (std::size_t i = 0; i < mViews.size(); ++i) {
                if (!result[i].visible) {
                    continue;
                }
                remainingToExpand -= result[i].mainSize + getAxisValue(mViews[i]->getMargin().occupiedSize());
            }
            remainingToExpand -= std::max(0, visibleCount - 1) * mSpacing;

            while (remainingToExpand > 0) {
                int totalWeight = 0;
                for (const auto& item : result) {
                    if (item.visible && item.weight > 0 && item.mainSize < item.maxMainSize) {
                        totalWeight += item.weight;
                    }
                }
                if (totalWeight == 0) {
                    break;
                }

                int distributed = 0;
                for (auto& item : result) {
                    if (!item.visible || item.weight == 0 || item.mainSize >= item.maxMainSize) {
                        continue;
                    }
                    const int share = std::max(1, remainingToExpand * item.weight / totalWeight);
                    const int delta = std::min(share, item.maxMainSize - item.mainSize);
                    item.mainSize += delta;
                    distributed += delta;
                }
                if (distributed == 0) {
                    break;
                }
                remainingToExpand -= distributed;
            }
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
