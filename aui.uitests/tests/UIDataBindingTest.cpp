/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gmock/gmock.h>
#include <AUI/UITest.h>
#include "AUI/Platform/AWindow.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/Test/UI/By.h"
#include "AUI/View/ATextField.h"

class UIDataBindingTest : public testing::UITest {
public:

};


// AUI_DOCS_OUTPUT: doxygen/intermediate/properties.h
// @defgroup property_system Property System
// @ingroup core
// @brief Property System is a data binding technique based on @ref signal_slot "signal-slot system".
// @details
// AUI property system, a compiler-agnostic alternative to __property or [property]. Based on
// @ref signal_slot "signal-slot system" for platform-independent C++ development.
//
// AUI property system is relatively complex, as it involves a lot of features in a single place:
// 1. thread safe
// 2. many-to-many relationships between objects
// 3. optional data modification when passing values between objects (like STL projections)
// 4. emitter can be either signal or property
// 5. slot can be either lambda, method or property
// 6. for the latter case, system must set up backward connection as well (including projection support)
// 7. again, for the latter case, there's an option to make property-to-slot connection, where the "slot" is property's
//    assignment operation
// 8. 2 syntax variants: procedural (straightforward) and declarative
// 9. two property variants: simple field (AProperty) and custom getter/setter (APropertyDef)
// 10. some properties can be readonly
//
// Learning curve is relatively flat, so be sure to
// [ask questions and open issues](https://github.com/aui-framework/aui/issues) on our GitHub page.
//
// Main difference between basic value lying somewhere inside your class and a property is that the latter explicitly
// ties getter, setter and a signal reporting value changes. Property acts almost transparently, as if there's no
// property wrapper. This allows to read the intermediate value of a property and subscribe to its changes via a single
// \c connect call:
TEST_F(UIDataBindingTest, TextField1) {
    // AUI_DOCS_CODE_BEGIN
    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(User { .name = "Robert" });
    auto tf = _new<ATextField>();
    AObject::connect(user->name, tf->text());
    auto window = _new<AWindow>();
    window->setContents(Centered { tf });
    window->show();
    // AUI_DOCS_CODE_END

    // The code above generates a window with a text field:
    saveScreenshot("1");
    // ![text field](imgs/UIDataBindingTest.TextField1_1.png)
    //
    // A single call of AObject::connect:
    //

    // - Prefilled text field with the current `user->name` value (pre fire):
    // AUI_DOCS_CODE_BEGIN
    EXPECT_EQ(tf->text(), "Robert");
    // AUI_DOCS_CODE_END

    // - Connected `user->named.changed` to `tf` to notify the text field about changes of `user->name`:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Angela";           // changing user->name programmatically...
    EXPECT_EQ(tf->text(), "Angela"); // ...should reflect on the text field
    // AUI_DOCS_CODE_END
    saveScreenshot("2");
    // ![text field](imgs/UIDataBindingTest.TextField1_2.png)

    // - Connected `tf->text().changed` to notify the `user->name` property about changes in text field (i.e., if the
    // user typed another value to the text field):
    // ![text field](imgs/UIDataBindingTest.TextField1_3.png)
    tf->selectAll();
    By::value(tf).perform(type("Snezhana"));
    saveScreenshot("3");
    // AUI_DOCS_CODE_BEGIN
    // user typed "Snezhana", now let's check the value in user->name:
    EXPECT_EQ(user->name, "Snezhana");
    // AUI_DOCS_CODE_END

    //
    // This is a basic example of setting up property-to-property connection.
}


//
// # Declaring Properties
// There are two ways to define a property in AUI:
//
TEST_F(UIDataBindingTest, AProperty) { // HEADER
    // To declare a property, use AProperty template inside your model struct:
    // AUI_DOCS_CODE_BEGIN
    struct User {
        AProperty<AString> name;
        AProperty<AString> surname;
    };
    // AUI_DOCS_CODE_END

    // AProperty behaves like a class/struct data member:
    {
        // AUI_DOCS_CODE_BEGIN
        User u;
        u.name = "Hello";
        EXPECT_EQ(u.name, "Hello");
        // AUI_DOCS_CODE_END
    }

    // You can even perform binary operations on it seamlessly:
    {
        // AUI_DOCS_CODE_BEGIN
        User u;
        u.name = "Hello";
        u.name += " world!";
        EXPECT_EQ(u.name, "Hello world!");
        EXPECT_EQ(u.name->length(), AString("Hello world!").length());
        // AUI_DOCS_CODE_END
    }

    // In most cases, property is implicitly convertible to its underlying type:
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

        // or use `.raw` field (AProperty only):
        // AUI_DOCS_CODE_BEGIN
        doSomethingWithName(u.name.raw);
        // AUI_DOCS_CODE_END
    }
}


// # UI data building with let
// @note
// This is a comprehensive, straightforward way of setting up a connection. We are demonstrating it here so you can get
// deeper understanding on how connections are made and what does declarative way do under the hood. This way may be
// used in favour of declarative way if the latter not work for you.
//
// This approach allows more control over the binding process by using AObject::connect which is a procedural way of
// setting up connections. As a downside, it requires "let" syntax clause which may seem as overkill for such a simple
// operation.
TEST_F(UIDataBindingTest, Label_via_let) { // HEADER
    // Use \c let expression to connect the model's username property to the label's @ref ALabel::text "text()"
    // property.
    // AUI_DOCS_CODE_BEGIN
    using namespace declarative;

    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            _<ALabel> label;
            setContents(Centered {
                label = _new<ALabel>() let {
                  // Data goes from left to right in the first place
                  // (i.e., user->name current value overrides it->text())
                  // if view's property gets changed (i.e., by user),
                  // these changes reflect on model as well
                  // because the connection is bidirectional (left to right
                  // is prioritized).
                  AObject::connect(user->name, it->text());
                  //                ->  ->  ->  ->  ->
                },
            });

            // Notice that label already displays a value stored in User.
            EXPECT_EQ(user->name, "Roza");
            EXPECT_EQ(label->text(), "Roza");
        }
    };
    _new<MyWindow>(user)->show();
    // AUI_DOCS_CODE_END

    auto label = _cast<ALabel>(By::type<ALabel>().one());

    //
    // Let's change the name:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Vasil";

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(label->text(), "Vasil");
    // AUI_DOCS_CODE_END

    // By simply performing assignment operation on \c name field of \c User struct we changed ALabel display text.
    // Magic, huh?

    user->name = "World";
    EXPECT_EQ(label->text(), "World");
}


TEST_F(UIDataBindingTest, Label_via_let_assignment) { // HEADER
    // We can use ".assignment()" syntax to make a property-to-setter connection instead of property-to-property.
    using namespace declarative;

    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            _<ALabel> label;
            setContents(Centered {
                // AUI_DOCS_CODE_BEGIN
                label = _new<ALabel>() let {
                  // Data goes from left to right in the first place
                  // (i.e., user->name current value overrides it->text())
                  // if view's property gets changed (i.e., by user),
                  // these changes DOES NOT reflect on model
                  // as we requested assignment() here
                  AObject::connect(user->name, it->text().assignment());
                  //                ->  ->  ->  ->  ->
                  // in other words, this connection essentially the same
                  // as
                  // AObject::connect(user->name, slot(it)::setText);
                },
                // AUI_DOCS_CODE_END
            });

            // Notice that label already displays a value stored in User.
            // AUI_DOCS_CODE_BEGIN
            EXPECT_EQ(user->name, "Roza");
            EXPECT_EQ(label->text(), "Roza");
            // AUI_DOCS_CODE_END
        }
    };
    _new<MyWindow>(user)->show();

    auto label = _cast<ALabel>(By::type<ALabel>().one());

    //
    // Let's change the name:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Vasil";

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(label->text(), "Vasil");
    // AUI_DOCS_CODE_END

    // Unlike property-to-property connection, property-to-slot is one-directional connection. So, if we try to change
    // labels text directly, it would not reflect on the model.
    // AUI_DOCS_CODE_BEGIN
    label->text() = "Sanya";

    EXPECT_EQ(label->text(), "Sanya");
    EXPECT_EQ(user->name, "Vasil"); // still
    // AUI_DOCS_CODE_END
}

/*
TEST_F(UIDataBindingTest, Label_via_declarative) { // HEADER
    // As said earlier, \c let syntax is a little bit clunky and requires extra boilerplate code to set up.
    //
    // Here's where declarative syntax comes into play. The example below is essentially the same as "Label via
    // declarative" but uses declarative connection set up syntax.
    //
    // Use \c && expression to connect the model's username property to the label's @ref ALabel::text "text()"
    // property.
    // AUI_DOCS_CODE_BEGIN
    using namespace declarative;
    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            _<ALabel> label;
            setContents(Centered {
                label = _new<ALabel>() && connection(&ALabel::text, std::identity{},)
            });

            // Both label and user should hold name Roza.
            EXPECT_EQ(user->name, "Roza");
            EXPECT_EQ(label->text(), "Roza");
        }
    };
    _new<MyWindow>(user)->show();
    // AUI_DOCS_CODE_END

    auto label = _cast<ALabel>(By::type<ALabel>().one());

    // Notice that label already displays a value stored in User.
    //
    // Let's change the name:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Vasil";

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(label->text(), "Vasil");
    // AUI_DOCS_CODE_END

    // By simply performing assignment operation on \c name field of \c User struct we changed ALabel display text.
    // Magic, huh?

    user->name = "World";
    EXPECT_EQ(label->text(), "World");
}
*/

/*
TEST_F(UIDataBindingTest, LabelViaLetAssignment) {

    struct Model {
        AProperty<AString> username;
    } model;

    mWindow->setContents(Centered {
      Label {} let {
          AObject::connect(model.username, it->text().assignment());
      },
    });

    auto label = _cast<ALabel>(By::type<_<ALabel>>().one());
    EXPECT_EQ(label->text(), "");

    model.username = "Hello";
    EXPECT_EQ(label->text(), "Hello");

    model.username = "World";
    EXPECT_EQ(label->text(), "World");
}

TEST_F(UIDataBindingTest, LabelViaLetAssignmentProjection) {
    // by using ".assignment()" syntax, we can now use "projection" feature to modify the contents of string displayed
    // by label yet keeping original value in Model.

    struct Model {
        AProperty<AString> username;
    } model;

    mWindow->setContents(Centered {
      Label {} let {
          // [](const AString& s) { return s.uppercase(); } is same as &AString::uppercase
          // we are using the latter here because it's shorter
          AObject::connect(model.username, it->text().assignment(), &AString::uppercase);
      },
    });

    auto label = _cast<ALabel>(By::type<_<ALabel>>().one());
    EXPECT_EQ(label->text(), "");

    model.username = "Hello";
    EXPECT_EQ(label->text(), "HELLO");

    model.username = "World";
    EXPECT_EQ(label->text(), "WORLD");
}

/*


TEST_F(UIDataBindingTest, CheckBox) {
    struct Model {
        AProperty<bool> checkbox;
    } model;

    mWindow->setContents(Centered {
      CheckBox { } && model.checkbox
    });
}
*/
