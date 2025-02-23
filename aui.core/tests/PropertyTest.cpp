// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

class PropertyTest: public testing::Test {
public:
    template<typename... Args>
    static auto& connections(ASignal<Args...>& signal) {
        return signal.mOutgoingConnections;
    }

    static auto& connections(AObject& object) {
        return object.mIngoingConnections;
    }
};

namespace {
// AUI_DOCS_OUTPUT: doxygen/intermediate/property.h
// @class AProperty
// # Declaration
// To declare a property inside your data model, use AProperty template:
// AUI_DOCS_CODE_BEGIN
struct User {
    AProperty<AString> name;
    AProperty<AString> surname;
};
// AUI_DOCS_CODE_END

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

// `AProperty<T>` is a container holding an instance of `T`. You can assign a value to it with `operator=` and read
// value with `value()` method or implicit conversion `operator T()`.

TEST_F(PropertyTest, Declaration) {
    //
    // AProperty behaves like a class/struct data member:
    {
        // AUI_DOCS_CODE_BEGIN
        User u;
        u.name = "Hello";
        EXPECT_EQ(u.name, "Hello");
        // AUI_DOCS_CODE_END
    }

    // Non-const operators have @ref PropertyTest_Write_operators "side effects"; const operators don't, so you can
    // perform seamlessly:
    {
        LogObserver mock;
        EXPECT_CALL(mock, log(testing::_)).Times(1);
        // AUI_DOCS_CODE_BEGIN
        User u;
        AObject::connect(u.name.changed, slot(mock)::log); // HIDE
        u.name = "Hello";
        AString helloWorld = u.name + " world";
        EXPECT_EQ(helloWorld, "Hello world");
        EXPECT_EQ(u.name->length(), AString("Hello").length()); // HIDE
        // AUI_DOCS_CODE_END
    }

    // In most cases, property is implicitly convertible to its underlying type (const only):
    {
        // AUI_DOCS_CODE_BEGIN
        auto doSomethingWithName = [](const AString& name) { EXPECT_EQ(name, "Hello"); };
        User u;
        u.name = "Hello";
        doSomethingWithName(u.name);
        // AUI_DOCS_CODE_END


        // If it doesn't, simply put an asterisk:
        // AUI_DOCS_CODE_BEGIN
        doSomethingWithName(*u.name);
        //                 ^^^ HERE
        // AUI_DOCS_CODE_END
    }
}

TEST_F(PropertyTest, Observing_changes) { // HEADER_H1
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
    {
        //
        // Example usage:
        // AUI_DOCS_CODE_BEGIN
        auto observer = _new<LogObserver>();
        auto u = aui::ptr::manage(new User { .name = "Chloe" });
        AObject::connect(u->name.changed, slot(observer)::log);
        // AUI_DOCS_CODE_END
        EXPECT_CALL(*observer, log(AString("Marinette")));
        //
        // At the moment, the program prints nothing. When we change the property:
        // AUI_DOCS_CODE_BEGIN
        u->name = "Marinette";
        // AUI_DOCS_CODE_END
        // Code produces the following output:
        // @code
        // [07:58:59][][LogObserver][INFO]: Received value: Marinette
        // @endcode
    }
    {
        testing::InSequence s;
        //
        // As you can see, observer received the update. But, for example, if we would like to display the value via
        // label, the label wouldn't display the current value until the next update. We want the label to display
        // *current* value without requiring an update. To do this, connect to the property directly, without explicitly
        // asking for `changed`:
        // AUI_DOCS_CODE_BEGIN
        auto observer = _new<LogObserver>();
        auto u = aui::ptr::manage(new User { .name = "Chloe" });

        EXPECT_CALL(*observer, log(AString("Chloe"))).Times(1);
        AObject::connect(u->name, slot(observer)::log);
        // AUI_DOCS_CODE_END
        // Code above produces the following output:
        // @code
        // [07:58:59][][LogObserver][INFO]: Received value: Chloe
        // @endcode
        // As you can see, observer receives the value without making updates to the value. The call of
        // `LogObserver::log` is made by `AObject::connect` itself. In this document, we will call this behaviour as
        // "pre-fire".
        //
        // Subsequent changes to field would send updates as well:
        // AUI_DOCS_CODE_BEGIN
        EXPECT_CALL(*observer, log(AString("Marinette"))).Times(1);
        u->name = "Marinette";
        // AUI_DOCS_CODE_END
        // Assignment operation above makes an additional line to output:
        // @code
        // [07:58:59][][LogObserver][INFO]: Received value: Marinette
        // @endcode
        //
        // Whole program output when connecting to property directly:
        // @code
        // [07:58:59][][LogObserver][INFO]: Received value: Chloe
        // [07:58:59][][LogObserver][INFO]: Received value: Marinette
        // @endcode
    }
}

TEST_F(PropertyTest, Copy_constructing_AProperty) { // HEADER_H1
    {
        User user { .name = "Hello" };
        auto copy = user;
        EXPECT_EQ(copy.name, "Hello");
        EXPECT_EQ(copy.surname, "");
    }

    // Copying `AProperty` is considered as a valid operation as it's a data holder. However, it's worth to note
    // that `AProperty` copies it's underlying data field only, the **signal-slot relations are not borrowed**.
    {
        // AUI_DOCS_CODE_BEGIN
        auto observer = _new<LogObserver>();
        auto original = aui::ptr::manage(new User { .name = "Chloe" });

        EXPECT_CALL(*observer, log(AString("Chloe"))).Times(1);
        AObject::connect(original->name, slot(observer)::log);
        // AUI_DOCS_CODE_END
        // This part is similar to previous examples, nothing new. Let's introduce a copy:
        // AUI_DOCS_CODE_BEGIN
        auto copy = _new<User>(*original);
        EXPECT_EQ(copy->name, "Chloe"); // copied name
        // AUI_DOCS_CODE_END
        // Now, let's change `origin->name` and check that observer received an update, but value in the `copy`
        // remains:
        // AUI_DOCS_CODE_BEGIN
        EXPECT_CALL(*observer, log(AString("Marinette"))).Times(1);
        original->name = "Marinette";
        EXPECT_EQ(copy->name, "Chloe"); // still
        // AUI_DOCS_CODE_END
        // In this example, observer is aware of changes `"Chloe"` -> `"Marinette"`. The copy is not aware because
        // it is a **copy**. If we try to change the `copy`'s name:
        // AUI_DOCS_CODE_BEGIN
        copy->name = "Adrien";
        // AUI_DOCS_CODE_END
        // The observer is not aware about changes in `copy`. In fact. `copy->name` has zero connections.
        EXPECT_EQ(connections(original->name.changed).size(), 1);
        EXPECT_EQ(connections(copy->name.changed).size(), 0);
        EXPECT_EQ(connections(*observer).size(), 1);
    }
}

TEST_F(PropertyTest, Copy_assigning_AProperty) { // HEADER_H1
    {
        User user1{ .name = "Hello" };
        EXPECT_EQ(user1.name, "Hello");
        User user2;
        EXPECT_EQ(user2.name, "");
        user2 = user1;
        EXPECT_EQ(user2.name, "Hello");
    }
    // The situation with copy assigning `auto copy = _new<User>(); *copy = *original;` is similar to copy
    // construction `auto copy = _new<User>(*original);`, except that we are copying to some pre-existing
    // data structure that potentially have signal-slot relations already. So, not only **connections should be kept
    // as is** but a notification for copy destination's observers is needed.
    //
    // As with copy construction, copy operation of `AProperty` does not affect signal-slot relations. Moreover,
    // it notifies the observers.
    // AUI_DOCS_CODE_BEGIN
    auto observer = _new<LogObserver>();
    auto original = aui::ptr::manage(new User { .name = "Chloe" });

    EXPECT_CALL(*observer, log(AString("Chloe"))).Times(1);
    AObject::connect(original->name, slot(observer)::log);
    // AUI_DOCS_CODE_END
    // This part is similar to previous examples, nothing new. Let's perform copy-assignment:
    // AUI_DOCS_CODE_BEGIN
    EXPECT_CALL(*observer, log(AString("Marinette"))).Times(1);
    User copy { .name = "Marinette" };
    *original = copy;
    // AUI_DOCS_CODE_END
    // See, not only the connection remains, but it also receives notification about the change.
    EXPECT_EQ(connections(original->name.changed).size(), 1);
    EXPECT_EQ(connections(*observer).size(), 1);
}

TEST_F(PropertyTest, Moving_AProperty) { // HEADER_H1
    {
        User user { .name = "Hello" };
        auto copy = std::move(user);
        EXPECT_EQ(user.name, "");
        EXPECT_EQ(user.surname, "");
        EXPECT_EQ(copy.name, "Hello");
        EXPECT_EQ(copy.surname, "");
    }
    {
        User user1{ .name = "Hello" };
        EXPECT_EQ(user1.name, "Hello");
        User user2;
        EXPECT_EQ(user2.name, "");
        user2 = std::move(user1);
        EXPECT_EQ(user1.name, "");
        EXPECT_EQ(user2.name, "Hello");
    }
    // Similary to copy, AProperty is both move assignable and constructible except that underlying value is moved
    // instead of copying. Also, the observers of the source object receive notification that the value was emptied. The
    // **signal-slot relations are left unchanged.**
    {
        // AUI_DOCS_CODE_BEGIN
        auto observer = _new<LogObserver>();
        auto original = aui::ptr::manage(new User { .name = "Chloe" });

        EXPECT_CALL(*observer, log(AString("Chloe"))).Times(1);
        AObject::connect(original->name, slot(observer)::log);
        // AUI_DOCS_CODE_END
        // This part is similar to previous examples, nothing new. Let's introduce a move:
        // AUI_DOCS_CODE_BEGIN
        // by move operation, we've affected the source, hence the
        // empty string notification
        EXPECT_CALL(*observer, log(AString(""))).Times(1);
        auto moved = _new<User>(std::move(*original));

        EXPECT_EQ(original->name, ""); // empty
        EXPECT_EQ(moved->name, "Chloe"); // moved name
        // AUI_DOCS_CODE_END
        // Now, let's change `origin->name` and check that observer received an update, but value in the `moved`
        // remains:
        // AUI_DOCS_CODE_BEGIN
        EXPECT_CALL(*observer, log(AString("Marinette"))).Times(1);
        original->name = "Marinette";
        EXPECT_EQ(moved->name, "Chloe"); // still
        // AUI_DOCS_CODE_END
        // In this example, observer is aware of changes `"Chloe"` -> `""` -> `"Marinette"`. The `moved` is not aware.
        // If we try to change the `moved`'s name:
        // AUI_DOCS_CODE_BEGIN
        moved->name = "Adrien";
        // AUI_DOCS_CODE_END
        // The observer is not aware about changes in `moved`. In fact. `moved->name` has zero connections.
        EXPECT_EQ(connections(original->name.changed).size(), 1);
        EXPECT_EQ(connections(moved->name.changed).size(), 0);
        EXPECT_EQ(connections(*observer).size(), 1);
    }

    // Move assignment work in a similar way to copy assignment:
    // AUI_DOCS_CODE_BEGIN
    auto observer = _new<LogObserver>();
    auto original = aui::ptr::manage(new User { .name = "Chloe" });

    EXPECT_CALL(*observer, log(AString("Chloe"))).Times(1);
    AObject::connect(original->name, slot(observer)::log);
    // AUI_DOCS_CODE_END
    // This part is similar to previous examples, nothing new. Let's perform move-assignment:
    // AUI_DOCS_CODE_BEGIN
    EXPECT_CALL(*observer, log(AString("Marinette"))).Times(1);
    User copy { .name = "Marinette" };
    *original = std::move(copy);
    // AUI_DOCS_CODE_END
    // See, not only the connection remains, but it also receives notification about the change.
    EXPECT_EQ(copy.name, "");
    EXPECT_EQ(connections(original->name.changed).size(), 1);
    EXPECT_EQ(connections(*observer).size(), 1);
}

// # Non-const operators {#PropertyTest_Write_operators}
// Refer to @ref aui::PropertyModifier.