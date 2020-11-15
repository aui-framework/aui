#pragma once

#include <queue>
#include <AUI/ACurl.h>

#include "AUI/IO/IInputStream.h"
#include <AUI/IO/APipe.h>

class AString;
typedef void CURL;

class API_AUI_CURL ACurl: public IInputStream {
private:
	CURL* mCURL;
    int mCURLcode;
    APipe mPipe;

	size_t onDataReceived(char* ptr, size_t size);
	static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata);

	_<AThread> mWorkerThread;

	bool mFinished = false;
	bool mDestructorFlag = false;

public:
	ACurl(const AString& url);
	~ACurl();

	int read(char* dst, int size) override;

	int64_t getContentLength() const;
};