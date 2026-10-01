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

#include "AUI/Logging/sinks/AAppleLogSink.h"

#import <Foundation/Foundation.h>
#include <string>

AAppleLogSink::AAppleLogSink() {}

void AAppleLogSink::write(const ALogMessage& message) {
    if (message.message.empty()) {
        NSLog(@"%s", std::string(message.prefix).c_str());
    } else {
        NSLog(@"[%s] %s", std::string(message.prefix).c_str(), std::string(message.message).c_str());
    }
}

void AAppleLogSink::flush() {}

AStringView AAppleLogSink::name() const noexcept {
    return "apple";
}
