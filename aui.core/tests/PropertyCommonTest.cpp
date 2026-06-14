// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
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

TEST(PropertyCommonTest, DesignatedInitializer) {
    auto u = aui::ptr::manage_shared(new User { .name = "Hello" });
    EXPECT_EQ(u->name, "Hello");
}

TEST(PropertyCommonTest, ValueCanBeChanged) {
    auto u = aui::ptr::manage_shared(new User { .name = "Hello" });
    u->name = "World";

    // this should not compile
    // u->name = {"World"};

    EXPECT_EQ(u->name, "World");
}

TEST(PropertyCommonTest, ValueCanBePassed) {
    auto u = aui::ptr::manage_shared(new User { .name = "Hello" });
    auto stringIdentity = [](const AString& value) {
        return value;
    };
    EXPECT_EQ(stringIdentity(u->name), "Hello");
}

TEST(PropertyCommonTest, ValueOperatorArrow) {
    auto u = aui::ptr::manage_shared(new User { .name = "Hello" });
    EXPECT_EQ(u->name->length(), 5);
}

TEST(PropertyCommonTest, ChangedSignal) {
    auto u = aui::ptr::manage_shared(new User { .name = "Hello" });
    auto receiver = _new<Receiver>();
    AObject::connect(u->name.changed, AUI_SLOT(receiver)::receiveStr);

    EXPECT_CALL(*receiver, receiveStr(AString("World")));
    u->name = "World";
}

TEST(PropertyCommonTest, PropertyConnection) {
    auto receiver = _new<Receiver>();
    auto u = aui::ptr::manage_shared(new User { .name = "Hello" });

    EXPECT_CALL(*receiver, receiveStr(AString("Hello"))).Times(1);
    AObject::connect(u->name, AUI_SLOT(receiver)::receiveStr);

    EXPECT_CALL(*receiver, receiveStr(AString("World"))).Times(1);
    u->name = "World";
}

TEST(PropertyCommonTest, PropertyConnectionWithProjection1) {
    auto receiver = _new<Receiver>();
    auto u = aui::ptr::manage_shared(new User { .name = "Hello" });

    EXPECT_CALL(*receiver, receiveInt(5)).Times(1);
    AObject::connect(AUI_REACT(u->name->length()), AUI_SLOT(receiver)::receiveInt);

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
            this,
            &CustomSetter::getName,
            &CustomSetter::setName,
            nameChanged,
        };
    }

    MOCK_METHOD(void, setName, (const AString& msg));

private:
    emits<AString> nameChanged;

    AString getName() const { return "can't change"; }
};
}

TEST(PropertyCommonTest, CustomSetter) {
    CustomSetter u;
    Receiver receiver;

    static_assert(AAnyProperty<decltype(u.name())>);

    EXPECT_CALL(u, setName(AString("World")));
    EXPECT_CALL(receiver, receiveStr(AString("World")));

    AObject::connect(u.name().changed, AUI_SLOT(receiver)::receiveStr);

    u.name() = "World";
    EXPECT_EQ(AString(u.name()), "can't change");
}

TEST(PropertyCommonTest, CustomSetterProperty) {
    CustomSetter u;
    auto receiver = _new<Receiver>();

    EXPECT_CALL(u, setName(AString("World")));
    EXPECT_CALL(*receiver, receiveStr(AString("can't change"))).Times(1);
    EXPECT_CALL(*receiver, receiveStr(AString("World"))).Times(1);

    AObject::connect(u.name(), AUI_SLOT(receiver)::receiveStr);

    u.name() = "World";
    EXPECT_EQ(AString(u.name()), "can't change");
}

TEST(PropertyCommonTest, Property2PropertyBoth) {
    auto u = aui::ptr::manage_shared(new User { .name = "initial" });
    auto r = _new<CustomSetter>();

    EXPECT_CALL(*r, setName(AString("initial"))).Times(1);
    AObject::biConnect(u->name, r->name());

    EXPECT_CALL(*r, setName(AString("New Name1"))).Times(1);
    u->name = "New Name1";

    EXPECT_CALL(*r, setName(AString("New Name2"))).Times(1);
    r->setName("New Name2");

    // the setName "New Name2" call above should reflect its change to u.
    EXPECT_EQ(u->name, "New Name2");

    // death of r should clear the link with u->name.
    r = nullptr;
    u->name = "Should not crash";
}

TEST(PropertyCommonTest, Property2PropertySetOnly) {
    auto u = aui::ptr::manage_shared(new User { .name = "initial" });
    auto r = _new<CustomSetter>();

    EXPECT_CALL(*r, setName(AString("initial"))).Times(1);
    AObject::connect(u->name, r->name());

    EXPECT_CALL(*r, setName(AString("New Name1"))).Times(1);
    u->name = "New Name1";

    EXPECT_CALL(*r, setName(AString("New Name2"))).Times(1);
    r->setName("New Name2");

    // the setName "New Name2" call above should NOT reflect its change to u.
    EXPECT_EQ(u->name, "New Name1");

    // death of r should clear the link with u->name.
    r = nullptr;
    u->name = "Should not crash";
}
