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

using namespace declarative;

class UIDataBindingTest : public testing::UITest {
};

TEST_F(UIDataBindingTest, ComplexBinaryOperations) {
    AProperty<float> less = 1;
    AProperty<float> larger = 2;
    EXPECT_TRUE(less != larger);

    EXPECT_TRUE(less < larger);
    EXPECT_TRUE(less <= larger);

    EXPECT_FALSE(less > larger);
    EXPECT_FALSE(less > larger);

    EXPECT_TRUE(less < larger && less < larger);
    EXPECT_TRUE(less <= larger && less <= larger);
    EXPECT_FALSE(less > larger && less > larger);
    EXPECT_FALSE(less >= larger && less >= larger);

    EXPECT_TRUE(less != 0.f);

    EXPECT_TRUE(less < 2.f);
    EXPECT_TRUE(less <= 2.f);

    EXPECT_FALSE(less > 2.f);
    EXPECT_FALSE(less > 2.f);

    EXPECT_TRUE(less < 2.f && less < 2.f);
    EXPECT_TRUE(less <= 2.f && less <= 2.f);
    EXPECT_FALSE(less > 2.f && less > 2.f);
    EXPECT_FALSE(less >= 2.f && less >= 2.f);

    EXPECT_TRUE(less != 0);

    EXPECT_TRUE(less < 2);
    EXPECT_TRUE(less <= 2);

    EXPECT_FALSE(less > 2);
    EXPECT_FALSE(less > 2);

    EXPECT_TRUE(less < 2 && less < 2);
    EXPECT_TRUE(less <= 2 && less <= 2);
    EXPECT_FALSE(less > 2 && less > 2);
    EXPECT_FALSE(less >= 2 && less >= 2);


    AProperty<bool> isValid;
    isValid = less > 0 && larger > 0 && larger > 0;
}


// Main difference between basic value lying somewhere inside your class and a property is that the latter explicitly
// ties getter, setter and a signal reporting value changes. Property acts almost transparently, as if there's no
// extra wrapper around your data. This allows to work with properties in the same way as with their underlying values.
// You can read the intermediate value of a property and subscribe to its changes via a single
// `connect` call. Also, when connecting property to property, it is possible to make them observe changes of each
// other bia `biConnect` call:
TEST_F(UIDataBindingTest, TextField1) {
    // AUI_DOCS_CODE_BEGIN
    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage_shared(new User { .name = "Robert" });
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
    // <figure markdown="span">
    // ![text field](imgs/UIDataBindingTest.TextField1_1.png)
    // </figure>
    //
    // A single call of `biConnect`:
    //

    // -  Prefills text field with the current `user->name` value (pre fire):
    // AUI_DOCS_CODE_BEGIN
    EXPECT_EQ(tf->text(), "Robert");
    // AUI_DOCS_CODE_END

    // -  Connects `user->named.changed` to `tf` to notify the text field about changes of `user->name`:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Angela";           // changing user->name programmatically...
    EXPECT_EQ(tf->text(), "Angela"); // ...should reflect on the text field
    // AUI_DOCS_CODE_END
    saveScreenshot("2");
    // <figure markdown="span">
    // ![text field](imgs/UIDataBindingTest.TextField1_2.png)
    // </figure>

    // -  Connects `tf->text().changed` to notify the `user->name` property about changes in text field (i.e., if the
    //    user typed another value to the text field):
    // <figure markdown="span">
    // ![text field](imgs/UIDataBindingTest.TextField1_3.png)
    // </figure>
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
// ## Declaring Properties
//
// There are several ways to define a property in AUI:
//
// - AProperty - basic wrapper property type for data models
// - APropertyDef - property-compliant view type to tie custom getter, setter and signal together
// - APropertyPrecomputed - readonly property whose value is determined by a callable that references other properties
//
// Please check their respective documentation pages for an additional information.
//
// ## UI data binding with AUI_LET
//
// !!! note
//
//     This is a comprehensive, straightforward way of setting up a connection. We are demonstrating it here so you can
//     get deeper understanding on how connections are made and what does declarative way do under the hood. This way
//     may be used in favour of declarative way if the latter not work for you. For declarative way, go to
//     [UI_declarative_data_binding].
//
// This approach allows more control over the binding process by using `AObject::connect`/`AObject::biConnect` which is
// a procedural way of setting up connections. As a downside, it requires "AUI_LET" syntax clause which may seem as overkill
// for such a simple operation.
TEST_F(UIDataBindingTest, Label_via_let) { // HEADER_H3
    // Use `AUI_LET` expression to connect the model's username property to the label's [text()](ALabel::text)
    // property.
    // AUI_DOCS_CODE_BEGIN
    using namespace declarative;

    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage_shared(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            setContents(Centered {
                _new<ALabel>() AUI_LET {
                  // Data goes from left to right:
                  // current value (pre fire) or changed event
                  // goes to assignment operation of it->text()
                  AObject::connect(user->name, it->text());
                  //                ->  ->  ->  ->  ->
                  // in other words, this connection is essentially the
                  // same as
                  // AObject::connect(user->name, AUI_SLOT(it)::setText);
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
    // ![](imgs/UIDataBindingTest.Label_via_declarative_1.png)
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
    // ![](imgs/UIDataBindingTest.Label_via_declarative_2.png)
    //
    // By simply performing assignment on `user` we changed ALabel display text. Magic, huh?

    user->name = "World";
    EXPECT_EQ(label->text(), "World");
}

TEST_F(UIDataBindingTest, Label_via_let_projection) { // HEADER_H3
    // It's fairly easy to define a projection because one-sided connection requires exactly one projection, obviously.
    using namespace declarative;

    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage_shared(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            setContents(Centered {
              // AUI_DOCS_CODE_BEGIN
              _new<ALabel>() AUI_LET {
                  // Data goes from left to right:
                  // current value (pre fire) or changed event
                  // goes through projection (&AString::uppercase) first
                  // then it goes to assignment operation of it->text()
                  // property.
                  AObject::connect(AUI_REACT(user->name->uppercase()), it->text());
              },
              // AUI_DOCS_CODE_END
            });
        }
    };
    _new<MyWindow>(user)->show();

    //
    // This gives the following result:
    //
    // ![](imgs/UIDataBindingTest.Label_via_declarative_projection_1.png)
    //
    // Note that the label already displays the **projected** value stored in User.

    auto label = _cast<ALabel>(By::type<ALabel>().one());

    //
    // Let's change the name:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Vasil";
    // AUI_DOCS_CODE_END
    //
    // ![](imgs/UIDataBindingTest.Label_via_declarative_projection_2.png)
    //
    // This way, we've set up data binding with projection.

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(label->text(), "VASIL"); // uppercased by projection!
}

TEST_F(UIDataBindingTest, Bidirectional_connection) { // HEADER_H3
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

    auto user = aui::ptr::manage_shared(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            setContents(Centered {
                // AUI_DOCS_CODE_BEGIN
                _new<ATextField>() AUI_LET {
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
    //
    // ![](imgs/UIDataBindingTest.Declarative_bidirectional_connection_1.png)
    //

    auto tf = _cast<ATextField>(By::type<ATextField>().one());

    //
    // Let's change the name programmatically:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Vasil";
    // AUI_DOCS_CODE_END
    //
    // ATextField will respond:
    // ![](imgs/UIDataBindingTest.Declarative_bidirectional_connection_2.png)

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(tf->text(), "Vasil");

    //
    // If the user changes the value from UI, these changes will reflect on `user->model` as well:
    tf->selectAll();
    By::value(tf).perform(type("Changed from UI"));
    // ![](imgs/UIDataBindingTest.Declarative_bidirectional_connection_3.png)
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

//
// ## UI declarative data binding { #UI_declarative_data_binding }
// As said earlier, `AUI_LET` syntax is a little bit clunky and requires extra boilerplate code to set up.
//
// Here's where declarative syntax comes into play. The logic behind the syntax is the same as in
// `AObject::connect`/`AObject::biConnect` (for ease of replacement/understanding).
//
// The example below is essentially the same as [UIDataBindingTest_Label_via_let] but uses declarative connection set up syntax.
TEST_F(UIDataBindingTest, Label_via_declarative) { // HEADER_H3
    // AUI_DOCS_CODE_BEGIN
    using namespace declarative;
    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage_shared(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            setContents(Centered {
              Label { AUI_REACT(user->name) },
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
    //
    // ![](imgs/UIDataBindingTest.Label_via_declarative_1.png)
    //

    // Note that the label already displays the value stored in User.
    //
    // Let's change the name:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Vasil";
    // AUI_DOCS_CODE_END

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(label->text(), "Vasil");
    saveScreenshot("2");
    //
    // ![](imgs/UIDataBindingTest.Label_via_declarative_2.png)
    //

    user->name = "World";
    EXPECT_EQ(label->text(), "World");

    // In this example, we've achieved the same intuitive behaviour of data binding of `user->name` (like in
    // [UIDataBindingTest_Label_via_let] example) but using declarative syntax.

    {
        auto l = Label {} & user->name > &ALabel::text;
        EXPECT_EQ(l->text(), "World");
    }
}

TEST_F(UIDataBindingTest, ADataBindingDefault_for_omitting_view_property) { // HEADER_H3
    //
    // !!! failure "Deprecated"
    //
    //     Use [declarative contracts](retained_immediate_ui.md) instead.
    //
    // In the previous example we have explicitly specified ALabel's property to connect with.
    //
    // One of notable features of declarative way (in comparison to procedural `AUI_LET` way) is that we can omit the view's
    // property to connect with if such `ADataBindingDefault` specialization exist for the target view and the property
    // type. Some views have already predefined such specialization for their underlying types. For instance, ALabel has
    // such specialization:
    //
    // ```cpp
    // /* PREDEFINED! You don't need to define it! This listing is an example */
    // template<>
    // struct ADataBindingDefault<ALabel, AString> {
    // public:
    //     static auto property(const _<ALabel>& view) { return view->text(); }
    // };
    // ```
    //
    // We can use this predefined specialization to omit the destination property:
    using namespace declarative;
    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage_shared(new User { .name = "Roza" });

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
    // Behaviour of such connection is equal to [UIDataBindingTest_Label_via_declarative]:
    //
    // ![](imgs/UIDataBindingTest.Label_via_declarative_1.png)
    //

    // Note that the label already displays the value stored in User.
    //
    // Let's change the name:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Vasil";
    // AUI_DOCS_CODE_END

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(label->text(), "Vasil");
    //
    // ![](imgs/UIDataBindingTest.Label_via_declarative_2.png)
    //

    user->name = "World";
    EXPECT_EQ(label->text(), "World");

    // In this example, we've omitted the destination property of the connection while maintaining the same behaviour
    // as in [UIDataBindingTest_Label_via_declarative].

    {
        auto l = Label {} & user->name;
        EXPECT_EQ(l->text(), "World");
    }
}

TEST_F(UIDataBindingTest, ADataBindingDefault_strong_type_propagation) { // HEADER_H3
    using namespace declarative;

    //
    // !!! failure "Deprecated"
    //
    //     Use [declarative contracts](retained_immediate_ui.md) instead.
    //
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
    // ```cpp
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
    // ```
    //
    // As you can see, this specialization pulls the min and max values from `aui::ranged_number` type and sets them
    // to `ANumberPicker`. This way `ANumberPicker` finds out the valid range of values by simply being bound to value
    // that has constraints encoded inside its type.


    auto user = aui::ptr::manage_shared(new User { .age = 18 });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            static constexpr auto& GENDERS = aui::enumerate::ALL_VALUES<Gender>;
            setContents(Centered {
              // AUI_DOCS_CODE_BEGIN
              _new<ANumberPicker>() && user->age,
              // AUI_DOCS_CODE_END
              // !!! note
              //
              //     We're using `operator&&` here to set up bidirectional connection. For more info, go to
              //     [UIDataBindingTest_Declarative_bidirectional_connection].
              //
            });
        }
    };
    _new<MyWindow>(user)->show();
    auto single = _new<ANumberPicker>() & user->age;
    auto numberPicker = _cast<ANumberPicker>(By::type<ANumberPicker>().one());

    //
    // By creating this connection, we've done a little bit more. We've set ANumberPicker::setMin and
    // ANumberPicker::setMax as well:
    // AUI_DOCS_CODE_BEGIN
    EXPECT_EQ(numberPicker->getMin(), 1);
    EXPECT_EQ(numberPicker->getMax(), 99);
    // AUI_DOCS_CODE_END

    EXPECT_EQ(single->getMin(), 1);
    EXPECT_EQ(single->getMax(), 99);
    //
    // This example demonstrates how to use declarative binding to propagate strong types. `aui::ranged_number`
    // propagates its constraints on `ANumberPicker` thanks to `ADataBindingDefault` specialization.
}

TEST_F(UIDataBindingTest, Label_via_declarative_projection) { // HEADER_H3
    // We can use projections in the same way as with `AUI_LET`.
    // AUI_DOCS_CODE_BEGIN
    using namespace declarative;
    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage_shared(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            setContents(Centered {
                Label { AUI_REACT(user->name->uppercase()) },
            });
        }
    };
    auto window = _new<MyWindow>(user);
    window->show();
    // AUI_DOCS_CODE_END
    window->setScalingParams({ .scalingFactor = 2.f });
    auto label = _cast<ALabel>(By::type<ALabel>().one());
    saveScreenshot("1");
    // ![](imgs/UIDataBindingTest.Label_via_declarative_projection_1.png)
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
    // ![](imgs/UIDataBindingTest.Label_via_declarative_projection_2.png)

    user->name = "World";
    EXPECT_EQ(label->text(), "WORLD");
}

TEST_F(UIDataBindingTest, Declarative_custom_slot1) {
    using namespace declarative;
    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage_shared(new User { .name = "Roza" });

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

    auto user = aui::ptr::manage_shared(new User { .name = "Roza" });

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

    auto user = aui::ptr::manage_shared(new User { .name = "Roza" });

    class MyWindow: public AWindow {
    public:
        MyWindow(const _<User>& user) {
            _<ALabel> label;
            setContents(Centered {
              _new<ALabel>() AUI_LET {
                  AObject::connect(user->name, it, [&it = *it](const AString& s) { it.setVisible(s != "hide"); });
              },
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

TEST_F(UIDataBindingTest, Declarative_bidirectional_connection) { // HEADER_H3
    //
    // !!! failure "Deprecated"
    //
    //     Use [declarative contracts](retained_immediate_ui.md) instead.
    //
    // In some cases, you might want to use property-to-property as it's bidirectional. It's used for populating view
    // from model and obtaining data from view back to the model.

    using namespace declarative;

    struct User {
        AProperty<AString> name;
    };

    auto user = aui::ptr::manage_shared(new User { .name = "Roza" });

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
    //
    // ![](imgs/UIDataBindingTest.Declarative_bidirectional_connection_1.png)
    //

    auto tf = _cast<ATextField>(By::type<ATextField>().one());

    //
    // Let's change the name programmatically:
    // AUI_DOCS_CODE_BEGIN
    user->name = "Vasil";
    // AUI_DOCS_CODE_END
    //
    // ATextField will respond:
    saveScreenshot("2");
    //
    // ![](imgs/UIDataBindingTest.Declarative_bidirectional_connection_2.png)
    //

    EXPECT_EQ(user->name, "Vasil");
    EXPECT_EQ(tf->text(), "Vasil");

    //
    // If the user changes the value from UI, these changes will reflect on `user->model` as well:
    tf->selectAll();
    By::value(tf).perform(type("Changed from UI"));
    saveScreenshot("3");
    //
    // ![](imgs/UIDataBindingTest.Declarative_bidirectional_connection_3.png)
    //
    // AUI_DOCS_CODE_BEGIN
    EXPECT_EQ(user->name, "Changed from UI");
    // AUI_DOCS_CODE_END
    //
    // This way we've set up bidirectional projection via `&&` which makes `user->name` aware of UI
    // changes.
}
