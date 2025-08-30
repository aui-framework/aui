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
// Created by alex2 on 29.11.2020.
//

#pragma once


#include "AViewContainer.h"

/**
 * @brief Wrapping container to drag views inside of it.
 *
 * ![](imgs/views/ADragArea.png)
 *
 * @details Contained views must be either wrapped by convertToDraggable or have an ADragArea::ADraggableHandle inside.
 * @ingroup views_containment
 */
class API_AUI_VIEWS ADragArea: public AViewContainer {
private:
    _weak<AViewContainerBase> mDraggedContainer;
    glm::ivec2 mInitialMousePos;


    void setValidPositionFor(const _<AView>& targetView, const glm::ivec2& newPosition);
public:
    ADragArea();
    void startDragging(AViewContainerBase* container);
    void handleMouseMove();

    void endDragging();

    void applyGeometryToChildren() override;

    class API_AUI_VIEWS ADraggableHandle: public AViewContainerBase {
        friend class ADragArea;
    private:
        std::tuple<ADragArea*, AViewContainerBase*> getDragAreaAndDraggingView();
        bool mDragging = false;
        bool mCheckForClickConsumption = true;

    public:
        ADraggableHandle(bool checkForClickConsumption = false) : mCheckForClickConsumption(checkForClickConsumption) {}

        void onPointerPressed(const APointerPressedEvent& event) override;
        void onPointerReleased(const APointerReleasedEvent& event) override;

    signals:
        emits<glm::ivec2> mouseMove;
    };

    static _<AView>            convertToDraggable(const _<AView>& view, bool checkForClickConsumption = true);
    static _<ADraggableHandle> convertToDraggableContainer(const _<AViewContainer>& view, bool checkForClickConsumption = true);
};


