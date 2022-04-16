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

#include <AUI/Core.h>
#include <AUI/IO/IOutputStream.h>
#include <AUI/Reflect/AReflect.h>
#include "AUI/Thread/AMutex.h"

class AString;

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
            struct StackBuffer {
            private:
                char* currentIterator = buffer;
                char buffer[4096];

            public:
                size_t write(const char* t, size_t s) {
                    if (currentIterator + s <= buffer + sizeof(buffer)) {
                        std::memcpy(currentIterator, t, s);
                        currentIterator += s;
                        return s;
                    }
                    return 0;
                }
                void write(char c) {
                    if (currentIterator + sizeof(c) <= buffer + sizeof(buffer)) {
                        *currentIterator = c;
                        currentIterator += 1;
                    }
                }

                [[nodiscard]]
                std::string_view str() const {
                    // assuming there's null terminator
                    return {buffer, static_cast<std::string_view::size_type>(currentIterator - buffer) - 1};
                }
            };

            struct LazyStreamBuf final: std::streambuf {
            private:
                StackBuffer& stackBuffer;
            public:
                std::ostream stream;
                LazyStreamBuf(StackBuffer& stackBuffer) : stackBuffer(stackBuffer), stream(this) {}

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

            StackBuffer mStackBuffer;

        public:
            LogWriter(ALogger& logger, Level level, AString prefix) :
                mLogger(logger),
                mLevel(level),
                mPrefix(std::move(prefix)) {

            }

            ~LogWriter() {
                mStackBuffer.write(0); // null terminator
                auto s = mStackBuffer.str();
                mLogger.log(mLevel, mPrefix.toStdString().c_str(), s);
            }

            template<typename T>
            LogWriter& operator<<(const T& t) {
                // avoid usage of std::ostream because it's expensive
                if constexpr(std::is_constructible_v<std::string_view, T>) {
                    std::string_view stringView(t);
                    mStackBuffer.write(stringView.data(), stringView.size());
                } else if constexpr(std::is_base_of_v<AString, T>) {
                    *this << t.toStdString();
                } else if constexpr(std::is_base_of_v<AException, T>) {
                    *this << "(" << AReflect::name(&t) << ") " << t.getMessage();
                } else if constexpr(std::is_base_of_v<std::exception, T>) {
                    *this << "(" << AReflect::name(&t) << ") " << t.what();
                } else {
                    if (!mStreamBuf) {
                        mStreamBuf.emplace(mStackBuffer);
                    }
                    mStreamBuf->stream << t;
                }
                return *this;
            }
    };

private:
	ALogger();
	static ALogger& instance();

    /**
     * Writes a log entry
     * @param level log level
     * @param prefix prefix
     * @param message log message. If empty, prefix used as a message
     */
    void log(Level level, std::string_view prefix, std::string_view message);

    bool mDebug = true;
public:

    static void setDebugMode(bool debug) {
        instance().mDebug = debug;
    }
    static bool isDebug() {
        return instance().mDebug;
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