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

#include "Poller.h"

namespace aui::remote {

namespace {
using Clock = Poller::Clock;
using namespace std::chrono_literals;
}   // namespace

void registerWaitMethods(JsonRpcDispatcher& dispatcher, RefTable& refs) {
    // params: timeoutMs (default 5000), quietMs (default 100): how long nothing may be going on
    dispatcher.registerMethod("wait.idle", JsonRpcDispatcher::Handler([](const AJson& params) {
        const auto timeout = millisParam(params, "timeoutMs", 5s);
        const auto quiet = millisParam(params, "quietMs", 100ms);
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
        const auto timeout = millisParam(params, "timeoutMs", 5s);
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
