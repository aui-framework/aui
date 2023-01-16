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

#pragma once
#include "AWindow.h"

#if AUI_PLATFORM_WIN

class API_AUI_VIEWS ACustomWindow: public AWindow
{
private:
    bool mDragging = false;

protected:
	LRESULT winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override;
	void doDrawWindow() override;

public:
	ACustomWindow(const AString& name, int width, int height);
	ACustomWindow();
	~ACustomWindow() override;
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

    void handleXConfigureNotify();

public:
    ACustomWindow(const AString& name, int width, int height);
    ACustomWindow() = default;
    ~ACustomWindow() override = default;

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;
    void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;

    virtual bool isCaptionAt(const glm::ivec2& pos);

signals:
    emits<glm::ivec2> dragBegin;
    emits<> dragEnd;
};
#endif