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

#include <AUI/Core.h>
#include <AUI/IO/APath.h>
#include <AUI/IO/IOutputStream.h>
#include <AUI/Reflect/AReflect.h>
#include <AUI/Util/ARaiiHelper.h>
#include "AUI/Thread/AMutex.h"
#include "AUI/IO/AFileOutputStream.h"
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <AUI/Thread/AMutexWrapper.h>

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
 * ```cpp
 * ALogger::info("MyApp") << "Hello world!";
 * ```
 * Possible output:
 * ```
 * [00:47:02][UI Thread][Logger][INFO]: Hello world!
 * ```
 *
 * It's convenient to define `LOG_TAG` variable for your class:
 * ```cpp
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
 * ```
 */
class API_AUI_CORE ALogger final {
public:
    enum Level {
        INFO,
        WARN,
        ERR,
        DEBUG,
        TRACE,
    };

    struct LogWriter {
    private:
        ALogger& mLogger;
        Level mLevel;
        AStringView mPrefix;
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
            using value_type = char;

            Buffer() noexcept : mBuffer({}) {
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

            void push_back(char c) {   // for std::back_inserter
                write(c);
            }

            [[nodiscard]]
            std::string_view str() const {
                // assuming there's a null terminator
                if (std::holds_alternative<StackBuffer>(mBuffer)) {
                    auto& stack = std::get<StackBuffer>(mBuffer);
                    return {
                        stack.buffer, static_cast<std::string_view::size_type>(stack.currentIterator - stack.buffer) - 1
                    };
                }
                auto& h = std::get<HeapBuffer>(mBuffer);
                return { h.data(), h.size() };
            }
        };

        struct LazyStreamBuf final : std::streambuf {
        private:
            Buffer& stackBuffer;

        public:
            std::ostream stream;
            LazyStreamBuf(Buffer& stackBuffer) : stackBuffer(stackBuffer), stream(this) {}

        protected:
            std::streamsize xsputn(const char_type* s, std::streamsize n) override { return stackBuffer.write(s, n); }

            int overflow(int_type __c) override {
                stackBuffer.write(__c);
                return 1;
            }
        };
        AOptional<LazyStreamBuf> mStreamBuf;

        Buffer mBuffer;

        void writeTimestamp(const char* fmt, std::chrono::system_clock::time_point t) noexcept {
            fmt::format_to(std::back_inserter(mBuffer), "{}", t);
        }

    public:
        LogWriter(ALogger& logger, Level level, AStringView prefix)
          : mLogger(logger), mLevel(level), mPrefix(std::move(prefix)) {}

        ~LogWriter() {
            mBuffer.write(0);   // null terminator
            auto s = mBuffer.str();
            mLogger.log(mLevel, mPrefix, s);
        }

        template <typename T>
        LogWriter& operator<<(const T& t) noexcept {
            // avoid usage of std::ostream because it's expensive
            if constexpr (std::is_constructible_v<std::string_view, T>) {
                std::string_view stringView(t);
                mBuffer.write(stringView.data(), stringView.size());
            } else if constexpr (std::is_base_of_v<AString, T>) {
                *this << t.toStdString();
            } else if constexpr (std::is_base_of_v<std::exception, T> && !std::is_base_of_v<AException, T>) {
                *this << "(" << AReflect::name(&t) << ") " << t.what();
            } else if constexpr (std::is_same_v<std::chrono::seconds, T>) {
                writeTimestamp("%D %T", std::chrono::system_clock::time_point(t));
            } else if constexpr (std::is_same_v<std::chrono::minutes, T> || std::is_same_v<std::chrono::hours, T>) {
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
    ALogger(AString filename) { setLogFileImpl(std::move(filename)); }
    ALogger();
    ~ALogger();

    static ALogger& global();

    void setDebugMode(bool debug) { global().mDebug = debug; }
    bool isDebug() { return global().mDebug; }

    bool isTrace() { return global().mTrace; }

    /**
     * @brief Sets log file.
     * @param path path to the log file.
     * @details
     * Log file is opened immediately in setLogFile.
     *
     * If you want to change the log file of ALogger::global(), consider using ALogger::setLogFileForGlobal instead.
     * `ALogger::global().setLogFile(...)` expression would cause the default log file location to open and to close
     * immediately, when opening a log file in the specified location, causing empty file and two `Log file:` entries.
     */
    void setLogFile(APath path) { setLogFileImpl(std::move(path)); }

    /**
     * @brief Sets log file for `ALogger::global()`.
     * @param path path to the log file.
     * @see ALogger::setLogFile
     */
    static void setLogFileForGlobal(APath path);

    [[nodiscard]]
    APath logFile() {
        return mLogFile.valueOrException().path();
    }

    void onLogged(std::function<void(const AString& prefix, const AString& message, Level level)> callback) {
        std::unique_lock lock(mOnLogged);
        mOnLogged = std::move(callback);
    }
    /**
     * @brief Allows to perform some action (access safely) on log file (which is opened all over the execution process)
     * @details
     * Useful when sending log file to remote server.
     *
     * On Windows, for instance, doesn't allow to read the file when it's already opened.
     */
    template <aui::invocable Callable>
    void doLogFileAccessSafe(Callable action) {
        std::unique_lock lock(mLogSync);
        ARaiiHelper opener = [&] {
            if (!mLogFile)
                return;
            try {
                mLogFile->open(true);
            } catch (const AException& e) {
                auto path = mLogFile->path();
                mLogFile.reset();
                lock.unlock();
                log(WARN, "Logger", AStringView(fmt::format("Unable to reopen file {}: {}", path, e.getMessage())));
            }
        };
        if (!mLogFile || !mLogFile->nativeHandle()) {
            action();
            return;
        }

        mLogFile->close();
        action();
    }

    static LogWriter info(AStringView str) { return { global(), INFO, str }; }
    static LogWriter warn(AStringView str) { return { global(), WARN, str }; }
    static LogWriter err(AStringView str) { return { global(), ERR, str }; }
    static LogWriter debug(AStringView str) { return { global(), DEBUG, str }; }
    static LogWriter trace(AStringView str) { return { global(), TRACE, str }; }

    /**
     * @brief Writer a log entry with LogWriter helper.
     * @param level level
     * @param prefix prefix
     */
    LogWriter log(Level level, AStringView prefix) { return { *this, level, prefix }; }

private:
    AOptional<AFileOutputStream> mLogFile;
    AMutex mLogSync;
    AMutexWrapper<std::function<void(const AString& prefix, const AString& message, Level level)>> mOnLogged;

    bool mDebug = AUI_DEBUG;
    bool mTrace = AUI_DEBUG;

    void setLogFileImpl(AString path);

    /**
     * @brief Writes a log entry.
     * @param level log level
     * @param prefix prefix
     * @param message log message. If empty, prefix used as a message
     */
    void log(Level level, AStringView prefix, AStringView message);
};
namespace glm {
template <glm::length_t L, typename T, glm::qualifier Q>
inline std::ostream& operator<<(std::ostream& o, vec<L, T, Q> vec) {
    o << "{ ";
    for (std::size_t i = 0; i < L; ++i) {
        if (i != 0)
            o << ", ";
        o << vec[i];
    }
    o << " }";

    return o;
}

}   // namespace glm

template <glm::length_t L, typename T, glm::qualifier Q>
struct fmt::formatter<glm::vec<L, T, Q>> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Context>
    constexpr auto format(glm::vec<L, T, Q> vec, Context& ctx) const {
        auto out = ctx.out();
        out = format_to(out, FMT_STRING("{{ "));
        for (glm::length_t i = 0; i < L; ++i) {
            if (i == 0) {
                out = format_to(out, FMT_STRING("{}"), vec[i]);
            } else {
                out = format_to(out, FMT_STRING(", {}"), vec[i]);
            }
        }
        out = format_to(out, FMT_STRING(" }}"));
        return out;
    }

private:
};

#define ALOG_DEBUG(str)              \
    if (ALogger::global().isDebug()) \
    ALogger::debug(str)
#define ALOG_TRACE(str)              \
    if (ALogger::global().isTrace()) \
    ALogger::trace(str)

#include <AUI/Traits/strings.h>
