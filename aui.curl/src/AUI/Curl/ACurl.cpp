/**
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
        ACurl mCurl;
        AFuture<> mFuture;
        APipe mPipe;

    public:
        CurlInputStream(ACurl curl) : mCurl(std::move(curl)) {
            mFuture = async {
                mCurl.mWriteCallback = [&](AByteBufferView buf) {
                    mPipe << buf;
                    return buf.size();
                };
                mCurl.run();
            };
        }

        size_t read(char* dst, size_t size) override {
            return mPipe.read(dst, size);
        }
    };
    return _new<CurlInputStream>(ACurl(*this));
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

ACurl::ACurl(Builder&& url):
	mCURLcode(0),
    mCURL(url.mCURL),
    mWriteCallback(std::move(url.mWriteCallback))
{
	url.mCURL = nullptr;

	CURLcode res = curl_easy_setopt(mCURL, CURLOPT_WRITEDATA, this);
	assert(res == 0);


}

ACurl::~ACurl()
{
    mDestructorFlag = true;
}


size_t ACurl::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) noexcept
{
    if (AThread::current()->isInterrupted()) {
        return 0;
    }
    auto c = static_cast<ACurl*>(userdata);
	return c->mWriteCallback({ptr, nmemb});
}

int64_t ACurl::getContentLength() const {
    curl_off_t s;
    curl_easy_getinfo(mCURL, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &s);
    return s;
}

void ACurl::run() {
    auto c = curl_easy_perform(mCURL);
    AThread::interruptionPoint();
    if (c != CURLE_OK) {
        throw Exception(curl_easy_strerror(c));
    }
}
