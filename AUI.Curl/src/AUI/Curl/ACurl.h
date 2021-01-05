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
    int mCURLcode{};
    APipe mPipe;

	size_t onDataReceived(char* ptr, size_t size);
	static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata);

	_<AThread> mWorkerThread;

	bool mFinished = false;
	bool mDestructorFlag = false;

public:
    class API_AUI_CURL Builder {
    friend class ACurl;
    private:
        CURL* mCURL;

    public:
        explicit Builder(const AString& url);
        Builder(const Builder&) = delete;
        ~Builder();

        /**
         * \brief Sets: Accept-Ranges: begin-end
         *        (download part of the file)
         * \param begin start index of the part
         * \param end end index of the part. Zero means end of the file.
         * \return this
         */
        Builder& setRanges(size_t begin, size_t end);
    };

    explicit ACurl(const AString& url);
	explicit ACurl(Builder& builder):
	    ACurl(std::move(builder))
	{

	}
	explicit ACurl(Builder&& builder);
	~ACurl();

	int read(char* dst, int size) override;

	int64_t getContentLength() const;
};