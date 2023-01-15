// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#include <AUI/Platform/AClipboard.h>
#include "AAbstractTextField.h"


#include "AUI/Platform/Platform.h"
#include <AUI/Util/AMetric.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Render/RenderHints.h>


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


int AAbstractTextField::getContentMinimumHeight(ALayoutDirection layout)
{
	return getFontStyle().size;
}


void AAbstractTextField::render()
{
	AView::render();

    prerenderStringIfNeeded();
	if (hasFocus()) {
	    auto absoluteCursorPos = ACursorSelectable::drawSelectionPre();

	    // text
        doDrawString();

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

            auto p = getMouseSelectionPadding();
            Render::rect(ASolidBrush{},
                         {p.x + absoluteCursorPos, p.y - 1},
                         {1, getMouseSelectionFont().size + 2});
        }

        Render::setBlending(Blending::NORMAL);
    } else {
        doDrawString();
	}

}

void AAbstractTextField::doDrawString() {
    RenderHints::PushMatrix m;
    Render::translate({ mPadding.left - mHorizontalScroll, mPadding.top + getVerticalAlignmentOffset() });
    if (mPrerenderedString) mPrerenderedString->draw();
}


void AAbstractTextField::setText(const AString& t)
{
    mHorizontalScroll = 0;
	mContents = t;
    if (t.empty()) {
        clearSelection();
        mCursorIndex = 0;
    }

	updateCursorBlinking();

    invalidatePrerenderedString();
	emit textChanged(t);
}


AString AAbstractTextField::getContentsPasswordWrap() {
    if (mIsPasswordTextField) {
        return AString(mContents.length(), L'•');
    }
    return mContents;
}

AString AAbstractTextField::getDisplayText() {
    return getContentsPasswordWrap();
}

void AAbstractTextField::doRedraw() {
    redraw();
}

const AString& AAbstractTextField::text() const {
    return mContents;
}

void AAbstractTextField::typeableErase(size_t begin, size_t end) {
    if (begin >= mContents.length()) {
        return;
    }
    mContents.erase(mContents.begin() + begin, mContents.begin() + end);
}

bool AAbstractTextField::typeableInsert(size_t at, const AString& toInsert) {
    mContents.insert(at, toInsert);
    if (!isValidText(mContents)) {
        mContents.erase(at, toInsert.length()); // undo insert
        return false;
    }
    return true;
}

bool AAbstractTextField::typeableInsert(size_t at, wchar_t toInsert) {
    mContents.insert(at, toInsert);
    if (!isValidText(mContents)) {
        mContents.erase(at, 1); // undo insert
        return false;
    }
    return true;
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
    mPrerenderedString = nullptr;
}

void AAbstractTextField::invalidateFont() {
    mPrerenderedString = nullptr;
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

void AAbstractTextField::prerenderStringIfNeeded() {
    if (!mPrerenderedString) {
        auto text = getContentsPasswordWrap();
        if (!text.empty()) {
            auto canvas = Render::newMultiStringCanvas(getFontStyle());
            canvas->enableCachingForTextLayoutHelper();
            canvas->addString({ 0, 0 }, text);
            setTextLayoutHelper(canvas->getTextLayoutHelper());
            mPrerenderedString = canvas->finalize();
        } else {
            setTextLayoutHelper({});
        }
    }
}

bool AAbstractTextField::isValidText(const AString& text) {
    return true;
}

size_t AAbstractTextField::textLength() const {
    return mContents.length();
}

AString AAbstractTextField::toString() const {
    return mContents;
}

