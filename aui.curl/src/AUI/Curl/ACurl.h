// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
 *
 * Analogous to Qt's QNetworkRequest.
 */
class API_AUI_CURL ACurl: public AObject {
friend class ACurlMulti;
public:
    enum class Http {
        VERSION_NONE, /* setting this means we don't care, and that we'd
                         like the library to choose the best possible
                         for us! */
        VERSION_1_0,  /* please use HTTP 1.0 in the request */
        VERSION_1_1,  /* please use HTTP 1.1 in the request */
        VERSION_2_0,  /* please use HTTP 2 in the request */
        VERSION_2TLS, /* use version 2 for HTTPS, version 1.1 for HTTP */
        VERSION_2_PRIOR_KNOWLEDGE,  /* please use HTTP 2 without HTTP/1.1
                                       Upgrade */
        VERSION_3 = 30, /* Makes use of explicit HTTP/3 without fallback.
                           Use CURLOPT_ALTSVC to enable HTTP/3 upgrade */
        VERSION_LAST /* *ILLEGAL* http version */
    };

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

    /**
     * @brief A read callback.
     * @param data received data
     * @return bytes written to the destination buffer. Zero means buffer does not have enough space to store supplied
     *         data (but the stream may be continued in the future), the supplied data is not discarded and being kept
     *         in the curl buffers.
     * @details
     * Unlike regular streams, blocking is not allowed. To indicate buffer overflow, return zero. To indicate
     * end of file, throw an AEOFException.
     */
    using WriteCallback = std::function<size_t(AByteBufferView data)>;

    /**
     * @brief A read callback.
     * @param dst destination buffer you should write to.
     * @param maxLen destination buffer size aka max length.
     * @return bytes written to the destination buffer. Zero means data unavailability (but the stream may be continued
     *         in the future).
     * @details
     * Unlike regular streams, blocking is not allowed. To indicate the data unavailability, return zero. To indicate
     * end of file, throw an AEOFException.
     */
    using ReadCallback = std::function<std::size_t(char* dst, size_t maxLen)>;
    using HeaderCallback = std::function<void(AByteBufferView)>;
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
        ReadCallback mReadCallback;
        ErrorCallback mErrorCallback;
        HeaderCallback mHeaderCallback;
        bool mThrowExceptionOnError = false;
        AVector<AString> mHeaders;
        AString mUrl;

    public:
        explicit Builder(AString url);
        Builder(const Builder&) = delete;
        ~Builder();

        Builder& withWriteCallback(WriteCallback callback) {
            assert(("write callback already set" && mWriteCallback == nullptr));
            mWriteCallback = std::move(callback);
            return *this;
        }
        Builder& withReadCallback(ReadCallback callback) {
            assert(("write callback already set" && mReadCallback == nullptr));
            mReadCallback = std::move(callback);
            return *this;
        }

        Builder& withHeaderCallback(HeaderCallback headerCallback) {
            mHeaderCallback = std::move(headerCallback);
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

        Builder& withHttpVersion(Http version);
        Builder& withUpload(bool upload);
        Builder& withCustomRequest(const AString& v);


        /**
         * @brief Appends HTTP GET params to the url.
         */
        Builder& withParams(const AVector<std::pair<AString, AString>>& params);

        Builder& withHeaders(AVector<AString> headers) {
            mHeaders = std::move(headers);
            return *this;
        }

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
	virtual ~ACurl();

    ACurl& operator=(Builder&& o) noexcept;
    ACurl& operator=(ACurl&& o) noexcept;

	int64_t getContentLength() const;

    void run();

    /**
     * @brief Breaks curl loop in the run() method, closing underlying curl connection.
     * @details
     * curl does not have a function which immediately stops the transfer
     * (see https://curl.se/docs/faq.html#How_do_I_stop_an_ongoing_transfe). The stop functionality is handled in ACurl
     * by returning error code on all callbacks. close() function is non-blocking, and some time would be taken until
     * the run() method finally returns.
     *
     * After calling close() method, none of the result signals (like fail, success) will be called.
     *
     * close() is non-blocking function.
     *
     * close() is thread-safe.
     */
    virtual void close();

    [[nodiscard]]
    void* handle() const noexcept {
        return mCURL;
    }

    [[nodiscard]]
    ResponseCode getResponseCode() const;

    [[nodiscard]]
    AString getErrorString() const noexcept {
        return AString::fromLatin1(mErrorBuffer);
    }

private:
    void* mCURL;
    struct curl_slist* mCurlHeaders = nullptr;
    char mErrorBuffer[256];
    bool mCloseRequested = false;

    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) noexcept;
    static size_t readCallback(char* ptr, size_t size, size_t nmemb, void* userdata) noexcept;
    static size_t headerCallback(char *buffer, size_t size, size_t nitems, void *userdata) noexcept;

    WriteCallback mWriteCallback;
    ReadCallback mReadCallback;
    HeaderCallback mHeaderCallback;

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


    emits<> closeRequested;
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
