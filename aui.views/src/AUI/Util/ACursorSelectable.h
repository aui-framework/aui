// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
// Created by alex2 on 30.11.2020.
//

#pragma once


#include <AUI/Common/AString.h>
#include <glm/glm.hpp>
#include <AUI/Font/AFontStyle.h>
#include <AUI/Render/ATextLayoutHelper.h>
#include <AUI/Platform/AInput.h>
#include <AUI/View/AView.h>

class API_AUI_VIEWS ACursorSelectable {
public:

    struct Selection
    {
        unsigned begin;
        unsigned end;

        bool operator==(const Selection& rhs) const noexcept {
            return std::tie(begin, end) == std::tie(rhs.begin, rhs.end);
        }

        bool operator!=(const Selection& rhs) const noexcept {
            return !(rhs == *this);
        }
    };

    /**
     * @return Text field text.
     * <dl>
     *   <dt><b>Performance note</b></dt>
     *   <dd>If text length is needed, use textLength() function instead. On some implementations it's faster that
     *   text().</dd>
     * </dl>
     */
    [[nodiscard]] virtual const AString& text() const = 0;

    /**
     * @return Text field text length.
     */
    [[nodiscard]] virtual size_t textLength() const = 0;
    [[nodiscard]] AString selectedText() const
    {
        if (!hasSelection())
            return {};
        auto t = text();
    	return {t.begin() + selection().begin, t.begin() + selection().end };
    }

    /**
     * @return Selection.
     */
    [[nodiscard]] Selection selection() const;

    /**
     * @return true if selection is present.
     */
    [[nodiscard]] bool hasSelection() const;

    /**
     * @return Character index by pixel position.
     */
    [[nodiscard]] unsigned cursorIndexByPos(glm::ivec2 pos);
    [[nodiscard]] int getPosByIndex(int end, int begin = 0);


    /**
     * @brief Select whole text in the text field.
     */
    void selectAll();

    /**
     * @brief Remove selection from the text field.
     */
    void clearSelection();

    void setSelection(int cursorIndex) {
        mCursorIndex = cursorIndex;
        mCursorSelection = -1;
        onSelectionChanged();
    }

    void setSelection(Selection selection) {
        mCursorIndex = selection.begin;
        mCursorSelection = selection.end;
        onSelectionChanged();
    }

protected:
    unsigned mCursorIndex = 0;
    unsigned mCursorSelection = -1;

    virtual glm::ivec2 getMouseSelectionPadding() = 0;
    virtual glm::ivec2 getMouseSelectionScroll() = 0;
    virtual AFontStyle getMouseSelectionFont() = 0;
    virtual bool isLButtonPressed() = 0;
    virtual AString getDisplayText() = 0;
    virtual void doRedraw() = 0;
    virtual void onSelectionChanged() = 0;


    void handleMouseDoubleClicked(const APointerPressedEvent& event);
    void handleMousePressed(const APointerPressedEvent& event);
    void handleMouseReleased(const APointerReleasedEvent& event);
    void handleMouseMove(const glm::ivec2& pos);

    /**
     * @return absoluteCursorPos
     */
    int drawSelectionPre();

    void drawSelectionPost();

    void drawSelectionRects();

    void setTextLayoutHelper(ATextLayoutHelper textLayoutHelper) {
        mTextLayoutHelper = std::move(textLayoutHelper);
    }

private:
    int mAbsoluteBegin, mAbsoluteEnd;
    bool mIgnoreSelection = false;
    ATextLayoutHelper mTextLayoutHelper;
};


