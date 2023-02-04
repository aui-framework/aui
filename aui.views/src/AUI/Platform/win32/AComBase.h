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

#pragma once

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


