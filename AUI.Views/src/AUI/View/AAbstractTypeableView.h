/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 5/23/2021.
//


#pragma once

#include <AUI/Util/ACursorSelectable.h>
#include "AView.h"
#include "AUI/Common/ATimer.h"
#include <AUI/Render/Render.h>


/**
 * Basic implementation of type shortcuts and selection for editable text fields.
 * Used as base in ATextArea and ATextField, both of them using own way of handling and rendering text
 */
class API_AUI_VIEWS AAbstractTypeableView: public AView, public ACursorSelectable {
private:
    static ATimer& blinkTimer();

    unsigned mCursorBlinkCount = 0;
    bool mCursorBlinkVisible = true;
    bool mTextChangedFlag = false;
    bool mIsMultiline = false;


protected:
    int mHorizontalScroll = 0;
    size_t mMaxTextLength = 0x200;

    bool isCursorBlinkVisible() const {
        return mCursorBlinkVisible;
    }

protected:

    void updateCursorBlinking();
    void updateCursorPos();

    virtual void invalidatePrerenderedString() = 0;
    virtual void typeableErase(size_t begin, size_t end) = 0;
    virtual void typeableInsert(size_t at, const AString& toInsert) = 0;
    virtual void typeableInsert(size_t at, wchar_t toInsert) = 0;
    virtual size_t typeableFind(wchar_t c, size_t startPos = -1) = 0;
    virtual size_t typeableReverseFind(wchar_t c, size_t startPos = -1) = 0;
    virtual size_t length() const = 0;

    /**
     * Char enter implementation. Should be called in onCharEntered.
     * @param c
     */
    void enterChar(wchar_t c);

    glm::ivec2 getMouseSelectionPadding() override;
    glm::ivec2 getMouseSelectionScroll() override;
    FontStyle getMouseSelectionFont() override;
    AString getDisplayText() override;
    void doRedraw() override;

public:
    AAbstractTypeableView();
    virtual ~AAbstractTypeableView();

    int getContentMinimumHeight() override;

    void onKeyDown(AInput::Key key) override;
    void onKeyRepeat(AInput::Key key) override;

    void onFocusLost() override;
    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    void onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button) override;

    void onMouseMove(glm::ivec2 pos) override;
    void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;

    void clear() {
        setText({});
    }
    void setMaxTextLength(size_t newTextLength) {
        mMaxTextLength = newTextLength;
    }

    void trimText() {
        setText(getText().trim());
    }


    virtual void setText(const AString& t);

    bool handlesNonMouseNavigation() override;
    void onFocusAcquired() override;

signals:
    /**
     * \brief Text changed.
     * \note This signal is also emitted by the AAbstractTextField::setText function
     */
    emits<AString> textChanged;

    /**
     * \brief Text is changing by the user.
     */
    emits<AString> textChanging;

    void selectAll();
    void copyToClipboard() const;
    void cutToClipboard();
    void pasteFromClipboard();
};


