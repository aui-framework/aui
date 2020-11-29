//
// Created by alex2 on 29.11.2020.
//

#pragma once


#include "AViewContainer.h"

class API_AUI_VIEWS ADragArea: public AViewContainer {
private:
    _weak<AViewContainer> mDraggedContainer;
    glm::ivec2 mInitialMousePos;

public:
    ADragArea();
    void startDragging(AViewContainer* container);
    void handleMouseMove();

    void endDragging();

    class API_AUI_VIEWS ADraggableHandle: public AView {
    private:
        std::tuple<ADragArea*, AViewContainer*> getDragAreaAndDraggingView() const;

    public:
        void onMousePressed(glm::ivec2 pos, AInput::Key button) override;
        void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;

        void onMouseMove(glm::ivec2 pos) override;

    signals:
        emits<glm::ivec2> mouseMove;
    };
};


