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

	mCaptionContent = _new<AViewContainer>();
	mCaptionContent->setLayout(_new<AHorizontalLayout>());
	mCaptionContent->setExpanding({ 1, 0 });
	mCaptionContent->addCssName(".window_title_content");
	caption->addView(mCaptionContent);

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
		addView(_container<AVerticalLayout>({
			caption,
			_new<ASpacer>(),
		}) by(AViewContainer, {
			setExpanding({1, 1});
		}));
	} else {
		setLayout(_new<AVerticalLayout>());
		addView(caption);
	}
}
