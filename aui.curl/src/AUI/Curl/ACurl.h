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
#include <AUI/Common/AByteBufferView.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Common/ASignal.h>

class AString;

class API_AUI_CURL ACurl: public AObject {
friend class ACurlMulti;
public:
    enum class ResponseCode {
        HTTP_100_CONTINUE                        = 100,
        HTTP_101_SWITCHING_PROTOCOL              = 101,
        HTTP_102_PROCESSING                      = 102,
        HTTP_103_EARLY_HINTS                     = 103,
        HTTP_200_OK                              = 200,
        HTTP_201_CREATED                         = 201,
        HTTP_202_ACCEPTED                        = 202,
        HTTP_203_NON_AUTHORITATIVE_INFORMATION   = 203,
        HTTP_204_NO_CONTENT                      = 204,
        HTTP_205_RESET_CONTENT                   = 205,
        HTTP_206_PARTIAL_CONTENT                 = 206,
        HTTP_300_MULTIPLE_CHOICE                 = 300,
        HTTP_301_MOVED_PERMANENTLY               = 301,
        HTTP_302_FOUND                           = 302,
        HTTP_303_SEE_OTHER                       = 303,
        HTTP_304_NOT_MODIFIED                    = 304,
        HTTP_305_USE_PROXY                       = 305,
        HTTP_306_SWITCH_PROXY                    = 306,
        HTTP_307_TEMPORARY_REDIRECT              = 307,
        HTTP_308_PERMANENT_REDIRECT              = 308,
        HTTP_400_BAD_REQUEST                     = 400,
        HTTP_401_UNAUTHORIZED                    = 401,
        HTTP_402_PAYMENT_REQUIRED                = 402,
        HTTP_403_FORBIDDEN                       = 403,
        HTTP_404_NOT_FOUND                       = 404,
        HTTP_405_METHOD_NOT_ALLOWED              = 405,
        HTTP_406_NOT_ACCEPTABLE                  = 406,
        HTTP_407_PROXY_AUTHENTICATION_REQUIRED   = 407,
        HTTP_408_REQUEST_TIMEOUT                 = 408,
        HTTP_409_CONFLICT                        = 409,
        HTTP_410_GONE                            = 410,
        HTTP_411_LENGTH_REQUIRED                 = 411,
        HTTP_412_PRECONDITION_FAILED             = 412,
        HTTP_413_REQUEST_ENTITY_TOO_LARGE        = 413,
        HTTP_414_REQUEST_URI_TOO_LONG            = 414,
        HTTP_415_UNSUPPORTED_MEDIA_TYPE          = 415,
        HTTP_416_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
        HTTP_417_EXPECTATION_FAILED              = 417,
        HTTP_500_INTERNAL_SERVER_ERROR           = 500,
        HTTP_501_NOT_IMPLEMENTED                 = 501,
        HTTP_502_BAD_GATEWAY                     = 502,
        HTTP_503_SERVICE_UNAVAILABLE             = 503,
        HTTP_504_GATEWAY_TIMEOUT                 = 504,
        HTTP_505_HTTP_VERSION_NOT_SUPPORTED      = 505,
    };


    using WriteCallback = std::function<size_t(AByteBufferView)>;

    class Exception: public AIOException {
    public:
        using AIOException::AIOException;
    };

    class API_AUI_CURL Builder {
    friend class ACurl;
    private:
        void* mCURL;
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
            return withWriteCallback([dst](AByteBufferView b) {
                (*dst) << b;
                return b.size();
            });
        }

        Builder& withOutputStream(_<IOutputStream> dst) {
            return withWriteCallback([dst = std::move(dst)](AByteBufferView b) {
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

    [[nodiscard]]
    void* handle() const noexcept {
        return mCURL;
    }

    [[nodiscard]]
    ResponseCode getResponseCode() const;

private:
    void* mCURL;
    int mCURLcode{};

    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) noexcept;

    bool mFinished = false;
    bool mDestructorFlag = false;

    WriteCallback mWriteCallback;

    void reportFinished() {
        emit finished;
    }

    template<typename Ret>
    Ret getInfo(int curlInfo) const;


signals:
    emits<> finished;
};
