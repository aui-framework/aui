#include "ARadioButton.h"
#include "AUI/Layout/AHorizontalLayout.h"


ARadioButton::ARadioButton()
{


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
    if (!mChecked)
        emit checked(mChecked = true);
}

void ARadioButton::Group::addRadioButton(_<ARadioButton> radio) {
    mButtons << radio;
    connect(radio->checked, this, [&, radio]() {
        if (auto s =  mSelected.lock()) {
            s->setChecked(false);
        }
        mSelected = radio;
        emit selectionChanged(getSelectedId());
    });
}

_<ARadioButton> ARadioButton::Group::getSelectedRadio() const {
    return mSelected.lock();
}

int ARadioButton::Group::getSelectedId() const {
    if (auto s = getSelectedRadio()) {
        return mButtons.indexOf(s);
    }
    return 0;
}

void ARadioButtonInner::update() {
    emit customCssPropertyChanged;
}
