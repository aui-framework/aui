/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ARadioButton.h"
#include "AUI/Layout/AHorizontalLayout.h"


ARadioButton::ARadioButton()
{


    setLayout(std::make_unique<AHorizontalLayout>());

    mText = _new<ALabel>();
    auto checkbox = _new<ARadioButtonInner>();
    addView(checkbox);
    addView(mText);

    connect(mCheckedChanged, checkbox, &ARadioButtonInner::update);
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
    mText->text() = text;
}

void ARadioButton::onPointerReleased(const APointerReleasedEvent& event)
{
    AView::onPointerReleased(event);
    if (!mChecked && event.triggerClick)
        emit mCheckedChanged(mChecked = true);
}

bool ARadioButton::selectableIsSelectedImpl() {
    return mChecked;
}

_<ARadioButton> ARadioButton::Group::addRadioButton(const _<ARadioButton>& radio, int id) {
    if (id == -1) id = int(mButtons.size());
    AUI_ASSERTX(!mButtons.contains(id), "this id is already used; please choose another id");
    mButtons[id] = radio;

    connect(radio->checked(), this, [&, radio, id](bool checked) {
        if (checked) {
            if (auto s = mSelectedRadio.lock()) {
                s->setChecked(false);
            }
            mSelectedRadio = radio;
            mSelectedId = id;
            emit selectionChanged(getSelectedId());
        }
    });
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
