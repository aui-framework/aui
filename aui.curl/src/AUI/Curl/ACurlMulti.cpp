//
// Created by Alex2772 on 6/7/2022.
//

#include "ACurlMulti.h"
#include <curl/curl.h>

ACurlMulti::ACurlMulti() noexcept:
    mMulti(curl_multi_init())
{

}

ACurlMulti::~ACurlMulti() {
    if (mMulti) {
        for (const auto&[handle,acurl] : mEasyCurls) {
            curl_multi_remove_handle(mMulti, handle);
        }
        curl_multi_cleanup(mMulti);
    }
}

void ACurlMulti::run() {
    int isStillRunning;
    while(!mCancelled && !mEasyCurls.empty()) {
        auto status = curl_multi_perform(mMulti, &isStillRunning);

        if (status) {
            throw ACurl::Exception("curl perform failed: {}"_format(status));
        }

        status = curl_multi_poll(mMulti, nullptr, 0, 100, nullptr);
        AThread::interruptionPoint();

        if (status) {
            throw ACurl::Exception("curl poll failed: {}"_format(status));
        }

        int messagesLeft;
        for (CURLMsg* msg; (msg = curl_multi_info_read(mMulti, &messagesLeft));) {
            if (msg->msg == CURLMSG_DONE) {
                if (auto c = mEasyCurls.contains(msg->easy_handle)) {
                    auto s = std::move(c->second);
                    *this >> s;
                    s->reportFinished();
                }
            }
        }
    }
}

ACurlMulti& ACurlMulti::operator<<(_<ACurl> curl) {
    auto c = curl_multi_add_handle(mMulti, curl->handle());
    assert(c == CURLM_OK);
    mEasyCurls[curl->handle()] = std::move(curl);
    return *this;
}

ACurlMulti& ACurlMulti::operator>>(const _<ACurl>& curl) {
    curl_multi_remove_handle(mMulti, curl->handle());
    mEasyCurls.erase(curl->handle());
    return *this;
}
