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

#include <gmock/gmock.h>
#include <AUI/Common/AObject.h>
#include "AUI/Common/AProperty.h"
#include "AUI/Util/kAUI.h"

class PropertyModifierTest: public testing::Test {};


namespace {
class LogObserver : public AObject {
public:
    LogObserver() {
        ON_CALL(*this, log(testing::_)).WillByDefault([](const AString& msg) {
        });
    }
    MOCK_METHOD(void, log, (const AString& msg), ());
    MOCK_METHOD(void, observeString, (const AString&), ());
    MOCK_METHOD(void, observeInt, (int), ());
};
}

// AUI_DOCS_OUTPUT: doxygen/intermediate/property_modifier.h
// @class aui::PropertyModifier
//
// Non-const operators of properties such as non-const versions of `operator=`, `operator+=`, `operator-=` have a side
// effect of emitting `changed` signal upon operation completion. This ensures that modifying access to the property can
// be observed.
//
TEST_F(PropertyModifierTest, Write_operators_observable1) {
    LogObserver observer;
    // AUI_DOCS_CODE_BEGIN
    AProperty<int> counter = 0;
    AObject::connect(counter.changed, AUI_SLOT(observer)::observeInt);
    EXPECT_CALL(observer, observeInt(1)).Times(1);
    counter += 1; // observable by observeInt
    // AUI_DOCS_CODE_END
}

TEST_F(PropertyModifierTest, Write_operators_observable2) {
    LogObserver observer;
    // AUI_DOCS_CODE_BEGIN
    AProperty<AString> name = "Hello";
    AObject::connect(name.changed, AUI_SLOT(observer)::observeString);
    EXPECT_CALL(observer, observeString("Hello world"_as)).Times(1);
    name += " world"; // observable by observeString
    // AUI_DOCS_CODE_END
}

//
// !!! note
//
//     Make sure your read-only operators (such as `operator+`, `operator-`) have marked const, otherwise property
//     would treat them as a writing access, resulting in unwanted signaling `changed` upon each access.
TEST_F(PropertyModifierTest, Write_operators_const_access) {
    LogObserver observer;
    // AUI_DOCS_CODE_BEGIN
    AProperty<int> counter = 0;
    AObject::connect(counter.changed, AUI_SLOT(observer)::observeInt);
    EXPECT_CALL(observer, observeInt(testing::_)).Times(0);
    int nextCounter = counter + 1; // read-only access; noone is notified
    // AUI_DOCS_CODE_END
}

//
// ## Member access operator (operator->)
// `operator->` is a special case. `operator->` having both non-const and const versions is a common practice, so
// there's should be a way to distinguish between non-const access and const access, preferring the latter if possible.
// The const version of `operator->` can be used directly on property:
TEST_F(PropertyModifierTest, Write_operators_prefer_const_access) {
    LogObserver observer;

    // AUI_DOCS_CODE_BEGIN
    EXPECT_CALL(observer, observeString(testing::_)).Times(0);
    AProperty<AString> name = "Hello";
    AObject::connect(name.changed, AUI_SLOT(observer)::observeString);

    [[maybe_unused]] // HIDE
    // returns const pointer
    auto data = name->data();
    // AUI_DOCS_CODE_END
}

// [property_system] is designed in such a way you would explicitly express a modifying operation via binary equals
// operator (and favours such as `+=`, `-=`):
//
TEST_F(PropertyModifierTest, Write_operators_write_equals) {
    LogObserver observer;
    // AUI_DOCS_CODE_BEGIN
    AProperty<AString> name = "Hello";
    AObject::connect(name.changed, AUI_SLOT(observer)::observeString);

    EXPECT_CALL(observer, observeString("Test"_as)).Times(1);
    name = "Test";
    // AUI_DOCS_CODE_END
}

// However, it is still possible to achieve non-const version of `operator->`. To do this, you need a
// [aui::PropertyModifier] object that grants such access:

TEST_F(PropertyModifierTest, Write_operators_write_operator_arrow1) {
    LogObserver observer;
    // AUI_DOCS_CODE_BEGIN
    AProperty<AString> name = "Hello";
    AObject::connect(name.changed, AUI_SLOT(observer)::observeString);

    EXPECT_CALL(observer, observeString("Hell"_as)).Times(1);
    name.writeScope()->removeAll('o');
    // AUI_DOCS_CODE_END
}

//
// You need to be careful when performing multiple operations at once. Design rationale behind `writeScope()` method
// makes it painful (by intention) performing multiple accesses, since it would lead to unwanted change notifications
// during the process:
TEST_F(PropertyModifierTest, Write_operators_write_operator_arrow2) {
    LogObserver observer;
    AProperty<AString> name = "Hello";
    AObject::connect(name.changed, AUI_SLOT(observer)::observeString);

    // AUI_DOCS_CODE_BEGIN
    // WRONG WAY
    EXPECT_CALL(observer, observeString("Hell"_as)).Times(1);
    name.writeScope()->removeAll('o');

    EXPECT_CALL(observer, observeString("He"_as)).Times(1);
    name.writeScope()->removeAll('l');
    // AUI_DOCS_CODE_END
}

//
// The right way is to create [aui::PropertyModifier] just once. This will produce exactly one notification, ensuring
// that modifications to the property are performed atomically. This means that all operations within
// the scope of [aui::PropertyModifier] produced by `writeScope()` will be treated as one unit, and only one change
// notification will be emitted.
TEST_F(PropertyModifierTest, Write_operators_write_operator_arrow3) {
    LogObserver observer;
    AProperty<AString> name = "Hello";
    AObject::connect(name.changed, AUI_SLOT(observer)::observeString);

    // AUI_DOCS_CODE_BEGIN
    // RIGHT WAY
    EXPECT_CALL(observer, observeString("He"_as)).Times(1);
    auto nameWriteable = name.writeScope();
    nameWriteable->removeAll('o');
    nameWriteable->removeAll('l');
    // AUI_DOCS_CODE_END
}
