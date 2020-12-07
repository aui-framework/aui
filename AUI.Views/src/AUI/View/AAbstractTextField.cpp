#include <AUI/Platform/AClipboard.h>
#include "AAbstractTextField.h"


#include "AUI/Platform/Platform.h"
#include "AUI/Render/Render.h"
#include "AUI/Render/RenderHints.h"
#include <AUI/Util/AMetric.h>

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
	AVIEW_CSS;
}


AAbstractTextField::~AAbstractTextField()
{
}


int AAbstractTextField::getContentMinimumHeight()
{
	return getFontStyle().size;
}

void AAbstractTextField::onKeyDown(AInput::Key key)
{
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
		if (mCursorIndex)
			mCursorIndex -= 1;
		break;

	case AInput::Right:
        fastenSelection();
		if (mCursorIndex < mContents.length())
			mCursorIndex += 1;
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
                case AInput::A: // выделить всё
                    ACursorSelectable::selectAll();
                    break;

                case AInput::C: // копировать
                    AClipboard::copyToClipboard(getSelectedText());
                    break;

                case AInput::X: // вырезать
                {
                    auto sel = getSelection();
                    AClipboard::copyToClipboard(getSelectedText());
                    mContents.erase(mContents.begin() + sel.begin, mContents.begin() + sel.end);
                    mCursorSelection = -1;
                }
                    break;

                case AInput::V: // вставить
                {
                    auto pastePos = mCursorIndex;
                    if (mCursorSelection != -1) {
                        auto sel = getSelection();
                        pastePos = sel.begin;
                        mContents.erase(mContents.begin() + sel.begin, mContents.begin() + sel.end);
                    }
                    auto toPaste = AClipboard::pasteFromClipboard();
                    mContents.insert(pastePos, toPaste);
                    mCursorIndex = pastePos + toPaste.length();
                    mCursorSelection = -1;
                    break;
                }
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

void AAbstractTextField::onCharEntered(wchar_t c)
{
	if (AInput::isKeyDown(AInput::LButton) || AInput::isKeyDown(AInput::LControl) || AInput::isKeyDown(AInput::RControl))
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

	    // текст
        Render::inst().drawString(mPadding.left - mHorizontalScroll, mPadding.top, getContentsPasswordWrap(), getFontStyle());

        ACursorSelectable::drawSelectionPost();

        // курсор
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
        Render::inst().drawString(mPadding.left - mHorizontalScroll, mPadding.top, getContentsPasswordWrap(), getFontStyle());
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
    ACursorSelectable::handleMousePressed(pos, button);
	updateCursorBlinking();
}

void AAbstractTextField::onMouseMove(glm::ivec2 pos)
{
	ACursorSelectable::handleMouseMove(pos);
}

void AAbstractTextField::onMouseReleased(glm::ivec2 pos, AInput::Key button)
{
    ACursorSelectable::handleMouseReleased(pos, button);
}

void AAbstractTextField::setText(const AString& t)
{
    mHorizontalScroll = 0;
	mContents = t;
	mCursorIndex = t.length();
	mCursorSelection = 0;
	updateCursorBlinking();

	emit textChanged;
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
