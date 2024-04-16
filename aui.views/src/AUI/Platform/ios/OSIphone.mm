// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.


#include "AUI/Util/ACommandLineArgs.h"
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/Entry.h>
#import <UIKit/UIKit.h>
#import "AppDelegate.h"

int(* _gEntry)(const AStringVector&);

const ACommandLineArgs& aui::args() noexcept {
    static ACommandLineArgs args;
    return args; 
}

AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(const AStringVector&)) {
    _gEntry = aui_entry;
    //AThread::setThreadName("UI thread");
    NSString * appDelegateClassName = appDelegateClassName = NSStringFromClass([AppDelegate class]);;

#ifdef AUI_CATCH_UNHANDLED
    extern void aui_init_signal_handler();
    aui_init_signal_handler();
#endif
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
