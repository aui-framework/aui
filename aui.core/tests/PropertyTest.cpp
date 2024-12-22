// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Util/kAUI.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "AUI/Common/AProperty.h"

namespace {
struct User {
    AProperty<AString> name;
};


class Receiver: public AObject {
public:
    MOCK_METHOD(void, receive, (const AString& msg));
};
}

TEST(PropertyTest, DesignatedInitializer) {
    auto u = aui::ptr::manage(User { .name = "Hello" });
    EXPECT_EQ(u->name, "Hello");
}

TEST(PropertyTest, ValueCanBeChanged) {
    auto u = aui::ptr::manage(User { .name = "Hello" });
    u->name = "World";

    // this should not compile
    // u->name = {"World"};

    EXPECT_EQ(u->name, "World");
}

TEST(PropertyTest, ValueCanBePassed) {
    auto u = aui::ptr::manage(User { .name = "Hello" });
    auto stringIdentity = [](const AString& value) {
        return value;
    };
    EXPECT_EQ(stringIdentity(u->name), "Hello");
}

TEST(PropertyTest, ValueOperatorArrow) {
    auto u = aui::ptr::manage(User { .name = "Hello" });
    EXPECT_EQ(u->name->length(), 5);
}

TEST(PropertyTest, ChangedSignal) {
    auto u = aui::ptr::manage(User { .name = "Hello" });
    auto receiver = _new<Receiver>();
    AObject::connect(u->name.changed, slot(receiver)::receive);

    EXPECT_CALL(*receiver, receive(AString("World")));
    u->name = "World";
}

