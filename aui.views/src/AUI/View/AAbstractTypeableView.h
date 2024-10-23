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
// Created by alex2 on 5/23/2021.
//


#pragma once

#include <AUI/Util/ACursorSelectable.h>
#include "AView.h"
#include "AUI/Common/ATimer.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Font/IFontView.h"


/**
 * @brief Basic implementation of type shortcuts and selection for editable text fields.
 * @details Used as base in ATextArea and ATextField, both of them using own way of text handling and rendering.
 */
class API_AUI_VIEWS AAbstractTypeableView: public AView, public ACursorSelectable, public IFontView {
private:
    static _<ATimer> blinkTimer();

    _<ATimer> mBlinkTimer = blinkTimer();

    unsigned mCursorBlinkCount = 0;
    bool mCursorBlinkVisible = true;
    bool mTextChangedFlag = false;
    bool mIsMultiline = false;
    bool mIsCopyable = true;

protected:
    int mHorizontalScroll = 0;
    size_t mMaxTextLength = 0x200;

    bool isCursorBlinkVisible() const {
        return mCursorBlinkVisible;
    }

protected:

    void updateCursorBlinking();
    void updateCursorPos();

    virtual void typeableErase(size_t begin, size_t end) = 0;

    /**
     * Inserts string.
     * @return true, if successfully inserted; false otherwise
     * @note when insert could not be completed (i.e. isValidText discarded the new string contents), the state is left
     *       unchanged as like if <code>typeableInsert</code> have not been called.
     */
    [[nodiscard]]
    virtual bool typeableInsert(size_t at, const AString& toInsert) = 0;

    /**
     * Inserts string.
     * @return true, if successfully inserted; false otherwise
     * @note when insert could not be completed (i.e. isValidText discarded the new string contents), the state is left
     *       unchanged as like if <code>typeableInsert</code> have not been called.
     */
    [[nodiscard]]
    virtual bool typeableInsert(size_t at, char16_t toInsert) = 0;
    virtual size_t typeableFind(char16_t c, size_t startPos = -1) = 0;
    virtual size_t typeableReverseFind(char16_t c, size_t startPos = -1) = 0;
    virtual size_t length() const = 0;
    virtual void updateSelectionOnTextSet(const AString& t);

    void onSelectionChanged() override;

    /**
     * Char enter implementation. Should be called in onCharEntered.
     * @param c
     */
    void enterChar(char16_t c);

    glm::ivec2 getMouseSelectionPadding() override;
    glm::ivec2 getMouseSelectionScroll() override;
    AFontStyle getMouseSelectionFont() override;
    AString getDisplayText() override;
    void doRedraw() override;

    bool isLButtonPressed() override;

    int getVerticalAlignmentOffset() noexcept {

        return (glm::max)(0, int(glm::ceil((getContentHeight() - getFontStyle().size) / 2.0)));
    }

    AMenuModel composeContextMenu() override;

    void invalidateStateStylesImpl(glm::ivec2 prevMinimumSizePlusField) override;

public:
    AAbstractTypeableView();
    virtual ~AAbstractTypeableView();

    int getContentMinimumHeight(ALayoutDirection layout) override;

    void onKeyDown(AInput::Key key) override;
    void onKeyRepeat(AInput::Key key) override;

    void onFocusLost() override;
    void onPointerPressed(const APointerPressedEvent& event) override;

    void onPointerDoubleClicked(const APointerPressedEvent& event) override;

    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;
    void onPointerReleased(const APointerReleasedEvent& event) override;

    void clear() {
        setText({});
    }
    void setMaxTextLength(size_t newTextLength) {
        mMaxTextLength = newTextLength;
    }

    void trimText() {
        setText(text().trim());
    }

    void setCopyable(bool isCopyable) {
        mIsCopyable = isCopyable;
    }

    virtual void setText(const AString& t);

    bool handlesNonMouseNavigation() override;
    void onFocusAcquired() override;


    void selectAll();
    void copyToClipboard() const;
    void cutToClipboard();
    void pasteFromClipboard();

    void invalidateAllStyles() override;

signals:
    /**
     * <dl>
     *   <dt><b>Emits</b></dt>
     *   <dd>When the user changed text and focused another view.</dd>
     *   <dd>When <code>AAbstractTextField::updateText</code> is called.</dd>
     * </dl>
     */
    emits<AString> textChanged;

    /**
     * <dl>
     *   <dt><b>Emits</b></dt>
     *   <dd>When the user added/removed one or more symbols.</dd>
     * </dl>
     */
    emits<AString> textChanging;

    emits<Selection> selectionChanged;
};


