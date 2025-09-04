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

#include "ACurl.h"
#include "ACurlMulti.h"

#include <cassert>
#include <chrono>
#include <curl/curl.h>
#include <AUI/Util/kAUI.h>
#include <numeric>

#include "AUI/Common/AString.h"
#include "AUI/Logging/ALogger.h"
#include "ACurlMulti.h"

#undef min

ACurl::Builder::Builder(AString url) : mUrl(std::move(url)) {
    class Global {
    public:
        Global() { curl_global_init(CURL_GLOBAL_ALL); }
        ~Global() { curl_global_cleanup(); }
    };

    static Global g;

    mCURL = curl_easy_init();
    assert(mCURL);
    CURLcode res;

    // at least 1kb/sec during 10sec
    res = curl_easy_setopt(mCURL, CURLOPT_LOW_SPEED_TIME, 10L);
    AUI_ASSERT(res == 0);
    res = curl_easy_setopt(mCURL, CURLOPT_LOW_SPEED_LIMIT, 1'024);
    AUI_ASSERT(res == 0);

    res = curl_easy_setopt(mCURL, CURLOPT_WRITEFUNCTION, ACurl::writeCallback);
    assert(res == 0);
    res = curl_easy_setopt(mCURL, CURLOPT_SSL_VERIFYPEER, false);
    assert(res == 0);
}

ACurl::Builder& ACurl::Builder::withRanges(size_t begin, size_t end) {
    if (begin || end) {
        std::string s = std::to_string(begin) + "-";
        if (end) {
            s += std::to_string(end);
        }
        auto res = curl_easy_setopt(mCURL, CURLOPT_RANGE, s.c_str());
        AUI_ASSERT(res == CURLE_OK);
    }
    return *this;
}

ACurl::Builder& ACurl::Builder::withHttpVersion(ACurl::Http version) {
    auto res = curl_easy_setopt(mCURL, CURLOPT_HTTP_VERSION, version);
    AUI_ASSERT(res == CURLE_OK);
    return *this;
}

ACurl::Builder& ACurl::Builder::withLowSpeedLimit(size_t speed) {
    auto res = curl_easy_setopt(mCURL, CURLOPT_LOW_SPEED_LIMIT, long(speed));
    AUI_ASSERT(res == CURLE_OK);
    return *this;
}

ACurl::Builder& ACurl::Builder::withLowSpeedTime(std::chrono::seconds duration) {
    auto res = curl_easy_setopt(mCURL, CURLOPT_LOW_SPEED_TIME, long(duration.count()));
    AUI_ASSERT(res == CURLE_OK);
    return *this;
}

ACurl::Builder& ACurl::Builder::withUpload(bool upload) {
    auto res = curl_easy_setopt(mCURL, CURLOPT_UPLOAD, upload ? 1L : 0L);
    AUI_ASSERT(res == CURLE_OK);
    return *this;
}
ACurl::Builder& ACurl::Builder::withCustomRequest(const AString& v) {
    auto res = curl_easy_setopt(mCURL, CURLOPT_CUSTOMREQUEST, v.toStdString().c_str());
    AUI_ASSERT(res == CURLE_OK);
    return *this;
}

ACurl::Builder::~Builder() { assert(mCURL == nullptr); }

_unique<IInputStream> ACurl::Builder::toInputStream() {
    class CurlInputStream : public IInputStream {
    private:
        _<ACurl> mCurl;
        APipe mPipe;

    public:
        CurlInputStream(_<ACurl> curl) : mCurl(std::move(curl)) {
            mCurl->mWriteCallback = [&](ACurl&, AByteBufferView buf) {
                mPipe << buf;
                return buf.size();
            };
            AObject::connect(mCurl->success, mCurl.get(), [this]() { mPipe.close(); });
            AObject::connect(mCurl->fail, mCurl.get(), [this]() { mPipe.close(); });
            ACurlMulti::global() << mCurl;
        }

        size_t read(char* dst, size_t size) override { return mPipe.read(dst, size); }
    };

    return std::make_unique<CurlInputStream>(_new<ACurl>(*this));
}

ACurl::Builder& ACurl::Builder::withParams(const AVector<std::pair<AString, AString>>& params) {
    std::string paramsString;
    paramsString.reserve(std::accumulate(
        params.begin(), params.end(), 1,
        [](std::size_t l, const std::pair<AString, AString>& p) { return l + p.first.size() + p.second.size() + 2; }));

    for (const auto& [key, value] : params) {
        if (!paramsString.empty()) {
            paramsString += '&';
        }
        paramsString += key.toStdString();
        paramsString += "=";

        for (std::uint8_t c : value.toStdString()) {
            if (std::isalnum(c)) {
                paramsString += (char) c;
            } else {
                char buf[128];
                paramsString += std::string_view(
                    buf, std::distance(std::begin(buf), fmt::format_to(std::begin(buf), "%{:02x}", c)));
            }
        }
    }
    mParams = std::move(paramsString);

    return *this;
}

static size_t readStub(char* ptr, size_t size, size_t nmemb, void* userdata) { return 0; }

ACurl& ACurl::operator=(Builder&& builder) noexcept {
    mCURL = builder.mCURL;

    mWriteCallback = std::move(builder.mWriteCallback);
    mReadCallback = std::move(builder.mReadCallback);
    mThrowExceptionOnError = builder.mThrowExceptionOnError;
    if (builder.mErrorCallback) {
        connect(fail, [callback = std::move(builder.mErrorCallback)](const ErrorDescription& e) { callback(e); });
    }
    AUI_ASSERTX(std::size(mErrorBuffer) == CURL_ERROR_SIZE, "buffer size mismatch");
    builder.mCURL = nullptr;

    switch (builder.mMethod) {
        case Method::HTTP_GET: {
            std::string url = builder.mUrl.toStdString();
            if (!builder.mParams.empty()) {
                url += '?';
                url += builder.mParams.toStdString();
            }
            auto res = curl_easy_setopt(mCURL, CURLOPT_URL, url.c_str());
            AUI_ASSERT(res == 0);
            break;
        }

        case Method::HTTP_DELETE: {
            std::string url = builder.mUrl.toStdString();
            if (!builder.mParams.empty()) {
                url += '?';
                url += builder.mParams.toStdString();
            }
            auto res = curl_easy_setopt(mCURL, CURLOPT_URL, url.c_str());
            AUI_ASSERT(res == 0);
            res = curl_easy_setopt(mCURL, CURLOPT_CUSTOMREQUEST, "DELETE");
            AUI_ASSERT(res == 0);
            break;
        }

        case Method::HTTP_PUT: {
            std::string url = builder.mUrl.toStdString();
            if (!builder.mParams.empty()) {
                url += '?';
                url += builder.mParams.toStdString();
            }
            auto res = curl_easy_setopt(mCURL, CURLOPT_URL, url.c_str());
            AUI_ASSERT(res == 0);
            res = curl_easy_setopt(mCURL, CURLOPT_CUSTOMREQUEST, "PUT");
            AUI_ASSERT(res == 0);
            break;
        }

        case Method::HTTP_POST: {
            auto res = curl_easy_setopt(mCURL, CURLOPT_URL, builder.mUrl.toStdString().c_str());
            AUI_ASSERT(res == 0);
            res = curl_easy_setopt(mCURL, CURLOPT_POST, true);
            AUI_ASSERT(res == 0);

            if (!builder.mParams.empty()) {
                mPostFieldsStorage = builder.mParams.toStdString();
                res = curl_easy_setopt(mCURL, CURLOPT_POSTFIELDS, mPostFieldsStorage.c_str());
                AUI_ASSERT(res == 0);
            }
            break;
        }
    }

    auto res = curl_easy_setopt(mCURL, CURLOPT_ERRORBUFFER, mErrorBuffer);
    AUI_ASSERT(res == 0);
    res = curl_easy_setopt(mCURL, CURLOPT_WRITEDATA, this);
    assert(res == 0);

    if (builder.mMethod == Method::HTTP_POST && !mReadCallback) {
        // if read func is not set, curl would block.
        res = curl_easy_setopt(mCURL, CURLOPT_READDATA, this);
        AUI_ASSERT(res == 0);
        res = curl_easy_setopt(mCURL, CURLOPT_READFUNCTION, readStub);
        AUI_ASSERT(res == 0);
    } else if (mReadCallback) {
        res = curl_easy_setopt(mCURL, CURLOPT_READDATA, this);
        AUI_ASSERT(res == 0);
        res = curl_easy_setopt(mCURL, CURLOPT_READFUNCTION, readCallback);
        AUI_ASSERT(res == 0);
    }

    if (!builder.mHeaders.empty()) {
        for (const auto& h : builder.mHeaders) {
            mCurlHeaders = curl_slist_append(mCurlHeaders, h.toStdString().c_str());
        }
        res = curl_easy_setopt(mCURL, CURLOPT_HTTPHEADER, mCurlHeaders);
        AUI_ASSERT(res == 0);
    }

    if (builder.mHeaderCallback) {
        mHeaderCallback = std::move(builder.mHeaderCallback);
        res = curl_easy_setopt(mCURL, CURLOPT_HEADERDATA, this);
        AUI_ASSERT(res == CURLE_OK);
        res = curl_easy_setopt(mCURL, CURLOPT_HEADERFUNCTION, ACurl::headerCallback);
        AUI_ASSERT(res == CURLE_OK);
    }

    if (builder.mOnSuccess) {
        connect(success, [this, success = std::move(builder.mOnSuccess)]() { success(*this); });
    }

    curl_easy_setopt(mCURL, CURLOPT_FOLLOWLOCATION, true);

    res = curl_easy_setopt(mCURL, CURLOPT_ACCEPT_ENCODING, "");
    AUI_ASSERT(res == CURLE_OK);

    return *this;
}

ACurl& ACurl::operator=(ACurl&& o) noexcept {
    mCURL = o.mCURL;
    mWriteCallback = std::move(o.mWriteCallback);

    o.mCURL = nullptr;
    CURLcode res = curl_easy_setopt(mCURL, CURLOPT_ERRORBUFFER, mErrorBuffer);
    AUI_ASSERT(res == 0);
    res = curl_easy_setopt(mCURL, CURLOPT_WRITEDATA, this);
    AUI_ASSERT(res == 0);
    return *this;
}

ACurl::~ACurl() {
    curl_easy_cleanup(mCURL);
    if (mCurlHeaders)
        curl_slist_free_all(mCurlHeaders);
}

size_t ACurl::readCallback(char* ptr, size_t size, size_t nmemb, void* userdata) noexcept {
    if (AThread::current()->isInterrupted()) {
        return 0;
    }
    auto c = static_cast<ACurl*>(userdata);
    if (c->mCloseRequested) {
        return 0;
    }

    try {
        auto r = c->mReadCallback(ptr, size * nmemb);
        if (c->mCloseRequested) {
            return 0;
        }
        if (r > 0) {
            return r;
        }
    } catch (const AEOFException&) {
        return 0;
    } catch (const AException& e) {
        ALogger::err("curl") << "Read callback failed: " << e;
    }
    return CURL_READFUNC_PAUSE;
}

size_t ACurl::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) noexcept {
    if (AThread::current()->isInterrupted()) {
        return 0;
    }
    auto c = static_cast<ACurl*>(userdata);
    if (c->mCloseRequested) {
        return 0;
    }
    try {
        auto r = c->mWriteCallback(*c, { ptr, nmemb });
        if (c->mCloseRequested) {
            return 0;
        }
        if (r > 0) {
            AUI_ASSERTX(r == size * nmemb,
                "You returned a non-zero value not matching the passed buffer size, which is treated by curl as "
                "an error, but it's more likely you accidentally have not read all the data. If you really wanted "
                "to raise an error, please throw an exception instead.");
            return r;
        }
    } catch (const AEOFException&) {
        return 0;
    } catch (const AException& e) {
        ALogger::err("curl") << "Write callback failed: " << e;
        return 0;
    }
    return CURL_WRITEFUNC_PAUSE;
}

size_t ACurl::headerCallback(char* buffer, size_t size, size_t nitems, void* userdata) noexcept {
    try {
        auto self = reinterpret_cast<ACurl*>(userdata);
        self->mHeaderCallback(AByteBufferView(buffer, size * nitems));
        return size * nitems;
    } catch (const AException& e) {
        ALogger::err("curl") << "Header callback failed: " << e;
    }
    return 0;
}

void ACurl::run() {
    auto c = curl_easy_perform(mCURL);
    if (mCloseRequested) {   // the failure may be caused by close() method
        return;
    }
    AThread::interruptionPoint();
    if (c != CURLE_OK) {
        ErrorDescription description{c, AString::fromLatin1(mErrorBuffer)};
        reportFail(description);
        if (mThrowExceptionOnError) {
            description.throwException();
        }
    } else {
        reportSuccess();
    }
}

void ACurl::close() {
    mCloseRequested = true;
    curl_easy_pause(mCURL, 0);   // unpause transfers in order to force curl to call callbacks
    emit closeRequested;
}

template <typename Ret>
Ret ACurl::getInfo(int curlInfo) const {
    Ret result;
    if (auto r = curl_easy_getinfo(mCURL, static_cast<CURLINFO>(curlInfo), &result); r != CURLE_OK) {
        throw Exception(mErrorBuffer);
    }
    return result;
}

int64_t ACurl::getContentLength() const { return getInfo<curl_off_t>(CURLINFO_CONTENT_LENGTH_DOWNLOAD_T); }

int64_t ACurl::getNumberOfBytesDownloaded() const { return getInfo<curl_off_t>(CURLINFO_SIZE_DOWNLOAD_T); }

AString ACurl::getContentType() const {
    auto v = getInfo<const char*>(CURLINFO_CONTENT_TYPE);
    return v ? v : "";
}

ACurl::ResponseCode ACurl::getResponseCode() const {
    return static_cast<ACurl::ResponseCode>(getInfo<long>(CURLINFO_RESPONSE_CODE));
}

void ACurl::ErrorDescription::throwException() const { throw ACurl::Exception(*this); }

static ACurl::Response makeResponse(ACurl& r, AByteBuffer body) {
    return {
        .code = r.getResponseCode(),
        .contentType = r.getContentType(),
        .body = std::move(body),
    };
}

ACurl::Response ACurl::Builder::runBlocking() {
    AByteBuffer out;
    mWriteCallback = [&](ACurl&, AByteBufferView buf) {
        out << buf;
        return buf.size();
    };
    ACurl r(*this);
    r.run();
    return makeResponse(r, std::move(out));
}

AFuture<ACurl::Response> ACurl::Builder::runAsync() { return runAsync(ACurlMulti::global()); }

AFuture<ACurl::Response> ACurl::Builder::runAsync(ACurlMulti& curlMulti) {
    AFuture<ACurl::Response> result;
    auto body = _new<AByteBuffer>();
    if (!mWriteCallback) withDestinationBuffer(*body);
    withOnSuccess([result, body = std::move(body)](ACurl& c) {
        result.supplyValue(makeResponse(c, std::move(*body)));
    });
    withErrorCallback([result](const ErrorDescription& error) {
        try {
            error.throwException();
        } catch (...) {
            result.supplyException();
        }
    });
    curlMulti << _new<ACurl>(std::move(*this));
    return result;
}

ACurl::Builder& ACurl::Builder::withTimeout(std::chrono::seconds timeout) {
    auto res = curl_easy_setopt(mCURL, CURLOPT_LOW_SPEED_TIME, timeout.count());
    AUI_ASSERT(res == 0);
    return *this;
}
