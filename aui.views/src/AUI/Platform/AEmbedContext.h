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
// Created by alex2 on 6/6/2021.
//


#pragma once


#include <AUI/View/AViewContainer.h>
#include <AUI/Platform/ASurface.h>

/**
 * @brief Helper for applications with custom window initialization.
 * @details
 * This class is abstract; use [AGLEmbedContext] or [ASoftwareEmbedAuiWrap] instead.
 */
class API_AUI_VIEWS AEmbedContext: public IEventLoop {
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
    explicit AEmbedContext();

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
     * @brief Handle mouse pressed in UI.
     * @param x x position
     * @param y y position
     * @param pointerIndex mouse button or finger index
     */
    void onPointerPressed(int x, int y, APointerIndex pointerIndex);

    /**
     * @brief Handle mouse released in UI.
     * @param x x position
     * @param y y position
     * @param pointerIndex mouse button or finger index
     */
    void onPointerReleased(int x, int y, APointerIndex pointerIndex);

    /**
     * @brief Handle mouse move in UI.
     * @param x x position
     * @param y y position
     */
    void onPointerMove(int x, int y);

    /**
     * @brief Handle char entered in UI.
     */
    void onCharEntered(AChar c);

    /**
     * @brief Handle onScroll in UI.
     */
    void onScroll(int mouseX, int mouseY, int scrollX, int scrollY);


    void onKeyPressed(AInput::Key key);
    void onKeyReleased(AInput::Key key);

    ASurface* getWindow();

    void loop() override;
};


