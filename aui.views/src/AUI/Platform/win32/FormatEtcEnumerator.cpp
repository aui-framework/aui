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

//
// Created by Alex2772 on 9/15/2022.
//

#include "FormatEtcEnumerator.h"
#include <glm/glm.hpp>

HRESULT FormatEtcEnumerator::Next(ULONG count, FORMATETC* out, ULONG* outElementsFetched) {
    auto initialCursorPos = mCursor;
    for (; count > 0 && mCursor != mContents.end(); --count, ++out, ++mCursor) {
        *out = *mCursor;
    }
    if (outElementsFetched) *outElementsFetched = std::distance(initialCursorPos, mCursor);

    return count == 0 ? S_OK : S_FALSE;
}

HRESULT FormatEtcEnumerator::Skip(ULONG celt) {
    mCursor += glm::min(std::size_t(celt), std::size_t(std::distance(mCursor, mContents.end())));
    return mCursor == mContents.end() ? S_FALSE : S_OK;
}

HRESULT FormatEtcEnumerator::Reset(void) {
    mCursor = mContents.begin();
    return S_OK;
}

HRESULT FormatEtcEnumerator::Clone(IEnumFORMATETC** ppenum) {
    auto c = new FormatEtcEnumerator(*this);
    *ppenum = c;
    return S_OK;
}
