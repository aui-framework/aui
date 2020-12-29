//
// Created by alex2 on 29.11.2020.
//

#include "ADragArea.h"
#include <AUI/Layout/AStackedLayout.h>
#include <AUI/Platform/ADesktop.h>

ADragArea::ADragArea() {
    setLayout(_new<AStackedLayout>());
}

std::tuple<ADragArea*, AViewContainer*> ADragArea::ADraggableHandle::getDragAreaAndDraggingView() const {
    AViewContainer* potentionalDragArea = mParent;
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
    AView::onMousePressed(pos, button);
    mDragging = true;

    auto [dragArea, container] = getDragAreaAndDraggingView();
    connect(mouseMove, dragArea, &ADragArea::handleMouseMove);
    dragArea->startDragging(container);
}

void ADragArea::ADraggableHandle::onMouseReleased(glm::ivec2 pos, AInput::Key button) {
    AView::onMouseReleased(pos, button);
    if (mDragging) {
        mDragging = false;
        auto[dragArea, _] = getDragAreaAndDraggingView();
        mouseMove.clearAllConnectionsWith(dragArea);
        dragArea->endDragging();
    }
}

void ADragArea::ADraggableHandle::onMouseMove(glm::ivec2 pos) {
    AView::onMouseMove(pos);
    if (mDragging) {
        emit mouseMove(pos);
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
