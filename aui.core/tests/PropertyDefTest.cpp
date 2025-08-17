// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <AUI/Common/AProperty.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include <gmock/gmock.h>

class PropertyDefTest : public testing::Test {
public:
    /*
        template<typename... Args>
        static auto& connections(ASignal<Args...>& signal) {
            return signal.mOutgoingConnections;
        }

        static auto& connections(AObject& object) {
            return object.mIngoingConnections;
        }*/
};

namespace {
// AUI_DOCS_OUTPUT: doxygen/intermediate/property_def.h
// @class APropertyDef
// ## Declaration
//
// To declare a property with custom getter/setter, use APropertyDef template. APropertyDef-based property is
// defined by const member function as follows:
// AUI_DOCS_CODE_BEGIN
class User : public AObject {
public:
    auto name() const {
        return APropertyDef {
            this,
            &User::getName,   // this works too: &User::mName
            &User::setName,
            mNameChanged,
        };
    }

private:
    AString mName;
    emits<AString> mNameChanged;

    void setName(AString name) {
        // APropertyDef requires us to emit
        // changed signal if value is actually
        // changed
        if (mName == name) {
            return;
        }
        mName = std::move(name);
        emit mNameChanged(mName);
    }

    const AString& getName() const { return mName; }
};
// AUI_DOCS_CODE_END

class LogObserver : public AObject {
public:
    LogObserver() {
        ON_CALL(*this, log(testing::_)).WillByDefault([](const AString& msg) {});
    }
    MOCK_METHOD(void, log, (const AString& msg), ());
};
}   // namespace

TEST_F(PropertyDefTest, Declaration) {
    // APropertyDef behaves like a class/struct function member:
    {
        // AUI_DOCS_CODE_BEGIN
        User u;
        u.name() = "Hello"; // calls setName implicitly
        EXPECT_EQ(u.name(), "Hello");
        // AUI_DOCS_CODE_END
    }
    // !!! note
    //
    //     Properties defined with APropertyDef instead of AProperty impersonate themselves by trailing braces `()`. We
    //     can't get rid of them, as APropertyDef is defined thanks to member function. In comparison to `user->name`,
    //     think of `user->name()` as the same kind of property except defining custom behaviour via function, hence the
    //     braces `()`.
    //
    // For the rest, APropertyDef is identical to AProperty including seamless interaction:
    {
        LogObserver mock;
        EXPECT_CALL(mock, log(testing::_)).Times(2);
        // AUI_DOCS_CODE_BEGIN
        User u;
        AObject::connect(u.name().changed, AUI_SLOT(mock)::log); // HIDE
        u.name() = "Hello";
        u.name() += " world!";
        EXPECT_EQ(u.name(), "Hello world!");
        EXPECT_EQ(u.name()->length(), AString("Hello world!").length());
        // AUI_DOCS_CODE_END
        //
        // !!! note
        //
        //     In order to honor getters/setters, `APropertyDef` calls getter/setter instead of using `+=` on your
        //     property directly. Equivalent code will be:
        //     ```cpp
        //     u.setName(u.getName() + " world!")
        //     ```
        //
    }

    {
        // The implicit conversions work the same way as with AProperty:
        // AUI_DOCS_CODE_BEGIN
        auto doSomethingWithName = [](const AString& name) { EXPECT_EQ(name, "Hello"); };
        User u;
        u.name() = "Hello";
        doSomethingWithName(u.name());
        // AUI_DOCS_CODE_END

        // If it doesn't, simply put an asterisk:
        // AUI_DOCS_CODE_BEGIN
        doSomethingWithName(*u.name());
        //                 ^^^ HERE
        // AUI_DOCS_CODE_END
    }
}

TEST_F(PropertyDefTest, Observing_changes) { // HEADER_H1
    // All property types offer `.changed` field which is a signal reporting value changes. Let's make little observer
    // object for demonstration:
    {
        // AUI_DOCS_CODE_BEGIN
        class LogObserver : public AObject {
        public:
            void log(const AString& msg) {
                ALogger::info("LogObserver") << "Received value: " << msg;
            }
        };
        // AUI_DOCS_CODE_END
    }
    // The usage is close to `AProperty`:
    {
        // AUI_DOCS_CODE_BEGIN
        auto observer = _new<LogObserver>();
        auto u = _new<User>();
        u->name() = "Chloe";
        // ...
        AObject::connect(u->name().changed, AUI_SLOT(observer)::log);
        // AUI_DOCS_CODE_END
        EXPECT_CALL(*observer, log(AString("Marinette")));
        // AUI_DOCS_CODE_BEGIN
        u->name() = "Marinette";
        // AUI_DOCS_CODE_END
        // Code produces the following output:
        // ```
        // [07:58:59][][LogObserver][INFO]: Received value: Marinette
        // ```
    }
    {
        testing::InSequence s;
        //
        // Making connection to property directly instead of `.changed`:
        // AUI_DOCS_CODE_BEGIN
        auto observer = _new<LogObserver>();
        EXPECT_CALL(*observer, log(AString("Chloe"))).Times(1);       // HIDE
        EXPECT_CALL(*observer, log(AString("Marinette"))).Times(1);   // HIDE
        auto u = _new<User>();
        u->name() = "Chloe";
        // ...
        AObject::connect(u->name(), AUI_SLOT(observer)::log);
        // AUI_DOCS_CODE_END
        // Code above produces the following output:
        // ```
        // [07:58:59][][LogObserver][INFO]: Received value: Chloe
        // ```
        //
        // Subsequent changes to field would send updates as well:
        // AUI_DOCS_CODE_BEGIN
        u->name() = "Marinette";
        // AUI_DOCS_CODE_END
        // Assignment operation above makes an additional line to output:
        // ```
        // [07:58:59][][LogObserver][INFO]: Received value: Marinette
        // ```
        //
        // Whole program output when connecting to property directly:
        // ```
        // [07:58:59][][LogObserver][INFO]: Received value: Chloe
        // [07:58:59][][LogObserver][INFO]: Received value: Marinette
        // ```
    }
}
