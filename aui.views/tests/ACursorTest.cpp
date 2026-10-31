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
#include <AUI/Platform/ACursor.h>

TEST(ACursor, HotspotDefaultsToTopLeft) {
    ACursor::Properties properties;
    EXPECT_EQ(properties.size, 16);
    EXPECT_EQ(properties.hotspot, ACursor::Hotspot::TOP_LEFT);
}

TEST(ACursor, DesignatedInitializers) {
    ACursor::Properties properties { .size = 32, .hotspot = ACursor::Hotspot::CENTER };
    EXPECT_EQ(properties.size, 32);
    EXPECT_EQ(properties.hotspot, glm::vec2(0.5f));
}

TEST(ACursor, SystemCursorKeepsDefaults) {
    ACursor cursor(ACursor::POINTER);
    EXPECT_EQ(cursor.hotspot(), ACursor::Hotspot::TOP_LEFT);
}

TEST(ACursor, HotspotInPixels) {
    EXPECT_EQ(ACursor::hotspotInPixels(ACursor::Hotspot::TOP_LEFT, { 32, 32 }), glm::ivec2(0, 0));
    EXPECT_EQ(ACursor::hotspotInPixels(ACursor::Hotspot::CENTER, { 32, 32 }), glm::ivec2(16, 16));
    EXPECT_EQ(ACursor::hotspotInPixels({ 1.f, 1.f }, { 32, 48 }), glm::ivec2(32, 48));
    // scales with the rasterization size: the same relative hotspot at 2x display scaling
    EXPECT_EQ(ACursor::hotspotInPixels(ACursor::Hotspot::CENTER, { 64, 64 }), glm::ivec2(32, 32));
}
