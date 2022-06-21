#pragma once

#include "ACurl.h"
#include <AUI/Common/AMap.h>

/**
 * @brief Multi curl instance.
 * @ingroup curl
 * @details
 * Provides support to multiple simultaneous curl requests in a one thread.
 */
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

    void clear();

    [[nodiscard]]
    const AMap<void*, _<ACurl>>& curls() const {
        return mEasyCurls;
    }

private:
    void* mMulti;
    bool mCancelled = false;
    AMap<void*, _<ACurl>> mEasyCurls;
};


