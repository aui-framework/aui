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
#include <AUI/Url/AUrl.h>

TEST(Url, LocalUnix1) {
    EXPECT_EQ(AUrl("/home/test/file.txt").full(), "file:///home/test/file.txt");
}

TEST(Url, LocalUnix2) {
    EXPECT_EQ(AUrl("./file.txt").full(), "file://file.txt");
}

TEST(Url, LocalWindows) {
    EXPECT_EQ(AUrl("C:/Users/test/file.txt").full(), "file://C:/Users/test/file.txt");
}

TEST(Url, Http) {
    AUrl url("https://github.com/aui-framework/aui");
    EXPECT_EQ(url.full(), "https://github.com/aui-framework/aui");
    EXPECT_EQ(url.schema(), "https");
    EXPECT_EQ(url.path(), "github.com/aui-framework/aui");
}

TEST(Url, Builtin) {
    AUrl url(":asset/test.txt");
    EXPECT_EQ(url.full(), "builtin://asset/test.txt");
    EXPECT_EQ(url.schema(), "builtin");
    EXPECT_EQ(url.path(), "asset/test.txt");
}
