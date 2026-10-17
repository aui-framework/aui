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
#include <AUI/Action/AShortcut.h>

TEST(AShortcut, SystemKeyName) {
#if AUI_PLATFORM_MACOS
    EXPECT_EQ(AInput::getName(AInput::LSYSTEM), "⌘");
#elif AUI_PLATFORM_WIN
    EXPECT_EQ(AInput::getName(AInput::LSYSTEM), "Win");
#else
    EXPECT_EQ(AInput::getName(AInput::LSYSTEM), "Meta");
#endif
    EXPECT_EQ(AInput::getName(AInput::LSYSTEM), AInput::getName(AInput::RSYSTEM));
}

TEST(AShortcut, Simple) {
    AString s = AShortcut(AInput::F5);
    EXPECT_EQ(s, "F5");
}

TEST(AShortcut, ModifierAndKey) {
    AString s = AShortcut(AInput::LCONTROL + AInput::C);
#if AUI_PLATFORM_MACOS
    EXPECT_EQ(s, "⌃C");
#else
    EXPECT_EQ(s, "Ctrl+C");
#endif
}

TEST(AShortcut, SystemAndKey) {
    AString s = AShortcut(AInput::LSYSTEM + AInput::C);
#if AUI_PLATFORM_MACOS
    EXPECT_EQ(s, "⌘C");
#elif AUI_PLATFORM_WIN
    EXPECT_EQ(s, "Win+C");
#else
    EXPECT_EQ(s, "Meta+C");
#endif
}

TEST(AShortcut, MultipleModifiers) {
    AString s = AShortcut(AShortcut(AInput::LSYSTEM + AInput::LSHIFT) + AInput::Z);
#if AUI_PLATFORM_MACOS
    // canonical macOS order: control, option, shift, command, then the key.
    EXPECT_EQ(s, "⇧⌘Z");
#elif AUI_PLATFORM_WIN
    EXPECT_EQ(s, "Win+Shift+Z");
#else
    EXPECT_EQ(s, "Meta+Shift+Z");
#endif
}

#if AUI_PLATFORM_MACOS
TEST(AShortcut, MacOSModifierOrder) {
    AString s = AShortcut(AShortcut(AShortcut(AInput::LSYSTEM + AInput::LSHIFT) + AInput::LALT) + AInput::LCONTROL);
    EXPECT_EQ(s, "⌃⌥⇧⌘");
    AString t = AShortcut(AShortcut(AInput::LSYSTEM + AInput::A) + AInput::LCONTROL);
    EXPECT_EQ(t, "⌃⌘A");
}

TEST(AShortcut, MacOSSpecialKeys) {
    EXPECT_EQ(AInput::getName(AInput::ESCAPE), "⎋");
    EXPECT_EQ(AInput::getName(AInput::RETURN), "⏎");
    EXPECT_EQ(AInput::getName(AInput::BACKSPACE), "⌫");
}
#endif

TEST(AShortcut, CmdCtrl) {
    AString s = AShortcut(AInput::CMD_CTRL + AInput::C);
#if AUI_PLATFORM_APPLE
    EXPECT_EQ(s, "⌘C");
#else
    EXPECT_EQ(s, "Ctrl+C");
#endif
    EXPECT_EQ(AInput::getName(AInput::CMD_CTRL), AInput::getName(AInput::RCMD_CTRL));
}
