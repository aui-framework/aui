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

#include "ACheckBox.h"
#include "AUI/Util/Declarative/Containers.h"

ACheckBox::ACheckBox(_<AView> content)
{
    mBox = _new<ACheckBox::Box>();
    setContents(declarative::Horizontal {
      declarative::Centered { mBox },
      std::move(content),
    });

    // legacy behavior
    connect(clicked, [this]() {
        mBox->checked = !mBox->checked;
    });
}

ACheckBox::Box::Box() {
    connect(checked.changed, [this] { emit customCssPropertyChanged; });
}

_<AView> declarative::CheckBox::operator()() {
    auto checkbox = _new<ACheckBox>(std::move(content));
    checkbox->clicked.clearAllOutgoingConnectionsWith(checkbox); // removes default legacy behavior
    AObject::connect(checkbox->clicked, checkbox, [&checkbox = *checkbox]() {
        AUI_EMIT_FOREIGN(checkbox.box(), userCheckedChange, !checkbox.box()->checked);
    });
    checked.bindTo(checkbox->box()->checked.assignment());
    onCheckedChange.bindTo(checkbox->box()->userCheckedChange);
    return checkbox;
}
