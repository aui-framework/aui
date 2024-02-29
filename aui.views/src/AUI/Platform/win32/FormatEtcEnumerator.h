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

#include <AUI/Common/AVector.h>
#include "AComBase.h"

/**
 * @brief Enumation interface implementation.
 */
class FormatEtcEnumerator final: public AComBase<FormatEtcEnumerator, IEnumFORMATETC> {
public:
    FormatEtcEnumerator(AVector<FORMATETC> contents) : mContents(std::move(contents)), mCursor(mContents.begin()) {}
    FormatEtcEnumerator(const FormatEtcEnumerator& rhs): mContents(rhs.mContents), mCursor(mContents.begin() + std::distance(rhs.mContents.cbegin(), AVector<FORMATETC>::const_iterator(rhs.mCursor))) {
    }

    HRESULT __stdcall Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched) override;
    HRESULT __stdcall Skip(ULONG celt) override;
    HRESULT __stdcall Reset(void) override;
    HRESULT __stdcall Clone(IEnumFORMATETC** ppenum) override;

private:
    AVector<FORMATETC> mContents;
    AVector<FORMATETC>::iterator mCursor;
};


