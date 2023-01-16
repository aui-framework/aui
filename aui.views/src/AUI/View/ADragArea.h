// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
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

#pragma once


#include "AViewContainer.h"

/**
 * @brief Wrapping container to drag views inside of it.
 * @details Contained views must be either wrapped by convertToDraggable or have an ADragArea::ADraggableHandle inside.
 * @ingroup useful_views
 */
class API_AUI_VIEWS ADragArea: public AViewContainer {
private:
    _weak<AViewContainer> mDraggedContainer;
    glm::ivec2 mInitialMousePos;


    void setValidPositionFor(const _<AView>& targetView, const glm::ivec2& newPosition);
public:
    ADragArea();
    void startDragging(AViewContainer* container);
    void handleMouseMove();

    void endDragging();

    void updateLayout() override;

    class API_AUI_VIEWS ADraggableHandle: public AViewContainer {
        friend class ADragArea;
    private:
        std::tuple<ADragArea*, AViewContainer*> getDragAreaAndDraggingView();
        bool mDragging = false;
        bool mCheckForClickConsumption = true;

    public:
        ADraggableHandle(bool checkForClickConsumption = false) : mCheckForClickConsumption(checkForClickConsumption) {}

        void onMousePressed(glm::ivec2 pos, AInput::Key button) override;
        void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;

    signals:
        emits<glm::ivec2> mouseMove;
    };

    static _<AView>          convertToDraggable(const _<AView>& view, bool checkForClickConsumption = true);
    static _<AViewContainer> convertToDraggableContainer(const _<AViewContainer>& view, bool checkForClickConsumption = true);
};


