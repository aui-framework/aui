#include "ACustomCaptionWindow.h"
#include <AUI/Util/UIBuildingHelpers.h>

ACustomCaptionWindow::ACustomCaptionWindow(const AString& name, int width, int height, bool stacked):
	ACustomWindow(name, width, height),
	CustomCaptionWindowImplWin32()
{
    initCustomCaption(name, stacked, this);
    connect(mMiddleButton->clickedButton, this, [&]() {
        if (isMaximized()) {
            restore();
        } else {
            maximize();
        }
    });

    connect(minimized, this, [&]() {updateMiddleButtonIcon();});
    connect(restored, this, [&]() {updateMiddleButtonIcon();});
    connect(maximized, this, [&]() {updateMiddleButtonIcon();});
    connect(mCloseButton->clickedButton, this, &AWindow::quit);
    connect(mMinimizeButton->clickedButton, this, &AWindow::minimize);
}

bool ACustomCaptionWindow::isCustomCaptionMaximized() {
    return isMaximized();
}
