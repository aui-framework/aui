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

#include "AAndroidSink.h"

#if AUI_PLATFORM_ANDROID
#include <android/log.h>
#endif

AAndroidSink::AAndroidSink() {}

void AAndroidSink::write(const ALogMessage& message) {
#if AUI_PLATFORM_ANDROID
    int prio;
    switch (message.level) {
        case 0: prio = ANDROID_LOG_INFO; break;
        case 1: prio = ANDROID_LOG_WARN; break;
        case 2: prio = ANDROID_LOG_ERROR; break;
        case 3: prio = ANDROID_LOG_DEBUG; break;
        default: prio = ANDROID_LOG_DEBUG; break;
    }
    if (message.message.empty()) {
        __android_log_print(prio, "AUI", "%s", message.prefix.data());
    } else {
        __android_log_print(prio, message.prefix.data(), "%s", message.message.data());
    }
#else
    (void)message;
#endif
}

void AAndroidSink::flush() {}

AStringView AAndroidSink::name() const noexcept {
    return "android";
}
