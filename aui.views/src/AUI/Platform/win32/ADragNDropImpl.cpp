//
// Created by Alex2772 on 9/15/2022.
//

#include <AUI/Platform/ADragNDrop.h>
#include "Ole.h"
#include "AComBase.h"
#include "AComPtr.h"
#include "ScopedHGlobal.h"
#include "AUI/Logging/ALogger.h"

constexpr auto LOG_TAG = "ADragNDropImpl/win32";

namespace {
    class MyDropSource final: public AComBase<MyDropSource, IDropSource>, public aui::noncopyable {
    private:
        bool mCancelled = false;

    public:
        HRESULT __stdcall QueryContinueDrag(BOOL escapePressed, DWORD keyState) override {
            if (mCancelled) return DRAGDROP_S_CANCEL;

            if (!(keyState & MK_LBUTTON) && !(keyState & MK_RBUTTON)) {
                return DRAGDROP_S_DROP;
            }

            return S_OK;
        }

        HRESULT __stdcall GiveFeedback(DWORD dwEffect) override {
            return DRAGDROP_S_USEDEFAULTCURSORS;
        }
    };
}

void ADragNDrop::perform(ABaseWindow* sourceWindow) {
    Ole::inst();
    auto obj = AComPtr(Ole::fromMime(mData));
    auto source = AComPtr(new MyDropSource);
    DWORD dwEffect;
    DoDragDrop(obj, source, DROPEFFECT_COPY | DROPEFFECT_LINK | DROPEFFECT_MOVE | DROPEFFECT_SCROLL | DROPEFFECT_NONE, &dwEffect);
}
