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

#include <gtest/gtest.h>
#include "AUI/Platform/AInput.h"

TEST(AInputOverride, ForcesKeyState) {
    AInput::overrideState(AInput::LSHIFT, true);
    EXPECT_TRUE(AInput::isKeyDown(AInput::LSHIFT));
    AInput::overrideState(AInput::LSHIFT, false);
    EXPECT_FALSE(AInput::isKeyDown(AInput::LSHIFT));
    AInput::resetOverride(AInput::LSHIFT);
}

TEST(AInputOverride, KeysAreIndependent) {
    AInput::overrideState(AInput::LBUTTON, true);
    AInput::overrideState(AInput::RBUTTON, false);
    EXPECT_TRUE(AInput::isKeyDown(AInput::LBUTTON));
    EXPECT_FALSE(AInput::isKeyDown(AInput::RBUTTON));
    AInput::resetOverride(AInput::LBUTTON);
    AInput::resetOverride(AInput::RBUTTON);
}
