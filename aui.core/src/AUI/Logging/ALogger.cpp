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

static const char* levelCStr(ALogger::Level level) {
    switch (level) {
        case ALogger::INFO:
            return "INFO";

        case ALogger::WARN:
            return "WARN";

        case ALogger::ERR:
            return "ERR";

        case ALogger::DEBUG:
            return "DEBUG";
    }

    return "UNKNOWN";
}

static ALogger& globalImpl(AOptional<APath> path = std::nullopt) {
#if AUI_PLATFORM_EMSCRIPTEN
    static ALogger l;
#else
    static ALogger l(std::move(path.valueOr(APath::getDefaultPath(APath::TEMP).makeDirs() / "aui.{}.log"_format(AProcess::self()->getPid()))));
#endif
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
            AUI_ASSERT(0);
    }
    if (message.length() == 0) {
        __android_log_print(prio, "AUI", "%s", prefix.data());
    }
    else {
        __android_log_print(prio, prefix.data(), "%s", message.data());
    }

    if (mLogFile) {
        std::time_t t = std::time(nullptr);
        std::tm* tm;
        tm = localtime(&t);
        const char* levelName = levelCStr(level);
        char timebuf[64];
        std::strftime(timebuf, sizeof(timebuf), "%H:%M:%S", tm);

        std::string threadName;
        if (auto currentThread = AThread::current()) {
            threadName = currentThread->threadName().toStdString();
        }
        else {
            threadName = "?";
        }

        std::unique_lock lock(mLogSync);
        if (message.length() == 0) {
            fprintf(mLogFile->nativeHandle(), "[%s][%s][%s]: %s\n", timebuf, threadName.c_str(), levelName, prefix.data());
        }
        else {
            fprintf(mLogFile->nativeHandle(), "[%s][%s][%s][%s]: %s\n", timebuf, threadName.c_str(), prefix.data(), levelName, message.data());
        }
        fflush(mLogFile->nativeHandle());
    }

#else
    std::time_t t = std::time(nullptr);
    std::tm* tm;
    tm = localtime(&t);
    char timebuf[64];
    std::strftime(timebuf, sizeof(timebuf), "%H:%M:%S", tm);

    std::string threadName;
    if (auto currentThread = AThread::current()) {
        threadName = currentThread->threadName().toStdString();
    }
    else {
        threadName = "?";
    }

    const char* levelName = levelCStr(level);

    std::unique_lock lock(mLogSync);
    if (message.length() == 0) {
        printf("[%s][%s][%s]: %s\n", timebuf, threadName.c_str(), levelName, prefix.data());
        if (mLogFile) {
            fprintf(mLogFile->nativeHandle(), "[%s][%s][%s]: %s\n", timebuf, threadName.c_str(), levelName, prefix.data());
        }
    }
    else {
        printf("[%s][%s][%s][%s]: %s\n", timebuf, threadName.c_str(), prefix.data(), levelName, message.data());
        if (mLogFile) {
            fprintf(mLogFile->nativeHandle(), "[%s][%s][%s][%s]: %s\n", timebuf, threadName.c_str(), prefix.data(), levelName, message.data());
        }
    }
    fflush(stdout);
    if (mLogFile) {
        fflush(mLogFile->nativeHandle());
    }
#endif
}


void ALogger::setLogFileImpl(AString path) {
    mLogFile = AFileOutputStream(std::move(path));
    log(INFO, "Logger",  ("Log file: " + mLogFile->path()).toStdString());
}

ALogger::~ALogger() {
    mLogFile.reset();
}
