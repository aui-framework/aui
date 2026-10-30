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

#include "AUI/View/AScrollArea.h"
#include "Poller.h"
#include "ViewJson.h"

namespace aui::remote {

namespace {
using Clock = Poller::Clock;
using namespace std::chrono_literals;

/// How long the size has to stay the same for the resize to be considered done; the window manager may adjust the
/// size a few times after the request.
constexpr auto SETTLE = 100ms;

/// The largest size window.resize asks for, per axis: renderers can't draw much more anyway, and a typo stays a typo.
constexpr int MAX_WINDOW_SIZE = 16384;

constexpr float MIN_SCALE = 0.25f;
constexpr float MAX_SCALE = 4.f;

int sizeParam(const AJson& params, const char* name) {
    auto v = params.containsOpt(name);
    if (!v || !v->isInt() || v->asInt() <= 0) {
        throw RpcException(rpc_code::INVALID_PARAMS, AString(name) + " must be a positive integer",
                           "pass width and height in the units of window.list");
    }
    return v->asInt();
}

ARect<int> rectInWindow(const AView& view) {
    return ARect<int>::fromTopLeftPositionAndSize(view.getPositionInWindow(), view.getSize());
}

bool contains(const ARect<int>& outer, const ARect<int>& inner) {
    return inner.p1.x >= outer.p1.x && inner.p1.y >= outer.p1.y && inner.p2.x <= outer.p2.x && inner.p2.y <= outer.p2.y;
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
                    window->setSize(glm::max(glm::min(requested, glm::ivec2(MAX_WINDOW_SIZE)), window->getMinimumSize()));
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
                    // the window doesn't go below the size its contents need nor above the limit, the window
                    // manager has a say as well
                    const auto minimum = window->getMinimumSize();
                    result["requestedWidth"] = requested.x;
                    result["requestedHeight"] = requested.y;
                    result["minWidth"] = minimum.x;
                    result["minHeight"] = minimum.y;
                    result["maxWidth"] = MAX_WINDOW_SIZE;
                    result["maxHeight"] = MAX_WINDOW_SIZE;
                }
                return result;
            },
            timeout, "the window to be resized");
        return poller->start();
    }));

    // params: factor (0.25..4) | reset: true, windowId (optional), timeoutMs (default 3000)
    dispatcher.registerMethod("window.setScale", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        float factor = 1.f;
        if (auto reset = params.containsOpt("reset"); reset && reset->isBool() && reset->asBool()) {
            factor = 1.f;
        } else if (auto v = params.containsOpt("factor"); v && v->isNumber()) {
            factor = float(v->asNumber());
            if (!(factor >= MIN_SCALE && factor <= MAX_SCALE)) {
                throw RpcException(rpc_code::INVALID_PARAMS, "factor must be within 0.25..4",
                                   "1 is the system scale; pass reset: true to return to it");
            }
        } else {
            throw RpcException(rpc_code::INVALID_PARAMS, "missing param: factor", "pass factor (i.e. 1.5) or reset: true");
        }
        const auto timeout = millisParam(params, "timeoutMs", 3s);

        struct State {
            bool started = false;
            _weak<AWindow> window;
            Clock::time_point stableSince;
        };
        auto state = std::make_shared<State>();
        auto poller = std::make_shared<Poller>(
            [&refs, params = params, factor, state](std::chrono::milliseconds) -> AOptional<AJson> {
                const auto now = Clock::now();
                if (!state->started) {
                    state->started = true;
                    auto window = windowFromParams(params, refs);
                    auto scaling = window->getScalingParams();
                    scaling.scalingFactor = factor;
                    window->setScalingParams(std::move(scaling));
                    state->window = window;
                    state->stableSince = now;
                    return std::nullopt;
                }
                auto window = state->window.lock();
                if (!window) {
                    throw RpcException(rpc_code::NO_SUCH_WINDOW, "the window was closed while scaling",
                                       "list the windows with window.list");
                }
                if (window->hasPendingLayout()) {
                    state->stableSince = now;
                    return std::nullopt;
                }
                if (now - state->stableSince < SETTLE) {
                    return std::nullopt;
                }
                return AJson {
                    { "scalingFactor", window->getScalingParams().scalingFactor },
                    { "dpi", window->getDpiRatio() },
                    { "width", window->getSize().x },
                    { "height", window->getSize().y },
                };
            },
            timeout, "the window to be rescaled");
        return poller->start();
    }));

    // params: ref, timeoutMs (default 3000)
    dispatcher.registerMethod("view.scrollIntoView", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        auto ref = params.containsOpt("ref");
        if (!ref || !ref->isString()) {
            throw RpcException(rpc_code::INVALID_PARAMS, "missing param: ref", "take a ref from view.find or view.tree");
        }
        const auto timeout = millisParam(params, "timeoutMs", 3s);

        struct State {
            bool started = false;
            _weak<AView> view;
            int scrollAreas = 0;
        };
        auto state = std::make_shared<State>();
        auto poller = std::make_shared<Poller>(
            [&refs, ref = ref->asString(), state](std::chrono::milliseconds) -> AOptional<AJson> {
                if (!state->started) {
                    state->started = true;
                    auto view = refs.resolve(ref);
                    state->view = view;
                    // the innermost area first: scrolling it moves the view within the outer ones
                    for (auto parent = view->getParent(); parent; parent = parent->getParent()) {
                        if (auto area = dynamic_cast<AScrollArea*>(parent)) {
                            area->scrollTo(view);
                            ++state->scrollAreas;
                        }
                    }
                    return std::nullopt;
                }
                auto view = state->view.lock();
                if (!view) {
                    throw RpcException(rpc_code::STALE_REF, "the view was removed while scrolling",
                                       "find it again with view.find");
                }
                auto window = view->getWindow();
                if (window && window->hasPendingLayout()) {
                    return std::nullopt;
                }
                const auto rect = rectInWindow(*view);
                bool visible = !window || contains(rectInWindow(*window), rect);
                for (auto parent = view->getParent(); parent && visible; parent = parent->getParent()) {
                    if (auto area = dynamic_cast<AScrollArea*>(parent)) {
                        visible = contains(rectInWindow(*area), rect);
                    }
                }
                return AJson {
                    { "rect", view_json::geometry(view) },
                    { "scrollAreas", state->scrollAreas },
                    { "visible", visible },
                };
            },
            timeout, "the scroll to finish");
        return poller->start();
    }));
}

}   // namespace aui::remote
