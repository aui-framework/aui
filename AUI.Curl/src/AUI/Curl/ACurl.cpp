#include "ACurl.h"


#include <cassert>
#include <curl/curl.h>

#include "AUI/Common/AString.h"


#undef min

ACurl::Builder::Builder(const AString& url) {
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

	res = curl_easy_setopt(mCURL, CURLOPT_WRITEFUNCTION, writeCallback);
	assert(res == 0);
	res = curl_easy_setopt(mCURL, CURLOPT_SSL_VERIFYPEER, false);
	assert(res == 0);
}

ACurl::Builder& ACurl::Builder::setRanges(size_t begin, size_t end) {
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


ACurl::ACurl(const AString& url):
	ACurl(Builder(url))
{

}

ACurl::ACurl(Builder&& url):
	mCURLcode(0)
{
	mCURL = url.mCURL;
	url.mCURL = nullptr;

	CURLcode res = curl_easy_setopt(mCURL, CURLOPT_WRITEDATA, this);
	assert(res == 0);

	mWorkerThread = _new<AThread>([&]()
	{
		CURLcode r = curl_easy_perform(mCURL);
		//assert(r == 0 || r == CURLE_PARTIAL_FILE);

		mCURLcode = r;
		mFinished = true;

        curl_easy_cleanup(mCURL);

        mPipe.close();
	});
	mWorkerThread->start();

}

ACurl::~ACurl()
{
    mDestructorFlag = true;
    mPipe.close();
	mWorkerThread->join();
}

int ACurl::read(char* dst, int size)
{
    if (mFinished && !mPipe.available()) {
        if (mCURLcode == CURLE_WRITE_ERROR)
            return -1;
        return 0;
    }
    return mPipe.read(dst, size);
}

size_t ACurl::onDataReceived(char* ptr, size_t size)
{
    if (mDestructorFlag)
        return -1;
    return mPipe.write(ptr, size);
}

size_t ACurl::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	ACurl* c = static_cast<ACurl*>(userdata);
	return c->onDataReceived(ptr, nmemb);;
}

int64_t ACurl::getContentLength() const {
    curl_off_t s;
    curl_easy_getinfo(mCURL, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &s);
    return s;
}
