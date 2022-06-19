/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include "ACurl.h"


#include <cassert>
#include <curl/curl.h>
#include <AUI/Util/kAUI.h>

#include "AUI/Common/AString.h"


#undef min

ACurl::Builder::Builder(const AString& url)
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


	res = curl_easy_setopt(mCURL, CURLOPT_URL, url.toStdString().c_str());
	assert(res == 0);

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
		curl_easy_setopt(mCURL, CURLOPT_RANGE, s.c_str());
	}
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

ACurl& ACurl::operator=(Builder&& builder) noexcept {
    mCURL = builder.mCURL;
    mWriteCallback = std::move(builder.mWriteCallback);
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

	CURLcode res = curl_easy_setopt(mCURL, CURLOPT_ERRORBUFFER, mErrorBuffer);
    assert(res == 0);
    res = curl_easy_setopt(mCURL, CURLOPT_WRITEDATA, this);
	assert(res == 0);
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
}


size_t ACurl::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) noexcept
{
    if (AThread::current()->isInterrupted()) {
        return 0;
    }
    auto c = static_cast<ACurl*>(userdata);
    try {
        return c->mWriteCallback({ptr, nmemb});
    } catch (...) {
        return 0;
    }
}


void ACurl::run() {
    auto c = curl_easy_perform(mCURL);
    AThread::interruptionPoint();
    if (c != CURLE_OK) {
        reportFail(c);
    } else {
        reportSuccess();
    }
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
