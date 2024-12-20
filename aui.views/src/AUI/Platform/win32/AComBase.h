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

#include <atomic>
#include <shlwapi.h>
#include <strsafe.h>
#include <shlobj.h>
#include <type_traits>

/**
 * @brief Helper class implementing COM AddRef Release, and QueryInterface.
 * @ingroup windows_specific
 */
template<typename Child, typename Base>
class AComBase: public Base {
public:
    virtual ~AComBase() = default;

    ULONG __stdcall AddRef() override {
        ++mRefCounter;
        return 0;
    }

    ULONG __stdcall Release() override {
        if (--mRefCounter == 0) {
            delete this;
        }
        return 0;
    }

    HRESULT __stdcall QueryInterface(const IID& riid, void** ppv) override {
        static const QITAB qit[] = {
                QITABENT(Child, Base),
                { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

private:
    std::atomic_uint mRefCounter = 1;
};


