// AUI Framework - Declarative UI toolkit for modern C++20
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
// Created by alex2 on 6/6/2021.
//


#pragma once


#include <AUI/View/AViewContainer.h>
#include "ABaseWindow.h"

/**
 * Helper for applications with custom window initialization.
 * @note This class is abstract; use <a href="AGLEmbedAuiWrap">AGLEmbedAuiWrap</a> or
 *       <a href="ASoftwareEmbedAuiWrap">ASoftwareEmbedAuiWrap</a> instead.
 */
class API_AUI_VIEWS AEmbedAuiWrap: public IEventLoop {
private:
    IEventLoop::Handle mEventLoopHandle;

protected:
    class EmbedWindow;
    _<EmbedWindow> mContainer;
    glm::ivec2 mSize;
    AOptional<float> mCustomDpiRatio;
    void windowInit(_unique<IRenderingContext> context);
    void windowMakeCurrent();
    void windowRender();

    virtual void onNotifyProcessMessages() = 0;
public:
    explicit AEmbedAuiWrap();

    void clearFocus();

    void setContainer(const _<AViewContainer>& container);
    virtual void setViewportSize(int width, int height);

    void setCustomDpiRatio(float r);

    bool requiresRedraw();

    void notifyProcessMessages() override;

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
    void onPointerPressed(int x, int y, AInput::Key button);

    /**
     * Handle mouse released in UI.
     * @param x x position
     * @param y y position
     * @param button mouse button
     */
    void onPointerReleased(int x, int y, AInput::Key button);

    /**
     * Handle mouse move in UI.
     * @param x x position
     * @param y y position
     */
    void onPointerMove(int x, int y);

    /**
     * Handle mouse move in UI.
     * @param x x position
     * @param y y position
     */
    void onCharEntered(wchar_t c);

    void onMouseScroll(int mouseX, int mouseY, int scrollX, int scrollY);


    void onKeyPressed(AInput::Key key);
    void onKeyReleased(AInput::Key key);

    ABaseWindow* getWindow();

    void loop() override;
};


