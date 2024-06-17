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

#include <AUI/Platform/ACursor.h>
#include "AUI/Traits/callables.h"
#include "AUI/Image/IDrawable.h"
#include "AUI/Platform/AWindow.h"



ACursor::~ACursor() {

}

ACursor::ACursor(aui::non_null<_<IDrawable>> drawable, int size): mValue(std::move(drawable)), mSize(size) {
}

ACursor::ACursor(AUrl imageUrl, int size): mValue([&] {
    if (auto image = IDrawable::fromUrl(imageUrl)) {
        return image;
    }
    throw AException("bad cursor url: {} (please check [Drawable] log)"_format(imageUrl.full()));
}()), mSize(size) {

}

