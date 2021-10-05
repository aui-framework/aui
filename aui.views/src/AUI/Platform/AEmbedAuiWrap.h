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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 6/6/2021.
//


#pragma once


#include <AUI/View/AViewContainer.h>
#include "ABaseWindow.h"

/**
 * Helper for applications which initialize OpenGL by themselves.
 */
class API_AUI_VIEWS AEmbedAuiWrap {
private:
    class FakeWindow;
    _<FakeWindow> mContainer;
    glm::ivec2 mSize;

public:
    explicit AEmbedAuiWrap();

    void clearFocus();

    void setContainer(const _<AViewContainer>& container);
    void setViewportSize(int width, int height);
    void render();

    void resetGLState();

    /**
     * @return true if UI is opaque for mouse at specified position
     */
    bool isUIConsumesMouseAt(int x, int y);

    /**
     * Handle mouse pressed in UI.
     * @param x x position
     * @param y y position
     * @param button mouse button
     */
    void onMousePressed(int x, int y, AInput::Key button);

    /**
     * Handle mouse released in UI.
     * @param x x position
     * @param y y position
     * @param button mouse button
     */
    void onMouseReleased(int x, int y, AInput::Key button);

    /**
     * Handle mouse move in UI.
     * @param x x position
     * @param y y position
     */
    void onMouseMove(int x, int y);

    /**
     * Handle mouse move in UI.
     * @param x x position
     * @param y y position
     */
    void onCharEntered(wchar_t c);


    void onKeyPressed(AInput::Key key);
    void onKeyReleased(AInput::Key key);

    ABaseWindow* getWindow();
};


