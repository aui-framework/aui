// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "AUI/Test/UI/By.h"
#include "AUI/Test/UI/UITestCase.h"
#include "AUI/Test/UI/Assertion/Color.h"
#include "AUI/Util/Declarative/Containers.h"
#include "AUI/View/ACheckBox.h"

namespace {

using namespace declarative;

class UIReactiveTest : public testing::UITest {
public:

protected:
    void SetUp() override {
        UITest::SetUp();

        mWindow = _new<AWindow>();
        mWindow->show();
    }

    void TearDown() override {
        mWindow = nullptr;
        UITest::TearDown();
    }

    _<AWindow> mWindow;
};
}   // namespace

TEST_F(UIReactiveTest, Label) {
    // Test that a reactive label updates its text when the bound property changes.
    // The test creates a simple state object with a reactive string property
    // and binds it to a label using the AUI_REACT macro. It then verifies
    // the initial rendered text and the updated text after changing the
    // property.
    using namespace ass;

    struct State {
        AProperty<AString> name;
    };
    auto state = _new<State>();

    mWindow->setContents(Vertical {
        Label { AUI_REACT("{}!"_format(state->name)) }
    });

    // Initially, the property is empty, so the label should display "!".
    EXPECT_EQ(*_cast<ALabel>(By::type<ALabel>().one())->text(), "!");
    // Update the property; the label should automatically reflect the new value.
    state->name = "Hello";
    EXPECT_EQ(*_cast<ALabel>(By::type<ALabel>().one())->text(), "Hello!");
}

struct TextColor {
    AColor color;

    void operator()(AView& view) const {

    }
};

TEST_F(UIReactiveTest, Test2) {
    // Test that a reactive label updates its text when the bound property changes.
    // The test creates a simple state object with a reactive string property
    // and binds it to a label using the AUI_REACT macro. It then verifies
    // the initial rendered text and the updated text after changing the
    // property.
    using namespace ass;

    struct State {
        AProperty<bool> option = false;
    };
    auto state = _new<State>();

    mWindow->setContents(Vertical {
      CheckBox { .checked = AUI_REACT(state->option), .onCheckedChange = [state](bool v) { state->option = v; } },
      Label { AUI_REACT("{}"_format(*state->option)) },
      Label {
        .text = "Test",
        .modifier =
            Modifier {
              ::TextColor { .color = state->option ? AColor::RED : AColor::GREEN },
            },
      },
        Label {
          .text = "Test",
        } AUI_LET {

        },
      });

    // Initially, the property is empty, so the label should display "!".
    EXPECT_EQ(*_cast<ALabel>(By::type<ALabel>().one())->text(), "!");
    // Update the property; the label should automatically reflect the new value.
    EXPECT_EQ(*_cast<ALabel>(By::type<ALabel>().one())->text(), "Hello!");
}

/*
TEST_F(UIReactiveTest, MultipleWithStyle) {
    // Test that multiple containers with overridden styles render correctly.
    // The test constructs a vertical layout containing a centered label
    // with a green background, and the vertical layout itself has a red
    // background. It then checks that the pixel at the center of the
    // label is green, confirming that the style override on the inner
    // container takes precedence over the outer container.
    using namespace ass;

    auto makeContainer = [](_<AView> view) {
        return Centered {
            std::move(view),
        } AUI_OVERRIDE_STYLE {
            BackgroundSolid { AColor::GREEN },
        };
    };

    mWindow->setContents(Vertical {
        makeContainer(Label {" "}) AUI_OVERRIDE_STYLE { Expanding() },
    } AUI_OVERRIDE_STYLE {
        BackgroundSolid { AColor::RED },
    });

    // Verify that the pixel at the center of the label is green.
    By::type<ALabel>().check(pixelColorAt({0.5f, 0.5f}, AColor::GREEN));
}*/
