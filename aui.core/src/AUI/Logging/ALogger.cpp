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
#include "sinks/AConsoleSink.h"
#include "sinks/AAndroidSink.h"
#include "sinks/AFileSink.h"
#include "sinks/AWindowDebugSink.h"
#include "AUI/Platform/AProcess.h"
#include "AUI/Platform/Entry.h"
#include "AUI/Util/ACommandLineArgs.h"
#include "sinks/detail/LogFormat.h"
#include <fmt/format.h>

#if AUI_PLATFORM_APPLE
#include "sinks/AAppleLogSink.h"
#endif

#include <chrono>

static ALogger& globalImpl(AOptional<APath> path = std::nullopt) {
#if AUI_PLATFORM_EMSCRIPTEN
    static ALogger l;
#else
    static ALogger l(std::move(
        path.valueOr(APath::getDefaultPath(APath::TEMP).makeDirs() / "aui.{}.log"_format(AProcess::self()->getPid()))));
#endif
    return l;
}

ALogger& ALogger::global() { return globalImpl(); }

void ALogger::setLogFileForGlobal(APath path) { globalImpl(std::move(path)); }


void ALogger::log(Level level, AStringView prefix, AStringView message) {
    {
        std::unique_lock lock(mOnLogged);
        if (mOnLogged.value() && isLevelEnabled(level)) {
            auto onLogged = mOnLogged.value();
            lock.unlock();
            onLogged(prefix, message, level);
        }
    }

    auto now = std::chrono::system_clock::now();
    long long timestampMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    std::string threadName;
    if (auto currentThread = AThread::current()) {
        threadName = currentThread->threadName().toStdString();
    } else {
        threadName = "?";
    }

    ALogMessage msg;
    msg.level = level;
    msg.prefix = prefix;
    msg.message = message;
    msg.threadName = threadName;
    msg.timestampMs = timestampMs;

    std::unique_lock lock(mLogSync);
    for (auto& sink : mSinks) {
        sink->write(msg);
    }
}

void ALogger::setLogFileImpl(AString path) {
    auto sink = _new<AFileSink>(std::move(path));
    mSinks.push_back(sink);
    mLogFile = sink->fileStream();
    log(INFO, "Logger", ("Log file: " + sink->path()));
}

ALogger::ALogger() {
    if (const char* logColor = std::getenv("AUI_LOG_COLOR"); logColor && AString(logColor) == "0")
        mColorsEnabled = false;

    mSinks = defaultSinks();

#ifdef AUI_SHARED_PTR_FIND_INSTANCES
    log(WARN, "Performance",
        "AUI_SHARED_PTR_FIND_INSTANCES is enabled which dramatically drops performance"
        " since it creates stacktrace on every shared_ptr (_<T>) construction. Use it if"
        " and only if it's actually needed.");
#endif
}

ALogger::ALogger(AString filename) {
    if (const char* logColor = std::getenv("AUI_LOG_COLOR"); logColor && AString(logColor) == "0")
        mColorsEnabled = false;

    mSinks = defaultSinks();
    setLogFileImpl(std::move(filename));
}

ALogger::~ALogger() {
    for (auto& sink : mSinks) {
        sink->flush();
    }
    mLogFile.reset();
}

AVector<_<ALogSink>> ALogger::defaultSinks() {
    AVector<_<ALogSink>> sinks;
#if AUI_PLATFORM_ANDROID
    sinks.push_back(_new<AAndroidSink>());
#elif AUI_PLATFORM_APPLE
    sinks.push_back(_new<AAppleLogSink>());
#elif AUI_PLATFORM_WIN
    sinks.push_back(_new<AWindowDebugSink>());
#else
    sinks.push_back(_new<AConsoleSink>());
#endif
    return sinks;
}

void ALogger::enableColors(bool enabled) {
    mColorsEnabled = enabled;
    for (auto& sink : mSinks) {
        if (auto* console = dynamic_cast<AConsoleSink*>(sink.get())) {
            console->setColorsEnabled(enabled);
        }
    }
}

APath ALogger::logFile() {
    if (mLogFile) {
        return mLogFile->path();
    }
    throw AException("No log file configured");
}

bool ALogger::isTraceImpl() {
    return std::getenv("AUI_TRACE") != nullptr;
}
