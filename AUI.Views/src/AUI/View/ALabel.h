#pragma once
#include "AView.h"
#include "AUI/Render/Render.h"
#include "AUI/Common/AString.h"
#include "AUI/Image/IDrawable.h"

class API_AUI_VIEWS ALabel: public AView
{
private:
	AString mText;
	Render::PrerendereredString mPrerendered;
	_<IDrawable> mIcon;
    _<AFont> mFontOverride;
    uint8_t mFontSizeOverride = 0;

	bool mMultiline = false;
	AStringVector mLines;

	void updateMultiline();

	FontStyle getFontStyleLabel();
	
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

	void setIcon(const _<IDrawable>& drawable)
	{
		mIcon = drawable;
		redraw();
	}

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

	void setGeometry(int x, int y, int width, int height) override;
};