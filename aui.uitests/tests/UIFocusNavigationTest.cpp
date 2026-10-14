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

#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/ARadioButton.h>
#include <AUI/View/AListView.h>
#include <AUI/View/ATreeView.h>
#include <AUI/Model/AListModel.h>
#include <AUI/Model/ATreeModel.h>
#include <AUI/Platform/AInput.h>
#include <AUI/View/AView.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/ASS/AStylesheet.h>

using namespace declarative;

class UIFocusNavigation : public testing::UITest {
protected:
    /**
     * @brief Minimal view that participates in keyboard (non-mouse) focus navigation.
     * @details
     * collectFocusableViews requires both capturesFocus() and handlesNonMouseNavigation() to be true.
     * AView::capturesFocus() returns true by default, so only handlesNonMouseNavigation() has to be overridden.
     */
    struct FocusableView : AView {
        bool handlesNonMouseNavigation() override { return true; }
    };

    _<AWindow> mWindow;
    _<FocusableView> v1, v2, v3;

    void SetUp() override {
        UITest::SetUp();

        v1 = _new<FocusableView>();
        v2 = _new<FocusableView>();
        v3 = _new<FocusableView>();

        mWindow = _new<AWindow>();
        mWindow->setContents(Vertical {
            v1,
            v2,
            v3,
        });
        mWindow->show();

        // process attach/layout messages so that getWindow()/focus() works
        uitest::frame();
    }

    /**
     * @brief Currently focused view as a raw pointer.
     */
    [[nodiscard]] AView* focused() const {
        return mWindow->getFocusedView().get();
    }

    /**
     * @brief Pumps UI messages so that focus() (dispatched to the UI thread) takes effect.
     */
    static void pump() {
        uitest::frame();
    }
};

/**
 * With no view focused, focusNextView() should focus the first focusable view.
 */
TEST_F(UIFocusNavigation, NoFocusFocusesFirst) {
    mWindow->setFocusedView(nullptr);
    pump();
    ASSERT_EQ(focused(), nullptr);

    mWindow->focusNextView();
    pump();

    EXPECT_EQ(focused(), static_cast<AView*>(v1.get()));
}

/**
 * With no view focused, focusPrevView() should focus the last focusable view.
 */
TEST_F(UIFocusNavigation, NoFocusFocusesLast) {
    mWindow->setFocusedView(nullptr);
    pump();
    ASSERT_EQ(focused(), nullptr);

    mWindow->focusPrevView();
    pump();

    EXPECT_EQ(focused(), static_cast<AView*>(v3.get()));
}

/**
 * focusNextView() moves focus to the next focusable view in order.
 */
TEST_F(UIFocusNavigation, SequentialNext) {
    v1->focus();
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(v1.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v2.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v3.get()));
}

/**
 * focusPrevView() moves focus to the previous focusable view in order.
 */
TEST_F(UIFocusNavigation, SequentialPrev) {
    v3->focus();
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(v3.get()));

    mWindow->focusPrevView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v2.get()));

    mWindow->focusPrevView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v1.get()));
}

/**
 * focusNextView() wraps around from the last focusable view back to the first.
 */
TEST_F(UIFocusNavigation, WrapAroundNext) {
    v3->focus();
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(v3.get()));

    mWindow->focusNextView();
    pump();

    EXPECT_EQ(focused(), static_cast<AView*>(v1.get()));
}

/**
 * focusPrevView() wraps around from the first focusable view back to the last.
 */
TEST_F(UIFocusNavigation, WrapAroundPrev) {
    v1->focus();
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(v1.get()));

    mWindow->focusPrevView();
    pump();

    EXPECT_EQ(focused(), static_cast<AView*>(v3.get()));
}

/**
 * Disabled views must be skipped by the focus traversal.
 */
TEST_F(UIFocusNavigation, SkipsDisabled) {
    v2->setEnabled(false);

    v1->focus();
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(v1.get()));

    // v2 is disabled, so next after v1 is v3
    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v3.get()));

    // wraps back to v1
    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v1.get()));

    // from v1, prev wraps to v3 (v2 is disabled)
    mWindow->focusPrevView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v3.get()));
}

/**
 * Invisible (INVISIBLE or GONE) views must be skipped by the focus traversal.
 */
TEST_F(UIFocusNavigation, SkipsInvisible) {
    v2->setVisibility(Visibility::INVISIBLE);

    v1->focus();
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(v1.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v3.get()));

    // GONE must also be skipped
    v2->setVisibility(Visibility::GONE);
    mWindow->focusPrevView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v1.get()));
}

/**
 * Focus traversal must visit nested views in depth-first order and skip non-navigable containers.
 */
TEST_F(UIFocusNavigation, DepthFirstOrder) {
    auto nested1 = _new<FocusableView>();
    auto nested2 = _new<FocusableView>();

    mWindow->setContents(Vertical {
        v1,
        Vertical {
            nested1,
            nested2,
        },
        v2,
        v3,
    });
    uitest::frame();

    v1->focus();
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(v1.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(nested1.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(nested2.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v2.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v3.get()));
}

/**
 * When there are no focusable views, focusNextView()/focusPrevView() must be no-ops and keep focus unset.
 */
TEST_F(UIFocusNavigation, NoFocusableViews) {
    v1->setEnabled(false);
    v2->setEnabled(false);
    v3->setEnabled(false);

    mWindow->setFocusedView(nullptr);
    pump();
    ASSERT_EQ(focused(), nullptr);

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), nullptr);

    mWindow->focusPrevView();
    pump();
    EXPECT_EQ(focused(), nullptr);
}

/**
 * AButton, ACheckBox and ATextField must all participate in the keyboard focus navigation.
 */
class UIFocusNavigationOnControls : public testing::UITest {
protected:
    class TestWindow : public AWindow {
    public:
        _<AButton> button;
        _<ACheckBox> checkBox;
        _<ATextField> textField;
        _<ARadioButton> radioButton;
        int buttonClicks = 0;

        TestWindow() {
            button = _new<AButton>("Button");
            checkBox = _new<ACheckBox>(Label { "Checkbox" });
            textField = _new<ATextField>();
            radioButton = _new<ARadioButton>(Label { "Radio" });

            setContents(Vertical {
                button,
                checkBox,
                textField,
                radioButton,
            });

            AObject::connect(button->clicked, this, [this] { ++buttonClicks; });
        }
    };

    _<TestWindow> mWindow;

    void SetUp() override {
        UITest::SetUp();

        mWindow = _new<TestWindow>();
        mWindow->show();
        uitest::frame();
    }

    [[nodiscard]] AView* focused() const {
        return mWindow->getFocusedView().get();
    }

    static void pump() {
        uitest::frame();
    }
};

/**
 * focusNextView() must focus AButton and RETURN must activate it.
 */
TEST_F(UIFocusNavigationOnControls, ButtonIsFocusedAndActivatedByEnter) {
    mWindow->setFocusedView(nullptr);
    pump();
    ASSERT_EQ(focused(), nullptr);

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(mWindow->button.get()));

    mWindow->onKeyDown(AInput::RETURN);
    EXPECT_EQ(mWindow->buttonClicks, 1);

    mWindow->onKeyDown(AInput::RETURN);
    EXPECT_EQ(mWindow->buttonClicks, 2);
}

/**
 * focusNextView() must focus ACheckBox and RETURN must toggle it.
 */
TEST_F(UIFocusNavigationOnControls, CheckBoxIsFocusedAndToggledByEnter) {
    mWindow->setFocusedView(nullptr);
    pump();

    mWindow->focusNextView(); // button
    pump();
    mWindow->focusNextView(); // checkbox
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(mWindow->checkBox.get()));

    EXPECT_FALSE(*mWindow->checkBox->checked());
    mWindow->onKeyDown(AInput::RETURN);
    EXPECT_TRUE(*mWindow->checkBox->checked());

    mWindow->onKeyDown(AInput::RETURN);
    EXPECT_FALSE(*mWindow->checkBox->checked());
}

/**
 * Focus traversal must cycle through button, checkbox and text field in order.
 */
TEST_F(UIFocusNavigationOnControls, NavigationOrder) {
    mWindow->setFocusedView(nullptr);
    pump();

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(mWindow->button.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(mWindow->checkBox.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(mWindow->textField.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(mWindow->radioButton.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(mWindow->button.get()));
}

/**
 * A focused AButton must draw the theme-color focus outline (like a text field).
 */
TEST_F(UIFocusNavigationOnControls, ButtonDrawsFocusOutline) {
    mWindow->setFocusedView(nullptr);
    pump();

    By::type<AButton>().check(pixelColorAt({0.5f, 0.02f}, 0xcacaca_rgb, 0.3f),
        "button should have the default border when unfocused");

    mWindow->focusNextView();
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(mWindow->button.get()));

    By::type<AButton>().check(pixelColorAt({0.5f, 0.02f}, AStylesheet::getOsThemeColor(), 0.3f),
        "button should draw the focus outline when focused");
}

/**
 * A focused ACheckBox must draw the theme-color focus outline (like a text field).
 */
TEST_F(UIFocusNavigationOnControls, CheckBoxDrawsFocusOutline) {
    mWindow->setFocusedView(nullptr);
    pump();

    mWindow->focusNextView(); // button
    pump();
    mWindow->focusNextView(); // checkbox
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(mWindow->checkBox.get()));

    By::type<ACheckBox>().check(pixelColorAt({0.5f, 0.02f}, AStylesheet::getOsThemeColor(), 0.3f),
        "checkbox should draw the focus outline when focused");
}

/**
 * focusNextView() must focus ARadioButton and RETURN must check it.
 */
TEST_F(UIFocusNavigationOnControls, RadioButtonIsFocusedAndCheckedByEnter) {
    mWindow->setFocusedView(nullptr);
    pump();

    mWindow->focusNextView(); // button
    pump();
    mWindow->focusNextView(); // checkbox
    pump();
    mWindow->focusNextView(); // text field
    pump();
    mWindow->focusNextView(); // radio button
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(mWindow->radioButton.get()));

    EXPECT_FALSE(*mWindow->radioButton->checked());
    mWindow->onKeyDown(AInput::RETURN);
    EXPECT_TRUE(*mWindow->radioButton->checked());
}

/**
 * A focused ARadioButton must draw the theme-color focus outline (like a text field).
 */
TEST_F(UIFocusNavigationOnControls, RadioButtonDrawsFocusOutline) {
    mWindow->setFocusedView(nullptr);
    pump();

    mWindow->focusNextView(); // button
    pump();
    mWindow->focusNextView(); // checkbox
    pump();
    mWindow->focusNextView(); // text field
    pump();
    mWindow->focusNextView(); // radio button
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(mWindow->radioButton.get()));

    By::type<ARadioButton>().check(pixelColorAt({0.5f, 0.02f}, AStylesheet::getOsThemeColor(), 0.3f),
        "radio button should draw the focus outline when focused");
}

/**
 * AListView must participate in the keyboard focus navigation and navigate its items with arrow keys.
 */
class UIFocusNavigationOnListView : public testing::UITest {
protected:
    class TestWindow : public AWindow {
    public:
        _<AListView> listView;
        int selectedRow = -1;
        int selectionCount = 0;

        TestWindow() {
            auto model = _new<AListModel<AString>>();
            for (int i = 0; i < 40; ++i) {
                model->push_back("Item {}"_format(i));
            }
            listView = _new<AListView>(model);
            setContents(Vertical { listView });

            AObject::connect(listView->selectionChanged, this,
                             [this](const AListModelSelection<AString>& selection) {
                                 ++selectionCount;
                                 selectedRow = selection.empty() ? -1 : int(selection.begin().getIndex().getRow());
                             });
        }
    };

    _<TestWindow> mWindow;

    void SetUp() override {
        UITest::SetUp();

        mWindow = _new<TestWindow>();
        mWindow->show();
        uitest::frame();
    }

    [[nodiscard]] AView* focused() const {
        return mWindow->getFocusedView().get();
    }

    static void pump() {
        uitest::frame();
    }

    void focusListView() {
        mWindow->setFocusedView(nullptr);
        pump();
        mWindow->focusNextView();
        pump();
        ASSERT_EQ(focused(), By::text("Item 0").one().get());
    }
};

/**
 * focusNextView() must focus the items inside AListView so that arrow keys move the focus and selection.
 */
TEST_F(UIFocusNavigationOnListView, IsFocusedByTab) {
    focusListView();
}

/**
 * UP and DOWN arrow keys must move the focus and selection within AListView, clamping at the edges.
 */
TEST_F(UIFocusNavigationOnListView, ArrowKeysNavigateSelection) {
    focusListView();
    EXPECT_EQ(mWindow->selectedRow, -1) << "tabbing in must not change the selection";

    mWindow->onKeyDown(AInput::DOWN);
    pump();
    EXPECT_EQ(mWindow->selectedRow, 1);
    EXPECT_EQ(focused(), By::text("Item 1").one().get());

    mWindow->onKeyDown(AInput::DOWN);
    pump();
    EXPECT_EQ(mWindow->selectedRow, 2);

    mWindow->onKeyDown(AInput::UP);
    pump();
    EXPECT_EQ(mWindow->selectedRow, 1);

    // clamping at the top
    mWindow->onKeyDown(AInput::UP);
    mWindow->onKeyDown(AInput::UP);
    mWindow->onKeyDown(AInput::UP);
    pump();
    EXPECT_EQ(mWindow->selectedRow, 0);
    EXPECT_EQ(focused(), By::text("Item 0").one().get());
}

/**
 * The selected item must be scrolled into view ("shown") when it is selected via the keyboard.
 */
TEST_F(UIFocusNavigationOnListView, SelectedItemIsShown) {
    focusListView();

    for (int i = 0; i < 40; ++i) {
        mWindow->onKeyDown(AInput::DOWN);
        pump();
    }
    ASSERT_EQ(mWindow->selectedRow, 39);
    EXPECT_GT(mWindow->listView->scroll().y, 0u) << "the list must scroll to reveal the selected item";

    const auto listTop = mWindow->listView->getPositionInWindow().y;
    const auto listBottom = listTop + mWindow->listView->getHeight();

    const auto item = By::text("Item 39").one();
    ASSERT_TRUE(item != nullptr);
    const auto itemTop = item->getPositionInWindow().y;
    const auto itemBottom = itemTop + item->getHeight();

    EXPECT_GE(itemTop, listTop - 1) << "the selected item must be within the visible area";
    EXPECT_LE(itemBottom, listBottom + 1) << "the selected item must be within the visible area";
}

/**
 * ATreeView must participate in the keyboard focus navigation and support arrow-key navigation,
 * including expanding/collapsing with LEFT/RIGHT.
 */
class UIFocusNavigationOnTreeView : public testing::UITest {
protected:
    class TestWindow : public AWindow {
    public:
        _<ATreeView> treeView;
        _<ATreeModel<AString>> model;
        AString selectedItem;
        int selectionCount = 0;

        TestWindow() {
            AVector<ATreeModel<AString>::Item> items;
            items << ATreeModel<AString>::Item {
                "Parent 1",
                {
                    ATreeModel<AString>::Item { "Child 1", {} },
                    ATreeModel<AString>::Item { "Child 2", {} },
                },
            };
            for (int i = 2; i <= 40; ++i) {
                items << ATreeModel<AString>::Item { "Parent {}"_format(i), {} };
            }
            model = _new<ATreeModel<AString>>(std::move(items));

            treeView = _new<ATreeView>(model);
            setContents(Vertical { treeView });

            AObject::connect(treeView->itemSelected, this, [this](const ATreeModelIndex& index) {
                ++selectionCount;
                selectedItem = model->itemAt(index);
            });
        }
    };

    _<TestWindow> mWindow;

    void SetUp() override {
        UITest::SetUp();

        mWindow = _new<TestWindow>();
        mWindow->show();
        uitest::frame();
    }

    [[nodiscard]] AView* focused() const {
        return mWindow->getFocusedView().get();
    }

    static void pump() {
        uitest::frame();
    }

    void focusTreeView() {
        mWindow->setFocusedView(nullptr);
        pump();
        mWindow->focusNextView();
        pump();
        const auto label = By::text("Parent 1").one();
        ASSERT_NE(label, nullptr);
        ASSERT_EQ(focused(), label->getParent());
    }
};

/**
 * focusNextView() must focus the items inside ATreeView so that arrow keys navigate them.
 */
TEST_F(UIFocusNavigationOnTreeView, IsFocusedByTab) {
    focusTreeView();
}

/**
 * UP/DOWN move the focus and selection across the visible items; RIGHT expands a collapsed group, DOWN enters
 * its children, and LEFT returns to the parent and collapses it.
 */
TEST_F(UIFocusNavigationOnTreeView, ArrowKeysNavigateSelection) {
    focusTreeView();
    EXPECT_EQ(mWindow->selectionCount, 0) << "tabbing in must not change the selection";

    mWindow->onKeyDown(AInput::DOWN);
    pump();
    EXPECT_EQ(mWindow->selectedItem, "Parent 2");

    mWindow->onKeyDown(AInput::DOWN);
    pump();
    EXPECT_EQ(mWindow->selectedItem, "Parent 3");

    mWindow->onKeyDown(AInput::UP);
    pump();
    EXPECT_EQ(mWindow->selectedItem, "Parent 2");

    // back to the parent group
    mWindow->onKeyDown(AInput::UP);
    pump();
    EXPECT_EQ(mWindow->selectedItem, "Parent 1");

    // expand "Parent 1" (was collapsed)
    mWindow->onKeyDown(AInput::RIGHT);
    pump();
    EXPECT_EQ(mWindow->selectedItem, "Parent 1");

    mWindow->onKeyDown(AInput::DOWN);
    pump();
    EXPECT_EQ(mWindow->selectedItem, "Child 1");

    mWindow->onKeyDown(AInput::DOWN);
    pump();
    EXPECT_EQ(mWindow->selectedItem, "Child 2");

    // go back to the parent
    mWindow->onKeyDown(AInput::LEFT);
    pump();
    EXPECT_EQ(mWindow->selectedItem, "Parent 1");

    // collapse "Parent 1"
    mWindow->onKeyDown(AInput::LEFT);
    pump();
    EXPECT_EQ(mWindow->selectedItem, "Parent 1");

    // children are now hidden, so the next DOWN lands on the next root item
    mWindow->onKeyDown(AInput::DOWN);
    pump();
    EXPECT_EQ(mWindow->selectedItem, "Parent 2");
}

/**
 * RETURN must expand a collapsed folder, collapse an expanded folder and merely select a leaf item.
 */
TEST_F(UIFocusNavigationOnTreeView, EnterOpensAndClosesFolder) {
    focusTreeView();

    // RETURN on a leaf ("Parent 2") just selects it
    mWindow->onKeyDown(AInput::DOWN);
    pump();
    ASSERT_EQ(mWindow->selectedItem, "Parent 2");

    mWindow->onKeyDown(AInput::UP); // back to folder "Parent 1"
    pump();

    // RETURN opens the folder
    mWindow->onKeyDown(AInput::RETURN);
    pump();
    mWindow->onKeyDown(AInput::DOWN);
    pump();
    EXPECT_EQ(mWindow->selectedItem, "Child 1");

    // RETURN closes the folder
    mWindow->onKeyDown(AInput::UP); // back to "Parent 1"
    pump();
    mWindow->onKeyDown(AInput::RETURN);
    pump();
    mWindow->onKeyDown(AInput::DOWN);
    pump();
    EXPECT_EQ(mWindow->selectedItem, "Parent 2");
}

/**
 * The selected item must be scrolled into view ("shown") when it is selected via the keyboard.
 */
TEST_F(UIFocusNavigationOnTreeView, SelectedItemIsShown) {
    focusTreeView();

    for (int i = 0; i < 60; ++i) {
        mWindow->onKeyDown(AInput::DOWN);
        pump();
    }
    ASSERT_EQ(mWindow->selectedItem, "Parent 40");
    EXPECT_GT(mWindow->selectionCount, 0);

    const auto treeTop = mWindow->treeView->getPositionInWindow().y;
    const auto treeBottom = treeTop + mWindow->treeView->getHeight();

    const auto item = By::text("Parent 40").one();
    ASSERT_TRUE(item != nullptr);
    const auto itemTop = item->getPositionInWindow().y;
    const auto itemBottom = itemTop + item->getHeight();

    EXPECT_GE(itemTop, treeTop - 1) << "the selected item must be within the visible area";
    EXPECT_LE(itemBottom, treeBottom + 1) << "the selected item must be within the visible area";
}