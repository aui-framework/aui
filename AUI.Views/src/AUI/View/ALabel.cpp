#include "ALabel.h"
#include <AUI/Render/RenderHints.h>
#include <glm/gtc/matrix_transform.hpp>

#include "AViewContainer.h"

#include <AUI/Platform/AWindow.h>

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

	doRenderText();
}


int ALabel::getContentMinimumWidth()
{
	if (mMultiline)
		return 10;
	if (!mPrerendered.mVao) {
	    doPrerender();
	}
    return mPrerendered.length;
	/*
	if (mPrerendered.mVao) {
	    return mPrerendered.length;
	}
	return getFontStyleLabel().getWidth(mText);*/
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
	}
    if (getParent())
        getParent()->updateLayout();
	redraw();
}

void ALabel::setMultiline(const bool multiline)
{
	mMultiline = multiline;
	mPrerendered.mVao = nullptr;
	if (multiline) {
		if (getParent())
			getParent()->updateLayout();
		//updateMultiline();
	}
	else
		mLines.clear();
	redraw();
}

void ALabel::setSize(int width, int height) {
    auto oldWidth = getWidth();
    AView::setSize(width, height);

    bool refresh = mMultiline && (mLines.empty() || oldWidth != getWidth());

    if (mMultiline) {
        updateMultiline();
        if (mLines.empty())
            return;
    }

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
    mTextTransform = TT_NORMAL;
    processor(css::T_TEXT_TRANSFORM, [&](property p)
    {
        if (p->getArgs().size() == 1) {
            if (p->getArgs()[0] == "uppercase")
                mTextTransform = TT_UPPERCASE;
            else if (p->getArgs()[0] == "lowercase")
                mTextTransform = TT_LOWERCASE;
        }
    });
}

void ALabel::doPrerender() {
    auto fs = getFontStyleLabel();
    auto t = getTargetText();
    if (t.empty())
        return;
    mPrerendered = Render::inst().preRendererString(t, fs);
}

void ALabel::doRenderText() {
    if (mMultiline && mLines.empty()) {
        updateMultiline();
    }
    if (!mPrerendered.mVao && !mText.empty())
    {
        doPrerender();
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
                    Render::inst().setTransform(glm::translate(glm::mat4(1.f),
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
                    Render::inst().setTransform(glm::translate(glm::mat4(1.f),
                                                               glm::vec3(leftPadding - (mPrerendered.mVao ? mPrerendered.fs.getWidth(mText) : 0) - requiredSpace.x / 2,
                                                                             (getContentHeight() - requiredSpace.y) / 2, 0)));
                    mIcon->draw(requiredSpace);
                }

                break;

            case ALIGN_RIGHT:
                leftPadding += getContentWidth();
                if (mIcon)
                {
                    auto requiredSpace = mIcon->getSizeHint();
                    RenderHints::PushState s;
                    Render::inst().setTransform(glm::translate(glm::mat4(1.f),
                                                               glm::vec3(leftPadding - (mPrerendered.mVao ? mPrerendered.fs.getWidth(mText) : 0) - requiredSpace.x / 2,
                                                                             (getContentHeight() - requiredSpace.y) / 2, 0)));
                    mIcon->draw(requiredSpace);
                }

                break;
        }
        if (mPrerendered.mVao) {
            int y = mPadding.top - (getFontStyleLabel().font->getDescenderHeight(getFontStyleLabel().size)) + 1;
            if (mVerticalAlign == ALIGN_CENTER) {
                if (mMultiline) {
                    y = (glm::max)(y, (getHeight() - getMinimumHeight()) / 2 - 1);
                } else {
                    y = (glm::max)(y, (getHeight() - getFontStyleLabel().size) / 2 - 1);
                }
            }
            Render::inst().drawString(leftPadding, y, mPrerendered);
        }
    }
}

AString ALabel::getTargetText() {
    AString targetString;
    if (mMultiline) {
        targetString = mLines.join('\n');
    } else
    {
        targetString = mText;
    }
    if (targetString.empty())
        return {};
    switch (mTextTransform) {
        case TT_UPPERCASE:
            targetString = targetString.uppercase();
            break;
        case TT_LOWERCASE:
            targetString = targetString.lowercase();
            break;
    }
    return targetString;
}
