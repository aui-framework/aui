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
// Created by alex2 on 30.11.2020.
//

#pragma once


#include <AUI/Common/AString.h>
#include <glm/glm.hpp>
#include <AUI/Font/AFontStyle.h>
#include <AUI/Render/ATextLayoutHelper.h>
#include <AUI/Render/RenderHints.h>
#include <AUI/Platform/AInput.h>
#include <AUI/View/AView.h>

class API_AUI_VIEWS ACursorSelectable {
public:
    virtual ~ACursorSelectable();

    struct Selection
    {
        unsigned begin;
        unsigned end;

        [[nodiscard]]
        bool operator==(const Selection& rhs) const noexcept = default;

        [[nodiscard]]
        bool operator!=(const Selection& rhs) const noexcept = default;

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
    [[nodiscard]] virtual AString getText() const = 0;

    /**
     * @return Text field text length.
     */
    [[nodiscard]] virtual size_t length() const = 0;
    [[nodiscard]] AString selectedText() const
    {
        if (!hasSelection())
            return {};
        auto t = getText();
    	return {t.bytes().begin() + selection().begin, t.bytes().begin() + selection().end };
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
    [[nodiscard]] virtual unsigned cursorIndexByPos(glm::ivec2 pos) = 0;

    [[nodiscard]] virtual glm::ivec2 getPosByIndex(size_t index) = 0;

    /**
     * @return Cursor position relative to this view.
     * @details
     * Returns position relative to top left corner of the view. That is, if implementation supports scrolling (i.e.,
     * ATextField) the returned position does not include overflowed contents.
     */
    [[nodiscard]] virtual glm::ivec2 getCursorPosition() = 0;


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

    virtual bool isLButtonPressed() = 0;
    virtual AString getDisplayText() = 0;
    virtual void cursorSelectableRedraw() = 0;
    virtual void onSelectionChanged() = 0;


    void handleMouseDoubleClicked(const APointerPressedEvent& event);
    void handleMousePressed(const APointerPressedEvent& event);
    void handleMouseReleased(const APointerReleasedEvent& event);
    void handleMouseMove(const glm::ivec2& pos);

    template<aui::invocable Callback>
    void drawSelectionBeforeAndAfter(IRenderer& render, std::span<ARect<int>> rects, Callback&& drawText) {
        if (rects.empty()) {
            drawText();
            return;
        }

        auto drawRects = [&] {
            for (auto r : rects) {
                render.rectangle(ASolidBrush{}, r.p1, r.size());
            }
        };
        {
            RenderHints::PushColor c(render);
            render.setColor(AColor(1.f) - AColor(0x0078d700u));
            drawRects();
        }

        drawText();

        render.setBlending(Blending::INVERSE_DST);
        AUI_DEFER { render.setBlending(Blending::NORMAL); };
        drawRects();
    }

private:
    bool mIgnoreSelection = false;

};

inline std::ostream& operator<<(std::ostream& o, const ACursorSelectable::Selection& e) noexcept{
    o << "Selection";
    if (e.empty()) {
        o << "{" << e.begin << "}";
    } else {
        o << "[" << e.begin << ";" << e.end << ")";
    }
    return o;
}
