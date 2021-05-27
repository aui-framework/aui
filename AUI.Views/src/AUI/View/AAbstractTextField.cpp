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

#include <AUI/Platform/AClipboard.h>
#include "AAbstractTextField.h"


#include "AUI/Platform/Platform.h"
#include <AUI/Util/AMetric.h>
#include <AUI/Util/kAUI.h>



AAbstractTextField::AAbstractTextField() {

}

void AAbstractTextField::onFocusAcquired() {
    AView::onFocusAcquired();
    updateCursorBlinking();
}

AAbstractTextField::~AAbstractTextField()
{
}

bool AAbstractTextField::handlesNonMouseNavigation() {
    return true;
}


int AAbstractTextField::getContentMinimumHeight()
{
	return getFontStyle().size;
}


void AAbstractTextField::render()
{
	AView::render();

	if (hasFocus()) {
	    auto absoluteCursorPos = ACursorSelectable::drawSelectionPre();

	    // text
        Render::inst().drawString(mPadding.left - mHorizontalScroll, mPadding.top, getContentsPasswordWrap(), getFontStyle());

        ACursorSelectable::drawSelectionPost();

        // cursor
        if (hasFocus() && isCursorBlinkVisible()) {
            if (absoluteCursorPos < 0) {
                mHorizontalScroll += absoluteCursorPos;
                redraw();
            } else if (getWidth() < absoluteCursorPos + mPadding.horizontal() + 1) {
                mHorizontalScroll += absoluteCursorPos - getWidth() + mPadding.horizontal() + 1;
                redraw();
            }

            Render::inst().drawRect(mPadding.left + absoluteCursorPos,
                                    mPadding.top, glm::ceil(1_dp), getFontStyle().size + 3);
        }
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
	    if (!mPrerenderedString.mVao) {
            mPrerenderedString = Render::inst().preRendererString(getContentsPasswordWrap(), getFontStyle());
	    }
        Render::inst().drawString(mPadding.left - mHorizontalScroll, mPadding.top, mPrerenderedString);
        Render::inst().setFill(Render::FILL_SOLID);
        Render::inst().setColor({1, 1, 1, 1 });
	}

}


void AAbstractTextField::setText(const AString& t)
{
    mHorizontalScroll = 0;
	mContents = t;
	mCursorIndex = t.length();
	mCursorSelection = 0;
	updateCursorBlinking();

    invalidatePrerenderedString();
	emit textChanged(t);
}


AString AAbstractTextField::getContentsPasswordWrap() {
    if (mIsPasswordTextField) {
        AString s;
        for (auto c : mContents)
            s += "•";
        return s;
    }
    return mContents;
}

AString AAbstractTextField::getDisplayText() {
    return getContentsPasswordWrap();
}

void AAbstractTextField::doRedraw() {
    redraw();
}

AString AAbstractTextField::getText() const {
    return mContents;
}

void AAbstractTextField::typeableErase(size_t begin, size_t end) {
    mContents.erase(mContents.begin() + begin - 1, mContents.begin() + end - 1);
}

void AAbstractTextField::typeableInsert(size_t at, const AString& toInsert) {
    mContents.insert(at, toInsert);
}

void AAbstractTextField::typeableInsert(size_t at, wchar_t toInsert) {
    mContents.insert(at, toInsert);
}

size_t AAbstractTextField::typeableFind(wchar_t c, size_t startPos) {
    return mContents.find(c, startPos);
}

size_t AAbstractTextField::typeableReverseFind(wchar_t c, size_t startPos) {
    return mContents.rfind(c, startPos);
}

size_t AAbstractTextField::length() const {
    return mContents.length();
}

void AAbstractTextField::invalidatePrerenderedString() {
    mPrerenderedString.mVao = nullptr;
}

void AAbstractTextField::invalidateFont() {
    mPrerenderedString.mVao = nullptr;
}

void AAbstractTextField::onCharEntered(wchar_t c) {
    if (c == '\n' || c == '\r')
        return;
    AView::onCharEntered(c);
    auto contentsCopy = mContents;
    auto cursorIndexCopy = mCursorIndex;

    enterChar(c);
    if (!isValidText(mContents))
    {
        mContents = std::move(contentsCopy);
        mCursorIndex = cursorIndexCopy;
        Platform::playSystemSound(Platform::S_ASTERISK);
    }
    emit textChanging(mContents);
}

