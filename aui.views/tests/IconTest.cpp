/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2026 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>
#include "AUI/View/ADrawableView.h"

using namespace declarative;

namespace {
constexpr auto COLLAPSED = ":uni/svg/tree-collapsed.svg";
constexpr auto EXPANDED = ":uni/svg/tree-expanded.svg";
}   // namespace

TEST(Icon, Url) {
    _<ADrawableIconView> icon = Icon { COLLAPSED };
    EXPECT_EQ(icon->getDrawable(), IDrawable::fromUrl(COLLAPSED));
}

TEST(Icon, Drawable) {
    auto drawable = IDrawable::fromUrl(COLLAPSED);
    _<ADrawableIconView> icon = Icon { drawable };
    EXPECT_EQ(icon->getDrawable(), drawable);
}

TEST(Icon, ReactiveUrl) {
    auto expanded = _new<AProperty<bool>>(false);
    _<ADrawableIconView> icon = Icon { AUI_REACT(**expanded ? EXPANDED : COLLAPSED) };
    EXPECT_EQ(icon->getDrawable(), IDrawable::fromUrl(COLLAPSED));
    *expanded = true;
    EXPECT_EQ(icon->getDrawable(), IDrawable::fromUrl(EXPANDED));
}

TEST(Icon, ReactiveDrawable) {
    auto expanded = _new<AProperty<bool>>(false);
    _<ADrawableIconView> icon = Icon { AUI_REACT(IDrawable::fromUrl(**expanded ? EXPANDED : COLLAPSED)) };
    EXPECT_EQ(icon->getDrawable(), IDrawable::fromUrl(COLLAPSED));
    *expanded = true;
    EXPECT_EQ(icon->getDrawable(), IDrawable::fromUrl(EXPANDED));
}
