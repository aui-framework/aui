//
// Created by Alex2772 on 9/15/2022.
//

#include <AUI/Platform/ADragNDrop.h>
#include <new>  // std::nothrow
#include "Ole.h"
#include "AComBase.h"
#include "AComPtr.h"
#include "FormatEtcEnumerator.h"
#include "ScopedHGlobal.h"
#include "AUI/Logging/ALogger.h"

constexpr auto LOG_TAG = "ADragNDropImpl/win32";

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

    class MyDataObject final: public AComBase<MyDataObject, IDataObject>, public aui::noncopyable {
    public:
        MyDataObject(const AMimedData& mimedData) {
            if (auto t = mimedData.text()) {
                STGMEDIUM medium = {};
                medium.tymed = TYMED_HGLOBAL;
                auto utf8String = t->toStdString();
                medium.hGlobal = GlobalAlloc(GPTR, utf8String.size() + 1);
                medium.pUnkForRelease = nullptr;
                {
                    ScopedHGlobal hGlobalData(medium.hGlobal);
                    std::memcpy(hGlobalData.data(), utf8String.c_str(), utf8String.size() + 1);
                }

                mContents.push_back({ FORMATETC{ CF_UNICODETEXT, 0, 0, 0, TYMED_HGLOBAL }, medium });
            }
        }

        HRESULT GetData(FORMATETC* inFormatEtc, STGMEDIUM* medium) override {
            for (const auto& content : mContents) {
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

        HRESULT GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium) override {
            return DATA_E_FORMATETC;
        }

        HRESULT QueryGetData(FORMATETC* pformatetc) override {

            for (const auto& content : mContents) {
                if (content.formatEtc.cfFormat == pformatetc->cfFormat) {
                    return S_OK;
                }
            }
            return DV_E_FORMATETC;
        }

        HRESULT GetCanonicalFormatEtc(FORMATETC* pformatetcIn, FORMATETC* pFormatetcOut) override {
            pFormatetcOut->ptd = nullptr;
            return E_NOTIMPL;
        }

        HRESULT SetData(FORMATETC* inFormatEtc, STGMEDIUM* inMedium, BOOL shouldRelease) override {
            /*
             * If `shouldRelease` is true, the ownership of the original data in `inMedium` is transferred to `this`.
             * Otherwise, it remains with the caller. To prevent lifetime issues, we perform a deep copy of `inMedium`.
             */
            auto localMedium = shouldRelease ? *inMedium : duplicateMedium(inFormatEtc->cfFormat, *inMedium);

            mContents.insert(mContents.begin(), // newly inserted data should appear first
                             { *inFormatEtc, localMedium });

            ALogger::debug(LOG_TAG) << "SetData: " << mContents.first().toString();

            return S_OK;
        }

        HRESULT EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc) override {
            if (dwDirection == DATADIR_GET)
            {
                *ppenumFormatEtc = new FormatEtcEnumerator({mContents.begin(), mContents.end()});
                return S_OK;
            }
            return E_NOTIMPL;
        }

        HRESULT DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection) override {
            return OLE_E_ADVISENOTSUPPORTED;
        }

        HRESULT DUnadvise(DWORD dwConnection) override {
            return OLE_E_ADVISENOTSUPPORTED;
        }

        HRESULT EnumDAdvise(IEnumSTATDATA** ppenumAdvise) override {
            return OLE_E_ADVISENOTSUPPORTED;
        }
    private:
        struct Entry: aui::noncopyable {
            FORMATETC formatEtc;
            STGMEDIUM medium;

            Entry(const FORMATETC& formatEtc, const STGMEDIUM& medium) : formatEtc(formatEtc), medium(medium) {}

            Entry(Entry&& rhs) noexcept: formatEtc(rhs.formatEtc), medium(rhs.medium) {
                rhs.medium.tymed = TYMED_NULL;
            }

            Entry& operator=(Entry&& rhs) noexcept{
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

class MyDropSource final: public AComBase<MyDropSource, IDropSource>, public aui::noncopyable {
    private:
        bool mCancelled = false;

    public:
        HRESULT QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) override {
            if (mCancelled) return DRAGDROP_S_CANCEL;

            return S_OK;
        }

        HRESULT GiveFeedback(DWORD dwEffect) override {
            return DRAGDROP_S_USEDEFAULTCURSORS;
        }

        HRESULT QueryInterface(const IID& riid, void** ppv) override {
            static const QITAB qit[] = {
                    QITABENT(MyDataObject, IDataObject),
                    { 0 },
            };
            return QISearch(this, qit, riid, ppv);
        }
    };
}

void ADragNDrop::perform(ABaseWindow* sourceWindow) {
    Ole::inst();
    auto obj = AComPtr(new MyDataObject(mData));
    auto source = AComPtr(new MyDropSource);
    DWORD dwEffect;
    auto r = DoDragDrop(obj, source, DROPEFFECT_COPY | DROPEFFECT_LINK | DROPEFFECT_MOVE | DROPEFFECT_SCROLL | DROPEFFECT_NONE, &dwEffect);
    return;
}
