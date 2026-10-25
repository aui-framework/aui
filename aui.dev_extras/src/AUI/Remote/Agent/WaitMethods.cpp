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

#include <chrono>
#include <functional>

#include "AUI/Platform/ASurface.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Thread/AThread.h"
#include "UiThread.h"

namespace aui::remote {

namespace {
using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;

constexpr auto POLL_PERIOD = 10ms;

bool surfaceBusy(const _<ASurface>& surface) { return surface->isFrameScheduled() || surface->hasPendingLayout(); }

/// Must be called on the UI thread.
bool uiBusy() {
    if (!AThread::main()->messageQueueEmpty()) {
        return true;
    }
    for (const auto& window : ASurface::getWindowManager().getWindows()) {
        if (surfaceBusy(window)) {
            return true;
        }
    }
    return false;
}

std::chrono::milliseconds millis(const AJson& params, const char* name, std::chrono::milliseconds fallback) {
    if (auto v = params.containsOpt(name)) {
        return std::chrono::milliseconds(std::max(0, v->asInt()));
    }
    return fallback;
}

/// Calls `check` on the UI thread every POLL_PERIOD until it returns a value or the timeout elapses.
/// `check` returns nothing while the condition isn't met.
class Poller : public std::enable_shared_from_this<Poller> {
public:
    using Check = std::function<AOptional<AJson>(std::chrono::milliseconds waited)>;

    Poller(Check check, std::chrono::milliseconds timeout, AString what)
      : mCheck(std::move(check)), mDeadline(Clock::now() + timeout), mStarted(Clock::now()), mWhat(std::move(what)) {}

    AFuture<AJson> start() {
        schedule();
        return mResult;
    }

private:
    Check mCheck;
    Clock::time_point mDeadline;
    Clock::time_point mStarted;
    AString mWhat;
    AFuture<AJson> mResult;

    void schedule() {
        AThread::main()->enqueue([self = shared_from_this()] { self->tick(); });
    }

    void tick() {
        const auto waited = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - mStarted);
        try {
            if (auto result = mCheck(waited)) {
                mResult.supplyValue(std::move(*result));
                return;
            }
            if (Clock::now() >= mDeadline) {
                throw RpcException(rpc_code::TIMEOUT, "timed out waiting for " + mWhat,
                                   "increase timeoutMs or check the application state with view.tree and screenshot");
            }
        } catch (...) {
            mResult.supplyException();
            return;
        }
        AThread::asyncSleep(POLL_PERIOD).onSuccess([self = shared_from_this()] { self->schedule(); });
    }
};

}   // namespace

void registerWaitMethods(JsonRpcDispatcher& dispatcher, RefTable& refs) {
    // params: timeoutMs (default 5000), quietMs (default 100): how long nothing may be going on
    dispatcher.registerMethod("wait.idle", JsonRpcDispatcher::Handler([](const AJson& params) {
        const auto timeout = millis(params, "timeoutMs", 5s);
        const auto quiet = millis(params, "quietMs", 100ms);
        auto quietSince = std::make_shared<AOptional<Clock::time_point>>();
        auto poller = std::make_shared<Poller>(
            [quiet, quietSince](std::chrono::milliseconds waited) -> AOptional<AJson> {
                if (uiBusy()) {
                    *quietSince = std::nullopt;
                    return std::nullopt;
                }
                if (!*quietSince) {
                    *quietSince = Clock::now();
                }
                if (Clock::now() - **quietSince >= quiet) {
                    return AJson { { "idle", true }, { "waitedMs", int(waited.count()) } };
                }
                return std::nullopt;
            },
            timeout, "the UI to become idle");
        return poller->start();
    }));

    // params: the filters of view.find (type, text, assName, ref, windowId), state (visible (default)|gone),
    //         timeoutMs (default 5000)
    dispatcher.registerMethod("wait.view", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        const auto timeout = millis(params, "timeoutMs", 5s);
        const bool gone = params.containsOpt("state") && params["state"].asString() == "gone";
        auto poller = std::make_shared<Poller>(
            [&refs, params = params, gone](std::chrono::milliseconds waited) -> AOptional<AJson> {
                auto found = findViews(refs, params);
                const auto& views = found.asArray();
                if (gone) {
                    if (views.empty()) {
                        return AJson { { "waitedMs", int(waited.count()) } };
                    }
                    return std::nullopt;
                }
                if (views.empty()) {
                    return std::nullopt;
                }
                return AJson { { "waitedMs", int(waited.count()) }, { "matches", std::move(found) } };
            },
            timeout, gone ? "the view to disappear" : "the view to appear");
        return poller->start();
    }));
}

}   // namespace aui::remote
