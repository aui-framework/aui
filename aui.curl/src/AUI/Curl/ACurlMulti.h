#pragma once

#include "ACurl.h"
#include <AUI/Common/AMap.h>

/**
 * @brief Multi curl instance.
 * @ingroup curl
 * @details
 * Provides support to multiple simultaneous curl requests in a one thread.
 *
 * Analogous to Qt's QNetworkAccessManager.
 *
 * All calls are processed by enqueueing them on ACurlMulti's thread, so the underlying curl handle is used by the
 * single thread. This means that the following code will fail:
 *
 * <code>
 * auto m = _new<ACurlMulti>();
 * m << _new<ACurl>(...);
 * assert(!m->empty()); // assertion failure here
 * </code>
 *
 * Use AThread::processMessages() function to avoid this limitation:
 *
 * <code>
 * auto m = _new<ACurlMulti>();
 * m << _new<ACurl>(...);
 * AThread::processMessages() // +
 * assert(!m->empty());       // ok!
 * </code>
 */
class API_AUI_CURL ACurlMulti: public AObject {
public:
    ACurlMulti() noexcept;
    ~ACurlMulti();

    ACurlMulti(ACurlMulti&& other) noexcept: mMulti(other.mMulti) {
        other.mMulti = nullptr;
    }

    ACurlMulti& operator<<(_<ACurl> curl);
    ACurlMulti& operator>>(const _<ACurl>& curl);

    void run() {
        run(false);
    }

    void cancel() {
        mCancelled = true;
    }

    void clear();

    [[nodiscard]]
    const AMap<void*, _<ACurl>>& curls() const {
        return mEasyCurls;
    }

    /**
     * @brief Global instance of ACurlMulti, running in a separate thread.
     */
    static ACurlMulti& global() noexcept;

private:
    void run(bool infinite);

    void* mMulti;
    bool mCancelled = false;
    AMap<void*, _<ACurl>> mEasyCurls;
};


