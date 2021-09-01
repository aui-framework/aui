/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

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

_<AView> ADragArea::makeDraggable(const _<AView>& view) {
    auto v = _new<ADraggableHandle>();
    v->setLayout(_new<AStackedLayout>());
    v->setExpanding({view->getExpandingHorizontal(), view->getExpandingVertical()});
    v->addView(view);
    return v;
}
