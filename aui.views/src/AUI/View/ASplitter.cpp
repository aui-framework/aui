/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2026 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ASplitter.h"

#include <cmath>
#include <limits>
#include <vector>
#include <AUI/Util/UIBuildingHelpers.h>

namespace {

template <ALayoutDirection Direction>
struct SplitterAxis {
  template <typename T>
  static T& main(glm::tvec2<T>& v) {
    return aui::layout_direction::getAxisValue(Direction, v);
  }

  template <typename T>
  static T main(const glm::tvec2<T>& v) {
    return aui::layout_direction::getAxisValue(Direction, v);
  }

  template <typename T>
  static T& cross(glm::tvec2<T>& v) {
    return aui::layout_direction::getPerpendicularAxisValue(Direction, v);
  }

  template <typename T>
  static T cross(const glm::tvec2<T>& v) {
    return aui::layout_direction::getPerpendicularAxisValue(Direction, v);
  }

  static glm::ivec2 pack(int mainAxis, int crossAxis) {
    if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
      return { mainAxis, crossAxis };
    } else {
      return { crossAxis, mainAxis };
    }
  }

  static AConstraints tightMain(int mainAxis, int crossLimit) {
    if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
      return {
        .minInline = mainAxis,
        .maxInline = mainAxis,
        .maxBlock = crossLimit,
      };
    } else {
      return {
        .maxInline = crossLimit,
        .minBlock = mainAxis,
        .maxBlock = mainAxis,
      };
    }
  }

  static AConstraints cappedCross(int crossLimit) {
    if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
      return { .maxBlock = crossLimit };
    } else {
      return { .maxInline = crossLimit };
    }
  }
};

inline bool consumesSpace(const _<AView>& view) {
  return static_cast<bool>(view->getVisibility() & Visibility::FLAG_CONSUME_SPACE);
}

template <ALayoutDirection Direction>
int childMinimumMainSize(const _<AView>& view, int crossConstraint) {
  if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
    return view->computeMinMaxAxis(crossConstraint).min;
  } else {
    const int fixed = SplitterAxis<Direction>::main(view->getFixedSize());
    if (fixed > 0) {
      return fixed;
    }
    return SplitterAxis<Direction>::main(view->getMinSize());
  }
}

template <ALayoutDirection Direction>
int childPreferredMainSize(const _<AView>& view, int crossConstraint) {
  if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
    return view->computeMinMaxAxis(crossConstraint).max;
  } else {
    return SplitterAxis<Direction>::main(view->measure(SplitterAxis<Direction>::cappedCross(crossConstraint)));
  }
}

template <ALayoutDirection Direction>
int childMaximumMainSize(const _<AView>& view) {
  const int fixed = SplitterAxis<Direction>::main(view->getFixedSize());
  if (fixed > 0) {
    return fixed;
  }
  const int maxSize = SplitterAxis<Direction>::main(view->getMaxSize());
  return maxSize == -1 ? std::numeric_limits<int>::max() : maxSize;
}

template <ALayoutDirection Direction>
int childMeasuredCrossSize(const _<AView>& view, int mainAxis, int crossLimit) {
  const int fixed = SplitterAxis<Direction>::cross(view->getFixedSize());
  if (fixed > 0) {
    return fixed;
  }
  return SplitterAxis<Direction>::cross(view->measure(SplitterAxis<Direction>::tightMain(mainAxis, crossLimit)));
}

}   // namespace

template <ALayoutDirection Direction>
class ASplitterLayout final : public ALayout {
public:
  explicit ASplitterLayout(ASplitter& splitter) : mSplitter(splitter) {}

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
    mSplitter.mHelper.setItems(mViews);
    mSplitter.resetRelativeSizes();
    ALayout::requestLayout();
  }

  void removeView(aui::no_escape<AView> view, size_t index) override {
    AUI_ASSERT(mViews[index].get() == view.ptr());
    mViews.removeAt(index);
    mSplitter.mHelper.setItems(mViews);
    mSplitter.resetRelativeSizes();
    ALayout::requestLayout();
  }

  AVector<_<AView>> getAllViews() override { return mViews; }

  ALayoutDirection getLayoutDirection() override { return Direction; }

  void layout(int x, int y, int width, int height) override {
    auto info = collectInfo(crossSize({ width, height }));
    const int mainAxis = mainSize({ width, height });
    const int contentMain = computeContentMain(info, mainAxis);
    const auto sizes = resolveMainSizes(info, contentMain);

    int cursor = mainPos({ x, y });
    for (size_t i = 0; i < mViews.size(); ++i) {
      if (!info[i].visible) {
        continue;
      }

      const auto margins = mViews[i]->getMargin();
      const int childCross = glm::max(0, crossSize({ width, height }) - info[i].marginCross);
      const auto childPos = SplitterAxis<Direction>::pack(
          cursor + mainPos(margins.leftTop()), crossPos({ x, y }) + crossPos(margins.leftTop()));
      const auto childSize = SplitterAxis<Direction>::pack(sizes[i], childCross);

      mViews[i]->layout(childPos.x, childPos.y, childSize.x, childSize.y);
      cursor += sizes[i] + info[i].marginMain + mSpacing;
    }
  }

  glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
    const int crossConstraint = crossConstraintOf(constraints);
    auto info = collectInfo(crossConstraint);

    AVector<int> sizes;
    const int boundedMain = boundedMainConstraintOf(constraints);
    if (boundedMain != -1) {
      sizes = resolveMainSizes(info, computeContentMain(info, boundedMain));
    } else {
      sizes = naturalMainSizes(info);
    }

    const int measuredMain = totalMainSize(info, sizes);
    const int measuredCross = measuredCrossSize(info, sizes, crossConstraint);
    return SplitterAxis<Direction>::pack(measuredMain, measuredCross);
  }

  AMinMaxAxis onComputeIntrinsicMinMaxAxis(int height) override {
    AMinMaxAxis result;
    int visibleCount = 0;

    if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
      for (const auto& view : mViews) {
        if (!consumesSpace(view)) {
          continue;
        }
        const auto minMax = view->computeMinMaxAxis(height);
        result.min += minMax.min + view->getMargin().horizontal();
        result.max += minMax.max + view->getMargin().horizontal();
        ++visibleCount;
      }
      const int totalSpacing = glm::max(0, visibleCount - 1) * mSpacing;
      result.min += totalSpacing;
      result.max += totalSpacing;
    } else {
      for (const auto& view : mViews) {
        if (!consumesSpace(view)) {
          continue;
        }
        const auto minMax = view->computeMinMaxAxis(height);
        result.min = glm::max(result.min, minMax.min + view->getMargin().horizontal());
        result.max = glm::max(result.max, minMax.max + view->getMargin().horizontal());
      }
    }

    result.max = glm::max(result.max, result.min);
    return result;
  }

private:
  struct ChildInfo {
    bool visible = false;
    int marginMain = 0;
    int marginCross = 0;
    int minMain = 0;
    int maxMain = std::numeric_limits<int>::max();
    int preferredMain = 0;
    int expandingWeight = 0;
  };

  ASplitter& mSplitter;
  AVector<_<AView>> mViews;
  int mSpacing = 0;

  static int mainSize(glm::ivec2 size) { return SplitterAxis<Direction>::main(size); }

  static int crossSize(glm::ivec2 size) { return SplitterAxis<Direction>::cross(size); }

  static int mainPos(glm::ivec2 position) { return SplitterAxis<Direction>::main(position); }

  static int crossPos(glm::ivec2 position) { return SplitterAxis<Direction>::cross(position); }

  static int crossConstraintOf(const AConstraints& constraints) {
    if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
      return constraints.maxBlock;
    } else {
      return constraints.maxInline;
    }
  }

  static int boundedMainConstraintOf(const AConstraints& constraints) {
    if constexpr (Direction == ALayoutDirection::HORIZONTAL) {
      return constraints.isUnlimitedInline() ? -1 : constraints.maxInline;
    } else {
      return constraints.isUnlimitedBlock() ? -1 : constraints.maxBlock;
    }
  }

  AVector<ChildInfo> collectInfo(int crossConstraint) const {
    AVector<ChildInfo> result(mViews.size());
    for (size_t i = 0; i < mViews.size(); ++i) {
      const auto& view = mViews[i];
      auto& info = result[i];
      info.visible = consumesSpace(view);
      if (!info.visible) {
        continue;
      }

      const auto margin = view->getMargin().occupiedSize();
      info.marginMain = SplitterAxis<Direction>::main(margin);
      info.marginCross = SplitterAxis<Direction>::cross(margin);
      info.minMain = childMinimumMainSize<Direction>(view, crossConstraint);
      info.maxMain = childMaximumMainSize<Direction>(view);
      info.preferredMain =
          glm::clamp(childPreferredMainSize<Direction>(view, crossConstraint), info.minMain, info.maxMain);
      info.expandingWeight = glm::max(0, SplitterAxis<Direction>::main(view->getExpanding()));
    }
    return result;
  }

  int computeContentMain(const AVector<ChildInfo>& info, int availableMain) const {
    int visibleCount = 0;
    int result = availableMain;
    for (const auto& item : info) {
      if (!item.visible) {
        continue;
      }
      result -= item.marginMain;
      ++visibleCount;
    }
    result -= glm::max(0, visibleCount - 1) * mSpacing;
    return result;
  }

  AVector<int> naturalMainSizes(const AVector<ChildInfo>& info) const {
    AVector<int> result(mViews.size(), 0);
    for (size_t i = 0; i < mViews.size(); ++i) {
      if (!info[i].visible) {
        continue;
      }
      result[i] = info[i].preferredMain;
    }
    return result;
  }

  AVector<int>
  resolveWeightedSizes(const AVector<ChildInfo>& info, int availableMain, const AVector<float>& weights) const {
    struct ActiveChild {
      size_t index;
      float weight;
    };

    AVector<int> result(mViews.size(), 0);
    std::vector<ActiveChild> active;
    active.reserve(mViews.size());

    int remainingMain = availableMain;
    float totalWeight = 0.f;

    for (size_t i = 0; i < mViews.size(); ++i) {
      if (!info[i].visible) {
        continue;
      }
      const float weight = glm::max(0.f, weights[i]);
      if (weight > 0.f) {
        active.push_back({ .index = i, .weight = weight });
        totalWeight += weight;
      }
    }

    if (active.empty()) {
      for (size_t i = 0; i < mViews.size(); ++i) {
        if (!info[i].visible) {
          continue;
        }
        active.push_back({ .index = i, .weight = 1.f });
        totalWeight += 1.f;
      }
    }

    while (true) {
      bool changed = false;

      for (auto it = active.begin(); it != active.end();) {
        const auto& childInfo = info[it->index];
        const float targetSize = totalWeight > 0.f ? float(remainingMain) * it->weight / totalWeight : 0.f;

        if (targetSize < float(childInfo.minMain)) {
          result[it->index] = childInfo.minMain;
          remainingMain -= childInfo.minMain;
          totalWeight -= it->weight;
          it = active.erase(it);
          changed = true;
          continue;
        }

        if (targetSize > float(childInfo.maxMain)) {
          result[it->index] = childInfo.maxMain;
          remainingMain -= childInfo.maxMain;
          totalWeight -= it->weight;
          it = active.erase(it);
          changed = true;
          continue;
        }

        ++it;
      }

      if (!changed) {
        break;
      }

      if (active.empty() || totalWeight <= 0.f) {
        return result;
      }
    }

    int assigned = 0;
    float cumulativeWeight = 0.f;
    for (size_t i = 0; i < active.size(); ++i) {
      const auto& child = active[i];
      int resolvedSize = 0;
      if (i + 1 == active.size()) {
        resolvedSize = remainingMain - assigned;
      } else {
        cumulativeWeight += child.weight;
        int targetAssigned = int(std::lround(float(remainingMain) * cumulativeWeight / totalWeight));
        resolvedSize = targetAssigned - assigned;
        assigned += resolvedSize;
      }
      result[child.index] = resolvedSize;
    }

    return result;
  }

  AVector<int> resolveInitialSizes(const AVector<ChildInfo>& info, int availableMain) const {
    AVector<int> result(mViews.size(), 0);
    AVector<float> expandingWeights(mViews.size(), 0.f);

    int remainingMain = availableMain;
    for (size_t i = 0; i < mViews.size(); ++i) {
      if (!info[i].visible) {
        continue;
      }
      if (info[i].expandingWeight <= 0) {
        result[i] = info[i].preferredMain;
        remainingMain -= result[i];
      } else {
        expandingWeights[i] = float(info[i].expandingWeight);
      }
    }

    const auto expandedSizes = resolveWeightedSizes(info, remainingMain, expandingWeights);
    for (size_t i = 0; i < mViews.size(); ++i) {
      if (info[i].visible && info[i].expandingWeight > 0) {
        result[i] = expandedSizes[i];
      }
    }
    return result;
  }

  bool hasStoredRelativeSizes() const {
    if (mSplitter.mRelativeSizes.size() != mViews.size()) {
      return false;
    }

    float total = 0.f;
    for (size_t i = 0; i < mViews.size(); ++i) {
      if (!consumesSpace(mViews[i])) {
        continue;
      }
      total += glm::max(0.f, mSplitter.mRelativeSizes[i]);
    }
    return total > 0.f;
  }

  AVector<int> resolveMainSizes(const AVector<ChildInfo>& info, int availableMain) const {
    if (hasStoredRelativeSizes()) {
      return resolveWeightedSizes(info, availableMain, mSplitter.mRelativeSizes);
    }
    return resolveInitialSizes(info, availableMain);
  }

  int totalMainSize(const AVector<ChildInfo>& info, const AVector<int>& sizes) const {
    int result = 0;
    int visibleCount = 0;
    for (size_t i = 0; i < mViews.size(); ++i) {
      if (!info[i].visible) {
        continue;
      }
      result += sizes[i] + info[i].marginMain;
      ++visibleCount;
    }
    result += glm::max(0, visibleCount - 1) * mSpacing;
    return result;
  }

  int measuredCrossSize(const AVector<ChildInfo>& info, const AVector<int>& sizes, int crossConstraint) const {
    int result = 0;
    for (size_t i = 0; i < mViews.size(); ++i) {
      if (!info[i].visible) {
        continue;
      }
      result = glm::max(
          result, childMeasuredCrossSize<Direction>(mViews[i], sizes[i], crossConstraint) + info[i].marginCross);
    }
    return result;
  }
};

ASplitter::ASplitter() = default;

void ASplitter::resetRelativeSizes() {
  mRelativeSizes.clear();
  mDragContentSize = 0;
  endDragDivider();
}

void ASplitter::normalizeRelativeSizes() {
  if (mRelativeSizes.size() != mHelper.items().size()) {
    return;
  }

  float total = 0.f;
  int visibleCount = 0;
  for (size_t i = 0; i < mRelativeSizes.size(); ++i) {
    if (!consumesSpace(mHelper.items()[i].view)) {
      mRelativeSizes[i] = 0.f;
      continue;
    }
    mRelativeSizes[i] = glm::max(0.f, mRelativeSizes[i]);
    total += mRelativeSizes[i];
    ++visibleCount;
  }

  if (visibleCount == 0) {
    mRelativeSizes.clear();
    return;
  }

  if (total <= 0.f) {
    const float equalShare = 1.f / float(visibleCount);
    for (size_t i = 0; i < mRelativeSizes.size(); ++i) {
      mRelativeSizes[i] = consumesSpace(mHelper.items()[i].view) ? equalShare : 0.f;
    }
    return;
  }

  for (size_t i = 0; i < mRelativeSizes.size(); ++i) {
    if (!consumesSpace(mHelper.items()[i].view)) {
      continue;
    }
    mRelativeSizes[i] /= total;
  }
}

void ASplitter::syncRelativeSizesFromCurrentLayout() {
  mRelativeSizes.resize(mHelper.items().size(), 0.f);

  int total = 0;
  for (const auto& item : mHelper.items()) {
    if (!consumesSpace(item.view)) {
      continue;
    }
    total += mHelper.getAxisValue(item.view->getSize());
  }

  if (total <= 0) {
    resetRelativeSizes();
    return;
  }

  for (size_t i = 0; i < mHelper.items().size(); ++i) {
    mRelativeSizes[i] =
        consumesSpace(mHelper.items()[i].view)
            ? float(mHelper.getAxisValue(mHelper.items()[i].view->getSize())) / float(total)
            : 0.f;
  }
  normalizeRelativeSizes();
}

bool ASplitter::dragDivider(glm::ivec2 mousePos) {
  if (!isDraggingDivider() || mDraggingDividerIndex + 1 >= mRelativeSizes.size() || mDragContentSize <= 0) {
    return false;
  }

  const int newDragOffset = mHelper.getAxisValue(mousePos);
  const int delta = newDragOffset - mDragOffset;
  if (delta == 0) {
    return false;
  }

  const float pairRatio = mRelativeSizes[mDraggingDividerIndex] + mRelativeSizes[mDraggingDividerIndex + 1];
  int leftSize = int(std::lround(mRelativeSizes[mDraggingDividerIndex] * float(mDragContentSize)));
  int pairSize = int(std::lround(pairRatio * float(mDragContentSize)));
  int rightSize = pairSize - leftSize;

  const int crossConstraint = glm::max(
      0, aui::layout_direction::getPerpendicularAxisValue(mHelper.mDirection, getSize() - getPadding().occupiedSize()));

  auto minMainSize = [&](const _<AView>& view) {
    if (mHelper.mDirection == ALayoutDirection::HORIZONTAL) {
      return childMinimumMainSize<ALayoutDirection::HORIZONTAL>(view, crossConstraint);
    }
    return childMinimumMainSize<ALayoutDirection::VERTICAL>(view, crossConstraint);
  };
  auto maxMainSize = [&](const _<AView>& view) {
    if (mHelper.mDirection == ALayoutDirection::HORIZONTAL) {
      return childMaximumMainSize<ALayoutDirection::HORIZONTAL>(view);
    }
    return childMaximumMainSize<ALayoutDirection::VERTICAL>(view);
  };

  const auto& leftView = mHelper.items()[mDraggingDividerIndex].view;
  const auto& rightView = mHelper.items()[mDraggingDividerIndex + 1].view;

  const int leftMin = minMainSize(leftView);
  const int rightMin = minMainSize(rightView);
  const int leftMax = maxMainSize(leftView);
  const int rightMax = maxMainSize(rightView);

  const int leftShrink = glm::max(0, leftSize - leftMin);
  const int rightShrink = glm::max(0, rightSize - rightMin);
  const int leftGrow =
      leftMax == std::numeric_limits<int>::max() ? std::numeric_limits<int>::max() : glm::max(0, leftMax - leftSize);
  const int rightGrow =
      rightMax == std::numeric_limits<int>::max() ? std::numeric_limits<int>::max() : glm::max(0, rightMax - rightSize);

  const int actualDelta = glm::clamp(delta, -glm::min(leftShrink, rightGrow), glm::min(leftGrow, rightShrink));
  if (actualDelta == 0) {
    return false;
  }

  mRelativeSizes[mDraggingDividerIndex] = float(leftSize + actualDelta) / float(mDragContentSize);
  mRelativeSizes[mDraggingDividerIndex + 1] = pairRatio - mRelativeSizes[mDraggingDividerIndex];
  mDragOffset += actualDelta;
  return true;
}

void ASplitter::onPointerPressed(const APointerPressedEvent& event) {
  if (auto dividerIndex = mHelper.dividerIndexAt(event.position)) {
    AView::onPointerPressed(event);   // NOLINT(*-parent-virtual-call)
    if (mRelativeSizes.empty()) {
      syncRelativeSizesFromCurrentLayout();
    }
    mDraggingDividerIndex = *dividerIndex;
    mDragOffset = mHelper.getAxisValue(event.position);
    mDragContentSize = 0;
    for (const auto& item : mHelper.items()) {
      if (!consumesSpace(item.view)) {
        continue;
      }
      mDragContentSize += mHelper.getAxisValue(item.view->getSize());
    }
    return;
  }
  AViewContainerBase::onPointerPressed(event);
}

void ASplitter::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
  if (isDraggingDivider() || (mHelper.isDraggingArea(pos) && !isPressed())) {
    setCursor(mHelper.mDirection == ALayoutDirection::HORIZONTAL ? ACursor::EW_RESIZE : ACursor::NS_RESIZE);
    AView::onPointerMove(pos, event);   // NOLINT(*-parent-virtual-call)
    if (dragDivider(pos)) {
      requestLayout();
      redraw();
    }
    return;
  }
  setCursor({});
  AViewContainerBase::onPointerMove(pos, event);
}

void ASplitter::onPointerReleased(const APointerReleasedEvent& event) {
  if (isDraggingDivider()) {
    AView::onPointerReleased(event);   // NOLINT(*-parent-virtual-call)
    endDragDivider();
    return;
  }
  AViewContainerBase::onPointerReleased(event);
}

void ASplitter::onClickPrevented() {
  AViewContainerBase::onClickPrevented();
  endDragDivider();
}

template <typename Layout>
_<AView> ASplitter::Builder<Layout>::build() {
  auto splitter = aui::ptr::manage_shared(new ASplitter);
  splitter->mHelper.setDirection(Layout::DIRECTION);
  splitter->setLayout(std::make_unique<ASplitterLayout<Layout::DIRECTION>>(*splitter));

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

  splitter->setExpanding(mExpanding.valueOr([&] {
    glm::ivec2 expanding {};
    for (const auto& item : mItems) {
      expanding = glm::max(expanding, item->getExpanding());
    }
    return expanding;
  }));

  splitter->mHelper.setItems(std::move(mItems));

  return splitter;
}

template _<AView> ASplitter::Builder<AHorizontalLayout>::build();
template _<AView> ASplitter::Builder<AVerticalLayout>::build();
