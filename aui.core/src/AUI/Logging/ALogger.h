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
#include "AUI/Thread/AMutex.h"
#include "AUI/IO/AFileOutputStream.h"
#include <sstream>

class AString;

class API_AUI_CORE ALogger
{
public:
	enum Level
	{
		INFO,
		WARN,
		ERR,
        DEBUG,
	};
private:
	ALogger();
	static ALogger& instance();
	AMutex mSync;
	_<AFileOutputStream> mLogFile;
	
	void log(Level level, const AString& str);

    void rawWrite(const char* data, size_t length);

public:

    static void setLogFile(const AString& str);

    struct RawLogPusher {
        friend class ALogger;
    private:
        ALogger& inst;

        explicit RawLogPusher(ALogger& l): inst(l) {

        }
    public:

        template<typename T>
        RawLogPusher& operator<<(const T& t) {
            std::stringstream ss;
            ss << t;
            auto s = ss.str();
            inst.rawWrite(s.c_str(), s.length());
            return *this;
        }
    };

    static RawLogPusher raw() {
        return RawLogPusher{instance()};
    }

	static void info(const AString& str)
	{
		instance().log(INFO, str);
	}	
	static void warn(const AString& str)
	{
		instance().log(WARN, str);
	}	
	static void err(const AString& str)
	{
		instance().log(ERR, str);
	}
	static void debug(const AString& str)
	{
		instance().log(DEBUG, str);
	}
};

#include <AUI/Traits/strings.h>