#include "ACustomCaptionWindow.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include "AUI/View/AButton.h"

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
	mCaptionContainer->addCssName(".window_title_content");
	caption->addView(mCaptionContainer);

	auto minimize = _new<AButton>();
	minimize->addCssName(".minimize");
	minimize->addCssName(".default");
	connect(minimize->clickedButton, this, &AWindow::minimize);
	caption->addView(minimize);

	auto middle = _new<AButton>();
	middle->addCssName(".middle");
	middle->addCssName(".default");
	//connect(middle->clickedButton, &w, &Window::quit);
	caption->addView(middle);

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
		}) by(AViewContainer, {
			setExpanding({1, 1});
		}));
	} else {
		setLayout(_new<AVerticalLayout>());
		addView(caption);
        addView(mContentContainer = _new<AViewContainer>());
	}
	mContentContainer->setExpanding({1, 1});
}
