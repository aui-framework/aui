#include "AButton.h"
#include <AUI/Platform/AWindow.h>

AButton::AButton()
{

}

AButton::AButton(const AString& text): ALabel(text)
{

}

void AButton::setDefault()
{
	mDefault = true;
	connect(AWindow::current()->keyDown, this, [&](AInput::Key k) {
	    if (!mDefault)
	        throw AAbstractSignal::Disconnect();
	    if (k == AInput::Return) {
            emit clicked();
        }
	});

}

void AButton::getCustomCssAttributes(AMap<AString, AVariant>& map)
{
	AView::getCustomCssAttributes(map);
	if (mDefault)
		map["default"] = true;
}
