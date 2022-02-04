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
#include "AUI/Traits/values.h"
#include <AUI/IO/APipe.h>

class AString;
typedef void CURL;

class API_AUI_CURL ACurl {
public:
    using WriteCallback = std::function<size_t(const AByteBufferRef&)>;
private:
	CURL* mCURL;
    int mCURLcode{};

	static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata);

	bool mFinished = false;
	bool mDestructorFlag = false;

    WriteCallback mWriteCallback;

public:
    class Exception: public AIOException {
    public:
        using AIOException::AIOException;
    };

    class API_AUI_CURL Builder {
    friend class ACurl;
    private:
        CURL* mCURL;
        WriteCallback mWriteCallback;

    public:
        explicit Builder(const AString& url);
        Builder(const Builder&) = delete;
        ~Builder();

        Builder& withWriteCallback(WriteCallback callback) {
            assert(("write callback already set" && mWriteCallback == nullptr));
            mWriteCallback = std::move(callback);
            return *this;
        }

        Builder& withDestinationBuffer(aui::promise::no_copy<AByteBuffer> dst) {
            return withWriteCallback([dst](const AByteBufferRef& b) {
                (*dst) << b;
                return b.size();
            });
        }

        Builder& withOutputStream(_<IOutputStream> dst) {
            return withWriteCallback([dst = std::move(dst)](const AByteBufferRef& b) {
                (*dst) << b;
                return b.size();
            });
        }

        /**
         * \brief Sets: Accept-Ranges: begin-end
         *        (download part of the file)
         * \param begin start index of the part
         * \param end end index of the part. Zero means end of the file.
         * \return this
         */
        Builder& withRanges(size_t begin, size_t end);

        /**
         * Makes input stream from curl builder.
         * @note creates async task where curl's loop lives in.
         * @throws AIOException
         * @return input stream
         */
        _<IInputStream> toInputStream();

        /**
         * Makes bytebuffer from curl builder.
         * @throws AIOException
         */
         AByteBuffer toByteBuffer();
    };

	explicit ACurl(Builder& builder):
	    ACurl(std::move(builder))
	{

	}
	explicit ACurl(Builder&& builder);
    ACurl(ACurl&&) noexcept = default;

	~ACurl();

	int64_t getContentLength() const;

    void run();
};