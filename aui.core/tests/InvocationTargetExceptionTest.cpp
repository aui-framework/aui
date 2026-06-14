// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "AUI/Thread/AFuture.h"
#include <gtest/gtest.h>

TEST(InvocationTargetException, Case1) {
    auto exception = [] {
        try {
            throw AException("inner");
        } catch (const AException& e) {
            return AInvocationTargetException("outer", std::current_exception());
        }
    }();
    EXPECT_EQ(exception.getMessage(), "outer: inner");
}

TEST(InvocationTargetException, Case2) {
    AInvocationTargetException exception("outer");
    EXPECT_EQ(exception.getMessage(), "outer");
}
