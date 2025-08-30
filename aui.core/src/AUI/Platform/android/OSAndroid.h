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

#pragma once
#include <AUI/JNI/AJni.h>

namespace com::github::aui::android {
    AUI_JNI_CLASS(com/github/aui/android/Platform, Platform) {
        AUI_JNI_STATIC_METHOD(float, getDpiRatio, ())
        AUI_JNI_STATIC_METHOD(void, openUrl, ((const AString&) url))
        AUI_JNI_STATIC_METHOD(void, requestRedraw, ())
        AUI_JNI_STATIC_METHOD(void, showKeyboard, ((int) keyboardType, (int) keyboardAction, (bool) isPassword))
        AUI_JNI_STATIC_METHOD(void, setMobileScreenOrientation, ((int) orientation))
        AUI_JNI_STATIC_METHOD(void, hideKeyboard, ())
        AUI_JNI_STATIC_METHOD(void, showMessageBox, ((const AString&) title, (const AString&) message))
        AUI_JNI_STATIC_METHOD(void, copyToClipboard, ((const AString&) text))
    };
}

