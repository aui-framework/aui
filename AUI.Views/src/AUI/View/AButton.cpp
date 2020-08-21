#include "AButton.h"


AButton::AButton()
{
	AVIEW_CSS;
}

AButton::AButton(const AString& text): ALabel(text)
{
	AVIEW_CSS;
}

void AButton::setDefault()
{
	mDefault = true;
}

void AButton::getCustomCssAttributes(AMap<AString, AVariant>& map)
{
	AView::getCustomCssAttributes(map);
	if (mDefault)
		map["default"] = true;
}
