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

#include "MessageBox.h"

#include <AUI/i18n/AI18n.h>
#include <AppKit/AppKit.h>

#undef YES
#undef NO

using namespace AMessageBox;

ResultButton aui::showMessageBox(AWindow* parent, const AString& title, const AString& message, Icon icon, Button b) {
    @autoreleasepool {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:[NSString stringWithUTF8String:title.c_str()]];
        [alert setInformativeText:[NSString stringWithUTF8String:message.c_str()]];

        switch (icon) {
            case Icon::CRITICAL:
                [alert setAlertStyle:NSAlertStyleCritical];
                break;
            case Icon::WARNING:
                [alert setIcon:[NSImage imageNamed:NSImageNameCaution]];
                [alert setAlertStyle:NSAlertStyleWarning];
                break;
            case Icon::INFO:
                [alert setIcon:[NSImage imageNamed:NSImageNameInfo]];
                [alert setAlertStyle:NSAlertStyleInformational];
                break;
            default:
                break;
        }

        if (b == Button::OK || b == Button::OK_CANCEL) {
            [alert addButtonWithTitle:[NSString stringWithUTF8String:"OK"_i18n.c_str()]];
        } else if (b == Button::YES_NO || b == Button::YES_NO_CANCEL) {
            [alert addButtonWithTitle:[NSString stringWithUTF8String:"Yes"_i18n.c_str()]];
            [alert addButtonWithTitle:[NSString stringWithUTF8String:"No"_i18n.c_str()]];
        }

        if (b == Button::OK_CANCEL || b == Button::YES_NO_CANCEL) {
            [alert addButtonWithTitle:[NSString stringWithUTF8String:"Cancel"_i18n.c_str()]];
        }

        NSWindow *nativeParentWindow = nil;
        if (auto d = dynamic_cast<AWindow *>(parent)) {
            nativeParentWindow = static_cast<NSWindow *>(d->nativeHandle());
        }

        __block NSModalResponse response = NSModalResponseCancel;

        if (nativeParentWindow) {
            [alert
                beginSheetModalForWindow:nativeParentWindow
                       completionHandler:^(NSModalResponse result) {
                         response = result;
                         [NSApp stopModal];
                       }];
            [NSApp runModalForWindow:alert.window];
        } else {
            response = [alert runModal];
        }

        switch (response) {
            case NSAlertFirstButtonReturn:
                if (b == Button::OK || b == Button::OK_CANCEL)
                    return ResultButton::OK;
                if (b == Button::YES_NO || b == Button::YES_NO_CANCEL)
                    return ResultButton::YES;
                return ResultButton::INVALID;
            case NSAlertSecondButtonReturn:
                if (b == Button::OK_CANCEL)
                    return ResultButton::CANCEL;
                if (b == Button::YES_NO || b == Button::YES_NO_CANCEL)
                    return ResultButton::NO;
                return ResultButton::INVALID;
            case NSAlertThirdButtonReturn:
                if (b == Button::YES_NO_CANCEL)
                    return ResultButton::CANCEL;
                return ResultButton::INVALID;
            default:
                return ResultButton::INVALID;
        }
    }
}
