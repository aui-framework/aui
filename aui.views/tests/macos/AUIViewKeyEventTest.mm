/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2026 Alex2772 and Contributors
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
#include <AUI/Platform/AInput.h>
// Cocoa defines the Nil macro that breaks range-v3; keep it after AUI headers
#import <Cocoa/Cocoa.h>
#include "AUI/Platform/macos/AUIView.h"

/**
 * Feeds synthetic NSEvents to AUIView. Neither a native window nor a screen is involved, so nothing is shown.
 */

@interface MenuProbe : NSObject
@property(nonatomic) int fired;
- (void)fire:(id)sender;
@end

@implementation MenuProbe
- (void)fire:(id)sender {
    _fired += 1;
}
@end

namespace {

constexpr unsigned short KEY_CODE_A = 0;   // kVK_ANSI_A
constexpr unsigned short KEY_CODE_Q = 12;  // kVK_ANSI_Q

class RecordingWindow : public AWindow {
public:
    RecordingWindow() : AWindow("recording", 100_px, 100_px) {}

    void onKeyDown(AInput::Key key) override { downs.push_back(key); }
    void onKeyUp(AInput::Key key) override { ups.push_back(key); }

    std::vector<AInput::Key> downs;
    std::vector<AInput::Key> ups;
};

NSEvent* keyEvent(NSEventType type, NSEventModifierFlags flags, unsigned short keyCode, NSString* characters) {
    return [NSEvent keyEventWithType:type
                            location:NSZeroPoint
                       modifierFlags:flags
                           timestamp:0
                        windowNumber:0
                             context:nil
                          characters:characters
         charactersIgnoringModifiers:characters
                           isARepeat:NO
                             keyCode:keyCode];
}

}   // namespace

class AUIViewKeyEventTest : public testing::UITest {
protected:
    void SetUp() override {
        UITest::SetUp();
        mWindow = _new<RecordingWindow>();
        mView = [[AUIView alloc] initWithWindow:mWindow.get()];
        // plain NSApplication (not MacosApp): only to have a main menu; stays invisible without activation policy
        [NSApplication sharedApplication];
    }

    void TearDown() override {
        [NSApp setMainMenu:nil];
        mView = nil;
        UITest::TearDown();
    }

    /**
     * Installs a main menu with a single Cmd+<key> item; the returned probe counts activations.
     */
    MenuProbe* installMainMenu(NSString* keyEquivalent) {
        auto probe = [MenuProbe new];
        mProbe = probe;
        auto mainMenu = [NSMenu new];
        auto topItem = [NSMenuItem new];
        auto submenu = [NSMenu new];
        auto item = [[NSMenuItem alloc] initWithTitle:@"Probe" action:@selector(fire:) keyEquivalent:keyEquivalent];
        [item setTarget:probe];
        [submenu addItem:item];
        [topItem setSubmenu:submenu];
        [mainMenu addItem:topItem];
        [NSApp setMainMenu:mainMenu];
        return probe;
    }

    _<RecordingWindow> mWindow;
    AUIView* mView = nil;
    MenuProbe* mProbe = nil;
};

TEST_F(AUIViewKeyEventTest, KeyUpIsDeliveredAfterKeyDown) {
    // regression: AUIView had no keyUp:, releases of regular keys never reached AWindow::onKeyUp
    // AppKit offers every key press to performKeyEquivalent: first, and AUIView consumes it
    [mView performKeyEquivalent:keyEvent(NSEventTypeKeyDown, 0, KEY_CODE_A, @"a")];
    [mView keyUp:keyEvent(NSEventTypeKeyUp, 0, KEY_CODE_A, @"a")];

    const auto a = AInput::fromNative(KEY_CODE_A);
    ASSERT_EQ(mWindow->downs.size(), 1u);
    EXPECT_EQ(mWindow->downs[0], a);
    ASSERT_EQ(mWindow->ups.size(), 1u);
    EXPECT_EQ(mWindow->ups[0], a);
}

TEST_F(AUIViewKeyEventTest, KeyUpWithoutKeyDownIsIgnored) {
    [mView keyUp:keyEvent(NSEventTypeKeyUp, 0, KEY_CODE_A, @"a")];
    EXPECT_TRUE(mWindow->ups.empty());
}

TEST_F(AUIViewKeyEventTest, CommandKeyReachesMainMenuFirst) {
    // regression: performKeyEquivalent returned YES unconditionally, so Cmd+Q never fired the menu item
    auto probe = installMainMenu(@"q");
    const BOOL handled = [mView performKeyEquivalent:keyEvent(NSEventTypeKeyDown, NSEventModifierFlagCommand,
                                                              KEY_CODE_Q, @"q")];
    EXPECT_TRUE(handled);
    EXPECT_EQ(probe.fired, 1);
    EXPECT_TRUE(mWindow->downs.empty()) << "a combination consumed by the menu must not reach the window";
}

TEST_F(AUIViewKeyEventTest, CommandKeyNotInMenuIsForwardedToWindow) {
    auto probe = installMainMenu(@"q");
    const BOOL handled = [mView performKeyEquivalent:keyEvent(NSEventTypeKeyDown, NSEventModifierFlagCommand,
                                                              KEY_CODE_A, @"a")];
    EXPECT_TRUE(handled);
    EXPECT_EQ(probe.fired, 0);
    ASSERT_EQ(mWindow->downs.size(), 1u);
    EXPECT_EQ(mWindow->downs[0], AInput::fromNative(KEY_CODE_A));
}
