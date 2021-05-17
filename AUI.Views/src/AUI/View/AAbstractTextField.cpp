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
#include "AUI/Render/Render.h"
#include "AUI/Render/RenderHints.h"
#include <AUI/Util/AMetric.h>
#include <AUI/Action/AMenu.h>
#include <AUI/Util/kAUI.h>
#include <AUI/i18n/AI18n.h>

ATimer& AAbstractTextField::blinkTimer()
{
	static ATimer t(500);
	if (!t.isStarted())
	{
		t.start();
	}
	return t;
}

void AAbstractTextField::updateCursorBlinking()
{
	blinkTimer().restart();
	mCursorBlinkVisible = true;
	mCursorBlinkCount = 0;
	redraw();
}

void AAbstractTextField::updateCursorPos()
{
	auto absoluteCursorPos = -mHorizontalScroll + int(getFontStyle().getWidth(getContentsPasswordWrap().mid(0, mCursorIndex)));

	const int SCROLL_ADVANCEMENT = getContentWidth() * 4 / 10;

	if (absoluteCursorPos < 0)
	{
		mHorizontalScroll += absoluteCursorPos - SCROLL_ADVANCEMENT;
	}
	else if (absoluteCursorPos >= getContentWidth())
	{
		mHorizontalScroll += absoluteCursorPos - getContentWidth() + SCROLL_ADVANCEMENT;
	}
	mHorizontalScroll = glm::clamp(mHorizontalScroll, 0, glm::max(int(getFontStyle().getWidth(getContentsPasswordWrap())) - getContentWidth() + 1, 0));
}



AAbstractTextField::AAbstractTextField()
{
	connect(blinkTimer().fired, this, [&]()
	{
		if (hasFocus() && mCursorBlinkCount < 60) {
			mCursorBlinkVisible = !mCursorBlinkVisible;
			mCursorBlinkCount += 1;
			redraw();
		}
	});

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

void AAbstractTextField::onKeyDown(AInput::Key key)
{
    AView::onKeyDown(key);
	onKeyRepeat(key);
}

void AAbstractTextField::onKeyRepeat(AInput::Key key)
{
	if (AInput::isKeyDown(AInput::LButton))
		return;


	auto fastenSelection = [&]() {
        if (!AInput::isKeyDown(AInput::LShift) && !AInput::isKeyDown(AInput::RShift)) {
            mCursorSelection = -1;
        } else if (mCursorSelection == -1)
        {
            mCursorSelection = mCursorIndex;
        }
	};
	
	mTextChangedFlag = true;
	switch (key)
	{
	case AInput::Delete:
		if (hasSelection()) {
			auto sel = getSelection();
			mContents.erase(mContents.begin() + sel.begin, mContents.begin() + sel.end);
			mCursorSelection = -1;
			mCursorIndex = sel.begin;
		} else
		{
			if (mCursorIndex < mContents.length())
			{
				mContents.removeAt(mCursorIndex);
			}
		}
		break;

	case AInput::Left:
        fastenSelection();
		if (mCursorIndex) {
		    if (AInput::isKeyDown(AInput::LControl)) {
		        if (mCursorIndex <= 1) {
		            mCursorIndex = 0;
		        } else {
                    mCursorIndex = mContents.rfind(' ', mCursorIndex - 2) + 1;
                }
            } else {
                mCursorIndex -= 1;
            }
        }
		break;

	case AInput::Right:
        fastenSelection();
		if (mCursorIndex < mContents.length()) {
            if (AInput::isKeyDown(AInput::LControl)) {
                auto index = mContents.find(' ', mCursorIndex);
                if (index == AString::NPOS) {
                    mCursorIndex = mContents.length();
                } else {
                    mCursorIndex = index + 1;
                }
            } else {
                mCursorIndex += 1;
            }
		}
		break;

	case AInput::Home:
        fastenSelection();
		mCursorIndex = 0;
		break;
	case AInput::End:
        fastenSelection();
		mCursorIndex = mContents.length();
		break;

    default:
		if (AInput::isKeyDown(AInput::LControl) || AInput::isKeyDown(AInput::RControl)) {
            switch (key) {
                case AInput::A: // select all
					selectAll();
					break;

                case AInput::C: // copy
					copyToClipboard();
					break;

                case AInput::X: // cut
					cutToClipboard();
                    break;

                case AInput::V: // paste
					pasteFromClipboard();
					break;

                default:
                    return;
            }
		} else {
            return;
		}
	}

	emit textChanging(mContents);
	
	updateCursorPos();
	updateCursorBlinking();
	
	redraw();
}

void AAbstractTextField::pasteFromClipboard() {
	auto pastePos = mCursorIndex;
	if (mCursorSelection != -1) {
		auto sel = getSelection();
		pastePos = sel.begin;
		mContents.erase(mContents.begin() + sel.begin, mContents.begin() + sel.end);
	}
	auto toPaste = AClipboard::pasteFromClipboard();
	if (mMaxTextLength <= mContents.length())
		return;
	if (!mIsMultiline) {
		toPaste = toPaste.replacedAll("\n", "");
	}
	mContents.insert(pastePos, toPaste);
	mCursorIndex = pastePos + toPaste.length();
	mCursorSelection = -1;

	invalidatePrerenderedString();
	emit textChanged;
}

void AAbstractTextField::cutToClipboard() {
	auto sel = getSelection();
	AClipboard::copyToClipboard(getSelectedText());
	mContents.erase(mContents.begin() + sel.begin, mContents.begin() + sel.end);
	mCursorIndex = sel.begin;
	mCursorSelection = -1;
}

void AAbstractTextField::copyToClipboard() const { AClipboard::copyToClipboard(getSelectedText()); }

void AAbstractTextField::selectAll() { ACursorSelectable::selectAll(); }

void AAbstractTextField::onCharEntered(wchar_t c)
{
	if (AInput::isKeyDown(AInput::LButton) ||
	    AInput::isKeyDown(AInput::LControl) ||
	    AInput::isKeyDown(AInput::RControl) ||
	    c == '\t')
		return;
	if (c == '\n' || c == '\r')
        return;

	mTextChangedFlag = true;
	AString contentsCopy = mContents;
	auto cursorIndexCopy = mCursorIndex;

	if (hasSelection()) {
		auto sel = getSelection();
		mContents.erase(mContents.begin() + sel.begin, mContents.begin() + sel.end);
		
		switch (c)
		{
		case '\b':
			mCursorIndex = sel.begin;
			break;
		default:
			mContents.insert(sel.begin, c);
			mCursorIndex = sel.begin + 1;
		}
		mCursorSelection = -1;
	} else {
		switch (c)
		{
		case '\b':
			if (mCursorIndex != 0) {
				mContents.removeAt(--mCursorIndex);
			}
			break;
		default:
		    if (mMaxTextLength <= mContents.length())
		        return;
			mContents.insert(mCursorIndex++, c);
		}
		if (!isValidText(mContents))
		{
			mContents = std::move(contentsCopy);
			mCursorIndex = cursorIndexCopy;
			Platform::playSystemSound(Platform::S_ASTERISK);
		}
	}
	emit textChanging(mContents);
    invalidatePrerenderedString();
    updateCursorBlinking();
	updateCursorPos();

	if (!AInput::isKeyDown(AInput::LShift) && !AInput::isKeyDown(AInput::RShift))
	{
		mCursorSelection = -1;
	}
	
	redraw();
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
        if (hasFocus() && mCursorBlinkVisible) {
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

void AAbstractTextField::onFocusLost()
{
	AView::onFocusLost();
	if (mTextChangedFlag)
	{
		mTextChangedFlag = false;
		emit textChanged(mContents);
	}
	
}

void AAbstractTextField::onMousePressed(glm::ivec2 pos, AInput::Key button)
{
    AView::onMousePressed(pos, button);
    ACursorSelectable::handleMousePressed(pos, button);
	updateCursorBlinking();
}

void AAbstractTextField::onMouseMove(glm::ivec2 pos)
{
    AView::onMouseMove(pos);
	ACursorSelectable::handleMouseMove(pos);
}

void AAbstractTextField::onMouseReleased(glm::ivec2 pos, AInput::Key button)
{
    AView::onMouseReleased(pos, button);

	if (button == AInput::RButton) {
		AMenu::show({
						{ "aui.cut"_i18n, [&]{cutToClipboard();}, AInput::LControl + AInput::X, hasSelection() },
						{ "aui.copy"_i18n, [&]{copyToClipboard();}, AInput::LControl + AInput::C, hasSelection() },
						{ "aui.paste"_i18n, [&]{pasteFromClipboard();}, AInput::LControl + AInput::V, !AClipboard::isEmpty() },
						AMenu::SEPARATOR,
						{ "aui.select_all"_i18n, [&]{selectAll();}, AInput::LControl + AInput::A, !getText().empty() }
				});
	} else {
		ACursorSelectable::handleMouseReleased(pos, button);
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

void AAbstractTextField::onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button) {
    AView::onMouseDoubleClicked(pos, button);
    ACursorSelectable::handleMouseDoubleClicked(pos, button);
    updateCursorBlinking();
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

glm::ivec2 AAbstractTextField::getMouseSelectionPadding() {
    return {mPadding.left, mPadding.top};
}

glm::ivec2 AAbstractTextField::getMouseSelectionScroll() {
    return {mHorizontalScroll, 0};
}

FontStyle AAbstractTextField::getMouseSelectionFont() {
    return getFontStyle();
}

AString AAbstractTextField::getMouseSelectionText() {
    return getContentsPasswordWrap();
}

void AAbstractTextField::doRedraw() {
    redraw();
}

void AAbstractTextField::invalidateFont() {
    mPrerenderedString.mVao = nullptr;
}
