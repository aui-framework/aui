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

#pragma once

#include <queue>
#include <AUI/ACurl.h>

#include "AUI/IO/IInputStream.h"
#include "AUI/Traits/values.h"
#include <AUI/IO/APipe.h>
#include <AUI/Common/AByteBufferView.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Reflect/AEnumerate.h>

class AString;

/**
 * @brief Easy curl instance.
 * @ingroup curl
 * @details
 * ACurl::Builder is used to construct ACurl.
 */
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

        /* don't forget to update AUI_ENUM_VALUES at the bottom */
    };


    struct ErrorDescription {
        int curlStatus;
        AString description;

        API_AUI_CURL void throwException() const;
    };

    using WriteCallback = std::function<size_t(AByteBufferView)>;
    using ErrorCallback = std::function<void(const ErrorDescription& description)>;


    class Exception: public AIOException {
    public:
        using AIOException::AIOException;
        Exception(const ErrorDescription& errorDescription): AIOException(errorDescription.description),
                                                             mCurlStatus(errorDescription.curlStatus) {

        }

        [[nodiscard]]
        int curlStatus() const noexcept {
            return mCurlStatus;
        }

    private:
        int mCurlStatus;
    };

    class API_AUI_CURL Builder {
    friend class ACurl;
    private:
        void* mCURL;
        WriteCallback mWriteCallback;
        ErrorCallback mErrorCallback;
        bool mThrowExceptionOnError = true;

    public:
        explicit Builder(const AString& url);
        Builder(const Builder&) = delete;
        ~Builder();

        Builder& withWriteCallback(WriteCallback callback) {
            assert(("write callback already set" && mWriteCallback == nullptr));
            mWriteCallback = std::move(callback);
            return *this;
        }

        /**
         * @note Also disables throwing exception on error
         */
        Builder& withErrorCallback(ErrorCallback callback) {
            assert(("error callback already set" && mErrorCallback == nullptr));
            mErrorCallback = std::move(callback);
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

        Builder& throwExceptionOnError(bool throwExceptionOnError) noexcept {
            mThrowExceptionOnError = throwExceptionOnError;
            return *this;
        }

        /**
         * @brief Sets: Accept-Ranges: begin-end
         *        (download part of the file)
         * @param begin start index of the part
         * @param end end index of the part. Zero means end of the file.
         * @return this
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
	explicit ACurl(Builder&& builder) noexcept {
        operator=(std::move(builder));
    }
    ACurl(ACurl&& o) noexcept {
        operator=(std::move(o));
    }
    ACurl() noexcept: mCURL(nullptr) {}
	~ACurl();

    ACurl& operator=(Builder&& o) noexcept;
    ACurl& operator=(ACurl&& o) noexcept;

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
    char mErrorBuffer[256];

    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) noexcept;

    WriteCallback mWriteCallback;

    void reportSuccess() {
        emit success;
    }

    void reportFail(int statusCode) {
        emit fail(ErrorDescription{statusCode, AString::fromLatin1(mErrorBuffer)});
    }

    template<typename Ret>
    Ret getInfo(int curlInfo) const;


signals:

    /**
     * @brief Emitted on network error.
     * @note A protocol-level error (like HTTP(S) 404) is not treated as a fail. Check for response code via the
     *       <code>getResponseCode()</code> function.
     */
    emits<ErrorDescription> fail;

    /**
     * @brief Emitted on success.
     * @note A protocol-level error (like HTTP(S) 404) is not treated as a fail. Check for response code via the
     *       <code>getResponseCode()</code> function.
     */
    emits<> success;
};


AUI_ENUM_VALUES(ACurl::ResponseCode,
                ACurl::ResponseCode::HTTP_100_CONTINUE,
                ACurl::ResponseCode::HTTP_101_SWITCHING_PROTOCOL,
                ACurl::ResponseCode::HTTP_102_PROCESSING,
                ACurl::ResponseCode::HTTP_103_EARLY_HINTS,
                ACurl::ResponseCode::HTTP_200_OK,
                ACurl::ResponseCode::HTTP_201_CREATED,
                ACurl::ResponseCode::HTTP_202_ACCEPTED,
                ACurl::ResponseCode::HTTP_203_NON_AUTHORITATIVE_INFORMATION,
                ACurl::ResponseCode::HTTP_204_NO_CONTENT,
                ACurl::ResponseCode::HTTP_205_RESET_CONTENT,
                ACurl::ResponseCode::HTTP_206_PARTIAL_CONTENT,
                ACurl::ResponseCode::HTTP_300_MULTIPLE_CHOICE,
                ACurl::ResponseCode::HTTP_301_MOVED_PERMANENTLY,
                ACurl::ResponseCode::HTTP_302_FOUND,
                ACurl::ResponseCode::HTTP_303_SEE_OTHER,
                ACurl::ResponseCode::HTTP_304_NOT_MODIFIED,
                ACurl::ResponseCode::HTTP_305_USE_PROXY,
                ACurl::ResponseCode::HTTP_306_SWITCH_PROXY,
                ACurl::ResponseCode::HTTP_307_TEMPORARY_REDIRECT,
                ACurl::ResponseCode::HTTP_308_PERMANENT_REDIRECT,
                ACurl::ResponseCode::HTTP_400_BAD_REQUEST,
                ACurl::ResponseCode::HTTP_401_UNAUTHORIZED,
                ACurl::ResponseCode::HTTP_402_PAYMENT_REQUIRED,
                ACurl::ResponseCode::HTTP_403_FORBIDDEN,
                ACurl::ResponseCode::HTTP_404_NOT_FOUND,
                ACurl::ResponseCode::HTTP_405_METHOD_NOT_ALLOWED,
                ACurl::ResponseCode::HTTP_406_NOT_ACCEPTABLE,
                ACurl::ResponseCode::HTTP_407_PROXY_AUTHENTICATION_REQUIRED,
                ACurl::ResponseCode::HTTP_408_REQUEST_TIMEOUT,
                ACurl::ResponseCode::HTTP_409_CONFLICT,
                ACurl::ResponseCode::HTTP_410_GONE,
                ACurl::ResponseCode::HTTP_411_LENGTH_REQUIRED,
                ACurl::ResponseCode::HTTP_412_PRECONDITION_FAILED,
                ACurl::ResponseCode::HTTP_413_REQUEST_ENTITY_TOO_LARGE,
                ACurl::ResponseCode::HTTP_414_REQUEST_URI_TOO_LONG,
                ACurl::ResponseCode::HTTP_415_UNSUPPORTED_MEDIA_TYPE,
                ACurl::ResponseCode::HTTP_416_REQUESTED_RANGE_NOT_SATISFIABLE,
                ACurl::ResponseCode::HTTP_417_EXPECTATION_FAILED,
                ACurl::ResponseCode::HTTP_500_INTERNAL_SERVER_ERROR,
                ACurl::ResponseCode::HTTP_501_NOT_IMPLEMENTED,
                ACurl::ResponseCode::HTTP_502_BAD_GATEWAY,
                ACurl::ResponseCode::HTTP_503_SERVICE_UNAVAILABLE,
                ACurl::ResponseCode::HTTP_504_GATEWAY_TIMEOUT,
                ACurl::ResponseCode::HTTP_505_HTTP_VERSION_NOT_SUPPORTED)
