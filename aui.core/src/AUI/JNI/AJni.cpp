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


#include <AUI/Common/AObject.h>

struct JavaVM;

namespace {
    JavaVM* gJavaVM = nullptr;
}

namespace aui::jni {
    extern "C" API_AUI_CORE JavaVM* javaVM() {
        AUI_ASSERTX(gJavaVM != nullptr, "java vm is not set");
        return gJavaVM;
    }

    extern "C" API_AUI_CORE void setJavaVM(JavaVM* vm) {
        gJavaVM = vm;
    }
}