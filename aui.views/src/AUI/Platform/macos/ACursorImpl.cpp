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

#include <AUI/Platform/ACursor.h>
#include "AUI/Traits/callables.h"
#include "AUI/Image/IDrawable.h"
#include "AUI/Platform/AWindow.h"


struct ACursor::Custom {
public:

private:
};

ACursor::ACursor(aui::no_escape<AImage> image, int size) : mValue(std::make_unique<ACursor::Custom>()), mSize(size) {}

void ACursor::applyNativeCursor(AWindow* pWindow) const {

}
