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
    MOCK_METHOD(void, receiveStr, (const AString& msg));
    MOCK_METHOD(void, receiveInt, (int msg));
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
    AObject::connect(u->name.changed, slot(receiver)::receiveStr);

    EXPECT_CALL(*receiver, receiveStr(AString("World")));
    u->name = "World";
}

TEST(PropertyTest, PropertyConnection) {
    auto receiver = _new<Receiver>();
    auto u = aui::ptr::manage(User { .name = "Hello" });

    EXPECT_CALL(*receiver, receiveStr(AString("Hello"))).Times(1);
    AObject::connect(u->name, slot(receiver)::receiveStr);

    EXPECT_CALL(*receiver, receiveStr(AString("World"))).Times(1);
    u->name = "World";
}

TEST(PropertyTest, PropertyConnectionWithProjection) {
    auto receiver = _new<Receiver>();
    auto u = aui::ptr::manage(User { .name = "Hello" });

    EXPECT_CALL(*receiver, receiveInt(5)).Times(1);
    AObject::connect(u->name, slot(receiver)::receiveInt, &AString::length);

    EXPECT_CALL(*receiver, receiveInt(6)).Times(1);
    u->name = "World!";
}

namespace {
class CustomSetter : public AObject {
public:
    CustomSetter() {
        ON_CALL(*this, setName(testing::_)).WillByDefault([&](const AString& s) {
            emit nameChanged(s);
        });
    }

    auto name() const {
        return APropertyDef {
            .base = this,
            .get = &CustomSetter::getName,
            .set = &CustomSetter::setName,
            .changed = nameChanged,
        };
    }

    MOCK_METHOD(void, setName, (const AString& msg));

private:
    emits<AString> nameChanged;

    AString getName() const { return "can't change"; }
};
}

TEST(PropertyTest, CustomSetter) {
    CustomSetter u;
    Receiver receiver;

    static_assert(AAnyProperty<decltype(u.name())>);

    EXPECT_CALL(u, setName(AString("World")));
    EXPECT_CALL(receiver, receiveStr(AString("World")));

    AObject::connect(u.name().changed, slot(receiver)::receiveStr);

    u.name() = "World";
    EXPECT_EQ(AString(u.name()), "can't change");
}

TEST(PropertyTest, CustomSetterProperty) {
    CustomSetter u;
    auto receiver = _new<Receiver>();

    EXPECT_CALL(u, setName(AString("World")));
    EXPECT_CALL(*receiver, receiveStr(AString("can't change"))).Times(1);
    EXPECT_CALL(*receiver, receiveStr(AString("World"))).Times(1);

    AObject::connect(u.name(), slot(receiver)::receiveStr);

    u.name() = "World";
    EXPECT_EQ(AString(u.name()), "can't change");
}
