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

//
// Created by alex2 on 29.11.2020.
//

#include "ADragArea.h"
#include <AUI/Layout/AStackedLayout.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Platform/AWindow.h>

namespace {
    class DragAreaLayout: public ALinearLayout<> {
    public:
        void addView(const _<AView>& view, AOptional<size_t> index) override {
            ALinearLayout::addView(view, index);
            markViewToBeCentered(*view);
        }

        void removeView(aui::no_escape<AView> view, size_t index) override {
            LinearLayoutImpl::removeView(view, index);
        }

        void onResize(int x, int y, int width, int height) override {

        }

        int getMinimumWidth() override {
            return 0;
        }

        int getMinimumHeight() override {
            return 0;
        }

        static void markViewToBeCentered(AView& v) {
            v.setPosition({-1, -1});
        }

        static bool isViewMarkedToBeCentered(AView& v) {
            return v.getPosition() == glm::ivec2(-1);
        }
    };
}

ADragArea::ADragArea() {
    setLayout(std::make_unique<DragAreaLayout>());
    setExpanding({1, 1});
}

std::tuple<ADragArea*, AViewContainer*> ADragArea::ADraggableHandle::getDragAreaAndDraggingView() {
    AViewContainer* potentionalDragArea = this;
    AViewContainer* potentionalDraggingView = mParent;

    while (potentionalDragArea) {
        potentionalDraggingView = potentionalDragArea;
        potentionalDragArea = potentionalDragArea->getParent();
        if (auto dragArea = dynamic_cast<ADragArea*>(potentionalDragArea)) {
            return {dragArea, potentionalDraggingView};
        }
    }

    return {nullptr, potentionalDraggingView};
}

void ADragArea::ADraggableHandle::onPointerPressed(const APointerPressedEvent& event) {
    AViewContainer::onPointerPressed(event);

    if (mCheckForClickConsumption) {
        auto p = getViewAt(event.position);
        if (p) {
            if (p->consumesClick(event.position - glm::vec2(p->getPosition()))) {
                return;
            }
        }
    }
    mDragging = true;

    auto [dragArea, container] = getDragAreaAndDraggingView();

    if (!dragArea) return;
    connect(mouseMove, dragArea, &ADragArea::handleMouseMove);
    dragArea->startDragging(container);

    connect(AWindow::current()->mouseMove,
            this,
            [this](const glm::ivec2& windowPos) {
                if (mDragging) {
                    emit mouseMove(windowPos - getPositionInWindow());
                } else {
                    AObject::disconnect();
                }
            });
}

void ADragArea::ADraggableHandle::onPointerReleased(const APointerReleasedEvent& event) {
    AViewContainer::onPointerReleased(event);
    if (mDragging) {
        mDragging = false;
        auto[dragArea, _] = getDragAreaAndDraggingView();
        if (!dragArea) return;
        mouseMove.clearAllConnectionsWith(dragArea);
        dragArea->endDragging();
    }
}

void ADragArea::startDragging(AViewContainer* container) {
    for (auto& v : getViews()) {
        if (v.get() == container) {
            mDraggedContainer = _cast<AViewContainer>(v);
            mInitialMousePos = ADesktop::getMousePosition() - container->getPosition();
            break;
        }
    }
}

void ADragArea::handleMouseMove() {
    if (auto s = mDraggedContainer.lock()) {
        auto newPos = ADesktop::getMousePosition() - mInitialMousePos;
        setValidPositionFor(s, newPos);
        redraw();
    }
}

void ADragArea::updateLayout() {
    const auto x = getPadding().left;
    const auto y = getPadding().top;
    const auto width = getWidth() - mPadding.horizontal();
    const auto height = getHeight() - mPadding.vertical();

    for (const auto& v : getViews()) {
        v->ensureAssUpdated();
        auto margins = v->getMargin();
        auto finalWidth = v->getMinimumWidth() + margins.horizontal();
        auto finalX = (width - finalWidth) / 2;

        auto finalHeight = v->getMinimumHeight() + margins.vertical();
        auto finalY = (height - finalHeight) / 2;

        if (DragAreaLayout::isViewMarkedToBeCentered(*v)) {
            v->setGeometry(finalX + x + margins.left,
                           finalY + y + margins.top,
                           finalWidth - margins.horizontal(),
                           finalHeight - margins.vertical());
        } else {
            v->setSize({finalWidth - margins.horizontal(),
                        finalHeight - margins.vertical()});
            setValidPositionFor(v, v->getPosition());
        }
    }
    AViewContainer::updateLayout();
}

void ADragArea::endDragging() {
    mDraggedContainer.reset();
}

void ADragArea::setValidPositionFor(const _<AView>& targetView, const glm::ivec2& newPosition) {
    targetView->setPosition(
            glm::clamp(newPosition, glm::ivec2{targetView->getMargin().left, targetView->getMargin().top},
                       getSize() - targetView->getSize() - glm::ivec2{targetView->getMargin().right, targetView->getMargin().bottom}));
}

_<AView> ADragArea::convertToDraggable(const _<AView>& view, bool checkForClickConsumption) {
    auto v = _new<ADraggableHandle>(checkForClickConsumption);
    v->setLayout(std::make_unique<AStackedLayout>());
    v->setExpanding({view->getExpandingHorizontal(), view->getExpandingVertical()});
    v->addView(view);
    return v;
}

_<AViewContainer> ADragArea::convertToDraggableContainer(const _<AViewContainer>& view, bool checkForClickConsumption) {
    auto v = _new<ADraggableHandle>(checkForClickConsumption);
    v->setContents(view);
    v->setExpanding(view->getExpanding());
    v->updateLayout();
    return v;
}
