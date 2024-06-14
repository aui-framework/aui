/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <AUI/Platform/AWindow.h>

#if AUI_PLATFORM_WIN

class API_AUI_VIEWS ACustomWindow: public AWindow
{
private:
    bool mDragging = false;
    uint32_t mTitleHeight = 30;

protected:
	LRESULT winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override;
	void doDrawWindow() override;

public:
	ACustomWindow(const AString& name, int width, int height);
	ACustomWindow();
	~ACustomWindow() override;

    void setTitleHeight(uint32_t height) {
        mTitleHeight = height;
    }

    void setSize(glm::ivec2 size) override;

	virtual bool isCaptionAt(const glm::ivec2& pos);

signals:
    emits<> dragBegin;
    emits<> dragEnd;
};

#else
class API_AUI_VIEWS ACustomWindow: public AWindow
{
    friend class AWindowManager;
private:
    bool mDragging = false;
    glm::ivec2 mDragPos;
    uint32_t mTitleHeight = 30;

    void handleXConfigureNotify();

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

signals:
    emits<glm::ivec2> dragBegin;
    emits<> dragEnd;
};
#endif