// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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

void ARadioButton::onPointerReleased(const APointerReleasedEvent& event)
{
    AView::onPointerReleased(event);
    if (!mChecked && event.triggerClick)
        emit checked(mChecked = true);
}

bool ARadioButton::selectableIsSelectedImpl() {
    return mChecked;
}

_<ARadioButton> ARadioButton::Group::addRadioButton(const _<ARadioButton>& radio, int id) {
    if (id == -1) id = int(mButtons.size());
    AUI_ASSERTX(!mButtons.contains(id), "this id is already used; please choose another id");
    mButtons[id] = radio;

    auto onChecked = [&, radio, id](bool checked) {
        if (checked) {
            if (auto s = mSelectedRadio.lock()) {
                s->setChecked(false);
            }
            mSelectedRadio = radio;
            mSelectedId = id;
            emit selectionChanged(getSelectedId());
        }
    };
    if (radio->isChecked()) onChecked(true);
    connect(radio->checked, this, std::move(onChecked));
    return radio;
}

_<ARadioButton> ARadioButton::Group::getSelectedRadio() const {
    return mSelectedRadio.lock();
}

int ARadioButton::Group::getSelectedId() const {
    return mSelectedId;
}

void ARadioButton::Group::setSelectedId(int id) {
    if (mSelectedId == id) return;
    mSelectedId = id;
    mButtons[id]->setChecked(true);
}

void ARadioButtonInner::update() {
    emit customCssPropertyChanged;
}
