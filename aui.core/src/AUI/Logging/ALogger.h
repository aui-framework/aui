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

#include <AUI/Core.h>
#include <AUI/IO/IOutputStream.h>
#include <AUI/Reflect/AReflect.h>
#include "AUI/Thread/AMutex.h"
#include "AUI/IO/AFileOutputStream.h"

class AString;

/**
 * @brief A logger class.
 * @ingroup core
 * @details
 * Logger is used for journaling application events like errors or some user actions.
 *
 * It provides some extra functions like logging to file and output formatting.
 *
 * It provides `std::cout`-like API.
 *
 * Example:
 * @code{cpp}
 * ALogger::info("MyApp") << "Hello world!";
 * @endcode
 * Possible output:
 * ```
 * [00:47:02][UI Thread][Logger][INFO]: Hello world!
 * ```
 *
 * It's convenient to define `LOG_TAG` variable for your class:
 * @code{cpp}
 * static constexpr auto LOG_TAG = "MyDownloader";
 * class MyDownloader {
 * public:
 *   void someAction() {
 *     ALogger::info(LOG_TAG) << "someAction() called";
 *   }
 *   void downloadFile(const AString& name) {
 *     ALogger::info(LOG_TAG) << "Downloading file: " << name;
 *   }
 * }
 * @endcode
 */
class API_AUI_CORE ALogger final
{
public:
	enum Level
	{
		INFO,
		WARN,
		ERR,
        DEBUG,
	};


    struct LogWriter {
        private:
            ALogger& mLogger;
            Level mLevel;
            AString mPrefix;
            struct Buffer {
            private:
                struct StackBuffer {
                    char buffer[2048];
                    char* currentIterator;
                };
                using HeapBuffer = AVector<char>;
                std::variant<StackBuffer, HeapBuffer> mBuffer;

                void switchToHeap() {
                    HeapBuffer h;
                    h.reserve(16384);
                    auto& stack = std::get<StackBuffer>(mBuffer);
                    h.insert(h.end(), stack.buffer, stack.currentIterator);
                    mBuffer = std::move(h);
                }
            public:
                Buffer() noexcept: mBuffer({}) {
                    auto& sb = std::get<StackBuffer>(mBuffer);
                    sb.currentIterator = sb.buffer;
                }
                size_t write(const char* t, size_t s) {
                    if (std::holds_alternative<StackBuffer>(mBuffer)) {
                        auto& stack = std::get<StackBuffer>(mBuffer);
                        if (stack.currentIterator + s <= stack.buffer + sizeof(stack.buffer)) {
                            std::memcpy(stack.currentIterator, t, s);
                            stack.currentIterator += s;
                            return s;
                        }
                        switchToHeap();
                    }
                    auto& h = std::get<HeapBuffer>(mBuffer);
                    h.insert(h.end(), t, t + s);
                    return s;
                }
                void write(char c) {
                    if (std::holds_alternative<StackBuffer>(mBuffer)) {
                        auto& stack = std::get<StackBuffer>(mBuffer);
                        if (stack.currentIterator + sizeof(c) <= stack.buffer + sizeof(stack.buffer)) {
                            *stack.currentIterator = c;
                            stack.currentIterator += 1;
                            return;
                        }
                        switchToHeap();
                    }
                    std::get<HeapBuffer>(mBuffer).push_back(c);
                }

                [[nodiscard]]
                std::string_view str() const {
                    // assuming there's a null terminator
                    if (std::holds_alternative<StackBuffer>(mBuffer)) {
                        auto& stack = std::get<StackBuffer>(mBuffer);
                        return {stack.buffer, static_cast<std::string_view::size_type>(stack.currentIterator - stack.buffer) - 1};
                    }
                    auto& h = std::get<HeapBuffer>(mBuffer);
                    return {h.data(), h.size()};
                }
            };

            struct LazyStreamBuf final: std::streambuf {
            private:
                Buffer& stackBuffer;
            public:
                std::ostream stream;
                LazyStreamBuf(Buffer& stackBuffer) : stackBuffer(stackBuffer), stream(this) {}

            protected:
                std::streamsize xsputn(const char_type* s, std::streamsize n) override {
                    return stackBuffer.write(s, n);
                }

                int overflow(int_type __c) override {
                    stackBuffer.write(__c);
                    return 1;
                }
            };
            std::optional<LazyStreamBuf> mStreamBuf;

            Buffer mBuffer;

            void writeTimestamp(const char* fmt, std::chrono::system_clock::time_point t) noexcept {
                char buf[128];
                auto inTimeT = std::chrono::system_clock::to_time_t(t);
                std::size_t strLen;
                {
                    std::unique_lock lock(mLogger.mLocalTimeMutex);
                    strLen = std::strftime(buf, sizeof(buf), fmt, std::localtime(&inTimeT));
                }
                mBuffer.write(buf, strLen);
            }

        public:
            LogWriter(ALogger& logger, Level level, AString prefix) :
                mLogger(logger),
                mLevel(level),
                mPrefix(std::move(prefix)) {

            }

            ~LogWriter() {
                mBuffer.write(0); // null terminator
                auto s = mBuffer.str();
                mLogger.log(mLevel, mPrefix.toStdString().c_str(), s);
            }

            template<typename T>
            LogWriter& operator<<(const T& t) noexcept {
                // avoid usage of std::ostream because it's expensive
                if constexpr(std::is_constructible_v<std::string_view, T>) {
                    std::string_view stringView(t);
                    mBuffer.write(stringView.data(), stringView.size());
                } else if constexpr(std::is_base_of_v<AString, T>) {
                    *this << t.toStdString();
                } else if constexpr(std::is_base_of_v<std::exception, T> && !std::is_base_of_v<AException, T>) {
                    *this << "(" << AReflect::name(&t) << ") " << t.what();
                } else if constexpr(std::is_same_v<std::chrono::seconds, T>) {
                    writeTimestamp("%D %T", std::chrono::system_clock::time_point(t));
                } else if constexpr(std::is_same_v<std::chrono::minutes, T> || std::is_same_v<std::chrono::hours, T>) {
                    writeTimestamp("%D %R", std::chrono::system_clock::time_point(t));
                } else {
                    if (!mStreamBuf) {
                        mStreamBuf.emplace(mBuffer);
                    }
                    mStreamBuf->stream << t;
                }
                return *this;
            }
    };

private:
	ALogger();
    ~ALogger();
	static ALogger& instance();

    AFileOutputStream mLogFile;
    AMutex mLocalTimeMutex;

    /**
     * Writes a log entry
     * @param level log level
     * @param prefix prefix
     * @param message log message. If empty, prefix used as a message
     */
    void log(Level level, std::string_view prefix, std::string_view message);

    bool mDebug = true;

    void setLogFileImpl(AString path);

public:

    static void setDebugMode(bool debug) {
        instance().mDebug = debug;
    }
    static bool isDebug() {
        return instance().mDebug;
    }

    static void setLogFile(APath path) {
        instance().setLogFileImpl(std::move(path));
    }
    static const AString& logFile() {
        return instance().mLogFile.path();
    }

	static LogWriter info(const AString& str)
	{
		return {instance(), INFO, str};
	}	
	static LogWriter warn(const AString& str)
	{
        return {instance(), WARN, str};
	}	
	static LogWriter err(const AString& str)
	{
        return {instance(), ERR, str};
	}
	static LogWriter debug(const AString& str)
	{
        return {instance(), DEBUG, str};
	}
};


#define ALOG_DEBUG(str) if (ALogger::isDebug()) ALogger::debug(str)

#include <AUI/Traits/strings.h>