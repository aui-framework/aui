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


#include "AUI/Util/ACommandLineArgs.h"
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/Entry.h>
#import <UIKit/UIKit.h>
#import "AUIAppDelegate.h"

int(* _gEntry)(const AStringVector&);

const ACommandLineArgs& aui::args() noexcept {
    static ACommandLineArgs args;
    return args; 
}

AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(const AStringVector&)) {
    _gEntry = aui_entry;
    //AThread::setThreadName("UI thread");
    NSString * appDelegateClassName = appDelegateClassName = NSStringFromClass([AUIAppDelegate class]);;

    {
        [[maybe_unused]] auto mainThread = AThread::main();
    }

#ifdef AUI_CATCH_UNHANDLED
    extern void aui_init_signal_handler();
    aui_init_signal_handler();
#endif
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
