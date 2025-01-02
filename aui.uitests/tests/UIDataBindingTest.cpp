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
// 9. two property variants: simple field (AProperty) and custom getter/setter (APropertyDef)
// 10. some properties can be readonly
// 11. propagating strong types' traits on views
//
// Learning curve is relatively flat, so be sure to
// [ask questions and open issues](https://github.com/aui-framework/aui/issues) on our GitHub page.
//
// Main difference between basic value lying somewhere inside your class and a property is that the latter explicitly
// ties getter, setter and a signal reporting value changes. Property acts almost transparently, as if there's no
// property wrapper. This allows to read the intermediate value of a property and subscribe to its changes via a single
// \c connect call. Also, when connecting property to property, it is possible to make them observe changes of each
// other bia \c biConnect call:
TEST_F(UIDataBindingTest, TextField1) {
    // AUI_DOCS_CODE_BEGIN
    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage(User { .name = "Robert" });
    auto tf = _new<ATextField>();
    AObject::biConnect(user->name, tf->text());
    auto window = _new<AWindow>();
    window->setContents(Centered { tf });
    window->show();
    // AUI_DOCS_CODE_END
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


//
// # Declaring Properties
// There are two ways to define a property in AUI:
//
TEST_F(UIDataBindingTest, AProperty) { // HEADER
    // To declare a property inside your data model, use AProperty template:
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
    }
}


// # UI data binding with let
// @note
// This is a comprehensive, straightforward way of setting up a connection. We are demonstrating it here so you can get
// deeper understanding on how connections are made and what does declarative way do under the hood. This way may be
// used in favour of declarative way if the latter not work for you. For declarative way, search for `"UI declarative
// data binding"` on this page.
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

    auto user = aui::ptr::manage(User { .name = "Roza" });

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

    auto user = aui::ptr::manage(User { .name = "Roza" });

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

    auto user = aui::ptr::manage(User { .name = "Roza" });

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
    auto user = aui::ptr::manage(User { .gender = Gender::MALE });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            // generate a string list model for genders from GENDERS array defined earlier
            auto gendersStr = AListModel<AString>::fromVector(
                GENDERS
                | ranges::view::transform(AEnumerate<Gender>::toName)
                | ranges::to_vector);

            // equivalent:
            // gendersStr = { "MALE", "FEMALE", "OTHER" }
            // you can customize the displayed strings by playing with
            // ranges::view::transform argument.

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

TEST_F(UIDataBindingTest, Propagating_strong_types) { // HEADER
    using namespace declarative;
    // In AUI, there's aui::ranged_number template which stores valid value range right inside the type:
    // AUI_DOCS_CODE_BEGIN
    struct User {
        AProperty<aui::ranged_number<int, 1, 99>> age;
    };
    // AUI_DOCS_CODE_END

    //
    // These strong types can be used to propagate their traits on views, i.e., ANumberPicker:

    auto user = aui::ptr::manage(User { .age = 18 });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            static constexpr auto& GENDERS = aui::enumerate::ALL_VALUES<Gender>;
            setContents(Centered {
                // AUI_DOCS_CODE_BEGIN
                _new<ANumberPicker>() let {
                  AObject::connect(user->age, it->value());
                },
                // AUI_DOCS_CODE_END
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
}

namespace declarative {
    template<typename Object, APropertyWritable Connectable>
    inline const _<Object>& operator&&(const _<Object>& object, Connectable&& binding) {
        aui::tuple_visitor<typename AAnySignalOrPropertyTraits<std::decay_t<Connectable>>::args>::for_each_all([&]<typename... T>() {
            using Binding = ADataBindingDefault<std::decay_t<Object>, std::decay_t<T>...>;
            AObject::biConnect(binding, Binding::property(object));
        });
        return object;
    }


    template<typename Object, APropertyReadable Connectable>
    inline const _<Object>& operator&(const _<Object>& object, Connectable&& binding) {
        aui::tuple_visitor<typename AAnySignalOrPropertyTraits<std::decay_t<Connectable>>::args>::for_each_all([&]<typename... T>() {
          using Binding = ADataBindingDefault<std::decay_t<Object>, std::decay_t<T>...>;
          AObject::connect(binding, Binding::property(object));
        });
        return object;
    }

    template<AAnyProperty Lhs, typename Destination>
    struct Binding {
        Lhs sourceProperty;
        Destination destinationPointerToMember;
        explicit Binding(Lhs sourceProperty, Destination destinationPointerToMember)
          : sourceProperty(sourceProperty), destinationPointerToMember(destinationPointerToMember) {}
    };

    template<AAnyProperty Property, typename Destination>
    inline decltype(auto) operator->*(Property&& sourceProperty, Destination&& rhs) {
        return Binding(std::forward<Property>(sourceProperty), std::forward<Destination>(rhs));
    }

    template<typename Object, APropertyWritable Property, typename Destination>
    inline const _<Object>& operator&&(const _<Object>& object, Binding<Property, Destination>&& binding) {
        AObject::biConnect(binding.sourceProperty, std::invoke(binding.destinationPointerToMember, *object));
        return object;
    }
}

//
// # UI declarative data binding
// As said earlier, \c let syntax is a little bit clunky and requires extra boilerplate code to set up.
//
// Here's where declarative syntax comes into play. Declarative syntax uses the same argument order as
// `AObject::connect` (for ease of replacement), besides that
//
// Declarative syntax uses `&` and `&&` to set up connections. This particular operator was chosen intentionally: `&&`
// resembles chain, so we "chaining view and property up".
//
// - `&` sets up one-directional connection.
// - `&&` sets up bidirectional connection.
//
// The example below is essentially the same as "Label via let" but uses declarative connection set up syntax.
TEST_F(UIDataBindingTest, Label_via_declarative) { // HEADER
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
            setContents(Centered {
              _new<ALabel>() & user->name
            });
        }
    };
    auto window = _new<MyWindow>(user);
    window->show();
    // AUI_DOCS_CODE_END
    window->setScalingParams({ .scalingFactor = 2.f });

    auto label = _cast<ALabel>(By::type<ALabel>().one());


    // AUI_DOCS_CODE_BEGIN
    // Both label and user should hold name Roza.
    EXPECT_EQ(user->name, "Roza");
    EXPECT_EQ(label->text(), "Roza");
    // AUI_DOCS_CODE_END
    saveScreenshot("1");
    // ![text](imgs/UIDataBindingTest.Label_via_declarative_1.png)

    // Note that the label already displays the **projected** value stored in User.
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

    // In this example, we've achieved the same intuitive behaviour of data binding of `user->name` (like in "Label via
    // let" example) but using declarative syntax. The logic behind `&&` is almost the same as with `let` +
    // `AObject::connect` so projection usecases can be adapted in a similar manner.
}

TEST_F(UIDataBindingTest, Label_via_declarative_projection) { // HEADER
    // We can use projections in the same way as with `let`.
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
    // Projection applies to value chhanges as well. Let's change the name:
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

    auto user = aui::ptr::manage(User { .name = "Roza" });

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
    // Let's repeat the `"Bidirectional projection"` sample in declarative way:
    using namespace declarative;
    struct User {
        AProperty<Gender> gender;
    };

    static constexpr auto GENDERS = aui::enumerate::ALL_VALUES<Gender>;
    static constexpr auto GENDER_INDEX_PROJECTION = aui::lambda_overloaded {
        [](Gender g) -> int { return aui::indexOf(GENDERS, g).valueOr(0); },
        [](int i) -> Gender { return GENDERS[i]; },
    };
    auto user = aui::ptr::manage(User { .gender = Gender::MALE });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            auto gendersStr = AListModel<AString>::fromVector(
                GENDERS
                | ranges::view::transform(AEnumerate<Gender>::toName)
                | ranges::to_vector);

            setContents(Centered {
                // AUI_DOCS_CODE_BEGIN
                _new<ADropdownList>(gendersStr) && user->gender.biProjected(GENDER_INDEX_PROJECTION)->*&ADropdownList::selectionId
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
