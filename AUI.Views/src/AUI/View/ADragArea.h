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

#pragma once


#include "AViewContainer.h"

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
    private:
        std::tuple<ADragArea*, AViewContainer*> getDragAreaAndDraggingView();
        bool mDragging = false;

    public:
        void onMousePressed(glm::ivec2 pos, AInput::Key button) override;
        void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;

    signals:
        emits<glm::ivec2> mouseMove;
    };

    static _<AView> makeDraggable(const _<AView>& view);
};


