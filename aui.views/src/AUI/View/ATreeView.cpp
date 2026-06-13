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
// Created by alex2 on 7/1/2021.
//

#include "ATreeView.h"

#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/Model/ATreeModelIndex.h>
#include <AUI/ASS/ASS.h>
#include <AUI/Util/UIBuildingHelpers.h>

class ATreeView::ItemView : public AViewContainerBase, public ass::ISelectable {
  public:
  ItemView(ATreeView* treeView, const _<AView>& display, bool hasChildren, const ATreeModelIndex& index)
    : mDisplay(display), mIndex(index), mTreeView(treeView) {
    addAssName(".list-item");
    setLayout(std::make_unique<AHorizontalLayout>());

    if (hasChildren) {
      addView(mCollapseDisplay = _new<ADrawableView>(IDrawable::fromUrl(":uni/svg/tree-collapsed.svg")) AUI_LET {
        it << ".list-item-icon";
        connect(it->clicked, me::toggleCollapse);
      });
    } else {
      addView(_new<AView>() << ".list-item-icon");
    }

    addView(mDisplay);
    if (hasChildren) {
      connect(mDisplay->doubleClicked, me::toggleCollapseRecursive);
    }

    connect(clicked, me::select);
  }

  void toggleCollapse() { setExpanded(!mExpanded); }
  void toggleCollapseRecursive() { expandOrCollapseRecursively(!mExpanded); }

  void expandOrCollapseRecursively(bool expand) {
    setExpanded(expand);
    auto& parentViews = getParent()->getViews();
    for (auto it = parentViews.begin(); it != parentViews.end(); ++it) {
      if (it->get() == this) {
        ++it;
        if (it != parentViews.end()) {
          if (auto container = _cast<AViewContainer>(*it)) {
            for (auto& v : container->getViews()) {
              if (auto treeItem = _cast<ItemView>(v)) {
                treeItem->expandOrCollapseRecursively(expand);
              }
            }
          }
        }
        break;
      }
    }
  }

  void setExpanded(bool expanded) {
    mExpanded = expanded;
    if (mExpanded) {
      AUI_NULLSAFE(mEnsureChildrenCreated)();
    }
    if (mCollapseDisplay) {
      mCollapseDisplay->setDrawable(
          IDrawable::fromUrl(mExpanded ? ":uni/svg/tree-expanded.svg" : ":uni/svg/tree-collapsed.svg"));
    }
    emit expandStateChanged(mExpanded);
  }

  void expand() { setExpanded(true); }

  void collapse() { setExpanded(false); }

  ~ItemView() override = default;

  const ATreeModelIndex& getIndex() const { return mIndex; }

  void setSelected(const bool selected) {
    if (selected) {
      AUI_NULLSAFE(mTreeView->mPrevSelection.lock())->setSelected(false);
      mTreeView->mPrevSelection = aui::ptr::shared_from_this(this);
      mTreeView->handleSelected(this);
    } else {
      mTreeView->mPrevSelection.reset();
    }
    mSelected = selected;
    emit customCssPropertyChanged;
  }

  void select() { setSelected(true); }

  void onPointerPressed(const APointerPressedEvent& event) override {
    AViewContainerBase::onPointerPressed(event);

    mTreeView->handleMousePressed(this);
  }

  void onPointerDoubleClicked(const APointerPressedEvent& event) override {
    AViewContainerBase::onPointerDoubleClicked(event);

    mTreeView->handleMouseDoubleClicked(this);
  }

  void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override {
    AViewContainerBase::onPointerMove(pos, event);
    mTreeView->handleMouseMove(this);
  }

  void setChildrenContainer(_<AViewContainer> childrenContainer) { mChildrenContainer = childrenContainer; }

  void setEnsureChildrenCreated(std::function<void()> ensureChildrenCreated) {
    mEnsureChildrenCreated = std::move(ensureChildrenCreated);
  }

  [[nodiscard]]
  const _<AViewContainer>& childrenContainer() const {
    return mChildrenContainer;
  }

  [[nodiscard]]
  bool isExpanded() const noexcept {
    return mExpanded;
  }

  signals:
  emits<bool> expandStateChanged;

  protected:
  bool selectableIsSelectedImpl() override { return mSelected; }

  private:
  _<AViewContainer> mChildrenContainer;
  std::function<void()> mEnsureChildrenCreated;

  bool mSelected = false;
  bool mExpanded = false;
  _<AView> mDisplay;
  _<ADrawableView> mCollapseDisplay;
  ATreeModelIndex mIndex;
  ATreeView* mTreeView;
};

ATreeView::ATreeView()
  : mViewFactory([](const _<ITreeModel<AString>>& model, const ATreeModelIndex& index) {
    return _new<ALabel>(model->itemAt(index));
  }) {}

ATreeView::ATreeView(const _<ITreeModel<AString>>& model) : ATreeView() { setModel(model); }

void ATreeView::setModel(const _<ITreeModel<AString>>& model) {
  if (mModel) {
    clearAllIngoingConnections();
  }
  mModel = model;
  setContents(mContent = _new<AViewContainer>());
  mContent->setLayout(std::make_unique<AVerticalLayout>());
  mContent->setExpanding();

  if (mModel) {
    rebuildContents();
    connect(mModel->dataInserted, [this](ATreeModelIndex) { rebuildContents(); });
    connect(mModel->dataRemoved, [this](ATreeModelIndex) { rebuildContents(); });
    connect(mModel->dataChanged, [this](ATreeModelIndex) { rebuildContents(); });
  }
  requestLayout();
  redraw();
}

void ATreeView::rebuildContents() {
  if (!mContent) {
    return;
  }

  AVector<ATreeModelIndex> expandedIndices;
  std::function<void(const _<AViewContainer>&)> collectExpandedIndices = [&](const _<AViewContainer>& container) {
    if (!container) {
      return;
    }
    for (const auto& view : container->getViews()) {
      if (auto item = _cast<ItemView>(view)) {
        if (item->isExpanded()) {
          expandedIndices << item->getIndex();
        }
        collectExpandedIndices(item->childrenContainer());
      } else if (auto nested = _cast<AViewContainer>(view)) {
        collectExpandedIndices(nested);
      }
    }
  };
  collectExpandedIndices(mContent);

  AOptional<ATreeModelIndex> selectedIndex;
  if (auto selected = mPrevSelection.lock()) {
    selectedIndex = selected->getIndex();
  }
  mPrevSelection.reset();

  mContent->removeAllViews();
  if (mModel) {
    fillViewsRecursively(mContent, ATreeModelIndex::ROOT);
  }

  for (const auto& index : expandedIndices) {
    try {
      if (auto itemView = indexToView(index)) {
        itemView->expand();
      }
    } catch (const AException&) {
    }
  }

  if (selectedIndex) {
    try {
      if (auto itemView = indexToView(*selectedIndex)) {
        itemView->setSelected(true);
      }
    } catch (const AException&) {
    }
  }

  requestLayout();
  redraw();
}

glm::ivec2 ATreeView::onIntrinsicMeasure(AConstraints constraints) {
  return AScrollArea::onIntrinsicMeasure(constraints);
}

AMinMaxAxis ATreeView::onComputeIntrinsicMinMaxAxis(int height) {
  return AScrollArea::onComputeIntrinsicMinMaxAxis(height);
}

void ATreeView::onLayout(int w, int h) {
  AScrollArea::onLayout(w, h);
}

void ATreeView::onScroll(const AScrollEvent& event) {
  AScrollArea::onScroll(event);
  onPointerMove(event.origin, { event.pointerIndex });   // update hover on scroll
}

void ATreeView::handleMousePressed(ATreeView::ItemView* v) { emit itemMouseClicked(v->getIndex()); }

void ATreeView::handleMouseDoubleClicked(ATreeView::ItemView* v) { emit itemMouseDoubleClicked(v->getIndex()); }

void ATreeView::handleSelected(ATreeView::ItemView* v) { emit itemSelected(v->getIndex()); }

void ATreeView::fillViewsRecursively(const _<AViewContainer>& content, const ATreeModelIndexOrRoot& index) {
  AVector<_<AView>> views;
  views.reserve(mModel->childrenCount(index) * 2);

  for (size_t i = 0; i < mModel->childrenCount(index); ++i) {
    auto childIndex = mModel->indexOfChild(i, 0, index);
    bool group = mModel->childrenCount(childIndex) != 0;
    auto item = _new<ItemView>(this, mViewFactory(mModel, childIndex), group, childIndex);

    // always add wrapper (even if isGroup = false) to simplify view walkthrough
    _<AViewContainer> wrapper = declarative::Vertical {};
    wrapper->setVisibility(Visibility::GONE);
    wrapper << ".list-item-group";

    if (group) {
      item->setChildrenContainer(wrapper);
      item->setEnsureChildrenCreated([this, wrapper, childIndex] {
        if (!wrapper->getViews().empty()) {
          return;
        }
        fillViewsRecursively(wrapper, childIndex);
      });

      connect(item->expandStateChanged, wrapper, [wrapper](bool expanded) {
        wrapper->setVisibility(expanded ? Visibility::VISIBLE : Visibility::GONE);
      });
    }

    views << item;
    views << wrapper;
  }

  content->setViews(std::move(views));
}

void ATreeView::handleMouseMove(ATreeView::ItemView* pView) { emit itemMouseHover(pView->getIndex()); }

/**
 * @brief Determines rows path from root node to the target node.
 * @details
 * Supplies the callable with row indices from root node directly to the specified target. The indices are passed to
 * callback function.
 */
template <aui::invocable<std::size_t /* row */> Callback>
static void traverseFromRootToTarget(
    const Callback& callback, const _<ITreeModel<AString>>& model, const ATreeModelIndexOrRoot& target) {
  if (target == ATreeModelIndex::ROOT) {
    return;
  }
  auto& asIndex = *target;
  traverseFromRootToTarget(callback, model, model->parent(asIndex));
  callback(asIndex.row());
}

void ATreeView::select(const ATreeModelIndex& indexToSelect) {
  try {
    auto itemView = indexToView(indexToSelect);
    if (itemView == nullptr) {
      return;
    }
    itemView->focus();
    itemView->setSelected(true);
    scrollTo(itemView);

  } catch (const AException& e) {
    ALogger::warn("ATreeView") << "Failed to select view by index (unsynced model?): " << e;
  }
}

_<ATreeView::ItemView> ATreeView::indexToView(const ATreeModelIndex& target) {
  auto currentTarget = _cast<AViewContainerBase>(mContent);
  _<ATreeView::ItemView> itemView;
  traverseFromRootToTarget(
      [&](std::size_t row) {
        if (!currentTarget) {
          return;
        }
        auto c = _cast<ATreeView::ItemView>(currentTarget->getViews().at(row * 2));
        if (!c) {
          throw AException("bad item view");
        }
        itemView = c;
        c->expand();
        currentTarget = c->childrenContainer();
      },
      mModel, target);
  return itemView;
}
