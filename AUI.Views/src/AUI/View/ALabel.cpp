#include "ALabel.h"
#include <AUI/Render/RenderHints.h>
#include <glm/gtc/matrix_transform.hpp>

#include "AViewContainer.h"

void ALabel::updateMultiline()
{
	mLines = getFontStyleLabel().font->trimStringToMultiline(mText, getWidth(), getFontStyleLabel().size, getFontStyleLabel().fontRendering);
	mPrerendered.mVao = nullptr;
}

ALabel::ALabel()
{
	AVIEW_CSS;
}

ALabel::ALabel(const AString& text):
	mText(text)
{
	AVIEW_CSS;
}

void ALabel::render()
{
	AView::render();
	
	if (!mPrerendered.mVao && !mText.empty())
	{
	    auto fs = getFontStyleLabel();
		if (mMultiline) {			
			mPrerendered = Render::instance().preRendererString(mLines.join('\n'), fs);
		} else
		{
			mPrerendered = Render::instance().preRendererString(mText, fs);
		}
	}

	
	if (mPrerendered.mVao || mIcon)
	{
		int leftPadding = mPadding.left;
		switch (getFontStyleLabel().align)
		{
		case ALIGN_LEFT:
			if (mIcon)
			{
				auto requiredSpace = mIcon->getSizeHint();
				RenderHints::PushState s;
				Render::instance().setTransform(glm::translate(glm::mat4(1.f),
					glm::vec3(leftPadding, (getContentHeight() - requiredSpace.y) / 2, 0)));
				mIcon->draw(requiredSpace);
				leftPadding += requiredSpace.x + 1;
			}
			break;
			
		case ALIGN_CENTER:
			leftPadding += getContentWidth() / 2;
			if (mIcon)
			{
				auto requiredSpace = mIcon->getSizeHint();
				RenderHints::PushState s;
				Render::instance().setTransform(glm::translate(glm::mat4(1.f),
					glm::vec3(leftPadding - (mPrerendered.mVao ? mPrerendered.fs.getWidth(mText) : 0) - requiredSpace.x / 2,
					(getContentHeight() - requiredSpace.y) / 2, 0)));
				mIcon->draw(requiredSpace);
			}
			
			break;
		}
		if (mPrerendered.mVao) {
		    int y = mPadding.top - (getFontStyleLabel().font->getDescenderHeight(getFontStyleLabel().size)) + 1;
		    if (mVerticalAlign == ALIGN_CENTER) {
		        y = glm::max(y, (getHeight() - getFontStyleLabel().size) / 2 - 1);
		    }
			Render::instance().drawString(leftPadding, y, mPrerendered);
		}
	}
}


int ALabel::getContentMinimumWidth()
{
	if (mMultiline)
		return 10;
	return getFontStyleLabel().getWidth(mText);
}

int ALabel::getContentMinimumHeight()
{
	if (mText.empty())
		return 0;
	if (mMultiline)
	{
		return mLines.size() * (getFontStyleLabel().font->getAscenderHeight(getFontStyleLabel().size)
		* (1.f + getFontStyleLabel().lineSpacing)) + getFontStyleLabel().font->getDescenderHeight(getFontStyleLabel().size) + 1;
	}
	return getFontStyleLabel().size;
}

void ALabel::setText(const AString& newText)
{
	mText = newText;
	mPrerendered.mVao = nullptr;
	if (mMultiline)
	{
		updateMultiline();
		if (getParent())
			getParent()->updateLayout();
	}
	redraw();
}

void ALabel::setMultiline(const bool multiline)
{
	mMultiline = multiline;
	mPrerendered.mVao = nullptr;
	if (multiline) {
		if (getParent())
			getParent()->updateLayout();
		updateMultiline();
	}
	else
		mLines.clear();
	redraw();
}

void ALabel::setGeometry(int x, int y, int width, int height)
{
    auto oldWidth = getWidth();
	AView::setGeometry(x, y, width, height);

    bool refresh = mMultiline && oldWidth != getWidth();

	if (mMultiline)
		updateMultiline();
	
	if (refresh) {
		AThread::current()->enqueue([&]()
		{
			if (getParent())
				getParent()->updateLayout();
		});
	}
}

FontStyle ALabel::getFontStyleLabel() {
    auto fs = getFontStyle();
    if (mFontOverride)
        fs.font = mFontOverride;
    if (mFontSizeOverride)
        fs.size = mFontSizeOverride;
    return fs;
}

void ALabel::userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor) {
    mPrerendered.mVao = nullptr;
    mVerticalAlign = ALIGN_LEFT;
    processor(css::T_VERTICAL_ALIGN, [&](property p)
    {
        if (p->getArgs().size() == 1) {
            if (p->getArgs()[0] == "middle")
                mVerticalAlign = ALIGN_CENTER;
            else
                mVerticalAlign = ALIGN_LEFT;
        }
    });
}
