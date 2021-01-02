#include "ACheckBox.h"
#include "AUI/Layout/AHorizontalLayout.h"

class ACheckBoxInner: public AView
{
public:
	ACheckBoxInner()
	{

	}
	virtual ~ACheckBoxInner() = default;

	void update()
	{
		emit customCssPropertyChanged;
	}

    void render() override {
        AView::render();
    }
};


ACheckBox::ACheckBox()
{

	
	setLayout(_new<AHorizontalLayout>());
	
	mText = _new<ALabel>();
	auto checkbox = _new<ACheckBoxInner>();
	addView(checkbox);
	addView(mText);

	connect(checked, checkbox, &ACheckBoxInner::update);
}

ACheckBox::ACheckBox(const ::AString& text): ACheckBox()
{
	setText(text);
}

ACheckBox::~ACheckBox()
{
}

void ACheckBox::setText(const AString& text)
{
	mText->setText(text);
}

void ACheckBox::getCustomCssAttributes(AMap<AString, AVariant>& map)
{
	AViewContainer::getCustomCssAttributes(map);
	if (mChecked)
		map["checked"] = true;
}

void ACheckBox::onMouseReleased(glm::ivec2 pos, AInput::Key button)
{
	AView::onMouseReleased(pos, button);
	if (button == AInput::LButton) {
        emit checked(mChecked = !mChecked);
    }
}

bool ACheckBox::consumesClick(const glm::ivec2& pos) {
    return true;
}
