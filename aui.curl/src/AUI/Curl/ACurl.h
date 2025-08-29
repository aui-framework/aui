/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstddef>
#include <optional>
#include <queue>
#include <AUI/ACurl.h>

#include "AUI/IO/AEOFException.h"
#include "AUI/IO/IInputStream.h"
#include "AUI/Traits/values.h"
#include "AFormMultipart.h"
#include <AUI/IO/APipe.h>
#include <AUI/Common/AByteBufferView.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Reflect/AEnumerate.h>
#include <AUI/Thread/AFuture.h>

class AString;
class ACurlMulti;

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

    enum class Method {
        HTTP_GET,
        HTTP_POST,
        HTTP_PUT,
        HTTP_DELETE,
    };

    /**
     * @brief Response struct for Builder::runBlocking() and Builder::runAsync()
     */
    struct Response {
        ResponseCode code;
        AString contentType;
        AByteBuffer body;
    };


    struct API_AUI_CURL ErrorDescription {
        int curlStatus;
        AString description;

        void throwException() const;
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
     * @param curl curl instance
     * @param data received data
     * @return bytes written to the destination buffer. Zero means buffer does not have enough space to store supplied
     *         data (but the stream may be continued in the future), the supplied data is not discarded and being kept
     *         in the curl buffers.
     * @details
     * Unlike regular streams, blocking is not allowed. To indicate buffer overflow, return zero. To indicate
     * end of file, throw an AEOFException.
     */
    using WriteCallbackV2 = std::function<size_t(ACurl& curl, AByteBufferView data)>;

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
        WriteCallbackV2 mWriteCallback;
        ReadCallback mReadCallback;
        ErrorCallback mErrorCallback;
        HeaderCallback mHeaderCallback;
        bool mThrowExceptionOnError = false;
        AVector<AString> mHeaders;
        AString mUrl, mParams;
        Method mMethod = Method::HTTP_GET;
        std::function<void(ACurl&)> mOnSuccess;

    public:
        explicit Builder(AString url);
        Builder(const Builder&) = delete;
        ~Builder();

        /**
         * @brief Called on server -> client data received (download).
         * @param callback callback to call.
         * @return this
         * @see withDestinationBuffer
         */
        Builder& withWriteCallback(WriteCallback callback) {
            return withWriteCallback([callback = std::move(callback)](ACurl&, AByteBufferView buffer) {
                return callback(buffer);
            });
        }

        /**
         * @brief Called on server -> client data received (download).
         * @param callback callback to call.
         * @return this
         * @see withDestinationBuffer
         */
        Builder& withWriteCallback(WriteCallbackV2 callback) {
            AUI_ASSERTX(mWriteCallback == nullptr, "write callback already set");
            mWriteCallback = std::move(callback);
            return *this;
        }

        /**
         * @brief Add multipart data.
         * @details
         * This function implies adding Content-Type: multipart and it's boundaries, setting withBody with multipart
         * data.
         */
         Builder& withMultipart(const AFormMultipart& multipart) {
            withInputStream(multipart.makeInputStream());
            mHeaders.push_back("Content-Type: multipart/form-data; boundary={}"_format(multipart.boundary()));
            return *this;
         }

        /**
         * @brief Called on client -> server data requested (upload).
         * @param callback callback to call.
         * @return this
         */
        Builder& withBody(ReadCallback callback) {
            AUI_ASSERTX(mReadCallback == nullptr, "write callback already set");
            mReadCallback = std::move(callback);
            return *this;
        }

        /**
         * @brief Called on client -> server data requested (upload).
         * @param inputStream input stream to get data to upload from.
         * @return this
         */
        Builder& withInputStream(_<IInputStream> inputStream) {
            withBody([inputStream = std::move(inputStream)](char* dst, std::size_t length) {
                auto v = inputStream->read(dst, length);
                if (v == 0) {
                    throw AEOFException();
                }
                return v;
            });
            return *this;
        }

        /**
         * @brief Specifies acceptable response time.
         * @return this
         */
        Builder& withTimeout(std::chrono::seconds timeout);

        /**
         * @brief Like withBody with callback, but wrapped with string.
         */
        Builder& withBody(std::string contents) {
            AUI_ASSERTX(mReadCallback == nullptr, "write callback already set");

            struct Body {
                explicit Body(std::string b) : contents(std::move(b)), i(contents.begin()) {}

                std::string contents;
                std::string::iterator i;
            };
            auto b = _new<Body>(std::move(contents));

            mReadCallback = [body = std::move(b)](char* dst, std::size_t length) mutable {
                if (body->i == body->contents.end()) {
                    throw AEOFException();
                }
                std::size_t remaining = std::distance(body->i, body->contents.end());
                length = glm::min(length, remaining);
                std::memcpy(dst, &*body->i, length);
                body->i += length;
                return length;
            };

            return *this;
        }

        /**
         * @brief Called on header received.
         * @param headerCallback callback to call.
         * @return this
         */
        Builder& withHeaderCallback(HeaderCallback headerCallback) {
            mHeaderCallback = std::move(headerCallback);
            return *this;
        }

        /**
         * Also disables throwing exception on error
         */
        Builder& withErrorCallback(ErrorCallback callback) {
            AUI_ASSERTX(mErrorCallback == nullptr, "error callback already set");
            mErrorCallback = std::move(callback);
            return *this;
        }

        Builder& withDestinationBuffer(aui::constraint::avoid_copy<AByteBuffer> dst) {
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
         * @param end end index of the part.
         * @return this
         */
        Builder& withRanges(size_t begin, size_t end);

        /**
         * @brief Sets: Accept-Ranges: begin-end
         *        (download part of the file)
         * @param begin start index of the part
         * @return this
         */
        Builder& withRanges(size_t begin) {
            return withRanges(begin, 0);
        }

        /**
         * @brief Set the average transfer speed in bytes per that the transfer should be below during 'low speed time'
         * seconds to consider it to be too slow and abort.
         * @param speed threshold speed (bytes per second).
         */
        Builder& withLowSpeedLimit(size_t speed);

        /**
         * @brief Duration that the transfer speed should be below the 'low speed limit' to consider it to be too slow
         * and abort
         * @param duration duration
         */
        Builder& withLowSpeedTime(std::chrono::seconds duration);


        Builder& withHttpVersion(Http version);
        Builder& withUpload(bool upload);
        Builder& withCustomRequest(const AString& v);
        Builder& withOnSuccess(std::function<void(ACurl&)> onSuccess) {
            mOnSuccess = std::move(onSuccess);
            return *this;
        }

        template<aui::invocable OnSuccess>
        Builder& withOnSuccess(OnSuccess&& onSuccess) {
            mOnSuccess = [onSuccess = std::forward<OnSuccess>(onSuccess)](ACurl&) {
                onSuccess();
            };
            return *this;
        }

        /**
         * @brief Sets HTTP method to the query.
         * @details
         * GET is by default.
         */
        Builder& withMethod(Method method) noexcept {
            mMethod = method;
            return *this;
        };


        /**
         * @brief Sets HTTP params to the query.
         * @param params params map in key,value pairs.
         * @details
         * In GET, the params are encoded and appended to the url.
         *
         * In POST, this value is used instead of readCallback (withBody).
         */
        Builder& withParams(const AVector<std::pair<AString /* key */, AString /* value */>>& params);

        /**
         * @brief Sets HTTP params to the query.
         * @details
         * In GET, the params are encoded and appended to the url.
         *
         * In POST, this value is used instead of readCallback (withBody).
         */
        Builder& withParams(AString params) noexcept {
            mParams = std::move(params);
            return *this;
        };

        Builder& withHeaders(AVector<AString> headers) {
            mHeaders = std::move(headers);
            return *this;
        }

        /**
         * @brief Makes input stream from curl builder.
         * @throws AIOException
         * @return input stream
         * @details
         * Creates async task where curl's loop lives in.
         */
        _unique<IInputStream> toInputStream();

        /**
         * @brief Constructs ACurl object and performs curl request in blocking mode. Use toFuture() instead if
         * possible.
         *
         * @throws AIOException
         */
        Response runBlocking();

         /**
          * @brief Constructs ACurl object and performs curl request in global ACurlMulti.
          * @return Response future.
          */
         AFuture<Response> runAsync();

        /**
         * @brief Constructs ACurl object and performs curl request in specified ACurlMulti.
         * @return Response future.
         */
         AFuture<Response> runAsync(ACurlMulti& curlMulti);
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

	virtual ~ACurl();

    ACurl& operator=(Builder&& o) noexcept;
    ACurl& operator=(ACurl&& o) noexcept;

	int64_t getContentLength() const;
    int64_t getNumberOfBytesDownloaded() const;
	AString getContentType() const;

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
        return AString({reinterpret_cast<const std::byte*>(mErrorBuffer), strlen(mErrorBuffer)}, AStringEncoding::LATIN1);
    }

private:
    void* mCURL;
    struct curl_slist* mCurlHeaders = nullptr;
    char mErrorBuffer[256];
    bool mCloseRequested = false;
    bool mThrowExceptionOnError = false;
    std::string mPostFieldsStorage;

    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) noexcept;
    static size_t readCallback(char* ptr, size_t size, size_t nmemb, void* userdata) noexcept;
    static size_t headerCallback(char *buffer, size_t size, size_t nitems, void *userdata) noexcept;

    WriteCallbackV2 mWriteCallback;
    ReadCallback mReadCallback;
    HeaderCallback mHeaderCallback;

    void reportSuccess() {
        emit success;
    }

    void reportFail(const ErrorDescription& errorDescription) {
        emit fail(errorDescription);
    }

    void reportFail(int statusCode) {
        reportFail(ErrorDescription{statusCode, AString({reinterpret_cast<const std::byte*>(mErrorBuffer), strlen(mErrorBuffer)}, AStringEncoding::LATIN1)});
    }

    template<typename Ret>
    Ret getInfo(int curlInfo) const;


signals:

    /**
     * @brief Emitted on network error.
     * @details
     * A protocol-level error (like HTTP(S) 404) is not treated as a fail. Check for response code via the
     * <code>getResponseCode()</code> function.
     */
    emits<ErrorDescription> fail;

    /**
     * @brief Emitted on success.
     * @details
     * A protocol-level error (like HTTP(S) 404) is not treated as a fail. Check for response code via the
     * <code>getResponseCode()</code> function.
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
