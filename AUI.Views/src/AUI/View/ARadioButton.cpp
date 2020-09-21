#include "ARadioButton.h"
#include "AUI/Layout/AHorizontalLayout.h"

class ARadioButtonInner: public AView
{
public:
    ARadioButtonInner()
    {
        AVIEW_CSS;
    }
    virtual ~ARadioButtonInner() = default;

    void update()
    {
        emit customCssPropertyChanged;
    }
};


ARadioButton::ARadioButton()
{
    AVIEW_CSS;

    setLayout(_new<AHorizontalLayout>());

    mText = _new<ALabel>();
    auto checkbox = _new<ARadioButtonInner>();
    addView(checkbox);
    addView(mText);

    connect(checked, checkbox, &ARadioButtonInner::update);
}

ARadioButton::ARadioButton(const ::AString& text): ARadioButton()
{
    setText(text);
}

ARadioButton::~ARadioButton()
{
}

void ARadioButton::setText(const AString& text)
{
    mText->setText(text);
}

void ARadioButton::getCustomCssAttributes(AMap<AString, AVariant>& map)
{
    AViewContainer::getCustomCssAttributes(map);
    if (mChecked)
        map["checked"] = true;
}

void ARadioButton::onMouseReleased(glm::ivec2 pos, AInput::Key button)
{
    AView::onMouseReleased(pos, button);
    emit checked(mChecked = !mChecked);
}