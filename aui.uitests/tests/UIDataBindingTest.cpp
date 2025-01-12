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

#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/algorithm/find.hpp>

#include <gmock/gmock.h>

#include <AUI/UITest.h>
#include "AUI/Platform/AWindow.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/Test/UI/By.h"
#include "AUI/View/ATextField.h"
#include "AUI/View/ADropdownList.h"
#include "AUI/Model/AListModel.h"
#include "AUI/View/ANumberPicker.h"

class UIDataBindingTest : public testing::UITest {
public:

};


// AUI_DOCS_OUTPUT: doxygen/intermediate/properties.h
// @defgroup property_system Property System
// @ingroup core
// @brief Property System is a data binding mechanism based on @ref signal_slot "signal-slot system".
// @details
// AUI property system, a compiler-agnostic alternative to __property or [property]. Based on
// @ref signal_slot "signal-slot system" for platform-independent C++ development. Unlike Qt, AUI's properties don't
// involve external tools (like `moc`). They are written in pure C++.
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
// 9. three property variants: simple field (AProperty), custom getter/setter (APropertyDef) and custom evaluation
//    (APropertyPrecomputed)
// 10. some properties can be readonly
// 11. propagating strong types' traits on views
//
// Learning curve is relatively flat, so be sure to
// [ask questions and open issues](https://github.com/aui-framework/aui/issues) on our GitHub page.
//
// Main difference between basic value lying somewhere inside your class and a property is that the latter explicitly
// ties getter, setter and a signal reporting value changes. Property acts almost transparently, as if there's no
// extra wrapper around your data. This allows to work with properties in the same way as with their underlying values.
// You can read the intermediate value of a property and subscribe to its changes via a single
// \c connect call. Also, when connecting property to property, it is possible to make them observe changes of each
// other bia \c biConnect call:
TEST_F(UIDataBindingTest, TextField1) {
    // AUI_DOCS_CODE_BEGIN
    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(new User { .name = "Robert" });
    auto tf = _new<ATextField>();
    AObject::biConnect(user->name, tf->text());
    auto window = _new<AWindow>();
    window->setContents(Centered { tf });
    window->show();
    // AUI_DOCS_CODE_END
    // Or simpler:
    if constexpr (false) {
        // AUI_DOCS_CODE_BEGIN
        // ...
        window->setContents(Centered {
          tf && user->name,
        });
        // ...
        // AUI_DOCS_CODE_END
    }

    window->setScalingParams({ .scalingFactor = 2.f });

    // The code above generates a window with a text field:
    saveScreenshot("1");
    // ![text field](imgs/UIDataBindingTest.TextField1_1.png)
    //
    // A single call of `biConnect`:
    //

    // - Prefills text field with the current `user->name` value (pre fire):
    // AUI_DOCS_CODE_BEGIN
    EXPECT_EQ(tf->text(), "Robert");
    // AUI_DOCS_CODE_END

    // - Connects `user->named.changed` to `tf` to notify the text field about changes of `user->name`:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Angela";           // changing user->name programmatically...
    EXPECT_EQ(tf->text(), "Angela"); // ...should reflect on the text field
    // AUI_DOCS_CODE_END
    saveScreenshot("2");
    // ![text field](imgs/UIDataBindingTest.TextField1_2.png)

    // - Connects `tf->text().changed` to notify the `user->name` property about changes in text field (i.e., if the
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
    // This is basic example of setting up property-to-property connection.
}

class LogObserver : public AObject {
public:
    LogObserver() {
        ON_CALL(*this, log(testing::_)).WillByDefault([](const AString& msg) {
//            ALogger::info("LogObserver") << "Received value: " << msg;
        });
    }
    MOCK_METHOD(void, log, (const AString& msg), ());
};

//
// # Declaring Properties
// There are three ways to define a property in AUI:
//
TEST_F(UIDataBindingTest, AProperty) { // HEADER
    // To declare a property inside your data model, use AProperty template:
    // AUI_DOCS_CODE_BEGIN
    struct User {
        AProperty<AString> name;
        AProperty<AString> surname;
    };
    // AUI_DOCS_CODE_END
    // `AProperty<T>` is a container holding an instance of `T`. You can assign a value to it with `operator=` and read
    // value with `value()` method or implicit conversion `operator T()`.

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
    }

    //
    // ### Observing changes
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
        EXPECT_CALL(*observer, log(AString("Chloe"))).Times(1);     // HIDE
        EXPECT_CALL(*observer, log(AString("Marinette"))).Times(1); // HIDE
        auto u = aui::ptr::manage(new User { .name = "Chloe" });
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

TEST_F(UIDataBindingTest, APropertyDef) { // HEADER
    // You can use this way if you are required to define custom behaviour on getter/setter. As a downside, you have to
    // write extra boilerplate code: define property, data field, signal, getter and setter checking equality. Also,
    // APropertyDef requires the class to derive `AObject`. Most of AView's properties are defined this way.
    //
    // To declare a property with custom getter/setter, use APropertyDef template. APropertyDef-based property is
    // defined by const member function as follows:
    // AUI_DOCS_CODE_BEGIN
    class User: public AObject {
    public:
        auto name() const {
            return APropertyDef {
                this,
                &User::getName, // this works too: &User::mName
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

        const AString& getName() const {
            return mName;
        }
    };
    // AUI_DOCS_CODE_END

    // APropertyDef behaves like a class/struct function member:
    {
        // AUI_DOCS_CODE_BEGIN
        User u;
        u.name() = "Hello";
        EXPECT_EQ(u.name(), "Hello");
        // AUI_DOCS_CODE_END
    }
    // @note
    // Properties defined with APropertyDef instead of AProperty impersonate themselves by trailing braces `()`. We
    // can't get rid of them, as APropertyDef is defined thanks to member function. In comparison to `user->name`, think
    // of `user->name()` as the same kind of property except defining custom behaviour via function, hence the braces
    // `()`.
    //
    // For the rest, APropertyDef is identical to AProperty including seamless interaction:
    {
        // AUI_DOCS_CODE_BEGIN
        User u;
        u.name() = "Hello";
        u.name() += " world!";
        EXPECT_EQ(u.name(), "Hello world!");
        EXPECT_EQ(u.name()->length(), AString("Hello world!").length());
        // AUI_DOCS_CODE_END
        //
        // @note
        // In order to honor getters/setters, `APropertyDef` calls getter/setter instead of using `+=` on your property
        // directly. Equivalent code will be:
        // @code{cpp}
        // u.setName(u.getName() + " world!")
        // @endcode
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


        // AUI_DOCS_CODE_BEGIN
        doSomethingWithName(*u.name());
        // AUI_DOCS_CODE_END
    }

    //
    // ### Observing changes
    // Close to `AProperty`:
    {
        // AUI_DOCS_CODE_BEGIN
        auto observer = _new<LogObserver>();
        auto u = _new<User>();
        u->name() = "Chloe";
        // ...
        AObject::connect(u->name().changed, slot(observer)::log);
        // AUI_DOCS_CODE_END
        EXPECT_CALL(*observer, log(AString("Marinette")));
        // AUI_DOCS_CODE_BEGIN
        u->name() = "Marinette";
        // AUI_DOCS_CODE_END
        // Code produces the following output:
        // @code
        // [07:58:59][][LogObserver][INFO]: Received value: Marinette
        // @endcode
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
        AObject::connect(u->name(), slot(observer)::log);
        // AUI_DOCS_CODE_END
        // Code above produces the following output:
        // @code
        // [07:58:59][][LogObserver][INFO]: Received value: Chloe
        // @endcode
        //
        // Subsequent changes to field would send updates as well:
        // AUI_DOCS_CODE_BEGIN
        u->name() = "Marinette";
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

TEST_F(UIDataBindingTest, APropertyPrecomputed) { // HEADER
    testing::InSequence s;
    // Despite properties offer @ref Label_via_declarative_projection "projection methods", you might want to track
    // and process values of several properties.
    //
    // `APropertyPrecomputed<T>` is a readonly property similar to `AProperty<T>`. It holds an instance of `T` as well.
    // Its value is determined by the C++ function specified in its constructor, typically a C++ lambda expression.
    //
    // It's convenient to access values from another properties inside the expression. The properties accessed during
    // invocation of the expression are tracked behind the scenes so they become dependencies of `APropertyPrecomputed`
    // automatically. If one of the tracked properties fires `changed` signal, `APropertyPrecomputed` invalidates its
    // `T`. `APropertyPrecomputed` follows @ref aui::lazy "lazy semantics" so the expression is re-evaluated and the new
    // result is applied to `APropertyPrecomputed` as soon as the latter is accessed for the next time.
    //
    // In other words, it allows to specify relationships between different object properties and reactively update
    // `APropertyPrecomputed` value whenever its dependencies change. `APropertyPrecomputed<T>` is somewhat similar to
    // [Qt Bindable Properties](https://doc.qt.io/qt-6/bindableproperties.html).
    //
    // `APropertyPrecomputed` is a readonly property, hence you can't update its value with assignment. You can get its
    // value with `value()` method or implicit conversion `operator T()` as with other properties.
    //
    // AUI_DOCS_CODE_BEGIN
    struct User {
        AProperty<AString> name;
        AProperty<AString> surname;
        APropertyPrecomputed<AString> fullName = [&] { return "{} {}"_format(name, surname); };
    };

    auto u = aui::ptr::manage(new User {
        .name = "Emma",
        .surname = "Watson",
    });

    auto observer = _new<LogObserver>();
    EXPECT_CALL(*observer, log(AString("Emma Watson"))).Times(1); // HIDE
    EXPECT_CALL(*observer, log(AString("Emma Stone"))).Times(1); // HIDE
    AObject::connect(u->fullName, slot(observer)::log);
    // AUI_DOCS_CODE_END
    EXPECT_EQ(u->fullName, "Emma Watson");
    //
    // The example above prints "Emma Watson". If we try to update one of dependencies of `APropertyPrecomputed` (i.e.,
    // `name` or `surname`), `APropertyPrecomputed` responds immediately:

    // AUI_DOCS_CODE_BEGIN
    u->surname = "Stone";
    // AUI_DOCS_CODE_END
    //
    // The example above prints "Emma Stone".
    EXPECT_EQ(u->fullName, "Emma Stone");

    //
    // ### Observing changes
    // Similar to `AProperty`.
}

TEST_F(UIDataBindingTest, APropertyPrecomputed_Complex) {
    //
    // ### Valid Expressions
    // Any C++ callable evaluating to `T` can be used as an expression for `APropertyPrecomputed<T>`. However, to
    // formulate correct expression, some rules must be satisfied.
    //
    // Dependency tracking only works on other properties. It is the developer's responsibility to ensure all values
    // referenced in the expression are properties, or, at least, non-property values that wouldn't change or whose
    // changes are not interesting. You definitely can use branching inside the expression, but you must be confident
    // about what are you doing. Generally speaking, use as trivial expressions as possible.
    //
    // AUI_DOCS_CODE_BEGIN
    struct User {
        AProperty<AString> name;
        AProperty<AString> surname;
        APropertyPrecomputed<AString> fullName = [&]() -> AString {
            if (name->empty()) {
                return "-";
            }
            if (surname->empty()) {
                return "-";
            }
            return "{} {}"_format(name, surname);
        };
    };
    // AUI_DOCS_CODE_END
    //
    // In this expression, we have a fast path return if `name` is empty.
    // AUI_DOCS_CODE_BEGIN
    User u = {
        .name = "Emma",
        .surname = "Watson",
    };
    // trivial: we've accessed all referenced properties
    EXPECT_EQ(u.fullName, "Emma Watson");
    // AUI_DOCS_CODE_END

    EXPECT_EQ(u.name.changed.mOutgoingConnections.size(), 1);
    EXPECT_EQ(u.surname.changed.mOutgoingConnections.size(), 1);

    // As soon as we set `name` to `""`, we don't access `surname`. If we try to trigger the fast path return:
    // AUI_DOCS_CODE_BEGIN
    u.name = "";
    // AUI_DOCS_CODE_END
    EXPECT_EQ(u.fullName, "-");
    EXPECT_EQ(u.name.changed.mOutgoingConnections.size(), 1);
    EXPECT_EQ(u.surname.changed.mOutgoingConnections.size(), 0);
    // `surname` can't trigger re-evaluation anyhow. Re-evaluation can be triggered by `name` only. So, at the moment,
    // we are interested in `name` changes only.
    //
    // `APropertyPrecomputed` might evaluate its expression several times during its lifetime. The developer must make
    // sure that all objects referenced in the expression live longer than `APropertyPrecomputed`.
    //
    // The expression should not read from the property it's a binding for. Otherwise, there's an infinite evaluation
    // loop.
}


// # UI data binding with let
// @note
// This is a comprehensive, straightforward way of setting up a connection. We are demonstrating it here so you can get
// deeper understanding on how connections are made and what does declarative way do under the hood. This way may be
// used in favour of declarative way if the latter not work for you. For declarative way, go to
// @ref "UI_declarative_data_binding".
//
// This approach allows more control over the binding process by using `AObject::connect`/`AObject::biConnect` which is
// a procedural way of setting up connections. As a downside, it requires "let" syntax clause which may seem as overkill
// for such a simple operation.
TEST_F(UIDataBindingTest, Label_via_let) { // HEADER
    // Use \c let expression to connect the model's username property to the label's @ref ALabel::text "text()"
    // property.
    // AUI_DOCS_CODE_BEGIN
    using namespace declarative;

    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            setContents(Centered {
                _new<ALabel>() let {
                  // Data goes from left to right:
                  // current value (pre fire) or changed event
                  // goes to assignment operation of it->text()
                  AObject::connect(user->name, it->text());
                  //                ->  ->  ->  ->  ->
                  // in other words, this connection is essentially the
                  // same as
                  // AObject::connect(user->name, slot(it)::setText);
                  //
                  // if you want user->name to be aware or it->text()
                  // changes (i.e., if it were an editable view
                  // like ATextField) use AObject::biConnect instead
                  // (see "Bidirectional connection" sample).
                },
            });
        }
    };
    _new<MyWindow>(user)->show();
    // AUI_DOCS_CODE_END
    //
    // This gives the following result:
    // ![text](imgs/UIDataBindingTest.Label_via_declarative_1.png)
    // Note that label already displays the value stored in User.

    auto label = _cast<ALabel>(By::type<ALabel>().one());

    EXPECT_EQ(user->name, "Roza");
    EXPECT_EQ(label->text(), "Roza");

    //
    // Let's change the name:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Vasil";
    // AUI_DOCS_CODE_END
    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(label->text(), "Vasil");
    // ![text](imgs/UIDataBindingTest.Label_via_declarative_2.png)
    //
    // By simply performing assignment on `user` we changed ALabel display text.
    // Magic, huh?

    user->name = "World";
    EXPECT_EQ(label->text(), "World");
}

TEST_F(UIDataBindingTest, Label_via_let_projection) { // HEADER
    // It's fairly easy to define a projection because one-sided connection requires exactly one projection, obviously.
    using namespace declarative;

    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            setContents(Centered {
              // AUI_DOCS_CODE_BEGIN
              _new<ALabel>() let {
                  // Data goes from left to right:
                  // current value (pre fire) or changed event
                  // goes through projection (&AString::uppercase) first
                  // then it goes to assignment operation of it->text()
                  // property.
                  AObject::connect(user->name.readProjected(&AString::uppercase), it->text());
                  //                ->  ->  ->  ->  ->  ->  ->  ->  ->  ->  ->  ->
                  // in other words, this connection is essentially the same as
                  // AObject::connect(user->name.projected(&AString::uppercase), slot(it)::setText);

                  // if view's property gets changed (i.e., by user or by occasional
                  // ALabel::setText), these changes DO NOT reflect on model
                  // as we requested connect() here instead of biConnect().
              },
              // AUI_DOCS_CODE_END
            });
        }
    };
    _new<MyWindow>(user)->show();

    //
    // This gives the following result:
    // ![text](imgs/UIDataBindingTest.Label_via_declarative_projection_1.png)
    // Note that the label already displays the **projected** value stored in User.

    auto label = _cast<ALabel>(By::type<ALabel>().one());

    //
    // Let's change the name:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Vasil";
    // AUI_DOCS_CODE_END

    // ![text](imgs/UIDataBindingTest.Label_via_declarative_projection_2.png)
    //
    // This way, we've set up data binding with projection.

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(label->text(), "VASIL"); // uppercased by projection!
}

TEST_F(UIDataBindingTest, Bidirectional_connection) { // HEADER
    // In previous examples, we've used `AObject::connect` to make one directional (one sided) connection. This is
    // perfectly enough for ALabel because it cannot be changed by user.
    //
    // In some cases, you might want to use property-to-property as it's bidirectional. It's used for populating view
    // from model and obtaining data from view back to the model.
    //
    // For this example, let's use ATextField instead of ALabel as it's an editable view. In this case, we'd want to use
    // `AObject::biConnect` because we do want `user->name` to be aware of changes of the view.

    using namespace declarative;

    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            setContents(Centered {
                // AUI_DOCS_CODE_BEGIN
                _new<ATextField>() let {
                  // Data goes from left to right in the first place
                  // (i.e., user->name current value overrides it->text())
                  // if view's property gets changed (i.e., by user),
                  // these changes reflect on model
                  // as we requested biConnect here

                  //                -> value + changes ->
                  AObject::biConnect(user->name, it->text());
                  //                <-  changes only   <-
                },
                // AUI_DOCS_CODE_END
            });
        }
    };
    _new<MyWindow>(user)->show();
    //
    // This gives the following result:
    // ![text](imgs/UIDataBindingTest.Declarative_bidirectional_connection_1.png)

    auto tf = _cast<ATextField>(By::type<ATextField>().one());

    //
    // Let's change the name programmatically:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Vasil";
    // AUI_DOCS_CODE_END
    //
    // ATextField will respond:
    // ![text](imgs/UIDataBindingTest.Declarative_bidirectional_connection_2.png)

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(tf->text(), "Vasil");

    //
    // If the user changes the value from UI, these changes will reflect on `user->model` as well:
    tf->selectAll();
    By::value(tf).perform(type("Changed from UI"));
    // ![text](imgs/UIDataBindingTest.Declarative_bidirectional_connection_3.png)
    // AUI_DOCS_CODE_BEGIN
    EXPECT_EQ(user->name, "Changed from UI");
    // AUI_DOCS_CODE_END
    //
    // This way we've set up bidirectional projection via `AObject::biConnect` which makes `user->name` aware of UI
    // changes.
}

enum class Gender {
    MALE,
    FEMALE,
    OTHER,
};
AUI_ENUM_VALUES(Gender,
                Gender::MALE,
                Gender::FEMALE,
                Gender::OTHER)

TEST_F(UIDataBindingTest, Bidirectional_projection) { // HEADER
    using namespace declarative;
    // Bidirectional connection updates values in both directions, hence it requires the projection to work in both
    // sides as well.
    //
    // It is the case for ADropdownList with enums. ADropdownList works with string list model and indices. It does not
    // know anything about underlying values.
    //
    // For example, define enum with @ref AUI_ENUM_VALUES "AUI_ENUM_VALUES" and model:
    //
    // @code{cpp}
    // enum class Gender {
    //     MALE,
    //     FEMALE,
    //     OTHER,
    // };
    // AUI_ENUM_VALUES(Gender,
    //                 Gender::MALE,
    //                 Gender::FEMALE,
    //                 Gender::OTHER)
    // @endcode
    // AUI_DOCS_CODE_BEGIN
    struct User {
        AProperty<Gender> gender;
        // we've omitted other fields for sake of simplicity
    };
    // AUI_DOCS_CODE_END

    //
    // Now, let's get a mapping for our `Gender` enum:
    // AUI_DOCS_CODE_BEGIN
    static constexpr auto GENDERS = aui::enumerate::ALL_VALUES<Gender>;
    // AUI_DOCS_CODE_END
    //
    // The compile-time constant above is equivalent to:
    // @code{cpp}
    // /* pseudocode */
    // GENDERS = std::array { Gender::MALE, Gender::FEMALE, GENDER::OTHER };
    // @endcode
    //
    // We just using `aui::enumerate::ALL_VALUES` because it was provided conveniently by `AUI_ENUM_VALUES` for us.
    //
    // It's not hard to guess that we'll use indices of this array to uniquely identify `Gender` associated with this
    // index:
    // AUI_DOCS_CODE_BEGIN
    /* pseudocode */
    GENDERS[0]; // -> MALE
    GENDERS[1]; // -> FEMALE
    GENDERS[2]; // -> OTHER
    // AUI_DOCS_CODE_END
    //
    // To perform opposite operation (i.e., `Gender` to int), we can use `aui::indexOf`:
    // AUI_DOCS_CODE_BEGIN
    /* pseudocode */
    aui::indexOf(GENDERS, Gender::MALE);   // -> 0
    aui::indexOf(GENDERS, Gender::FEMALE); // -> 1
    aui::indexOf(GENDERS, Gender::OTHER);  // -> 2
    // AUI_DOCS_CODE_END
    //
    // To bring these conversions together, let's use overloaded lambda:
    // AUI_DOCS_CODE_BEGIN
    static constexpr auto GENDER_INDEX_PROJECTION = aui::lambda_overloaded {
        [](Gender g) -> int { return aui::indexOf(GENDERS, g).valueOr(0); },
        [](int i) -> Gender { return GENDERS[i]; },
    };
    // AUI_DOCS_CODE_END
    // @note
    // It's convenient to use lambda trailing return type syntax (i.e., `... -> int`, `... -> Gender`)
    // to make it obvious what do transformations do and how one type is transformed to another.
    //
    // The function-like object above detects the direction of transformation and performs as follows:
    // AUI_DOCS_CODE_BEGIN
    GENDER_INDEX_PROJECTION(0); // -> MALE
    GENDER_INDEX_PROJECTION(Gender::MALE); // -> 0
    // AUI_DOCS_CODE_END
    //
    // It is all what we need to set up bidirectional transformations. Inside AUI_ENTRY:
    // AUI_DOCS_CODE_BEGIN
    auto user = aui::ptr::manage(new User { .gender = Gender::MALE });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            // generate a string list model for genders from GENDERS array defined earlier
            auto gendersStr = AListModel<AString>::fromVector(
                GENDERS
                | ranges::views::transform(AEnumerate<Gender>::toName)
                | ranges::to_vector);

            // equivalent:
            // gendersStr = { "MALE", "FEMALE", "OTHER" }
            // you can customize the displayed strings by playing with
            // ranges::views::transform argument.

            setContents(Centered {
              _new<ADropdownList>(gendersStr) let {
                  // AObject::connect(user->gender, it->selectionId());
                  //
                  // The code above would break, because Gender and int
                  // (selectionId() type) are incompatible.
                  //
                  // Instead, define bidirectional projection:
                   AObject::biConnect(
                       user->gender.biProjected(GENDER_INDEX_PROJECTION),
                       it->selectionId());
                  },
            });
        }
    };
    _new<MyWindow>(user)->show();
    // AUI_DOCS_CODE_END
    auto dropdownList = _cast<ADropdownList>(By::type<ADropdownList>().one());
    // ![dropdownlist](imgs/UIDataBindingTest.Declarative_bidirectional_projection_1.png)

    //
    // - If we try to change `user->gender` programmatically, ADropdownList will respond:
    // AUI_DOCS_CODE_BEGIN
    user->gender = Gender::FEMALE;
    EXPECT_EQ(dropdownList->getSelectedId(), 1); // second option
    // AUI_DOCS_CODE_END
    // ![dropdownlist](imgs/UIDataBindingTest.Declarative_bidirectional_projection_2.png)

    //
    // - If the user changes the value of ADropdownList, it reflects on the model as well:
    dropdownList->setSelectionId(2);
    // AUI_DOCS_CODE_BEGIN
    EXPECT_EQ(user->gender, Gender::OTHER);
    // AUI_DOCS_CODE_END
    // ![dropdownlist](imgs/UIDataBindingTest.Declarative_bidirectional_projection_3.png)
}

//
// # UI declarative data binding {#UI_declarative_data_binding}
// As said earlier, \c let syntax is a little bit clunky and requires extra boilerplate code to set up.
//
// Here's where declarative syntax comes into play. The logic behind the syntax is the same as in
// `AObject::connect`/`AObject::biConnect` (for ease of replacement/understanding).
//
// Declarative syntax uses `&` and `&&` operators to set up connections. These were chosen intentionally: `&&` resembles
// chain, so we "chaining view and property up".
//
// - `&` sets up one-directional connection (`AObject::connect`).
// - `&&` sets up bidirectional connection (`AObject::biConnect`).
//
// Also, `>` operator (resembles arrow) is used to specify the destination slot.
//
// The example below is essentially the same as @ref "Label_via_let" but uses declarative connection set up syntax.
TEST_F(UIDataBindingTest, Label_via_declarative) { // HEADER
    // Use `&` and `>` expression to connect the model's username property to the label's @ref ALabel::text "text"
    // property.
    // AUI_DOCS_CODE_BEGIN
    using namespace declarative;
    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            setContents(Centered {
              _new<ALabel>() & user->name > &ALabel::text
            });
        }
    };
    auto window = _new<MyWindow>(user);
    window->show();
    // AUI_DOCS_CODE_END
    window->setScalingParams({ .scalingFactor = 2.f });

    auto label = _cast<ALabel>(By::type<ALabel>().one());

    EXPECT_EQ(user->name, "Roza");
    EXPECT_EQ(label->text(), "Roza");

    saveScreenshot("1");
    // ![text](imgs/UIDataBindingTest.Label_via_declarative_1.png)

    // Note that the label already displays the value stored in User.
    //
    // Let's change the name:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Vasil";
    // AUI_DOCS_CODE_END

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(label->text(), "Vasil");
    saveScreenshot("2");
    // ![text](imgs/UIDataBindingTest.Label_via_declarative_2.png)

    user->name = "World";
    EXPECT_EQ(label->text(), "World");

    // In this example, we've achieved the same intuitive behaviour of data binding of `user->name` (like in
    // @ref "Label_via_let" example) but using declarative syntax. The logic behind `&` is almost the same as with `let`
    // and `AObject::connect` so projection use cases can be adapted in a similar manner.

    {
        auto l = Label {} & user->name > &ALabel::text;
        EXPECT_EQ(l->text(), "World");
    }
}

TEST_F(UIDataBindingTest, ADataBindingDefault_for_omitting_view_property) { // HEADER
    // In previous example we have explicitly specified ALabel's property to connect with.
    //
    // One of notable features of declarative way (in comparison to procedural `let` way) is that we can omit the view's
    // property to connect with if such `ADataBindingDefault` specialization exist for the target view and the property
    // type. Some views have already predefined such specialization for their underlying types. For instance, ALabel has
    // such specialization:
    //
    // @code{cpp}
    // /* PREDEFINED! You don't need to define it! This listing is an example */
    // template<>
    // struct ADataBindingDefault<ALabel, AString> {
    // public:
    //     static auto property(const _<ALabel>& view) { return view->text(); }
    // };
    // @endcode
    //
    // We can use this predefined specialization to omit the destination property:
    using namespace declarative;
    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            setContents(Centered {
                // AUI_DOCS_CODE_BEGIN
                _new<ALabel>() & user->name
                // AUI_DOCS_CODE_END
            });
        }
    };
    auto window = _new<MyWindow>(user);
    window->show();

    auto label = _cast<ALabel>(By::type<ALabel>().one());

    EXPECT_EQ(user->name, "Roza");
    EXPECT_EQ(label->text(), "Roza");

    //
    // Behaviour of such connection is equal to @ref "Label_via_declarative":
    //
    // ![text](imgs/UIDataBindingTest.Label_via_declarative_1.png)

    // Note that the label already displays the value stored in User.
    //
    // Let's change the name:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Vasil";
    // AUI_DOCS_CODE_END

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(label->text(), "Vasil");
    // ![text](imgs/UIDataBindingTest.Label_via_declarative_2.png)

    user->name = "World";
    EXPECT_EQ(label->text(), "World");

    // In this example, we've omitted the destination property of the connection while maintaining the same behaviour
    // as in @ref "Label_via_declarative".

    {
        auto l = Label {} & user->name;
        EXPECT_EQ(l->text(), "World");
    }
}

TEST_F(UIDataBindingTest, ADataBindingDefault_strong_type_propagation) { // HEADER
    using namespace declarative;
    // Think of `ADataBindingDefault` as we're not only connecting properties to properties, but also creating a
    // "property to view" relationship. This philosophy covers the following scenario.
    //
    // In AUI, there's aui::ranged_number template which stores valid value range right inside the type:
    // AUI_DOCS_CODE_BEGIN
    struct User {
        AProperty<aui::ranged_number<int, 1, 99>> age;
    };
    // AUI_DOCS_CODE_END

    //
    // These strong types can be used to propagate their traits on views, i.e., ANumberPicker. When using declarative
    // syntax, the property system calls `ADataBindingDefault::setup` to apply some extra traits of the bound value on
    // the view. Here's an abstract on how `ANumberPicker` defines specialization of `ADataBingingDefault` with
    // `aui::ranged_number`:
    // @code{cpp}
    // /* PREDEFINED! You don't need to define it! This listing is an example */
    // template <aui::arithmetic UnderlyingType, auto min, auto max>
    // struct ADataBindingDefault<ANumberPicker, aui::ranged_number<UnderlyingType, min, max>> {
    // public:
    //     static auto property(const _<ANumberPicker>& view) {
    //         return view->value();
    //     }
    //     static void setup(const _<ANumberPicker>& view) {
    //         view->setMin(aui::ranged_number<UnderlyingType, min, max>::MIN);
    //         view->setMax(aui::ranged_number<UnderlyingType, min, max>::MAX);
    //     }
    //     // ...
    // };
    // @endcode
    //
    // As you can see, this specialization pulls the min and max values from `aui::ranged_number` type and sets them
    // to `ANumberPicker`. This way `ANumberPicker` finds out the valid range of values by simply being bound to value
    // that has constraints encoded inside its type.

    auto user = aui::ptr::manage(new User { .age = 18 });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            static constexpr auto& GENDERS = aui::enumerate::ALL_VALUES<Gender>;
            setContents(Centered {
              // AUI_DOCS_CODE_BEGIN
              _new<ANumberPicker>() && user->age,
              // AUI_DOCS_CODE_END
              // @note
              // We're using `operator&&` here to set up bidirectional connection. For more info, go to
              // @ref "Declarative_bidirectional_connection".
              //
            });
        }
    };
    _new<MyWindow>(user)->show();
    auto numberPicker = _cast<ANumberPicker>(By::type<ANumberPicker>().one());

    //
    // By creating this connection, we've done a little bit more. We've set ANumberPicker::setMin and
    // ANumberPicker::setMax as well:
    // AUI_DOCS_CODE_BEGIN
    EXPECT_EQ(numberPicker->getMin(), 1);
    EXPECT_EQ(numberPicker->getMax(), 99);
    // AUI_DOCS_CODE_END
    //
    // This example demonstrates how to use declarative binding to propagate strong types. `aui::ranged_number`
    // propagates its constraints on `ANumberPicker` thanks to `ADataBindingDefault` specialization.
}

TEST_F(UIDataBindingTest, Label_via_declarative_projection) { // HEADER
    // We can use projections in the same way as with `let`.
    // AUI_DOCS_CODE_BEGIN
    using namespace declarative;
    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            _<ALabel> label;
            setContents(Centered {
                _new<ALabel>() & user->name.readProjected(&AString::uppercase)
            });
        }
    };
    auto window = _new<MyWindow>(user);
    window->show();
    // AUI_DOCS_CODE_END
    window->setScalingParams({ .scalingFactor = 2.f });
    auto label = _cast<ALabel>(By::type<ALabel>().one());
    saveScreenshot("1");
    // ![text](imgs/UIDataBindingTest.Label_via_declarative_projection_1.png)
    //
    // Note that the label already displays the **projected** value stored in User.
    //
    // Projection applies to value changes as well. Let's change the name:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Vasil";

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(label->text(), "VASIL"); // projected
    // AUI_DOCS_CODE_END
    saveScreenshot("2");
    // ![text](imgs/UIDataBindingTest.Label_via_declarative_projection_2.png)

    user->name = "World";
    EXPECT_EQ(label->text(), "WORLD");
}

TEST_F(UIDataBindingTest, Declarative_custom_slot1) {
    using namespace declarative;
    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            _<ALabel> label;
            setContents(Centered {
                _new<ALabel>() & user->name > [](ALabel& label, const AString& s) {
                  label.setText("custom slot! {}"_format(s));
                }
            });
        }
    };
    auto window = _new<MyWindow>(user);
    window->show();
    window->setScalingParams({ .scalingFactor = 2.f });
    auto label = _cast<ALabel>(By::type<ALabel>().one());
    user->name = "Vasil";

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(label->text(), "custom slot! Vasil");
    user->name = "World";
    EXPECT_EQ(label->text(), "custom slot! World");
}

TEST_F(UIDataBindingTest, Declarative_custom_slot2) {
    using namespace declarative;
    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            _<ALabel> label;
            setContents(Centered {
                _new<ALabel>() & user->name > &ALabel::setText
            });
        }
    };
    auto window = _new<MyWindow>(user);
    window->show();
    window->setScalingParams({ .scalingFactor = 2.f });
    auto label = _cast<ALabel>(By::type<ALabel>().one());
    user->name = "Vasil";

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(label->text(), "Vasil");
    user->name = "World";
    EXPECT_EQ(label->text(), "World");
}

TEST_F(UIDataBindingTest, Declarative_custom_slot3) {
    using namespace declarative;
    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            _<ALabel> label;
            setContents(Centered {
              _new<ALabel>() & user->name.readProjected([](const AString& s) { return s != "hide"; }) > &AView::setVisible
            });
        }
    };
    auto window = _new<MyWindow>(user);
    window->show();
    auto label = _cast<ALabel>(By::type<ALabel>().one());
    user->name = "Vasil";
    EXPECT_EQ(label->visibility(), Visibility::VISIBLE);
    user->name = "hide";
    EXPECT_EQ(label->visibility(), Visibility::INVISIBLE);
}

TEST_F(UIDataBindingTest, Declarative_bidirectional_connection) { // HEADER
    // In previous examples, we've used `&` to make one directional (one sided) connection. This is
    // perfectly enough for ALabel because it cannot be changed by user.
    //
    // In some cases, you might want to use property-to-property as it's bidirectional. It's used for populating view
    // from model and obtaining data from view back to the model.
    //
    // For this example, let's use ATextField instead of ALabel as it's an editable view. In this case, we'd want to use
    // `&&` because we do want `user->name` to be aware of changes of the view.

    using namespace declarative;

    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            setContents(Centered {
              // AUI_DOCS_CODE_BEGIN
              _new<ATextField>() && user->name
              // AUI_DOCS_CODE_END
            });
        }
    };
    auto window = _new<MyWindow>(user);
    window->setScalingParams({ .scalingFactor = 2.f });
    window->show();
    //
    // This gives the following result:
    saveScreenshot("1");
    // ![text](imgs/UIDataBindingTest.Declarative_bidirectional_connection_1.png)

    auto tf = _cast<ATextField>(By::type<ATextField>().one());

    //
    // Let's change the name programmatically:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Vasil";
    // AUI_DOCS_CODE_END
    //
    // ATextField will respond:
    saveScreenshot("2");
    // ![text](imgs/UIDataBindingTest.Declarative_bidirectional_connection_2.png)

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(tf->text(), "Vasil");

    //
    // If the user changes the value from UI, these changes will reflect on `user->model` as well:
    tf->selectAll();
    By::value(tf).perform(type("Changed from UI"));
    saveScreenshot("3");
    // ![text](imgs/UIDataBindingTest.Declarative_bidirectional_connection_3.png)
    // AUI_DOCS_CODE_BEGIN
    EXPECT_EQ(user->name, "Changed from UI");
    // AUI_DOCS_CODE_END
    //
    // This way we've set up bidirectional projection via `&&` which makes `user->name` aware of UI
    // changes.
}

TEST_F(UIDataBindingTest, Declarative_bidirectional_projection) { // HEADER
    // We can use projections in the same way as with `let`.
    //
    // Let's repeat the @ref "Bidirectional_projection" sample in declarative way:
    using namespace declarative;
    struct User {
        AProperty<Gender> gender;
    };

    static constexpr auto GENDERS = aui::enumerate::ALL_VALUES<Gender>;
    static constexpr auto GENDER_INDEX_PROJECTION = aui::lambda_overloaded {
        [](Gender g) -> int { return aui::indexOf(GENDERS, g).valueOr(0); },
        [](int i) -> Gender { return GENDERS[i]; },
    };
    auto user = aui::ptr::manage(new User { .gender = Gender::MALE });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            auto gendersStr = AListModel<AString>::fromVector(
                GENDERS
                | ranges::views::transform(AEnumerate<Gender>::toName)
                | ranges::to_vector);

            setContents(Centered {
                // AUI_DOCS_CODE_BEGIN
                _new<ADropdownList>(gendersStr) && user->gender.biProjected(GENDER_INDEX_PROJECTION) > &ADropdownList::selectionId
                // AUI_DOCS_CODE_END
                // ![dropdownlist](imgs/UIDataBindingTest.Declarative_bidirectional_projection_1.png)
                // @note
                // We used the `&&` operator here instead of `&` because we want the connection work in both
                // directions: `user.gender -> ADropdownList` and `ADropdownList -> user.gender`.
                //
            });
        }
    };
    auto window = _new<MyWindow>(user);
    window->setScalingParams({ .scalingFactor = 2.f });
    window->show();
    auto dropdownList = _cast<ADropdownList>(By::type<ADropdownList>().one());
    saveScreenshot("1");

    //
    // - If we try to change `user->gender` programmatically, ADropdownList will respond:
    // AUI_DOCS_CODE_BEGIN
    user->gender = Gender::FEMALE;
    EXPECT_EQ(dropdownList->getSelectedId(), 1); // second option
    // AUI_DOCS_CODE_END
    saveScreenshot("2");
    // ![dropdownlist](imgs/UIDataBindingTest.Declarative_bidirectional_projection_2.png)

    //
    // - If the user changes the value of ADropdownList, it reflects on the model as well:
    dropdownList->setSelectionId(2);
    saveScreenshot("3");
    // AUI_DOCS_CODE_BEGIN
    EXPECT_EQ(user->gender, Gender::OTHER);
    // AUI_DOCS_CODE_END
    // ![dropdownlist](imgs/UIDataBindingTest.Declarative_bidirectional_projection_3.png)
}

//
// # Functions
//