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

#include "Methods.h"

#include <deque>

#include "AUI/Logging/ALogger.h"

namespace aui::remote {

namespace {

constexpr std::size_t LOG_CAPACITY = 2000;

const char* levelName(ALogger::Level level) {
    switch (level) {
        case ALogger::INFO: return "info";
        case ALogger::WARN: return "warn";
        case ALogger::ERR: return "error";
        case ALogger::DEBUG: return "debug";
        case ALogger::TRACE: return "trace";
    }
    return "info";
}

/// Keeps the recent log entries; every entry gets a growing sequence number so a client can ask for what is new.
class LogBuffer {
public:
    LogBuffer() {
        mSink = _new<ALogger::CallbackSink>([this](const ALogger::Entry& entry) {
            std::lock_guard lock(mMutex);
            mEntries.push_back(Stored { mNext++, entry.level, AString(entry.prefix), AString(entry.message) });
            if (mEntries.size() > LOG_CAPACITY) {
                mEntries.pop_front();
            }
        });
        ALogger::global().addSink(mSink);
    }

    ~LogBuffer() { ALogger::global().removeSink(mSink); }

    AJson query(std::uint64_t since, AOptional<ALogger::Level> maxLevel, std::size_t limit) {
        std::lock_guard lock(mMutex);
        aui::impl::JsonArray entries;
        for (const auto& e : mEntries) {
            if (e.seq < since || (maxLevel && int(e.level) > int(*maxLevel))) {
                continue;
            }
            entries.push_back(AJson {
              { "seq", static_cast<std::int64_t>(e.seq) },
              { "level", levelName(e.level) },
              { "tag", e.prefix },
              { "message", e.message },
            });
        }
        // the most recent ones
        while (entries.size() > limit) {
            entries.erase(entries.begin());
        }
        return AJson {
            { "entries", AJson(std::move(entries)) },
            { "next", static_cast<std::int64_t>(mNext) },
        };
    }

private:
    struct Stored {
        std::uint64_t seq;
        ALogger::Level level;
        AString prefix;
        AString message;
    };
    std::mutex mMutex;
    std::deque<Stored> mEntries;
    std::uint64_t mNext = 0;
    _<ALogger::CallbackSink> mSink;
};

}   // namespace

void registerDiagnosticsMethods(JsonRpcDispatcher& dispatcher) {
    auto buffer = std::make_shared<LogBuffer>();

    // params: since? (the `next` of the previous call), level? (error|warn|info|debug|trace: the most verbose to
    //         include, default debug), limit? (default 200)
    dispatcher.registerMethod("diag.log", JsonRpcDispatcher::SyncHandler([buffer](const AJson& params) {
        const std::uint64_t since = params.containsOpt("since") ? std::uint64_t(params["since"].asInt()) : 0;
        const std::size_t limit = params.containsOpt("limit") ? std::size_t(std::max(0, params["limit"].asInt())) : 200;
        AOptional<ALogger::Level> level;
        if (auto l = params.containsOpt("level")) {
            const auto name = l->asString().lowercase();
            if (name == "error") level = ALogger::ERR;
            else if (name == "warn") level = ALogger::WARN;
            else if (name == "info") level = ALogger::INFO;
            else if (name == "debug") level = ALogger::DEBUG;
            else if (name == "trace") level = ALogger::TRACE;
            else {
                throw RpcException(rpc_code::INVALID_PARAMS, "unknown level: " + name,
                                   "error, warn, info, debug or trace");
            }
        }
        return buffer->query(since, level, limit);
    }));
}

}   // namespace aui::remote
