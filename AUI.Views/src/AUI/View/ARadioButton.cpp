/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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
    connect(radio->checked, this, [&, radio](bool checked) {
        if (checked) {
            if (auto s = mSelected.lock()) {
                s->setChecked(false);
            }
            mSelected = radio;
            emit selectionChanged(getSelectedId());
        }
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

void ARadioButton::Group::setSelectedId(int id) {
    mButtons[id]->setChecked(true);
}

void ARadioButtonInner::update() {
    emit customCssPropertyChanged;
}
