﻿/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <AUI/Platform/AWindow.h>

/**
 * @brief Represents a window without native caption but still draggable by top side.
 * @ingroup views
 * @details
 * @see ACustomCaptionWindow
 */
class API_AUI_VIEWS ACustomWindow: public AWindow
{
    friend class AWindowManager;
public:
    ACustomWindow(const AString& name, int width, int height);
    ACustomWindow() = default;
    ~ACustomWindow() override = default;

    void setTitleHeight(uint32_t height) {
        mTitleHeight = height;
    }

    void onPointerPressed(const APointerPressedEvent& event) override;
    void onPointerReleased(const APointerReleasedEvent& event) override;

    virtual bool isCaptionAt(const glm::ivec2& pos);

    void setSize(glm::ivec2 size) override;

signals:
    emits<glm::ivec2> dragBegin;
    emits<> dragEnd;

private:
    bool mDragging = false;
    glm::ivec2 mDragPos{};
    uint32_t mTitleHeight = 30;

    void handleXConfigureNotify();

#if AUI_PLATFORM_WIN
protected:
    LRESULT winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override;
    void doDrawWindow() override;

#endif
};