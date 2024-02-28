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

#pragma once
#include <AUI/JNI/AJni.h>

namespace com::github::aui::android {
    AUI_JNI_CLASS(com/github/aui/android/Platform, Platform) {
        AUI_JNI_STATIC_METHOD(float, getDpiRatio, ())
        AUI_JNI_STATIC_METHOD(void, openUrl, ((const AString&) url))
        AUI_JNI_STATIC_METHOD(void, requestRedraw, ())
        AUI_JNI_STATIC_METHOD(void, showKeyboard, ())
        AUI_JNI_STATIC_METHOD(void, hideKeyboard, ())
        AUI_JNI_STATIC_METHOD(void, showMessageBox, ((const AString&) title, (const AString&) message))
    };
}

