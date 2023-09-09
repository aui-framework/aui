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

#include "ALogger.h"
#include "AUI/Platform/AProcess.h"

#if AUI_PLATFORM_ANDROID
#include <android/log.h>
#else
#include <ctime>
#include <AUI/IO/AFileOutputStream.h>

#endif

ALogger::ALogger()
{
#ifdef AUI_SHARED_PTR_FIND_INSTANCES
    log(WARN, "Performance", "AUI_SHARED_PTR_FIND_INSTANCES is enabled which dramatically drops performance"
                             " since it creates stacktrace on every shared_ptr (_<T>) construction. Use it if"
                             " and only if it's actually needed.");
#endif
}

static ALogger& globalImpl(AOptional<APath> path = std::nullopt) {
    static ALogger l(std::move(path.valueOr(APath::getDefaultPath(APath::TEMP).makeDirs() / "aui.{}.log"_format(AProcess::self()->getPid()))));
    return l;
}

ALogger& ALogger::global()
{
    return globalImpl();
}

void ALogger::setLogFileForGlobal(APath path) {
    globalImpl(std::move(path));
}

void ALogger::log(Level level, std::string_view prefix, std::string_view message)
{
    {
        std::unique_lock lock(mOnLogged);
        if (mOnLogged.value()) {
            auto onLogged = mOnLogged.value();
            lock.unlock();
            onLogged(prefix, message, level);
        }
    }

#if AUI_PLATFORM_ANDROID
    int prio;
    switch (level) {
        case INFO:
            prio = ANDROID_LOG_INFO;
            break;
        case WARN:
            prio = ANDROID_LOG_WARN;
            break;
        case ERR:
            prio = ANDROID_LOG_ERROR;
            break;
        case DEBUG:
            prio = ANDROID_LOG_DEBUG;
            break;
        default:
            assert(0);
    }
    if (message.length() == 0) {
        __android_log_print(prio, "AUI", "%s", prefix.data());
    } else {
        __android_log_print(prio, prefix.data(), "%s", message.data());
    }
#else
    const char* levelName = "UNKNOWN";

    switch (level)
    {
    case INFO:
        levelName = "INFO";
        break;
    case WARN:
        levelName = "WARN";
        break;
    case ERR:
        levelName = "ERR";
        break;
    case DEBUG:
        levelName = "DEBUG";
        break;
    }

    std::time_t t = std::time(nullptr);
    std::tm* tm;
    tm = localtime(&t);
    char timebuf[64];
    std::strftime(timebuf, sizeof(timebuf), "%H:%M:%S", tm);

    std::string threadName;
    if (auto currentThread = AThread::current()) {
        threadName = currentThread->threadName().toStdString();
    } else {
        threadName = "?";
    }

    std::unique_lock lock(mLogSync);
    if (message.length() == 0) {
        printf("[%s][%s][%s]: %s\n", timebuf, threadName.c_str(), levelName, prefix.data());
        if (mLogFile) fprintf(mLogFile->nativeHandle(), "[%s][%s[%s]: %s\n", timebuf, threadName.c_str(), levelName, prefix.data());
    } else {
        printf("[%s][%s][%s][%s]: %s\n", timebuf, threadName.c_str(), prefix.data(), levelName, message.data());
        if (mLogFile) fprintf(mLogFile->nativeHandle(), "[%s][%s][%s][%s]: %s\n", timebuf, threadName.c_str(), prefix.data(), levelName, message.data());
    }
    fflush(stdout);
    if (mLogFile) fflush(mLogFile->nativeHandle());
#endif
}


void ALogger::setLogFileImpl(AString path) {
    mLogFile = AFileOutputStream(std::move(path));
    log(INFO, "Logger",  ("Log file: " + mLogFile->path()).toStdString());
}

ALogger::~ALogger() {
    mLogFile.reset();
}
