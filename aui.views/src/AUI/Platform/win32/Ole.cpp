
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

// Created by Alex2772 on 9/15/2022.
//

#include <Windows.h>
#include <cassert>
#if AUI_COMPILER_GCC
#include <shlobj.h>
#else
#include <ShlObj_core.h>
#endif
#include "Ole.h"
#include "AComPtr.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Util/ARaiiHelper.h"
#include "ScopedHGlobal.h"
#include "AUI/Url/AUrl.h"
#include "AComBase.h"
#include "FormatEtcEnumerator.h"


constexpr auto LOG_TAG = "Ole/win32";

Ole::Ole() {
    OleInitialize(0);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    AUI_ASSERT(SUCCEEDED(hr));
}

Ole::~Ole() {
    CoUninitialize();
    OleUninitialize();
}

Ole& Ole::inst() {
    thread_local Ole ole;
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
                AString text((const char16_t*)accessor.data(), accessor.size());

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
                begin += dropFiles->pFiles;
                AVector<AUrl> pathList;
                for (auto current = reinterpret_cast<const wchar_t*>(begin); current < end;) {
                    auto nullTerminator = std::find(current, end, wchar_t('\0'));
                    if (nullTerminator == end || current == nullTerminator) break;
                    pathList << AUrl::file(APath((char16_t*)current, nullTerminator - current));
                    current = nullTerminator + 1;
                }

                result.setUrls(pathList);

                break;
        }
    }
    return result;
}

namespace {
    static STGMEDIUM duplicateMedium(CLIPFORMAT format, const STGMEDIUM& storage) {
        STGMEDIUM copied = {0};
        switch (storage.tymed) {
            case TYMED_HGLOBAL:
                copied.hGlobal = static_cast<HGLOBAL>(OleDuplicateData(storage.hGlobal, format, 0));
                break;
            case TYMED_MFPICT:
                copied.hMetaFilePict = static_cast<HMETAFILEPICT>(OleDuplicateData(storage.hMetaFilePict, format, 0));
                break;
            case TYMED_GDI:
                copied.hBitmap = static_cast<HBITMAP>(OleDuplicateData(storage.hBitmap, format, 0));
                break;
            case TYMED_ENHMF:
                copied.hEnhMetaFile = static_cast<HENHMETAFILE>(OleDuplicateData(storage.hEnhMetaFile, format, 0));
                break;
            case TYMED_FILE:
                copied.lpszFileName = static_cast<LPOLESTR>(OleDuplicateData(storage.lpszFileName, format, 0));
                break;
            case TYMED_ISTREAM:
                copied.pstm = storage.pstm;
                copied.pstm->AddRef();
                break;
            case TYMED_ISTORAGE:
                copied.pstg = storage.pstg;
                copied.pstg->AddRef();
                break;
        }

        copied.tymed = storage.tymed;
        copied.pUnkForRelease = storage.pUnkForRelease;
        if (copied.pUnkForRelease)
            copied.pUnkForRelease->AddRef();

        return copied;
    }

    class MyDataObject final : public AComBase<MyDataObject, IDataObject>, public aui::noncopyable {
    public:
        MyDataObject(const AMimedData& mimedData) {
            if (auto t = mimedData.text()) {
                STGMEDIUM medium = {};
                medium.tymed = TYMED_HGLOBAL;
                std::wstring utf16String(t->begin(), t->end());
                std::size_t lengthInBytes = (utf16String.size() + 1) * sizeof(wchar_t);
                medium.hGlobal = GlobalAlloc(GPTR, lengthInBytes);
                medium.pUnkForRelease = nullptr;
                {
                    ScopedHGlobal hGlobalData(medium.hGlobal);
                    std::memcpy(hGlobalData.data(), utf16String.c_str(), lengthInBytes);
                }

                mContents.push_back({FORMATETC{CF_UNICODETEXT, nullptr, 1, -1, TYMED_HGLOBAL}, medium});
            }
            if (auto t = mimedData.urls()) {

                AByteBuffer buffer;
                auto dropFiles = DROPFILES {
                        static_cast<DWORD>(sizeof(DROPFILES)),
                        {0, 0},
                        false,
                        true,
                };
                buffer << aui::serialize_raw(dropFiles);

                for (const auto& url : *t) {
                    auto path = url.path();
                    std::wstring str(path.begin(), path.end());
                    buffer.write(reinterpret_cast<const char*>(str.data()), (str.length() + 1) * sizeof(wchar_t));
                }

                {
                    wchar_t zero = 0;
                    buffer << zero;
                }
                STGMEDIUM medium = {};
                medium.tymed = TYMED_HGLOBAL;
                medium.hGlobal = GlobalAlloc(GPTR, buffer.size());
                medium.pUnkForRelease = nullptr;
                {
                    ScopedHGlobal hGlobalData(medium.hGlobal);
                    std::memcpy(hGlobalData.data(), buffer.data(), buffer.size());
                }

                mContents.push_back({FORMATETC{CF_HDROP, nullptr, 1, -1, TYMED_HGLOBAL}, medium});
            }
        }

        HRESULT __stdcall GetData(FORMATETC* inFormatEtc, STGMEDIUM* medium) override {
            for (const auto& content: mContents) {
                if (content.formatEtc.cfFormat != inFormatEtc->cfFormat ||
                    content.formatEtc.lindex != inFormatEtc->lindex ||
                    !(content.formatEtc.tymed & inFormatEtc->tymed)) {
                    continue;
                }
                // If medium is NULL, delay-rendering will be used.
                if (content.medium.tymed != TYMED_NULL) {
                    *medium = duplicateMedium(content.formatEtc.cfFormat, content.medium);

                    ALogger::debug(LOG_TAG) << "GetData: " << content.toString();
                    return S_OK;
                }
            }
            return DV_E_FORMATETC;
        }

        HRESULT __stdcall GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium) override {
            return DATA_E_FORMATETC;
        }

        HRESULT __stdcall QueryGetData(FORMATETC* pformatetc) override {
            for (const auto& content: mContents) {
                if (content.formatEtc.cfFormat == pformatetc->cfFormat) {
                    ALogger::debug(LOG_TAG) << "QueryGetData: " << content.toString();
                    return S_OK;
                }
            }
            //ALogger::debug(LOG_TAG) << "QueryGetData: DV_E_FORMATETC(" << pformatetc->cfFormat << ")";
            return DV_E_FORMATETC;
        }

        HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC* pformatetcIn, FORMATETC* pFormatetcOut) override {
            pFormatetcOut->ptd = nullptr;
            return E_NOTIMPL;
        }

        HRESULT __stdcall SetData(FORMATETC* inFormatEtc, STGMEDIUM* inMedium, BOOL shouldRelease) override {
            /*
             * If `shouldRelease` is true, the ownership of the original data in `inMedium` is transferred to `this`.
             * Otherwise, it remains with the caller. To prevent lifetime issues, we perform a deep copy of `inMedium`.
             */
            auto localMedium = shouldRelease ? *inMedium : duplicateMedium(inFormatEtc->cfFormat, *inMedium);

            mContents.insert(mContents.begin(), // newly inserted data should appear first
                             {*inFormatEtc, localMedium});

            ALogger::debug(LOG_TAG) << "SetData: " << mContents.first().toString();

            return S_OK;
        }

        HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc) override {
            if (dwDirection == DATADIR_GET) {
                *ppenumFormatEtc = new FormatEtcEnumerator({mContents.begin(), mContents.end()});
                return S_OK;
            }
            return E_NOTIMPL;
        }

        HRESULT __stdcall DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection) override {
            return OLE_E_ADVISENOTSUPPORTED;
        }

        HRESULT __stdcall DUnadvise(DWORD dwConnection) override {
            return OLE_E_ADVISENOTSUPPORTED;
        }

        HRESULT __stdcall EnumDAdvise(IEnumSTATDATA** ppenumAdvise) override {
            return OLE_E_ADVISENOTSUPPORTED;
        }

    private:
        struct Entry : aui::noncopyable {
            FORMATETC formatEtc;
            STGMEDIUM medium;

            Entry(const FORMATETC& formatEtc, const STGMEDIUM& medium) : formatEtc(formatEtc), medium(medium) {}

            Entry(Entry&& rhs) noexcept: formatEtc(rhs.formatEtc), medium(rhs.medium) {
                rhs.medium.tymed = TYMED_NULL;
            }

            Entry& operator=(Entry&& rhs) noexcept {
                formatEtc = rhs.formatEtc;
                medium = rhs.medium;

                rhs.medium.tymed = TYMED_NULL;
                return *this;
            }

            ~Entry() {
                if (isNull()) {
                    ReleaseStgMedium(&medium);
                }
            }

            [[nodiscard]]
            bool isNull() const noexcept { return medium.tymed != TYMED_NULL; }

            operator FORMATETC() const noexcept {
                return formatEtc;
            }


            AString toString() const {
                AString contentDebugStr;
                switch (medium.tymed) {
                    case TYMED_HGLOBAL: {
                        ScopedHGlobal accessor(medium.hGlobal);
                        contentDebugStr = "TYMED_HGLOBAL(\"{}\")"_format(accessor.data());
                        break;
                    }
                    case TYMED_MFPICT:
                        contentDebugStr = "TYMED_MFPICT";
                        break;
                    case TYMED_GDI:
                        contentDebugStr = "TYMED_GDI";
                        break;
                    case TYMED_ENHMF:
                        contentDebugStr = "TYMED_ENHMF";
                        break;
                    case TYMED_FILE:
                        contentDebugStr = "TYMED_FILE";
                        break;
                    case TYMED_ISTREAM:
                        contentDebugStr = "TYMED_ISTREAM";
                        break;
                    case TYMED_ISTORAGE:
                        contentDebugStr = "TYMED_ISTORAGE";
                        break;
                }
                return "CF_FORMAT = {}, content = {}"_format(formatEtc.cfFormat, contentDebugStr);
            }
        };

        AVector<Entry> mContents;
    };
}

IDataObject* Ole::fromMime(const AMimedData& data) {
    return new MyDataObject(data);
}
