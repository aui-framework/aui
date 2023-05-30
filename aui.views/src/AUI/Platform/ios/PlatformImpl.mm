// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

#include "AUI/Platform/APlatform.h"
#include "AUI/Common/AString.h"
#include "AUI/IO/APath.h"
#include <AUI/Util/kAUI.h>
#import <UIKit/UIKit.h>



float APlatform::getDpiRatio()
{
    return float([[UIScreen mainScreen] scale]);
}

void APlatform::openUrl(const AUrl &url) {
    @autoreleasepool {
        UIApplication *application = [UIApplication sharedApplication];
        NSURL *URL = [NSURL URLWithString:[NSString stringWithUTF8String: url.full().toStdString().c_str()]];
        [application openURL:URL options:@{} completionHandler:nil];
    }
}
