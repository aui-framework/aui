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
#include "AUI/Image/IDrawable.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Platform/CommonRenderingContext.h"
#include "IPlatformAbstraction.h"

ACursor::ACursor(AImageView image, Properties properties)
  : mValue(IPlatformAbstraction::current().createCustomCursor(
        image, hotspotInPixels(properties.hotspot, image.size()))),
    mSize(properties.size),
    mHotspot(properties.hotspot) {}

void ACursor::applyNativeCursor(AWindow* pWindow) const {
    IPlatformAbstraction::current().applyNativeCursor(*this, pWindow);
}
