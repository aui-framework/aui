#pragma once
#include "AView.h"
#include "AUI/Render/Render.h"
#include "AUI/Common/AString.h"
#include "AUI/Image/IDrawable.h"

class API_AUI_VIEWS ALabel: public AView
{
private:
	AString mText;
	_<IDrawable> mIcon;
    _<AFont> mFontOverride;
    uint8_t mFontSizeOverride = 0;
    TextAlign mVerticalAlign;
    enum {
        TT_NORMAL,
        TT_UPPERCASE,
        TT_LOWERCASE
    } mTextTransform = TT_NORMAL;
	bool mMultiline = false;

protected:
    AStringVector mLines;
    Render::PrerenderedString mPrerendered;

	void updateMultiline();
	FontStyle getFontStyleLabel();

	const Render::PrerenderedString& getPrerendered() {
	    return mPrerendered;
	}

    //void userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor) override;
    void doRenderText();
    AString getTargetText();


    // для корректного позиционирования выделения
    int mTextLeftOffset = 0;

public:
	ALabel();
	explicit ALabel(const AString& text);

	void render() override;

	int getContentMinimumWidth() override;
	int getContentMinimumHeight() override;


	const _<IDrawable>& getIcon() const
	{
		return mIcon;
	}

	void setIcon(const _<IDrawable>& drawable) {
        mIcon = drawable;
        redraw();
    }
    void doPrerender();


public:

    void setText(const AString& newText);


	[[nodiscard]] bool isMultiline() const
	{
		return mMultiline;
	}

	[[nodiscard]] const AString& getText() const
	{
		return mText;
	}

	void setMultiline(const bool multiline);
	void setFont(_<AFont> font) {
	    mFontOverride = font;
	}
	void setFontSize(uint8_t size) {
        mFontSizeOverride = size;
    }

    void setSize(int width, int height) override;

};