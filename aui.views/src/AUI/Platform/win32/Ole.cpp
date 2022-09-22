
// Created by Alex2772 on 9/15/2022.
//

#include <Windows.h>
#include <cassert>
#include <ShlObj_core.h>
#include "Ole.h"
#include "AComPtr.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Util/ARaiiHelper.h"
#include "ScopedHGlobal.h"
#include "AUI/Url/AUrl.h"

Ole::Ole() {
    OleInitialize(0);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    assert(SUCCEEDED(hr));
}

Ole::~Ole() {
    CoUninitialize();
    OleUninitialize();
}

Ole& Ole::inst() {
    static Ole ole;
    return ole;
}

AMimedData Ole::toMime(IDataObject* dataObject) {
    AMimedData result;
    AComPtr<IEnumFORMATETC> enumerator;
    auto r = dataObject->EnumFormatEtc(DATADIR_GET, &enumerator);
    if (!enumerator || r != S_OK) {
        return result;
    }

    FORMATETC formatEtc;
    while (enumerator->Next(1, &formatEtc, nullptr) == S_OK) {
        switch (formatEtc.cfFormat) {
            case CF_UNICODETEXT: {
                STGMEDIUM medium;
                if (dataObject->GetData(&formatEtc, &medium) != S_OK) {
                    break;
                }
                ARaiiHelper release = [&] { ReleaseStgMedium(&medium); };
                if (medium.tymed != TYMED_HGLOBAL) break;

                ScopedHGlobal accessor(medium.hGlobal);
                std::wstring text((const wchar_t*)accessor.data(), accessor.size());

                result.setText(text);
                break;
            }

            case CF_HDROP:
                STGMEDIUM medium;
                if (dataObject->GetData(&formatEtc, &medium) != S_OK) {
                    break;
                }
                ARaiiHelper release = [&] { ReleaseStgMedium(&medium); };
                if (medium.tymed != TYMED_HGLOBAL) break;

                ScopedHGlobal accessor(medium.hGlobal);
                auto begin = accessor.data();
                auto end = reinterpret_cast<const wchar_t*>(begin + accessor.size());
                auto dropFiles = reinterpret_cast<LPDROPFILES>(accessor.data());

                // it's so safe to access the blob!
                begin += sizeof(DROPFILES);
                AVector<AUrl> pathList;
                for (auto current = reinterpret_cast<const wchar_t*>(begin); current < end;) {
                    auto nullTerminator = std::find(current, end, wchar_t('\0'));
                    if (nullTerminator == end || current == nullTerminator) break;
                    pathList << AUrl::file(APath(current, nullTerminator - current));
                    current = nullTerminator + 1;
                }

                result.setUrls(pathList);

                break;
        }
    }
    return result;
}
