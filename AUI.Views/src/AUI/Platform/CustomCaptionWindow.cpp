#include "CustomCaptionWindow.h"

#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/View/AButton.h"
#include "AUI/View/ALabel.h"

CustomCaptionWindow::CustomCaptionWindow(const AString& name, int width, int height): CustomWindow(name, width, height)
{
	setLayout(_new<AVerticalLayout>());
	
	auto caption = _new<AViewContainer>();
	caption->setLayout(_new<AHorizontalLayout>());
	caption->addCssName(".window-title");

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

	addView(caption);
}
