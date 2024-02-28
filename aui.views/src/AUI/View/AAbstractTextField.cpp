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

#include <AUI/Platform/AClipboard.h>
#include <AUI/Platform/ABaseWindow.h>
#include "AAbstractTextField.h"


#include "AUI/Platform/APlatform.h"
#include "AUI/Platform/ADesktop.h"
#include <AUI/Util/AMetric.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Render/RenderHints.h>


AAbstractTextField::AAbstractTextField() {
}

void AAbstractTextField::onFocusAcquired() {
    AView::onFocusAcquired();
    updateCursorBlinking();
}

void AAbstractTextField::onFocusLost() {
    AAbstractTypeableView::onFocusLost();
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


void AAbstractTextField::render(ClipOptimizationContext context)
{
	AView::render(context);

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
            ARender::rect(ASolidBrush{},
                          {p.x + absoluteCursorPos, p.y - 1},
                          {1, getMouseSelectionFont().size + 2});
        }

        ARender::setBlending(Blending::NORMAL);
    } else {
        doDrawString();
	}

}

glm::ivec2 AAbstractTextField::getMouseSelectionPadding() {
    auto p = AAbstractTypeableView::getMouseSelectionPadding();
    p.x += mTextAlignOffset;
    return p;
}

void AAbstractTextField::doDrawString() {
    RenderHints::PushMatrix m;
    ARender::translate({ mPadding.left - mHorizontalScroll + mTextAlignOffset, mPadding.top + getVerticalAlignmentOffset() });
    if (mPrerenderedString) mPrerenderedString->draw();
}


void AAbstractTextField::setText(const AString& t)
{
    mHorizontalScroll = 0;
	mContents = t;
    if (t.empty()) {
        clearSelection();
    }

    mCursorIndex = t.size();
	updateCursorBlinking();

    invalidatePrerenderedString();
	emit textChanged(t);
}

bool AAbstractTextField::wantsTouchscreenKeyboard() {
    return true;
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
    mCursorIndex = std::min(mCursorIndex, static_cast<unsigned int> (mContents.size()));
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
        ADesktop::playSystemSound(ADesktop::SystemSound::ASTERISK);
    }
    emit textChanging(mContents);
}

void AAbstractTextField::prerenderStringIfNeeded() {
    if (!mPrerenderedString) {
        auto text = getContentsPasswordWrap();
        updateTextAlignOffset();
        if (!text.empty()) {
            auto canvas = ARender::newMultiStringCanvas(getFontStyle());
            canvas->enableCachingForTextLayoutHelper();
            switch (getFontStyle().align) {
                case ATextAlign::LEFT:
                    canvas->addString({ 0, 0 }, text);
                    break;
                case ATextAlign::CENTER:
                    canvas->addString({ 0, 0 }, text);
                    break;

                case ATextAlign::RIGHT:
                    canvas->addString({ 0, 0 }, text);
                    break;
            }
            setTextLayoutHelper(canvas->getTextLayoutHelper());
            mPrerenderedString = canvas->finalize();
        } else {
            setTextLayoutHelper({});
        }
    }
}

void AAbstractTextField::updateTextAlignOffset() {
    switch (getFontStyle().align) {
        case ATextAlign::JUSTIFY:
        case ATextAlign::LEFT:
            mTextAlignOffset = 0;
            return;
        default:
            break;
    }

    auto w = getFontStyle().getWidth(getContentsPasswordWrap());
    if (w >= getContentWidth()) {
        mTextAlignOffset = 0; // unbreak the scroll
        return;
    }

    switch (getFontStyle().align) {
        case ATextAlign::CENTER:
            mTextAlignOffset = (getContentWidth() - w) / 2;
            return;

        case ATextAlign::RIGHT:
            mTextAlignOffset = getContentWidth() - w;
            return;
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

void AAbstractTextField::setSize(glm::ivec2 size) {
    AView::setSize(size);
    updateTextAlignOffset();
}

