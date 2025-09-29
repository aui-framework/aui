/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ARadioButton.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Util/Declarative/Containers.h"

ARadioButton::ARadioButton(_<AView> content)
{
    mCircle = _new<ARadioButton::Circle>();
    setContents(declarative::Horizontal {
      mCircle,
      std::move(content),
    });

    // legacy behavior
    connect(clicked, [this]() {
        mCircle->checked = true;
    });
}

ARadioButton::~ARadioButton()
{
}

ARadioButton::Circle::Circle() {
    connect(checked.changed, [this] { emit customCssPropertyChanged; });
}

_<AView> declarative::RadioButton::operator()() {
    auto radiobutton = _new<ARadioButton>(std::move(content));
//    radiobutton->clicked.clearAllOutgoingConnectionsWith(radiobutton); // removes default legacy behavior
    checked.bindTo(radiobutton->circle()->checked);
    onClick.bindTo(radiobutton->clicked);
    return radiobutton;
}



// legacy stuff, subject to be removed =================================================================================
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
