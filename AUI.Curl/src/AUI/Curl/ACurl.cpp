#include "ACurl.h"


#include <cassert>
#include <curl/curl.h>

#include "AUI/Common/AString.h"


#undef min

ACurl::ACurl(const AString& url)
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

	res = curl_easy_setopt(mCURL, CURLOPT_WRITEFUNCTION, writeCallback);
	assert(res == 0);
	
	res = curl_easy_setopt(mCURL, CURLOPT_WRITEDATA, this);
	assert(res == 0);
	res = curl_easy_setopt(mCURL, CURLOPT_SSL_VERIFYPEER, false);
	assert(res == 0);

	mWorkerThread = _new<AThread>([&]()
	{
		CURLcode r = curl_easy_perform(mCURL);
		assert(r == 0);

		mFinished = true;
		
		mDataWaiter.notify_all();
	});
	mWorkerThread->start();
	
}

ACurl::~ACurl()
{
	mWorkerThread->join();
	curl_easy_cleanup(mCURL);
}

int ACurl::read(char* dst, int size)
{
	std::unique_lock<std::mutex> lock(mDataQueueLock);
	while (mDataQueue.empty())
	{
		if (mFinished)
			return 0;
		mDataWaiter.wait(lock);
	}

	auto obj = mDataQueue.front();
	int available = obj->getAvailable();
	if (available > size)
	{
		lock.unlock();
		obj->get(dst, size);
		return size;
	}
	mDataQueue.pop();
	lock.unlock();
	obj->get(dst, available);
	return available;
}

void ACurl::onDataReceived(char* ptr, size_t size)
{
	auto buffer = _new<AByteBuffer>();
	buffer->put(ptr, size);
	{
		std::unique_lock lock(mDataQueueLock);
		mDataQueue.push(buffer);
	}
	mDataWaiter.notify_all();
}

size_t ACurl::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	ACurl* c = static_cast<ACurl*>(userdata);
	c->onDataReceived(ptr, nmemb);

	return nmemb;
}