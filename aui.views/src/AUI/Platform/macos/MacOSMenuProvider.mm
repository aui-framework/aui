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

#include "MacOSMenuProvider.h"
#include <AUI/Action/AMenu.h>
#include <AUI/Action/AShortcut.h>
#include <AUI/Image/IDrawable.h>
#include <AUI/Image/AImage.h>
#include <AUI/Platform/AInput.h>

#import <Cocoa/Cocoa.h>

#include <functional>
#include <utility>

namespace {

// Trampoline so an NSMenuItem target/action pair can invoke a std::function.
}

@interface AUIMenuTarget : NSObject
- (instancetype)initWithCallback:(std::function<void()>)cb;
- (void)invoke:(id)sender;
@end

@implementation AUIMenuTarget {
    std::function<void()> _cb;
}
- (instancetype)initWithCallback:(std::function<void()>)cb {
    if ((self = [super init])) {
        _cb = std::move(cb);
    }
    return self;
}
- (void)invoke:(id)sender {
    if (_cb) _cb();
}
@end

namespace {

NSString* toNs(const AString& s) {
    const std::string utf8 = s.toStdString();
    return [[NSString alloc] initWithBytes:utf8.data()
                                    length:utf8.size()
                                  encoding:NSUTF8StringEncoding];
}

// Convert an AShortcut into an NSMenuItem keyEquivalent + modifierMask pair.
// AUI's shortcut uses Ctrl as the "command" modifier; on macOS we map Ctrl→⌘ for
// idiomatic menu display. This is best-effort — more exotic keys fall through as
// no key equivalent (the menu still works via the stored onAction callback).
void applyShortcut(NSMenuItem* item, const AShortcut& shortcut) {
    const AString& display = shortcut.toString();
    if (display.empty()) return;

    // Parse the last character as the key (common pattern: "Ctrl+Shift+S" → 'S').
    const auto plus = display.rfind('+');
    const AString keyPart = (plus == AString::NPOS) ? display : display.substr(plus + 1);
    if (keyPart.empty()) return;

    NSString* keyNs = toNs(keyPart.lowercase());
    [item setKeyEquivalent:keyNs];

    NSEventModifierFlags mods = 0;
    if (display.contains("Ctrl"))  mods |= NSEventModifierFlagCommand;
    if (display.contains("Shift")) mods |= NSEventModifierFlagShift;
    if (display.contains("Alt"))   mods |= NSEventModifierFlagOption;
    if (display.contains("Meta"))  mods |= NSEventModifierFlagControl;
    [item setKeyEquivalentModifierMask:mods];
}

// Convert an AUI IDrawable to an NSImage suitable for a menu item (16pt nominal).
// Returns nil on failure; menu will render without an icon in that case.
NSImage* drawableToNSImage(const _<IDrawable>& drawable) {
    if (!drawable) return nil;
    const glm::ivec2 size { 32, 32 };   // 16pt @ 2x for Retina clarity
    AImage rasterized = drawable->rasterize(size);
    const auto buf = rasterized.buffer();
    if (buf.empty()) return nil;

    const int w = rasterized.width();
    const int h = rasterized.height();
    const int bytesPerRow = w * 4;

    NSBitmapImageRep* rep = [[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes:nullptr
                      pixelsWide:w
                      pixelsHigh:h
                   bitsPerSample:8
                 samplesPerPixel:4
                        hasAlpha:YES
                        isPlanar:NO
                  colorSpaceName:NSDeviceRGBColorSpace
                     bytesPerRow:bytesPerRow
                    bitsPerPixel:32];
    if (!rep) return nil;
    if (int(buf.size()) < bytesPerRow * h) return nil;
    std::memcpy([rep bitmapData], buf.data(), bytesPerRow * h);

    NSImage* image = [[NSImage alloc] initWithSize:NSMakeSize(16, 16)];
    [image addRepresentation:rep];
    [image setTemplate:NO];
    return image;
}

NSMenu* buildMenu(const AVector<AMenuItem>& items, NSMutableArray* keepAlive) {
    NSMenu* menu = [[NSMenu alloc] init];
    [menu setAutoenablesItems:NO];

    for (const auto& item : items) {
        switch (item.type) {
            case AMenu::SEPARATOR: {
                [menu addItem:[NSMenuItem separatorItem]];
                break;
            }
            case AMenu::SUBLIST: {
                NSMenuItem* mi = [[NSMenuItem alloc] initWithTitle:toNs(item.name)
                                                            action:nil
                                                     keyEquivalent:@""];
                [mi setSubmenu:buildMenu(item.subItems, keepAlive)];
                [mi setEnabled:item.enabled];
                if (NSImage* icon = drawableToNSImage(item.icon)) {
                    [mi setImage:icon];
                }
                [menu addItem:mi];
                break;
            }
            case AMenu::SINGLE: {
                AUIMenuTarget* tgt = [[AUIMenuTarget alloc] initWithCallback:item.onAction];
                [keepAlive addObject:tgt];

                NSMenuItem* mi = [[NSMenuItem alloc] initWithTitle:toNs(item.name)
                                                            action:@selector(invoke:)
                                                     keyEquivalent:@""];
                [mi setTarget:tgt];
                [mi setEnabled:item.enabled];
                applyShortcut(mi, item.shortcut);
                if (NSImage* icon = drawableToNSImage(item.icon)) {
                    [mi setImage:icon];
                }
                [menu addItem:mi];
                break;
            }
        }
    }
    return menu;
}

}   // namespace

MacOSMenuProvider::MacOSMenuProvider() = default;

MacOSMenuProvider::~MacOSMenuProvider() {
    closeMenu();
}

void MacOSMenuProvider::createMenu(const AVector<AMenuItem>& vector) {
    closeMenu();

    NSMutableArray* keepAlive = [[NSMutableArray alloc] init];
    NSMenu* menu = buildMenu(vector, keepAlive);

    // Retain both objects across the modal tracking loop; release after popUp returns.
    mTrampolines = (__bridge_retained void*) keepAlive;
    mCurrentMenu = (__bridge_retained void*) menu;

    mOpen = true;
    const NSPoint loc = [NSEvent mouseLocation];   // screen coords (bottom-left origin)
    [menu popUpMenuPositioningItem:nil atLocation:loc inView:nil];
    mOpen = false;

    if (mCurrentMenu) {
        CFRelease(mCurrentMenu);
        mCurrentMenu = nullptr;
    }
    if (mTrampolines) {
        CFRelease(mTrampolines);
        mTrampolines = nullptr;
    }
}

void MacOSMenuProvider::closeMenu() {
    if (!mCurrentMenu) return;
    NSMenu* menu = (__bridge NSMenu*) mCurrentMenu;
    [menu cancelTracking];
    // `popUpMenuPositioningItem` is modal; cancelTracking unblocks it and the release
    // happens on the return path from createMenu.
}

bool MacOSMenuProvider::isOpen() {
    return mOpen;
}
