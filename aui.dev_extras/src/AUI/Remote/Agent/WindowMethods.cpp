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

/// How long the size has to stay the same for the resize to be considered done; the window manager may adjust the
/// size a few times after the request.
constexpr auto SETTLE = 100ms;

int sizeParam(const AJson& params, const char* name) {
    auto v = params.containsOpt(name);
    if (!v || !v->isInt() || v->asInt() <= 0) {
        throw RpcException(rpc_code::INVALID_PARAMS, AString(name) + " must be a positive integer",
                           "pass width and height in the units of window.list");
    }
    return v->asInt();
}
}   // namespace

void registerWindowMethods(JsonRpcDispatcher& dispatcher, RefTable& refs) {
    // params: width, height, windowId (optional), timeoutMs (default 3000)
    dispatcher.registerMethod("window.resize", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        const glm::ivec2 requested { sizeParam(params, "width"), sizeParam(params, "height") };
        const auto timeout = millisParam(params, "timeoutMs", 3s);

        struct State {
            bool started = false;
            _weak<AWindow> window;
            glm::ivec2 lastSize { 0, 0 };
            Clock::time_point stableSince;
        };
        auto state = std::make_shared<State>();
        auto poller = std::make_shared<Poller>(
            [&refs, params = params, requested, state](std::chrono::milliseconds) -> AOptional<AJson> {
                const auto now = Clock::now();
                if (!state->started) {
                    state->started = true;
                    auto window = windowFromParams(params, refs);
                    // the window manager would clamp it with the size hints, but not every platform has them
                    window->setSize(glm::max(requested, window->getMinimumSize()));
                    // setSize does not lay the contents out, and the window manager does not report a size the window
                    // already has
                    window->requestLayout();
                    state->window = window;
                    state->lastSize = window->getSize();
                    state->stableSince = now;
                    return std::nullopt;
                }
                auto window = state->window.lock();
                if (!window) {
                    throw RpcException(rpc_code::NO_SUCH_WINDOW, "the window was closed while resizing",
                                       "list the windows with window.list");
                }
                // an animation may keep scheduling frames, so only the layout is waited for
                if (window->hasPendingLayout() || window->getSize() != state->lastSize) {
                    state->lastSize = window->getSize();
                    state->stableSince = now;
                    return std::nullopt;
                }
                if (now - state->stableSince < SETTLE) {
                    return std::nullopt;
                }
                const auto size = window->getSize();
                AJson result {
                    { "width", size.x },
                    { "height", size.y },
                };
                if (size != requested) {
                    // the window doesn't go below the size its contents need, the window manager has a say as well
                    const auto minimum = window->getMinimumSize();
                    result["requestedWidth"] = requested.x;
                    result["requestedHeight"] = requested.y;
                    result["minWidth"] = minimum.x;
                    result["minHeight"] = minimum.y;
                }
                return result;
            },
            timeout, "the window to be resized");
        return poller->start();
    }));
}

}   // namespace aui::remote
