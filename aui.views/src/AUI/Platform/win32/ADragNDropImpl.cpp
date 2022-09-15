//
// Created by Alex2772 on 9/15/2022.
//

#include <AUI/Platform/ADragNDrop.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <shlobj.h>
#include <new>  // std::nothrow
#include "Ole.h"
#include "AComBase.h"

namespace {
    class MyDataObject: public AComBase<MyDataObject, IDataObject> {
    public:
        HRESULT GetData(FORMATETC* pformatetcIn, STGMEDIUM* pmedium) override {
            return 0;
        }

        HRESULT GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium) override {
            return E_NOTIMPL;
        }

        HRESULT QueryGetData(FORMATETC* pformatetc) override {

            HRESULT hr = S_FALSE;
            if (pformatetc->cfFormat == CF_UNICODETEXT)
            {
                hr = S_OK;
            }
            return hr;
        }

        HRESULT GetCanonicalFormatEtc(FORMATETC* pformatetcIn, FORMATETC* pFormatetcOut) override {

            *pFormatetcOut = *pformatetcIn;
            pFormatetcOut->ptd = nullptr;
            return DATA_S_SAMEFORMATETC;
        }

        HRESULT SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease) override {
            return E_NOTIMPL;
        }

        HRESULT EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc) override {
            *ppenumFormatEtc = NULL;
            HRESULT hr = E_NOTIMPL;
            if (dwDirection == DATADIR_GET)
            {
                FORMATETC rgfmtetc[] =
                        {
                                // the order here defines the accuarcy of rendering
                                { CF_UNICODETEXT,          NULL, 0,  0, TYMED_HGLOBAL },
                        };
                hr = SHCreateStdEnumFmtEtc(ARRAYSIZE(rgfmtetc), rgfmtetc, ppenumFormatEtc);
            }
            return hr;
        }

        HRESULT DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection) override {

            return E_NOTIMPL;
        }

        HRESULT DUnadvise(DWORD dwConnection) override {
            return E_NOTIMPL;
        }

        HRESULT EnumDAdvise(IEnumSTATDATA** ppenumAdvise) override {
            return E_NOTIMPL;
        }

    };

    class MyDropSource: public AComBase<MyDropSource, IDropSource> {
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
    auto obj = new MyDataObject;
    auto source = new MyDropSource;
    DWORD dwEffect;
    auto r = DoDragDrop(obj, source, DROPEFFECT_COPY | DROPEFFECT_LINK | DROPEFFECT_MOVE | DROPEFFECT_SCROLL | DROPEFFECT_NONE, &dwEffect);
    obj->Release();
    source->Release();
}
