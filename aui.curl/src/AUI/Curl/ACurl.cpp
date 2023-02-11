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

#include "ACurl.h"


#include <cassert>
#include <curl/curl.h>
#include <AUI/Util/kAUI.h>
#include <numeric>

#include "AUI/Common/AString.h"
#include "AUI/Logging/ALogger.h"


#undef min

ACurl::Builder::Builder(AString url): mUrl(std::move(url))
{
	class Global
	{
	public:
		Global()
		{
			curl_global_init(CURL_GLOBAL_ALL);
		}
		~Global()
		{
			curl_global_cleanup();
		}
	};

	static Global g;


	mCURL = curl_easy_init();
	assert(mCURL);
	CURLcode res;

    // at least 1kb/sec during 10sec
    res = curl_easy_setopt(mCURL, CURLOPT_LOW_SPEED_TIME, 10L);
    assert(res == 0);
    res = curl_easy_setopt(mCURL, CURLOPT_LOW_SPEED_LIMIT, 1'024);
    assert(res == 0);

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
        assert(res == CURLE_OK);
	}
	return *this;
}

ACurl::Builder& ACurl::Builder::withHttpVersion(ACurl::Http version) {
    auto res = curl_easy_setopt(mCURL, CURLOPT_HTTP_VERSION, version);
    assert(res == CURLE_OK);
    return *this;
}

ACurl::Builder& ACurl::Builder::withUpload(bool upload) {
    auto res = curl_easy_setopt(mCURL, CURLOPT_UPLOAD, upload ? 1L : 0L);
    assert(res == CURLE_OK);
    return *this;
}
ACurl::Builder& ACurl::Builder::withCustomRequest(const AString& v) {
    auto res = curl_easy_setopt(mCURL, CURLOPT_CUSTOMREQUEST, v.toStdString().c_str());
    assert(res == CURLE_OK);
    return *this;
}

ACurl::Builder::~Builder() {
	assert(mCURL == nullptr);
}

_<IInputStream> ACurl::Builder::toInputStream() {
    class CurlInputStream: public IInputStream {
    private:
        _<ACurl> mCurl;
        AFuture<> mFuture;
        APipe mPipe;

    public:
        CurlInputStream(_<ACurl> curl) : mCurl(std::move(curl)) {
            mFuture = async {
                mCurl->mWriteCallback = [&](AByteBufferView buf) {
                    mPipe << buf;
                    return buf.size();
                };
                mCurl->run();
            };
        }

        size_t read(char* dst, size_t size) override {
            return mPipe.read(dst, size);
        }
    };
    return _new<CurlInputStream>(_new<ACurl>(*this));
}

AByteBuffer ACurl::Builder::toByteBuffer() {
    AByteBuffer out;
    mWriteCallback = [&](AByteBufferView buf) {
        out << buf;
        return buf.size();
    };
    ACurl r(*this);
    r.run();
    return out;
}

ACurl::Builder& ACurl::Builder::withParams(const AVector<std::pair<AString, AString>>& params) {
    mUrl.reserve(std::accumulate(params.begin(), params.end(), mUrl.size() + 1, [](std::size_t l, const std::pair<AString, AString>& p) {
        return l + p.first.size() + p.second.size() + 2;
    }));

    bool first = true;

    for (const auto&[key, value] : params) {
        if (first) {
            mUrl += '?';
            first = false;
        } else {
            mUrl += '&';
        }
        mUrl += key;
        mUrl += "=";
        mUrl += value;
    }

    return *this;
}

ACurl& ACurl::operator=(Builder&& builder) noexcept {
    mCURL = builder.mCURL;

    mWriteCallback = std::move(builder.mWriteCallback);
    mReadCallback = std::move(builder.mReadCallback);
    if (builder.mErrorCallback) {
        connect(fail, [callback = std::move(builder.mErrorCallback)](const ErrorDescription& e) {
            callback(e);
        });
    } else if (builder.mThrowExceptionOnError) {
        connect(fail, [](const ErrorDescription& e) {
            e.throwException();
        });
    }
    assert(("buffer size mismatch", std::size(mErrorBuffer) == CURL_ERROR_SIZE));
    builder.mCURL = nullptr;


    auto res = curl_easy_setopt(mCURL, CURLOPT_URL, builder.mUrl.toStdString().c_str());
    assert(res == 0);

	res = curl_easy_setopt(mCURL, CURLOPT_ERRORBUFFER, mErrorBuffer);
    assert(res == 0);
    res = curl_easy_setopt(mCURL, CURLOPT_WRITEDATA, this);
	assert(res == 0);

    if (mReadCallback) {
        curl_easy_setopt(mCURL, CURLOPT_READDATA, this);
        curl_easy_setopt(mCURL, CURLOPT_READFUNCTION, readCallback);
    }

    if (!builder.mHeaders.empty()) {
        for (const auto& h : builder.mHeaders) {
            mCurlHeaders = curl_slist_append(mCurlHeaders, h.toStdString().c_str());
        }
        res = curl_easy_setopt(mCURL, CURLOPT_HTTPHEADER, mCurlHeaders);
        assert(res == 0);
    }

    if (builder.mHeaderCallback){
        mHeaderCallback = std::move(builder.mHeaderCallback);
        res = curl_easy_setopt(mCURL, CURLOPT_HEADERDATA, this);
        assert(res == CURLE_OK);
        res = curl_easy_setopt(mCURL, CURLOPT_HEADERFUNCTION, ACurl::headerCallback);
        assert(res == CURLE_OK);
    }

    return *this;
}

ACurl& ACurl::operator=(ACurl&& o) noexcept {
    mCURL = o.mCURL;
    mWriteCallback = std::move(o.mWriteCallback);

    o.mCURL = nullptr;
    CURLcode res = curl_easy_setopt(mCURL, CURLOPT_ERRORBUFFER, mErrorBuffer);
    assert(res == 0);
    res = curl_easy_setopt(mCURL, CURLOPT_WRITEDATA, this);
    assert(res == 0);
    return *this;
}

ACurl::~ACurl()
{
    curl_easy_cleanup(mCURL);
    if (mCurlHeaders) curl_slist_free_all(mCurlHeaders);
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

size_t ACurl::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) noexcept
{
    if (AThread::current()->isInterrupted()) {
        return 0;
    }
    auto c = static_cast<ACurl*>(userdata);
    if (c->mCloseRequested) {
        return 0;
    }
    try {
        auto r = c->mWriteCallback({ptr, nmemb});
        if (c->mCloseRequested) {
            return 0;
        }
        if (r > 0) {
            assert(("You returned a non-zero value not matching the passed buffer size, which is treated by curl as "
                    "an error, but it's more likely you accidentally have not read all the data. If you really wanted "
                    "to raise an error, please throw an exception instead.", r == size * nmemb));
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
    if (mCloseRequested) { // the failure may be caused by close() method
        return;
    }
    AThread::interruptionPoint();
    if (c != CURLE_OK) {
        reportFail(c);
    } else {
        reportSuccess();
    }
}

void ACurl::close() {
    mCloseRequested = true;
    curl_easy_pause(mCURL, 0); // unpause transfers in order to force curl to call callbacks
    emit closeRequested;
}

template<typename Ret>
Ret ACurl::getInfo(int curlInfo) const {
    Ret result;
    if (auto r = curl_easy_getinfo(mCURL, static_cast<CURLINFO>(curlInfo), &result); r != CURLE_OK) {
        throw Exception(mErrorBuffer);
    }
    return result;
}


int64_t ACurl::getContentLength() const {
    return getInfo<curl_off_t>(CURLINFO_CONTENT_LENGTH_DOWNLOAD_T);
}

ACurl::ResponseCode ACurl::getResponseCode() const {
    return static_cast<ACurl::ResponseCode>(getInfo<long>(CURLINFO_RESPONSE_CODE));
}

void ACurl::ErrorDescription::throwException() const {
    throw ACurl::Exception(*this);
}
