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

#include "AButton.h"
#include <AUI/Platform/AWindow.h>

using namespace declarative;

AButton::AButton() {
}

void AButton::setText(AString text) {
    setContents(Centered { Label { std::move(text) } });
}

void AButton::setDefault(bool isDefault) {
    mDefault.set(this, isDefault);
    connect(AWindow::current()->keyDown, this, [&](AInput::Key k) {
        if (!mDefault) {
            AObject::disconnect();
        } else {
            if (k == AInput::RETURN) {
                emit clicked();
            }
        }
    });

}

bool AButton::consumesClick(const glm::ivec2& pos) {
    return true;
}
