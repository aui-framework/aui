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
