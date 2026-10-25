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

#include <algorithm>

#include "AUI/Event/AInjectedEvent.h"
#include "AUI/Platform/AInput.h"
#include "AUI/Platform/AWindow.h"
#include "UiThread.h"

namespace aui::remote {

namespace {

AInput::Key keyByName(AString name) {
    name = name.uppercase();
    static const AMap<AString, AString> ALIASES = {
        { "ENTER", "RETURN" },    { "ESC", "ESCAPE" },       { "CTRL", "LCONTROL" }, { "CONTROL", "LCONTROL" },
        { "SHIFT", "LSHIFT" },    { "ALT", "LALT" },         { "CMD", "LSYSTEM" },   { "META", "LSYSTEM" },
        { "SUPER", "LSYSTEM" },   { "DELETE", "DEL" },
    };
    if (auto alias = ALIASES.contains(name)) {
        name = ALIASES.at(name);
    }
    const auto& names = AEnumerate<AInput::Key>::nameToValueMap();
    for (const auto& candidate : { name, "AInput::" + name }) {
        if (auto it = names.find(candidate); it != names.end()) {
            return it->second;
        }
    }
    throw RpcException(rpc_code::INVALID_PARAMS, "unknown key: " + name,
                       "use AInput::Key names, i.e. RETURN, ESCAPE, TAB, LEFT, F5, A, NUM1");
}

AInput::Key buttonByName(const AJson& params) {
    if (auto b = params.containsOpt("button")) {
        const auto name = b->asString().lowercase();
        if (name == "left") return AInput::LBUTTON;
        if (name == "right") return AInput::RBUTTON;
        if (name == "middle") return AInput::CBUTTON;
        throw RpcException(rpc_code::INVALID_PARAMS, "unknown button: " + name, "left, right or middle");
    }
    return AInput::LBUTTON;
}

AVector<AInput::Key> modifiersOf(const AJson& params) {
    AVector<AInput::Key> result;
    if (auto m = params.containsOpt("modifiers")) {
        for (const auto& name : m->asArray()) {
            result << keyByName(name.asString());
        }
    }
    return result;
}

/// Makes AInput::isKeyDown report the keys as pressed while the synthetic input is being delivered.
class OverrideScope {
public:
    OverrideScope(AVector<AInput::Key> keys) : mKeys(std::move(keys)) {
        for (auto k : mKeys) {
            AInput::overrideState(k, true);
        }
    }
    ~OverrideScope() {
        for (auto k : mKeys) {
            AInput::resetOverride(k);
        }
    }
    OverrideScope(const OverrideScope&) = delete;

private:
    AVector<AInput::Key> mKeys;
};

struct Target {
    _<AWindow> window;
    _<AView> view;   // may be null
    glm::vec2 position;
};

float number(const AJson& params, const char* name) {
    if (auto v = params.containsOpt(name)) {
        return v->isInt() ? float(v->asInt()) : float(v->asNumber());
    }
    throw RpcException(rpc_code::INVALID_PARAMS, AString("missing param: ") + name);
}

/// A point by `<prefix>ref` (center of the view) or `<prefix>x`/`<prefix>y` (window coordinates).
Target resolveTarget(const AJson& params, RefTable& refs, const char* prefix = "") {
    const AString refName = AString(prefix) + "ref";
    if (auto ref = params.containsOpt(refName.toStdString().c_str())) {
        auto view = refs.resolve(ref->asString());
        auto window = _cast<AWindow>(aui::ptr::shared_from_this(view->getWindow()));
        if (!window) {
            throw RpcException(rpc_code::NO_SUCH_WINDOW, "the view is not attached to a window",
                               "it may have been removed; re-run view.find");
        }
        return { window, view, glm::vec2(view->getCenterPointInWindow()) };
    }
    const AString x = AString(prefix) + "x", y = AString(prefix) + "y";
    return { windowFromParams(params, refs), nullptr,
             glm::vec2(number(params, x.toStdString().c_str()), number(params, y.toStdString().c_str())) };
}

AJson describePoint(const Target& target) {
    return AJson {
        { "x", int(target.position.x) },
        { "y", int(target.position.y) },
    };
}

/// Runs `first` on the UI thread, then `second` on the next iteration of its queue, so asynchronous effects of `first`
/// (i.e. AView::focus) are applied.
template <typename First, typename Second>
AFuture<AJson> inTwoSteps(First first, Second second) {
    AFuture<AJson> result;
    AThread::main()->enqueue([result, first = std::move(first), second = std::move(second)]() mutable {
        try {
            first();
        } catch (...) {
            result.supplyException();
            return;
        }
        AThread::main()->enqueue([result, second = std::move(second)]() mutable {
            try {
                result.supplyValue(second());
            } catch (...) {
                result.supplyException();
            }
        });
    });
    return result;
}

}   // namespace

void registerInputMethods(JsonRpcDispatcher& dispatcher, RefTable& refs) {
    // params: action (click|doubleClick|press|release|move|drag|scroll; default click), ref | x, y,
    //         button (left|right|middle), modifiers, drag: toRef | toX, toY, scroll: dx, dy
    dispatcher.registerMethod("input.pointer", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        return onUiThread([&refs, params = params] {
            const auto action = params.containsOpt("action") ? params["action"].asString() : AString("click");
            const auto button = buttonByName(params);
            const auto index = APointerIndex::button(button);
            auto target = resolveTarget(params, refs);
            auto& window = target.window;
            auto position = target.position;

            OverrideScope modifiers(modifiersOf(params));
            auto press = [&] {
                AInput::overrideState(button, true);
                window->injectEvent(APointerPressedEvent { position, index });
            };
            auto release = [&] {
                window->injectEvent(APointerReleasedEvent { position, index });
                AInput::resetOverride(button);
            };

            if (action == "move") {
                window->injectEvent(AInjectedPointerMove { position });
            } else if (action == "press") {
                // the button stays "down" for AInput until `release`; views read it via the event, so don't leak it.
                window->injectEvent(AInjectedPointerMove { position });
                press();
                AInput::resetOverride(button);
            } else if (action == "release") {
                window->injectEvent(APointerReleasedEvent { position, index });
            } else if (action == "click") {
                window->injectEvent(AInjectedPointerMove { position });
                press();
                release();
            } else if (action == "doubleClick") {
                window->injectEvent(AInjectedPointerMove { position });
                press();
                release();
                AInput::overrideState(button, true);
                window->injectEvent(AInjectedPointerDoubleClick { APointerPressedEvent { position, index } });
                release();
            } else if (action == "drag") {
                const auto to = resolveTarget(params, refs, "to").position;
                constexpr int STEPS = 10;
                window->injectEvent(AInjectedPointerMove { position });
                press();
                AInput::overrideState(button, true);
                for (int i = 1; i <= STEPS; ++i) {
                    window->injectEvent(AInjectedPointerMove { glm::mix(position, to, float(i) / STEPS) });
                }
                position = to;
                release();
            } else if (action == "scroll") {
                window->injectEvent(AInjectedPointerMove { position });
                window->injectEvent(AScrollEvent {
                  .origin = position,
                  .delta = { params.containsOpt("dx") ? number(params, "dx") : 0.f,
                             params.containsOpt("dy") ? number(params, "dy") : 0.f },
                });
            } else {
                throw RpcException(rpc_code::INVALID_PARAMS, "unknown action: " + action,
                                   "click, doubleClick, press, release, move, drag or scroll");
            }
            return AJson {
                { "at", describePoint(target) },
            };
        });
    }));

    // params: text, ref? (focuses the view first)
    dispatcher.registerMethod("input.type", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        if (!params.containsOpt("text")) {
            throw RpcException(rpc_code::INVALID_PARAMS, "missing param: text");
        }
        const auto text = params["text"].asString();
        _<AView> view;
        if (auto ref = params.containsOpt("ref")) {
            view = refs.resolve(ref->asString());
        }
        const auto windowParams = params;
        return inTwoSteps(
            [view] {
                if (view) {
                    view->focus();
                }
            },
            [&refs, text, windowParams] {
                auto window = windowFromParams(windowParams, refs);
                int count = 0;
                for (AChar c : text) {
                    window->injectEvent(AInjectedChar { c });
                    ++count;
                }
                return AJson { { "typed", count } };
            });
    }));

    // params: key, modifiers?, ref? (focuses the view first), action? (press (default)|down|up)
    dispatcher.registerMethod("input.key", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        if (!params.containsOpt("key")) {
            throw RpcException(rpc_code::INVALID_PARAMS, "missing param: key");
        }
        const auto key = keyByName(params["key"].asString());
        const auto modifiers = modifiersOf(params);
        const auto action = params.containsOpt("action") ? params["action"].asString() : AString("press");
        _<AView> view;
        if (auto ref = params.containsOpt("ref")) {
            view = refs.resolve(ref->asString());
        }
        const auto windowParams = params;
        return inTwoSteps(
            [view] {
                if (view) {
                    view->focus();
                }
            },
            [&refs, key, modifiers, action, windowParams] {
                auto window = windowFromParams(windowParams, refs);
                OverrideScope scope(modifiers);
                if (action == "press" || action == "down") {
                    for (auto m : modifiers) {
                        window->injectEvent(AInjectedKey { AInjectedKey::Action::DOWN, m });
                    }
                    window->injectEvent(AInjectedKey { AInjectedKey::Action::DOWN, key });
                }
                if (action == "press" || action == "up") {
                    window->injectEvent(AInjectedKey { AInjectedKey::Action::UP, key });
                    for (auto m : modifiers) {
                        window->injectEvent(AInjectedKey { AInjectedKey::Action::UP, m });
                    }
                }
                return AJson { { "key", AEnumerate<AInput::Key>::toName(key) } };
            });
    }));
}

}   // namespace aui::remote
