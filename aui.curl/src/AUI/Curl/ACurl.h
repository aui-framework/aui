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

	size_t read(char* dst, size_t size) override;

	int64_t getContentLength() const;
};