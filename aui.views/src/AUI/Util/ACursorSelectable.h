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

        [[nodiscard]]
        bool empty() const noexcept {
            return begin == end;
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
    [[nodiscard]] virtual size_t length() const = 0;
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
        mCursorSelection.reset();
        onSelectionChanged();
    }

    void setSelection(Selection selection) {
        mCursorIndex = selection.begin;
        mCursorSelection = selection.end;
        onSelectionChanged();
    }

protected:
    unsigned mCursorIndex = 0;
    AOptional<unsigned> mCursorSelection;

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
    int drawSelectionPre(IRenderer& render);

    void drawSelectionPost(IRenderer& render);

    void drawSelectionRects(IRenderer& render);

    void setTextLayoutHelper(ATextLayoutHelper textLayoutHelper) {
        mTextLayoutHelper = std::move(textLayoutHelper);
    }

private:
    int mAbsoluteBegin, mAbsoluteEnd;
    bool mIgnoreSelection = false;
    ATextLayoutHelper mTextLayoutHelper;
};

inline std::ostream& operator<<(std::ostream& o, const ACursorSelectable::Selection& e) noexcept{
    o << "Selection";
    if (e.empty()) {
        o << "{" << e.begin << "}";
    } else {
        o << "(" << e.begin << ";" << e.end << "]";
    }
    return o;
}
