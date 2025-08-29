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

#pragma once


#include <AUI/Util/ACursorSelectable.h>
#include <AUI/Enum/ATextInputActionIcon.h>
#include <AUI/Enum/ATextInputType.h>
#include "AUI/Common/ATimer.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Font/IFontView.h"

/**
 * @brief Base class for AAbstractTypeableView which is template class.
 */
class API_AUI_VIEWS AAbstractTypeable: public ACursorSelectable {
public:
    AAbstractTypeable();
    ~AAbstractTypeable() override;

    void clear() {
        setText({});
    }
    void setMaxTextLength(size_t newTextLength) {
        mMaxTextLength = newTextLength;
    }

    void trimText() {
        setText(getText().trim());
    }

    void setCopyable(bool isCopyable) {
        mIsCopyable = isCopyable;
    }

    virtual void setText(const AString& t);

    /**
     * @return true if this typeable should be treated as password.
     */
    [[nodiscard]]
    virtual bool isPasswordField() const noexcept = 0;

    /**
     * @return text input type of this typeable.
     * @see ATextInputType
     */
    [[nodiscard]]
    virtual ATextInputType textInputType() const noexcept = 0;

    /**
     * @see ATextInputActionIcon
     */
    void setTextInputActionIcon(ATextInputActionIcon textInputActionIcon) noexcept {
        mTextInputActionIcon = textInputActionIcon;
    }

    /**
     * @return text input type of this typeable.
     * @see ATextInputActionIcon
     */
    [[nodiscard]]
    ATextInputActionIcon textInputActionIcon() const noexcept {
        return mTextInputActionIcon;
    }

    /**
     * @brief Performs copy operation (CTRL+C) to system clipboard.
     */
    void copyToClipboard() const;

    /**
     * @brief Performs cut operation (CTRL+X) to system clipboard.
     */
    void cutToClipboard();

    /**
     * @brief Performs paste operation (CTRL+V) from system clipboard.
     * @details
     * Effectively:
     * ```cpp
     * paste(AClipboard::pasteFromClipboard());
     * ```
     */
    void pasteFromClipboard();

    /**
     * @brief Performs paste operation (CTRL+V).
     * @param content string to insert
     * @details
     * Performs paste operation at cursor's position. If the view has selection, the selected contents are removed
     * before insertion.
     *
     * After operation, selection is reset and cursor is set to the end of inserted contents.
     */
    void paste(AString content);

    /**
     * @brief Performs move left operation (like AInput::LEFT)
     */
    void moveCursorLeft();

    /**
     * @brief Performs move right operation (like AInput::RIGHT)
     */
    void moveCursorRight();

    virtual void emitTextChanged(const AString& text) = 0;
    virtual void emitTextChanging(const AString& text) = 0;
    virtual void emitActionButtonPressed() = 0;

signals:
    /**
     * @brief The user changed text and focused another view or `AAbstractTextField::updateText` is called.
     */
    emits<AString> textChanged;

    /**
     * @brief When the user changed one or more symbols.
     */
    emits<AString> textChanging;

    /**
     * @brief Selection is changed due to user's action or code operation.
     */
    emits<Selection> selectionChanged;

    /**
     * @brief When action button of touchscreen keyboard or AInput::RETURN is pressed.
     */
    emits<> actionButtonPressed;

protected:
    size_t mMaxTextLength = 0x200;
    bool mIsMultiline = false;

protected:
    bool isCursorBlinkVisible() const {
        return mCursorBlinkVisible;
    }

    void updateCursorBlinking();

    virtual void typeableErase(size_t begin, size_t end) = 0;

    /**
     * Inserts string.
     * @return true, if successfully inserted; false otherwise
     * @details
     * When insert could not be completed (i.e. isValidText discarded the new string contents), the state is left
     * unchanged as like if <code>typeableInsert</code> have not been called.
     */
    [[nodiscard]]
    virtual bool typeableInsert(size_t at, const AString& toInsert) = 0;

    /**
     * Inserts string.
     * @return true, if successfully inserted; false otherwise
     * When insert could not be completed (i.e. isValidText discarded the new string contents), the state is left
     * unchanged as like if <code>typeableInsert</code> have not been called.
     */
    [[nodiscard]]
    virtual bool typeableInsert(size_t at, AChar toInsert) = 0;
    virtual size_t typeableFind(char16_t c, size_t startPos = -1) = 0;
    virtual size_t typeableReverseFind(char16_t c, size_t startPos = -1) = 0;
    virtual void updateSelectionOnTextSet(const AString& t);

    virtual void onCursorIndexChanged() = 0;


    /**
     * Char enter implementation. Should be called in onCharEntered.
     * @param c
     */
    void enterChar(AChar c);

    AString getDisplayText() override;
    AMenuModel composeContextMenuImpl();
    void handleKey(AInput::Key key);

private:
    _<ATimer> mBlinkTimer = blinkTimer();

    /**
     * @see ATextInputActionIcon
     */
    ATextInputActionIcon mTextInputActionIcon = ATextInputActionIcon::DEFAULT;
    unsigned mCursorBlinkCount = 0;
    bool mCursorBlinkVisible = true;
    bool mTextChangedFlag = false;
    bool mIsCopyable = true;

private:
    static _<ATimer> blinkTimer();

    template<aui::derived_from<AView> Super>
    friend class AAbstractTypeableView;

    virtual void typeableInvalidateFont() = 0;
    void drawCursorImpl(IRenderer& renderer, glm::ivec2 position, unsigned lineHeight);

    void fastenSelection();
    void eraseSelection();
};


