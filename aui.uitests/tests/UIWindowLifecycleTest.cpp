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

#include <gtest/gtest.h>
#include <AUI/UITest.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/AWindowManager.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ATitlebar.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ALabel.h>

using namespace declarative;

namespace {
class UIWindowLifecycle : public testing::UITest {};

bool isManaged(const _<AWindow>& window) {
    return AWindow::getWindowManager().getWindows().contains(window);
}
}   // namespace

TEST_F(UIWindowLifecycle, InitDefaults) {
    AWindow::Init init;
    EXPECT_TRUE(init.resizable);
    EXPECT_FALSE(init.parent);
    EXPECT_TRUE(std::holds_alternative<AWindow::Init::DecorationDefault>(init.decoration));

    auto window = _new<AWindow>();
    EXPECT_TRUE(window->init().resizable);
    EXPECT_EQ(*window->title(), "My window");
    EXPECT_EQ(window->parentWindow(), nullptr);
    EXPECT_FALSE(window->isPopup());
}

TEST_F(UIWindowLifecycle, ConvenienceConstructorFillsInit) {
    auto window = _new<AWindow>("Hello", 300_dp, 200_dp);
    EXPECT_EQ(window->init().title, "Hello");
    EXPECT_EQ(*window->title(), "Hello");
}

TEST_F(UIWindowLifecycle, SetTitle) {
    auto window = _new<AWindow>("Hello");
    window->setTitle("World");
    EXPECT_EQ(*window->title(), "World");
    EXPECT_EQ(window->init().title, "Hello") << "init() is the initial state";
}

TEST_F(UIWindowLifecycle, TitleChangedEmittedOnlyOnChange) {
    auto window = _new<AWindow>("Hello");
    int count = 0;
    AObject::connect(window->titleChanged, AObject::GENERIC_OBSERVER, [&](const AString&) { ++count; });
    window->setTitle("Hello");
    EXPECT_EQ(count, 0);
    window->title() = "World";
    EXPECT_EQ(count, 1);
    EXPECT_EQ(*window->title(), "World");
}

TEST_F(UIWindowLifecycle, WindowTitleFollowsWindow) {
    auto window = _new<AWindow>(AWindow::Init {
      .title = "Hello",
      .decoration = AWindow::Init::DecorationExtended {},
    });
    _<AWindowTitle> title = WindowTitle {}();
    window->setContents(Vertical { Titlebar { title } });
    EXPECT_EQ(*title->text(), "Hello");
    window->setTitle("World");
    EXPECT_EQ(*title->text(), "World");
}

TEST_F(UIWindowLifecycle, TitlebarWithoutContentsShowsTitle) {
    auto window = _new<AWindow>(AWindow::Init {
      .title = "Hello",
      .decoration = AWindow::Init::DecorationExtended {},
    });
    window->setContents(Vertical { Titlebar {} });
    window->show();
    By::type<AWindowTitle>().check(text("Hello"));
}

TEST_F(UIWindowLifecycle, WindowTitleRegistersInTitlebar) {
    auto window = _new<AWindow>(AWindow::Init {
      .decoration = AWindow::Init::DecorationExtended {},
    });
    _<AWindowTitle> title = WindowTitle {}();
    window->setContents(Vertical { Titlebar { Horizontal { title } } });
    window->show();
    auto titlebar = _cast<ATitlebar>(By::type<ATitlebar>().one());
    ASSERT_TRUE(titlebar);
    EXPECT_EQ(titlebar->windowTitle(), title.get());
    EXPECT_EQ(title->isNative(), window->hasNativeWindowTitle());

    title = nullptr;
    window->setContents(Vertical { Titlebar { Label { "No title" } } });
    titlebar = _cast<ATitlebar>(By::type<ATitlebar>().one());
    EXPECT_EQ(titlebar->windowTitle(), nullptr);
}

TEST_F(UIWindowLifecycle, ExtendedWindowHasNoPadding) {
    auto window = _new<AWindow>(AWindow::Init {
      .decoration = AWindow::Init::DecorationExtended {},
    });
    window->setContents(Vertical { Titlebar {} });
    window->show();
    EXPECT_EQ(window->getPadding(), ABoxFields {}) << "contents must extend to the window edges";
}

TEST_F(UIWindowLifecycle, SetResizable) {
    auto window = _new<AWindow>();
    EXPECT_TRUE(window->isResizable());
    window->setResizable(false);
    EXPECT_FALSE(window->isResizable());
    window->setResizable(true);
    EXPECT_TRUE(window->isResizable());

    auto fixed = _new<AWindow>(AWindow::Init { .resizable = false });
    EXPECT_FALSE(fixed->isResizable());
}

TEST_F(UIWindowLifecycle, CloseTwiceEmitsClosedOnce) {
    auto window = _new<AWindow>();
    int closedCount = 0;
    AObject::connect(window->closed, AObject::GENERIC_OBSERVER, [&] { ++closedCount; });
    window->show();
    window->close();
    window->close();
    EXPECT_EQ(closedCount, 1);
    EXPECT_TRUE(window->isClosed());
    EXPECT_FALSE(isManaged(window));
}

TEST_F(UIWindowLifecycle, CloseWithoutShow) {
    auto window = _new<AWindow>();
    int closedCount = 0;
    AObject::connect(window->closed, AObject::GENERIC_OBSERVER, [&] { ++closedCount; });
    window->close();
    window->close();
    EXPECT_EQ(closedCount, 1);
}

TEST_F(UIWindowLifecycle, HideShowIsReversible) {
    auto window = _new<AWindow>();
    window->show();
    EXPECT_TRUE(isManaged(window));
    window->hide();
    EXPECT_TRUE(isManaged(window)) << "hidden window is still alive";
    window->show();
    EXPECT_TRUE(isManaged(window));
    window->hide();
    window->show();
    EXPECT_FALSE(window->isClosed());
}

TEST_F(UIWindowLifecycle, ShowAfterCloseDoesNotResurrect) {
    auto window = _new<AWindow>();
    window->show();
    window->close();
    try {
        window->show();
    } catch (const AException&) {
        // AUI_ASSERTX in debug builds
    }
    EXPECT_FALSE(isManaged(window));
    EXPECT_TRUE(window->isClosed());
}

TEST_F(UIWindowLifecycle, CloseButtonCanBeRefused) {
    class RefusingWindow : public AWindow {
    public:
        void onCloseButtonClicked() override {}
    };
    auto window = _new<RefusingWindow>();
    window->show();
    window->onCloseButtonClicked();
    EXPECT_FALSE(window->isClosed());
    window->close();
    EXPECT_TRUE(window->isClosed());
}

TEST_F(UIWindowLifecycle, ModalWindowClosesBeforeParent) {
    auto parent = _new<AWindow>();
    parent->show();
    auto child = _new<AWindow>(AWindow::Init {
      .parent = AWindow::Init::Parent { .window = *parent, .modal = true },
      .resizable = false,
    });
    EXPECT_EQ(child->parentWindow(), parent.get());
    child->show();
    child->close();
    EXPECT_TRUE(child->isClosed());
    EXPECT_FALSE(parent->isClosed());
}

TEST_F(UIWindowLifecycle, ResizableOnlyIsNotModal) {
    auto parent = _new<AWindow>();
    auto child = _new<AWindow>(AWindow::Init {
      .parent = AWindow::Init::Parent { .window = *parent },
      .resizable = false,
    });
    EXPECT_FALSE(child->init().parent->modal);
}

TEST_F(UIWindowLifecycle, TitlebarCaptionHitTest) {
    auto window = _new<AWindow>(AWindow::Init {
      .decoration = AWindow::Init::DecorationExtended {},
    });
    auto label = _new<ALabel>("Title");
    auto button = _new<AButton>("Button");
    auto dragExcluded = _new<ALabel>("Excluded");
    dragExcluded->addAssName(".override-title-dragging");
    auto outside = _new<ALabel>("Outside");
    window->setContents(Vertical {
      Titlebar { Horizontal { label, button, dragExcluded } },
      outside,
    });
    window->show();
    window->pack();
    window->setSize({ 400, 300 });
    window->layout(glm::ivec2(0), glm::ivec2(400, 300));

    auto center = [](const _<AView>& v) { return v->getPositionInWindow() + v->getSize() / 2; };
    EXPECT_TRUE(window->isCaptionAt(center(label)));
    EXPECT_FALSE(window->isCaptionAt(center(button)));
    EXPECT_FALSE(window->isCaptionAt(center(dragExcluded)));
    EXPECT_FALSE(window->isCaptionAt(center(outside)));
}

TEST_F(UIWindowLifecycle, TitlebarHeightIsMaxOfContentsAndControls) {
    auto window = _new<AWindow>(AWindow::Init {
      .decoration = AWindow::Init::DecorationExtended {},
    });
    window->windowControlsInsets = AWindowControlsInsets { .left = 70, .right = 0, .height = 40 };
    auto content = _new<ALabel>("Title");
    _<AView> titlebar = Titlebar { content };
    window->setContents(Vertical { titlebar, SpacerExpanding {} });
    window->show();
    window->layout(glm::ivec2(0), glm::ivec2(400, 300));
    EXPECT_GE(titlebar->getHeight(), content->getHeight());
}

TEST_F(UIWindowLifecycle, SafeAreaTopFollowsWindowControlsHeight) {
    auto window = _new<AWindow>(AWindow::Init {
      .decoration = AWindow::Init::DecorationExtended {},
    });
    window->windowControlsInsets = AWindowControlsInsets { .left = 70, .right = 0, .height = 40 };
#if !(AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS)
    EXPECT_EQ(window->safeAreaInsets->top, 40);
    EXPECT_EQ(window->safeAreaInsets->left, 0);
#endif
}

TEST_F(UIWindowLifecycle, SafeAreaIsZeroForDefaultDecoration) {
    auto window = _new<AWindow>();
    window->show();
    window->layout(glm::ivec2(0), glm::ivec2(400, 300));
    EXPECT_EQ(window->safeAreaInsets.value(), ABoxFields {});
}

#if !(AUI_PLATFORM_MACOS || AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS || AUI_PLATFORM_EMSCRIPTEN)
TEST_F(UIWindowLifecycle, DrawnWindowControlsAreReportedInInsets) {
    auto window = _new<AWindow>(AWindow::Init {
      .decoration = AWindow::Init::DecorationExtended {},
    });
    if (window->hasNativeWindowControls()) {
        GTEST_SKIP() << "native controls";
    }
    window->setContents(Vertical { Titlebar { _new<ALabel>("Title") }, SpacerExpanding {} });
    window->show();
    window->layout(glm::ivec2(0), glm::ivec2(400, 300));
    window->redraw();
    EXPECT_GT(window->windowControlsInsets->right, 0);
    EXPECT_GT(window->windowControlsInsets->height, 0);
    EXPECT_EQ(window->windowControlsInsets->left, 0);
    EXPECT_EQ(window->safeAreaInsets->top, window->windowControlsInsets->height);
}
#endif
