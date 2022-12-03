// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
// Created by alex2 on 29.11.2020.
//

#include "ADragArea.h"
#include <AUI/Layout/AStackedLayout.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Platform/AWindow.h>

ADragArea::ADragArea() {
    setLayout(_new<AStackedLayout>());
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

void ADragArea::ADraggableHandle::onMousePressed(glm::ivec2 pos, AInput::Key button) {
    AViewContainer::onMousePressed(pos, button);

    if (mCheckForClickConsumption) {
        auto p = getViewAt(pos);
        if (p) {
            if (p->consumesClick(pos - p->getPosition())) {
                return;
            }
        }
    }
    mDragging = true;

    auto [dragArea, container] = getDragAreaAndDraggingView();
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

void ADragArea::ADraggableHandle::onMouseReleased(glm::ivec2 pos, AInput::Key button) {
    AViewContainer::onMouseReleased(pos, button);
    if (mDragging) {
        mDragging = false;
        auto[dragArea, _] = getDragAreaAndDraggingView();
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
    for (auto& v : getViews()) {
        setValidPositionFor(v, v->getPosition());
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
    v->setLayout(_new<AStackedLayout>());
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
