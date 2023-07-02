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
            AOptional<LazyStreamBuf> mStreamBuf;

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


    /**
     * @brief Constructor for an extra log file.
     * @param filename file name
     * @details
     * For the global logger, use ALogger::info, ALogger::warn, etc...
     */
    ALogger(AString filename) {
        setLogFileImpl(std::move(filename));
    }

    ~ALogger();

    static ALogger& global();

    static void setDebugMode(bool debug) {
        global().mDebug = debug;
    }
    static bool isDebug() {
        return global().mDebug;
    }

    static void setLogFile(APath path) {
        global().setLogFileImpl(std::move(path));
    }
    static const AString& logFile() {
        return global().mLogFile.path();
    }

    static LogWriter info(const AString& str)
    {
        return {global(), INFO, str};
    }
    static LogWriter warn(const AString& str)
    {
        return {global(), WARN, str};
    }
    static LogWriter err(const AString& str)
    {
        return {global(), ERR, str};
    }
    static LogWriter debug(const AString& str)
    {
        return {global(), DEBUG, str};
    }

    /**
     * @brief Writer a log entry with LogWriter helper.
     * @param level level
     * @param prefix prefix
     */
    LogWriter log(Level level, const AString& prefix)
    {
        return {*this, level, prefix};
    }


private:
	ALogger();

    AFileOutputStream mLogFile;
    AMutex mLocalTimeMutex;

    bool mDebug = true;

    void setLogFileImpl(AString path);


    /**
     * @brief Writes a log entry.
     * @param level log level
     * @param prefix prefix
     * @param message log message. If empty, prefix used as a message
     */
    void log(Level level, std::string_view prefix, std::string_view message);
};


template<std::size_t L, typename T, glm::qualifier Q>
inline std::ostream& operator<<(std::ostream& o, glm::vec<L, T, Q> vec) {
    o << "{ ";
    for (std::size_t i = 0; i < L; ++i) {
        if (i != 0) o << ", ";
        o << vec[i];
    }
    o << " }";

    return o;
}

#define ALOG_DEBUG(str) if (ALogger::isDebug()) ALogger::debug(str)

#include <AUI/Traits/strings.h>