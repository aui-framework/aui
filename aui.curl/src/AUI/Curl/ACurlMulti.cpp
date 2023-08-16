// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by Alex2772 on 6/7/2022.
//

#include "ACurlMulti.h"
#include "AUI/Util/kAUI.h"
#include <curl/curl.h>
#include <AUI/Thread/ACutoffSignal.h>
#include <AUI/Util/ACleanup.h>
#include <AUI/Util/ARaiiHelper.h>

ACurlMulti::ACurlMulti() noexcept:
    mMulti(curl_multi_init())
{
    setThread(AThread::current());
}

ACurlMulti::~ACurlMulti() {
    if (mMulti) {
        clear();
        curl_multi_cleanup(mMulti);
    }
}

void ACurlMulti::run(bool infinite) {
    setThread(AThread::current());
    int isStillRunning;
    processQueueAndThreadMessages();
    while(!mCancelled && (!mEasyCurls.empty() || !mFunctionQueue.empty() || infinite)) {
        processQueueAndThreadMessages();
        auto status = curl_multi_perform(mMulti, &isStillRunning);

        if (status) { // failure
            for (const auto&[handle, curl] : mEasyCurls) {
                removeCurl(curl);
                curl->reportFail(0);
            }
            continue;
        }

        processQueueAndThreadMessages();
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
                    removeCurl(s);

                    if (msg->data.result != CURLE_OK) {
                        s->reportFail(msg->data.result);
                    } else {
                        s->reportSuccess();
                    }
                }
            }
        }
    }
}


ACurlMulti& ACurlMulti::operator<<(_<ACurl> curl) {
    mFunctionQueue << [this, curl = std::move(curl)]() mutable {
        connect(curl->closeRequested, [this, curl = curl.weak()] {
            if (auto c = curl.lock()) {
                *this >> c;
            }
        });
        auto c = curl_multi_add_handle(mMulti, curl->handle());
        assert(c == CURLM_OK);
        mEasyCurls[curl->handle()] = std::move(curl);
    };
    return *this;
}

ACurlMulti& ACurlMulti::operator>>(const _<ACurl>& curl) {
    mFunctionQueue << [=] {
        removeCurl(curl);
    };
    return *this;
}

void ACurlMulti::removeCurl(const _<ACurl>& curl) {
    curl_multi_remove_handle(mMulti, curl->handle());
    mEasyCurls.erase(curl->handle());
    curl->closeRequested.clearAllConnectionsWith(curl.get());
}

void ACurlMulti::clear() {
    mFunctionQueue << [this] {
        assert(mMulti);
        for (const auto& [handle, acurl]: mEasyCurls) {
            curl_multi_remove_handle(mMulti, handle);
        }
        mEasyCurls.clear();
    };
}

ACurlMulti& ACurlMulti::global() noexcept {
    static struct Instance {
        AOptional<ACurlMulti> multi = ACurlMulti();
        _<AThread> thread = _new<AThread>([this] {
            AThread::setName("AUI CURL IO");
            ARaiiHelper h = [&] {
                multi.reset();
            };
            multi->run(true);
        });

        Instance() {
            thread->start();

            ACutoffSignal cs;
            thread->enqueue([&] {
                cs.makeSignal();
            });
            cs.waitForSignal();

            ACleanup::afterEntry([&] {
                thread->interrupt();
            });
        }
    } instance;

    return *instance.multi;
}

void ACurlMulti::processQueueAndThreadMessages() {
    AThread::processMessages();
    mFunctionQueue.process();
}
