#pragma once

#include "ACurl.h"
#include <AUI/Common/AMap.h>

class API_AUI_CURL ACurlMulti: public aui::noncopyable {
public:
    ACurlMulti() noexcept;
    ~ACurlMulti();

    ACurlMulti(ACurlMulti&& other) noexcept: mMulti(other.mMulti) {
        other.mMulti = nullptr;
    }

    ACurlMulti& operator<<(_<ACurl> curl);
    ACurlMulti& operator>>(const _<ACurl>& curl);

    void run();

    void cancel() {
        mCancelled = true;
    }

    [[nodiscard]]
    const AMap<void*, _<ACurl>>& curls() const {
        return mEasyCurls;
    }

private:
    void* mMulti;
    bool mCancelled = false;
    AMap<void*, _<ACurl>> mEasyCurls;
};


