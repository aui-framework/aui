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

    HRESULT Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched) override;
    HRESULT Skip(ULONG celt) override;
    HRESULT Reset(void) override;
    HRESULT Clone(IEnumFORMATETC** ppenum) override;

private:
    AVector<FORMATETC> mContents;
    AVector<FORMATETC>::iterator mCursor;
};


