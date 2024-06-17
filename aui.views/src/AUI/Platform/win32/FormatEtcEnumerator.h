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


