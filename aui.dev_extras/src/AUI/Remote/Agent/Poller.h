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

#include <chrono>
#include <functional>
#include <memory>

#include "AUI/Json/AJson.h"
#include "AUI/Platform/ASurface.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Thread/AFuture.h"
#include "AUI/Thread/AThread.h"
#include "JsonRpc.h"

namespace aui::remote {

/**
 * @brief Whether a frame or a layout is pending in any window, or the UI thread has queued work. Must be called on the
 * UI thread.
 */
inline bool uiBusy() {
    if (!AThread::main()->messageQueueEmpty()) {
        return true;
    }
    for (const auto& window : ASurface::getWindowManager().getWindows()) {
        if (window->isFrameScheduled() || window->hasPendingLayout()) {
            return true;
        }
    }
    return false;
}

inline std::chrono::milliseconds millisParam(const AJson& params, const char* name,
                                             std::chrono::milliseconds fallback) {
    if (auto v = params.containsOpt(name)) {
        return std::chrono::milliseconds(std::max(0, v->asInt()));
    }
    return fallback;
}

/**
 * @brief Calls `check` on the UI thread every few milliseconds until it returns a value or the timeout elapses.
 * @details
 * `check` returns nothing while the condition isn't met. On timeout the future fails with TIMEOUT.
 */
class Poller : public std::enable_shared_from_this<Poller> {
public:
    using Clock = std::chrono::steady_clock;
    using Check = std::function<AOptional<AJson>(std::chrono::milliseconds waited)>;

    static constexpr auto PERIOD = std::chrono::milliseconds(10);

    Poller(Check check, std::chrono::milliseconds timeout, AString what)
      : mCheck(std::move(check)), mDeadline(Clock::now() + timeout), mStarted(Clock::now()), mWhat(std::move(what)) {}

    AFuture<AJson> start() {
        return AFuture<AJson>::create([&](AFuture<AJson>::Resolver resolver) {
            mResolver.emplace(std::move(resolver));
            schedule();
        });
    }

private:
    Check mCheck;
    Clock::time_point mDeadline;
    Clock::time_point mStarted;
    AString mWhat;
    AOptional<AFuture<AJson>::Resolver> mResolver;

    void schedule() {
        AThread::main()->enqueue([self = shared_from_this()] { self->tick(); });
    }

    void tick() {
        if (mResolver->isCancelled()) {
            return;
        }
        const auto waited = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - mStarted);
        try {
            if (auto result = mCheck(waited)) {
                mResolver->supplyValue(std::move(*result));
                return;
            }
            if (Clock::now() >= mDeadline) {
                throw RpcException(rpc_code::TIMEOUT, "timed out waiting for " + mWhat,
                                   "increase timeoutMs or check the application state with view.tree and screenshot");
            }
        } catch (...) {
            mResolver->supplyException();
            return;
        }
        AThread::asyncSleep(PERIOD).onValue([self = shared_from_this()] { self->schedule(); }).keepAlive();
    }
};

}   // namespace aui::remote
