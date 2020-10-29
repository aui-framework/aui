#include "ACustomCaptionWindow.h"
#include <AUI/Util/UIBuildingHelpers.h>

ACustomCaptionWindow::ACustomCaptionWindow(const AString& name, int width, int height, bool stacked):
	ACustomWindow(name, width, height)
{

	auto caption = _new<AViewContainer>();
	caption->setLayout(_new<AHorizontalLayout>());
	caption->addCssName(".window-title");
	caption->setExpanding({1, 0});

	auto titleLabel = _new<ALabel>(name);
	caption->addView(titleLabel);

    mCaptionContainer = _new<AViewContainer>();
	mCaptionContainer->setLayout(_new<AHorizontalLayout>());
	mCaptionContainer->setExpanding({1, 0 });
	mCaptionContainer->addCssName(".window-title-content");
	caption->addView(mCaptionContainer);

	auto minimize = _new<AButton>();
	minimize->addCssName(".minimize");
	minimize->addCssName(".default");
	connect(minimize->clickedButton, this, &AWindow::minimize);
	caption->addView(minimize);

	mMiddle = _new<AButton>();
	mMiddle->addCssName(".middle");
	mMiddle->addCssName(".default");
	connect(mMiddle->clickedButton, this, [&]() {
	    if (isMaximized()) {
            restore();
	    } else {
            maximize();
        }
	});

	connect(minimized, this, &ACustomCaptionWindow::updateMiddleButtonIcon);
	connect(restored, this, &ACustomCaptionWindow::updateMiddleButtonIcon);
	connect(maximized, this, &ACustomCaptionWindow::updateMiddleButtonIcon);

	caption->addView(mMiddle);

	auto close = _new<AButton>();
	close->addCssName(".close");
	close->addCssName(".default");
	connect(close->clickedButton, this, &AWindow::quit);
	caption->addView(close);

	if (stacked) {
        setLayout(_new<AStackedLayout>());
        addView(mContentContainer = _new<AViewContainer>());
        addView(_container<AVerticalLayout>({
                                                    caption,
                                                    _new<ASpacer>(),
                                            }) let (AViewContainer, {
            setExpanding({1, 1});
        }));
	} else {
		setLayout(_new<AVerticalLayout>());
		addView(caption);
        addView(mContentContainer = _new<AViewContainer>());
	}
	mContentContainer->setExpanding({1, 1});

	updateMiddleButtonIcon();
}

void ACustomCaptionWindow::updateMiddleButtonIcon() {
    if (isMaximized()) {
        mMiddle->setCss("background: url(':win/caption/restore.svg')");
    } else {
        mMiddle->setCss("background: url(':win/caption/maximize.svg')");
    }
}
