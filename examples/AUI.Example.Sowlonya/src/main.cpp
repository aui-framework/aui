#include <AUI/IO/FileInputStream.h>
#include <AUI/Platform/CustomWindow.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/View/ALabel.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Platform/Desktop.h>
#include <AUI/View/AButton.h>
#include <AUI/Render/Stylesheet.h>
#include <AUI/Util/Random.h>
#include <AUI/View/ASpacer.h>
#include <glm/glm.hpp>
#include <AUI/Image/AImageLoaderRegistry.h>

#include "AUI/Platform/CustomCaptionWindow.h"

int main()
{
	Random rand;

	CustomCaptionWindow w(u8"Соооонька ^^ <3 <3", 854, 400);
	AObject::connect(w.closed, &w, &AWindow::quit);


	auto label = _new<ALabel>(u8"Сонечка, я тебя люблю <3");
	label->addCssName(".window-content");
	label->setExpanding({ 1, 0 });
	w.addView(label);

	auto buttonPanel = _new<AViewContainer>();
	buttonPanel->setLayout(_new<AHorizontalLayout>());
	w.addView(buttonPanel);

	auto yes = _new<AButton>(u8"Мярр^^");
	yes->setDefault();
	AObject::connect(yes->clickedButton, &w, [&]()
	{
		AMessageBox::show(&w, u8"<3", u8"<3");
	});
	
	buttonPanel->addCssName(".button_box");
	buttonPanel->addView(_new<ASpacer>());
	buttonPanel->addView(yes);

	auto no = _new<AButton>(u8"Пошёл в жопу");
	AObject::connect(no->mouseEnter, no, [&]()
	{
		auto pos = Desktop::getMousePos();
		Desktop::setMousePos(pos + glm::ivec2((rand.nextVec2(0.2f, 1.f) * 2.f - 1.f) * 10.f * glm::vec2(no->getSize())));
	});
	AObject::connect(no->clickedButton, no, [&]()
	{
		AMessageBox::show(&w, u8"Божественный фреймворк алекса", u8"Ррррр");
		no->setEnabled(false);
	});

	buttonPanel->addView(no);

	w.pack();
	
	w.loop();

	return 0;
}

#ifdef _WIN32
int __stdcall WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd
)
{
	return main();
}
#endif