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


#include <AUI/Util/ACursorSelectable.h>
#include "AUI/Common/ATimer.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Font/IFontView.h"

/**
 * @brief Base class for AAbstractTypeableView which is template class.
 */
class AAbstractTypeable: public ACursorSelectable {
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
        setText(text().trim());
    }

    void setCopyable(bool isCopyable) {
        mIsCopyable = isCopyable;
    }

    virtual void setText(const AString& t);

    void copyToClipboard() const;
    void cutToClipboard();
    void pasteFromClipboard();

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

protected:
    size_t mMaxTextLength = 0x200;

    bool isCursorBlinkVisible() const {
        return mCursorBlinkVisible;
    }

protected:

    void updateCursorBlinking();

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
    virtual void updateSelectionOnTextSet(const AString& t);

    virtual void onCursorIndexChanged() = 0;


    /**
     * Char enter implementation. Should be called in onCharEntered.
     * @param c
     */
    void enterChar(char16_t c);

    AString getDisplayText() override;
    AMenuModel composeContextMenuImpl();
    void handleKey(AInput::Key key);

private:
    static _<ATimer> blinkTimer();

    _<ATimer> mBlinkTimer = blinkTimer();

    unsigned mCursorBlinkCount = 0;
    bool mCursorBlinkVisible = true;
    bool mTextChangedFlag = false;
    bool mIsMultiline = false;
    bool mIsCopyable = true;

    template<aui::derived_from<AView> Super>
    friend class AAbstractTypeableView;

    virtual void emitTextChanged(const AString& text) = 0;
    virtual void emitTextChanging(const AString& text) = 0;
    virtual void typeableInvalidateFont() = 0;
    void drawCursorImpl(IRenderer& renderer, glm::ivec2 position, unsigned lineHeight);
};


