/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Common/AString.h>

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif

namespace AReflect {
    template<typename T>
    AString name(T* v) {
        if (!v) return "nullptr";
#ifdef _MSC_VER
        return typeid(*v).name();
#else
        int status;
        auto mangledName = typeid(*v).name();
        auto ptr = abi::__cxa_demangle(mangledName, nullptr, nullptr, &status);
        AString result = ptr;
        free(ptr);
        return result;
#endif
    }
}